#include "groupingthread.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStringList>
#include <QMessageBox>
#include <algorithm>

#include <QDebug>

#include "math.h"

#include "generalsettings.h"
#include "enum_interclustersimilaritymeasures.h"
#include "enum_interobjectsimilaritymeasure.h"

groupingThread::groupingThread(groupingSettings_Detailed *dGrpSettings,
                               groupingSettings_General *groupingSettings,
                               generalSettings* settings)
{
    nextClusterID = 0;
    minMDI = maxMDBI = -1;
    maxMDBIClustersNumber = minMDIClustersNumber = 1;

    this->settings = settings;
    this->grpSettings =  groupingSettings;
    this->dGrpSettings = dGrpSettings;

    wasGroupingCanceled = false;

    initializeGroupingProgressbar();
    initializeCreatingMatrixProgressbar();
}

void groupingThread::setSettings(groupingSettings_Detailed *dGrpSettings, groupingSettings_General *groupingSettings, generalSettings *settings)
{
  this->settings = settings;
  this->grpSettings =  groupingSettings;
  this->dGrpSettings = dGrpSettings;
}

groupingThread::groupingThread(groupingSettings *settings)
{
    nextClusterID = 0;
    minMDI = maxMDBI = -1;
    maxMDBIClustersNumber = minMDIClustersNumber = 1;

    this->settings = settings->genSet;
    this->grpSettings = settings->grpSet;
    this->dGrpSettings = settings->dGrpSet;

    wasGroupingCanceled = false;

    initializeGroupingProgressbar();
    initializeCreatingMatrixProgressbar();
}

groupingThread::~groupingThread(){}

void groupingThread::run()
{    
    initializeDataPreparator();
    groupObjects();
}

void groupingThread::initializeGroupingProgressbar()
{
    groupingProgress = new QProgressDialog(tr("gThreadDialog.grouping"),tr("gThreadDialog.cancel"),1, settings->objectsNumber,0,0);
    groupingProgress->setValue(0);
    groupingProgress->setWindowTitle(tr("gThreadDialog.grouping"));
    groupingProgress->setFixedSize(groupingProgress->sizeHint());
    groupingProgress->setWindowFlags(groupingProgress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    groupingProgress->setWindowModality(Qt::WindowModal);
    groupingProgress->move(QApplication::desktop()->availableGeometry().center()
                           - groupingProgress->rect().center()
                           - QPoint(groupingProgress->width()/2, 0));
}

void groupingThread::initializeCreatingMatrixProgressbar()
{
    creatingSimMatrixProgress  = new QProgressDialog(tr("gThreadDialog.creatingSimilarityMatrix")
                                                     ,tr("gThreadDialog.cancel"),1,settings->objectsNumber,0,0);
    creatingSimMatrixProgress->setValue(0);
    creatingSimMatrixProgress->setWindowTitle(tr("gThreadDialog.creatingSimilarityMatrix"));
    creatingSimMatrixProgress->setFixedSize(creatingSimMatrixProgress->sizeHint());
    creatingSimMatrixProgress->setWindowFlags(creatingSimMatrixProgress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    creatingSimMatrixProgress->setWindowModality(Qt::WindowModal);
    creatingSimMatrixProgress->move(QApplication::desktop()->availableGeometry().center()
                                    - creatingSimMatrixProgress->rect().center()
                                    + QPoint(20 + groupingProgress->width()/2,0));
}

void groupingThread::initializeDataPreparator()
{
    switch(settings->dataTypeID)
    {
        case RSESRulesId:
        {
            emit passLogMsg(tr("log.rsesGroupingStarted"));

            groupingSettings* sets = new groupingSettings();
            sets->dGrpSet = dGrpSettings;
            sets->grpSet = grpSettings;
            sets->genSet = settings;

            grpDataPrep = new groupingDataPreparator_RSESRules(sets);

            break;
        }
        default:
        {
            emit passLogMsg(tr("log.unknownObjectsType"));
            emit passLogMsg(tr("log.operationWontStart"));

            return;
        }
    }
}

void groupingThread::groupObjects()
{   
    // Clear zero representative cluster occurence

    grpSettings->zeroRepresentativeClusterOccurence = -1;

    emit passLogMsg(tr("log.gatheringAttributesData"));

    grpDataPrep->fillAttributesData(&attributes);

    // Clusters are initialized here, so they wont be deleted after prep method finishes.
    clusters = new cluster*[settings->objectsNumber];

    emit passLogMsg(tr("log.placingObjectsInClusters"));

    grpDataPrep->clusterObjects(clusters, &attributes);

    grpDataPrep->fillAttributesValues(&attributes, clusters);

    emit passLogMsg(tr("log.creatingSimMatrix"));

    fillSimMatrix(&simMatrix, settings->objectsNumber);

    emit passLogMsg(tr("log.groupingProcessStarted"));

    groupingProgress->show();

    for(int i = 0; i < settings->objectsNumber - settings->stopCondition; ++i)
    {
        groupingProgress->setValue(i);

        QApplication::processEvents();

        if(groupingProgress->wasCanceled())
            wasGroupingCanceled = true;

        if(wasGroupingCanceled)
        {
            creatingSimMatrixProgress->close();
            groupingProgress->close();

            emit passLogMsg(tr("log.groupingCancelled"));
            emit passLogMsg(tr("log.visualizationImpossible"));
            return;
        }

        if(grpSettings->findBestClustering)
        {
            // Count indexes each time, so we can find best one.
            countMDI(simMatrix.size());
            countMDBI(simMatrix.size());

            if(minMDI == -1)
                minMDI = MDI;

            if((MDI > -1) && (MDI < minMDI))
            {
                minMDI = MDI;
                minMDIClustersNumber = simMatrix.size();
            }

            if(MDBI > maxMDBI)
            {
                maxMDBI = MDBI;
                maxMDBIClustersNumber = simMatrix.size();
            }
        }

        joinMostSimilarClusters(&simMatrix);

        updateSimMatrix(&simMatrix);
    }

    if(!grpSettings->findBestClustering)
    {
        countMDI(simMatrix.size());
        countMDBI(simMatrix.size());
    }

    emit passLogMsg(
        QString(tr("log.clustersNumber"))
        .arg(QString::number(settings->stopCondition))
    );

    emit passLogMsg(
        QString(tr("log.mdiPointer"))
        .arg(QString::number(MDI))
    );

    emit passLogMsg(
        QString(tr("log.mdbiPointer"))
        .arg(QString::number(MDBI))
    );

    creatingSimMatrixProgress->close();
    groupingProgress->close();

    emit passLogMsg(tr("log.groupingFinished"));
    emit passLogMsg(tr("log.sendingResultatntStructure"));
    emit passMDIData(MDI, minMDI, minMDIClustersNumber);
    emit passMDBIData(MDBI, maxMDBI, maxMDBIClustersNumber);

    settings->clusters->clear();

    for(int i = 0; i < settings->stopCondition; ++i)
      settings->clusters->push_back(clusters[i]);

    emit passClusters(clusters);
}

void groupingThread::fillSimMatrix(std::vector<simData> *simMatrix, int simMatrixSize)
{
    creatingSimMatrixProgress->show();

    simMatrix->clear();

    for(int i = 0; i < simMatrixSize; ++i)
    {
        creatingSimMatrixProgress->setValue(i);

        QApplication::processEvents();

        simMatrix->push_back(simData(new clusterSimilarityData));

        for(int j = 0; j <= i; ++j)
        {
            if(i == j)
            {
                simMatrix->at(i)->push_back(qreal_ptr(new qreal(-1.0)));
            }
            else
            {
                qreal simValue = getClustersSimilarityValue(clusters[i], clusters[j]);
                simMatrix->at(i)->push_back(qreal_ptr(new qreal(simValue)));
            }
        }
    }

    creatingSimMatrixProgress->close();
}

void groupingThread::updateSimMatrix(std::vector<simData> *simMatrix)
{
    /*
     * I'll explain update algorithm here. It takes 3 steps:
     * 1) Add an empty column representing new cluster.
     * 2) Fill the column with similarity values.
     * 3) Fill the row with new clusters similarity.
     * 
     * Consider following structure;
     * 
     *      ###
     *       ##
     *        #
     * 
     * I won't bother with details, but I'm working with triangle
     * structures to describe objects similarity. Let's imagine,
     * that new cluster's index is equal to 1. One can graphically
     * show how algorithm works:
     * 
     *   Add empty column.          Fill column.             Add to row.
     *
     *      # ##                        #*##                    #*##
     *        ##       =>                *##            =>       *##
     *         #                           #                      **                                                          #
     *                                                             #
     */

    //First column which would represent new cluster is added.
    simMatrix->insert(simMatrix->begin()+newClusterIdx,simData(new clusterSimilarityData()));

    //Then, the column is filled.
    for(int i = 0; i <= newClusterIdx; ++i)
    {
        if(i == newClusterIdx)
        {
            simMatrix->at(newClusterIdx)->push_back(qreal_ptr(new qreal(-1)));
        }
        else
        {
            qreal simValue = getClustersSimilarityValue(clusters[newClusterIdx], clusters[i]);

            simMatrix->at(newClusterIdx)->push_back(qreal_ptr(new qreal(simValue)));
        }
    }

    //Then, the row is filled.
    for(unsigned int i = (newClusterIdx + 1); i <= (simMatrix->size() - 1) ; ++i)
    {
        qreal simValue = getClustersSimilarityValue(clusters[newClusterIdx], clusters[i]);
        simMatrix->at(i)->insert(simMatrix->at(i)->begin()+newClusterIdx, qreal_ptr(new qreal(simValue)));
    }
}

//TODO: Consides placing similarity counting in another class.

qreal groupingThread::getClustersSimilarityValue(cluster *c1, cluster *c2)
{
    //In case average link is selected
    if(grpSettings->interClusterSimMeasureID == AverageLinkId)
        return getClustersAverageLinkValue(c1, c2) / attributes.size();

    //In case both clusters are objects or centroid link is selected
    if( grpSettings->interClusterSimMeasureID == CentroidLinkId ||
        (c1->size()+c2->size()) == 2 )
        return getObjectsSimValue(c1, c2) / attributes.size();

    //Otherwise
    if(!(c1->hasBothNodes())) std::swap(c1, c2);

    switch(grpSettings->interClusterSimMeasureID)
    {
        case SingleLinkId:
            return qMax(getClustersSimilarityValue(c1->leftNode,c2),
                        getClustersSimilarityValue(c1->rightNode,c2)) / attributes.size();
            break;
        case CompleteLinkId:
            return qMin(getClustersSimilarityValue(c1->leftNode,c2),
                        getClustersSimilarityValue(c1->rightNode,c2)) / attributes.size();
            break;
        default:
            return -1;
    }
}

qreal groupingThread::getClustersAverageLinkValue(cluster *c1, cluster *c2)
{
    qreal result = 0, denumerator;
    QList<cluster*> c1Objects, c2Objects;

    c1Objects = c1->getObjects();
    c2Objects = c2->getObjects();

    denumerator = c1->size() * c2->size();

    for(QList<cluster*>::iterator i = c1Objects.begin();
        i != c1Objects.end(); ++i)
    {
        for(QList<cluster*>::iterator j = c2Objects.begin();
            j != c2Objects.end(); ++j)
        {
            result += getObjectsSimValue(*i, *j);
        }
    }

    result /= denumerator;

    return result;
}

qreal groupingThread::getObjectsSimValue(cluster *c1, cluster *c2)
{
    switch(grpSettings->interObjectSimMeasureID)
    {
        case GowersMeasureId:
            return getObjectsGowersSimValue(c1, c2);
        case SMCId:
            return getObjectsSMCValue(c1, c2);
        case WSMCId:
            return getObjectsWMCValue(c1, c2);
        case OF:
            return getObjectsOFSimValue(c1, c2);
        case IOF:
            return getObjectsIOFSimValue(c1, c2);
        case Goodall1:
            return getObjectsGoodall1SimValue(c1, c2);
        case Goodall2:
            return getObjectsGoodall2SimValue(c1, c2);
        case Goodall3:
            return getObjectsGoodall3SimValue(c1, c2);
        case Goodall4:
            return getObjectsGoodall4SimValue(c1, c2);
        default:
            return -1;
    }
}

qreal groupingThread::getObjectsGowersSimValue(cluster *c1, cluster *c2)
{
    qreal result = 0.0;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // Check if it's symbolic
        if(attributes.value(attribute)->type == "symbolic")
        {
            // If so check if their values intersects
            if((c1Attributes.value(attribute))->toSet().intersects(c2Attributes.value(attribute)->toSet()))
            {
                // If so increment results
                ++result;
            }
        }
        else
        {
            // If attribute is numeric count Gower's Similarity Coefficient addend and add it to result
            result += getGowersSimilarityMeasureNumericAttributesSimilarity(attribute, c1Attributes, c2Attributes);
        }
    }

    return result;
}

qreal groupingThread::getGowersSimilarityMeasureNumericAttributesSimilarity
    (const QString attribute, QHash<QString, QStringList*> c1Attributes, QHash<QString, QStringList*> c2Attributes)
{
    numericAttributeData* atrData = static_cast<numericAttributeData*>(attributes.value(attribute));

    qreal   similarityValue = 1.0,
            c1NaiveAverage = getAttributesNaiveAverageValue(c1Attributes.value(attribute)),
            c2NaiveAverage = getAttributesNaiveAverageValue(c2Attributes.value(attribute));

    similarityValue -= qAbs(c1NaiveAverage - c2NaiveAverage) / atrData->getMaxMinAbsDiff();

    return similarityValue;
}

qreal groupingThread::getAttributesNaiveAverageValue(QStringList* values)
{
    qreal naiveAverage = 0.0;

    // For each value in list of values
    foreach(const QString value, *values)
    {
        // Add value to naive average
        naiveAverage += value.toDouble();
    }

    // Denumerate naive average by size of values
    naiveAverage /= values->size();

    return naiveAverage;
}

qreal groupingThread::getObjectsSMCValue(cluster *c1, cluster *c2)
{
    qreal result = 0.0;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // Check if their values intersect
        if(c2Attributes.value(attribute)->toSet().intersects(c1Attributes.value(attribute)->toSet()))
        {
            // If so increment result
            ++result;
        }
    }

    return result;
}

qreal groupingThread::getObjectsWMCValue(cluster *c1, cluster *c2)
{
    // Get objects SMC Value
    qreal result = getObjectsSMCValue(c1, c2);

    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QStringList allAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Get all attributes occuring in both rules
    allAttributes = c1Attributes.keys() + c2Attributes.keys();
    allAttributes.removeDuplicates();

    // Check if their number is equal to 0
    if(allAttributes.size() == 0)
    {
        // If so return 0
        return 0.0;
    }

    // Denumerate results by number of all attributes
    result /= allAttributes.size();

    return result;
}

qreal groupingThread::getObjectsOFSimValue(cluster* c1, cluster* c2)
{
    qreal result = 0.0, denumerator, addend;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // Ccheck if it's symbolic
        if(attributes.value(attribute)->type == "symbolic")
        {
            // If so check if attributes values sets intersects
            if(c1Attributes.value(attribute)->toSet().intersects(c2Attributes.value(attribute)->toSet()))
            {
                // If so then increment result
                ++result;
            }
            else
            {
                // Otherwise count it's OF Similarity

                // One want's to maximize the similarity thus most appriopriate values are considered
                // In this case denumerator is to be the lowest possible

                categoricalAttributeData* atrData = static_cast<categoricalAttributeData*>(attributes.value(attribute));

                // Set addend to first value
                addend = log2(settings->objectsNumber / atrData->getValuesFrequency(c1Attributes.value(attribute)->at(0)));

                // Check if for any other value in values list this measure is lower
                foreach(const QString value, *(c1Attributes.value(attribute)))
                {
                    qreal potentialNewValue = log2(settings->objectsNumber / atrData->getValuesFrequency(value));

                    if(addend > potentialNewValue)
                    {
                        // If so then set it as new addend
                        addend = potentialNewValue;
                    }
                }

                // Set denumerator as this
                denumerator = addend;

                // Repreat procedure for second values list

                // Set addend to first value
                addend = log2(settings->objectsNumber / atrData->getValuesFrequency(c2Attributes.value(attribute)->at(0)));

                // Check if for any other value in values list this measure is lower
                foreach(const QString value, *(c2Attributes.value(attribute)))
                {
                    qreal potentialNewValue = log2(settings->objectsNumber / atrData->getValuesFrequency(value));

                    if(addend > potentialNewValue)
                    {
                        // If so then set it as new addend
                        addend = potentialNewValue;
                    }
                }

                // Multiply denumerator by new addend
                denumerator *= addend;

                ++denumerator;

                // Add it to result
                result += 1.0 / denumerator;
            }
        }
        else
        {
            // If it's numeric count it's Gower Similarity
            result += getGowersSimilarityMeasureNumericAttributesSimilarity(attribute, c1Attributes, c2Attributes);
        }
    }

    return result;
}

qreal groupingThread::getObjectsIOFSimValue(cluster* c1, cluster* c2)
{
    qreal result = 0.0, denumerator, addend;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // If so then check if it's symbolic
        if(attributes.value(attribute)->type == "symbolic")
        {
            // If so check if attributes values sets intersects
            if(c1Attributes.value(attribute)->toSet().intersects(c2Attributes.value(attribute)->toSet()))
            {
                // If so then increment result
                ++result;
            }
            else
            {
                // Otherwise count it's OF Similarity

                // One want's to maximize the similarity thus most appriopriate values are considered
                // In this case denumerator is to be the lowest possible

                categoricalAttributeData* atrData = static_cast<categoricalAttributeData*>(attributes.value(attribute));

                // Set addend to first value
                addend = log2(atrData->getValuesFrequency(c1Attributes.value(attribute)->at(0)));

                // Check if for any other value in values list this measure is lower
                foreach(const QString value, *(c1Attributes.value(attribute)))
                {
                    qreal potentialNewValue = log2(atrData->getValuesFrequency(value));

                    if(addend > potentialNewValue)
                    {
                        // If so then set it as new addend
                        addend = potentialNewValue;
                    }
                }

                // Set denumerator as this
                denumerator = addend;

                // Repreat procedure for second values list

                // Set addend to first value
                addend = log2(settings->objectsNumber / atrData->getValuesFrequency(c2Attributes.value(attribute)->at(0)));

                // Check if for any other value in values list this measure is lower
                foreach(const QString value, *(c2Attributes.value(attribute)))
                {
                    qreal potentialNewValue = log2(atrData->getValuesFrequency(value));

                    if(addend > potentialNewValue)
                    {
                        // If so then set it as new addend
                        addend = potentialNewValue;
                    }
                }

                // Multiply denumerator by new addend
                denumerator *= addend;

                ++denumerator;

                // Add it to result
                result += 1.0 / denumerator;
            }
        }
        else
        {
            // If it's numeric count it's Gower Similarity
            result += getGowersSimilarityMeasureNumericAttributesSimilarity(attribute, c1Attributes, c2Attributes);
        }
    }

    return result;
}

qreal groupingThread::getObjectsGoodall1SimValue(cluster* c1, cluster* c2)
{
    qreal result = 0, addend;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // Check if it's symbolic
        if(attributes.value(attribute)->type == "symbolic")
        {
            // If so then check if their values intersects
            if(c1Attributes.value(attribute)->toSet().intersects(c2Attributes.value(attribute)->toSet()))
            {
                // If so find value for which Goodall1 measure is maximal

                qreal possibleNewAddend, probabilitySampleValue, valuesProbabilitySampleValue;
                categoricalAttributeData* atrData = static_cast<categoricalAttributeData*>(attributes.value(attribute));

                QStringList values = c1Attributes.value(attribute)->toSet().intersect(c2Attributes.value(attribute)->toSet()).toList();

                // Count addend for first value on the list
                addend = 0;

                foreach(const QString value, atrData->valuesFrequency.keys())
                {
                    probabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, value);
                    valuesProbabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, values.at(0));

                    // Check if second sample probability is lower or equal to second sample probability of given value
                    if(probabilitySampleValue <= valuesProbabilitySampleValue)
                    {
                        // If so add it to addend
                        addend += probabilitySampleValue;
                    }
                }

                // For each value in set
                foreach(const QString value, values)
                {
                    possibleNewAddend = 0.0;

                    // Compare it with all values (including itself) in this set
                    foreach(const QString comparator, values)
                    {
                        probabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, comparator);
                        valuesProbabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, value);

                        // Check if second sample probability is lower or equal to second sample probability of given value
                        if( probabilitySampleValue <= valuesProbabilitySampleValue)
                        {
                            // If so add it to addend
                            possibleNewAddend += probabilitySampleValue;
                        }
                    }

                    // Remember the lowest addend
                    if(possibleNewAddend < addend)
                    {
                        addend = possibleNewAddend;
                    }
                }

                // Add it's Goodall1 value to result
                addend = 1 - addend;
                result += addend;
            }
            else
            {
                // If not continue (add 0 to result)
                continue;
            }
        }
        else
        {
            // If it's numeric add it's Gower Measure value to result
            result += getGowersSimilarityMeasureNumericAttributesSimilarity(attribute, c1Attributes, c2Attributes);
        }
    }

    return result;
}

qreal groupingThread::countSampleProbabilityOfAttributesValue(QString attribute, QString value)
{
    qreal result = static_cast<categoricalAttributeData*>(attributes.value(attribute))->getValuesFrequency(value);
    result /= settings->objectsNumber;

    return result;
}

qreal groupingThread::countSecondSampleProbabilityOfAttributesValue(QString attribute, QString value)
{
    qreal result = countSampleProbabilityOfAttributesValue(attribute, value);
    result *= static_cast<categoricalAttributeData*>(attributes.value(attribute))->getValuesFrequency(value) - 1;
    result /= settings->objectsNumber - 1;

    return result;
}

// TODO: Note that only difference between these implementations of Goodall1 and Goodall2
// differs only on >= and <= signs. Consider writing one function for that.

qreal groupingThread::getObjectsGoodall2SimValue(cluster* c1, cluster* c2)
{
    qreal result = 0, addend;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // Check if it's symbolic
        if(attributes.value(attribute)->type == "symbolic")
        {
            // If so then check if their values intersects
            if(c1Attributes.value(attribute)->toSet().intersects(c2Attributes.value(attribute)->toSet()))
            {
                // If so find value for which Goodall2 measure is maximal
                qreal possibleNewAddend, probabilitySampleValue, valuesProbabilitySampleValue;
                categoricalAttributeData* atrData = static_cast<categoricalAttributeData*>(attributes.value(attribute));

                QStringList values = c1Attributes.value(attribute)->toSet().intersect(c2Attributes.value(attribute)->toSet()).toList();

                // Count addend for first value on the list
                addend = 0;

                foreach(const QString value, atrData->valuesFrequency.keys())
                {
                    probabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, value);
                    valuesProbabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, values.at(0));

                    // Check if second sample probability is higher or equal to second sample probability of given value
                    if(probabilitySampleValue >= valuesProbabilitySampleValue)
                    {
                        // If so add it to addend
                        addend += probabilitySampleValue;
                    }
                }

                // For each value in set
                foreach(const QString value, values)
                {
                    possibleNewAddend = 0.0;

                    // Compare it with all values (including itself) in this set
                    foreach(const QString comparator, values)
                    {
                        probabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, comparator);
                        valuesProbabilitySampleValue = countSecondSampleProbabilityOfAttributesValue(attribute, value);

                        // Check if second sample probability is higher or equal to second sample probability of given value
                        if( probabilitySampleValue >= valuesProbabilitySampleValue)
                        {
                            // If so add it to addend
                            possibleNewAddend += probabilitySampleValue;
                        }
                    }

                    // Remember the lowest addend
                    if(possibleNewAddend < addend)
                    {
                        addend = possibleNewAddend;
                    }
                }

                // Add it's Goodall2 value to result
                addend = 1 - addend;
                result += addend;
            }
            else
            {
                // If not continue (add 0 to result)
                continue;
            }
        }
        else
        {
            // If it's numeric add it's Gower Measure value to result
            result += getGowersSimilarityMeasureNumericAttributesSimilarity(attribute, c1Attributes, c2Attributes);
        }
    }

    return result;
}

qreal groupingThread::getObjectsGoodall3SimValue(cluster* c1, cluster* c2)
{
    qreal result = 0, addend;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // Check if it's symbolic
        if(attributes.value(attribute)->type == "symbolic")
        {
            // If so then check if their values intersects
            if(c1Attributes.value(attribute)->toSet().intersects(c2Attributes.value(attribute)->toSet()))
            {
                // If so find value for which Goodall3 measure is maximal
                qreal possibleNewAddend;

                QStringList values = c1Attributes.value(attribute)->toSet().intersect(c2Attributes.value(attribute)->toSet()).toList();

                // Set first addend to it's first value
                addend = countSecondSampleProbabilityOfAttributesValue(attribute, values.at(0));

                // For each value
                foreach(const QString value, values)
                {
                    // Find one for which addend is lowest
                    possibleNewAddend = countSecondSampleProbabilityOfAttributesValue(attribute, value);

                    if(addend > possibleNewAddend)
                    {
                        addend = possibleNewAddend;
                    }
                }

                // Add Goodall3 measure value to result
                addend = 1 - addend;
                result += addend;
            }
            else
            {
                // If not continue (add 0 to result)
                continue;
            }
        }
        else
        {
            // If it's numeric add it's Gower Measure value to result
            result += getGowersSimilarityMeasureNumericAttributesSimilarity(attribute, c1Attributes, c2Attributes);
        }
    }

    return result;
}

qreal groupingThread::getObjectsGoodall4SimValue(cluster* c1, cluster* c2)
{
    qreal result = 0, addend;
    QHash<QString, QStringList*> c1Attributes, c2Attributes;
    QSet<QString> commonAttributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    // Find common attributes of both objects
    commonAttributes = c1Attributes.keys().toSet().intersect(c2Attributes.keys().toSet());

    // For each common attribute
    foreach(const QString attribute, commonAttributes)
    {
        // Check if it's symbolic
        if(attributes.value(attribute)->type == "symbolic")
        {
            // If so then check if their values intersects
            if(c1Attributes.value(attribute)->toSet().intersects(c2Attributes.value(attribute)->toSet()))
            {
                // If so find value for which Goodall4 measure is maximal
                qreal possibleNewAddend;

                QStringList values = c1Attributes.value(attribute)->toSet().intersect(c2Attributes.value(attribute)->toSet()).toList();

                // Set first addend to it's first value
                addend = countSecondSampleProbabilityOfAttributesValue(attribute, values.at(0));

                // For each value
                foreach(const QString value, values)
                {
                    // Find one for which addend is highest
                    possibleNewAddend = countSecondSampleProbabilityOfAttributesValue(attribute, value);

                    if(addend < possibleNewAddend)
                    {
                        addend = possibleNewAddend;
                    }
                }

                // Add Goodall4 measure value to result
                result += addend;
            }
            else
            {
                // If not continue (add 0 to result)
                continue;
            }
        }
        else
        {
            // If it's numeric add it's Gower Measure value to result
            result += getGowersSimilarityMeasureNumericAttributesSimilarity(attribute, c1Attributes, c2Attributes);
        }
    }

    return result;
}

void groupingThread::joinMostSimilarClusters(std::vector<simData> *simMatrix)
{
    int i, j;

    findHighestSimilarityIndexes(&i, &j, simMatrix);

    clusters[j] = joinClusters(clusters[i], clusters[j]);

    //TODO: Consider deleting / smart_ptr.
    std::swap(clusters[i], clusters[simMatrix->size()-1]);

    // This is used in updating matrix.
    newClusterIdx = j;

    /*
     *  Order is important!
     *
     *  Longest column (always i), must be deleted first.
     *  If this was implemented other way around then
     *  columns would move and an additional -1 would be
     *  needed.
     */

    deleteClusterSimilarityData(i, simMatrix);
    deleteClusterSimilarityData(j, simMatrix);
}

void groupingThread::findHighestSimilarityIndexes(int *targetI, int *targetJ, std::vector<simData> *simMatrix)
{
    /*
     * This method fills i and j references. It's used to find
     * which clusters should be joined.
     */

    qreal highestSim = -1;

    for(unsigned int i = 0; i < simMatrix->size(); ++i)
    {
        for(unsigned int j = 0; j < i; ++j)
        {
            if(*(simMatrix->at(i)->at(j)) > highestSim)
            {
                *targetI = i;
                *targetJ = j;
                highestSim = *(simMatrix->at(i)->at(j));
            }
        }
    }
}

cluster* groupingThread::joinClusters(cluster* c1, cluster* c2)
{
    QStringList atrNames;
    ruleCluster* newGrp = new ruleCluster(++nextClusterID);


    newGrp->leftNode = c1;
    newGrp->rightNode = c2;

    newGrp->decisionAttributes
            .unite(((ruleCluster*) c1)->decisionAttributes);
    newGrp->decisionAttributes
            .unite(((ruleCluster*) c2)->decisionAttributes);

    newGrp->premiseAttributes
            .unite(((ruleCluster*) c1)->premiseAttributes);
    newGrp->premiseAttributes
            .unite(((ruleCluster*) c2)->premiseAttributes);

    newGrp->support =
            static_cast<ruleCluster*>(c1)->support +
            static_cast<ruleCluster*>(c2)->support;

    newGrp->attributes = c1->attributes;
    atrNames = c2->attributes.keys();

    for(int i = 0; i < atrNames.length(); ++i)
    {
        if(c1->attributes.keys().contains(atrNames.at(i)))
            continue;

        newGrp->attributes.insert(atrNames.at(i), c2->attributes.value(atrNames.at(i)));
    }

    newGrp->fillRepresentativesAttributesValues(grpSettings->repCreationStrategyID, grpSettings->repTreshold);

    if(newGrp->representativeAttributesValues.keys().size() == 0 &&
       grpSettings->zeroRepresentativeClusterOccurence == -1)
    {
        grpSettings->zeroRepresentativeClusterOccurence = nextClusterID - 1;
    }

    newGrp->compactness = countClustersCompactness(newGrp);

    return newGrp;
}

qreal groupingThread::countClustersCompactness(cluster* c)
{
    /*
     *  In this application cluster compactness is average similarity
     *  of all the objects to clusters representative. It has to be
     *  count in grouping thread as only grouping thread has knowledge
     *  about similarity measures (countSim methods).
     */

    qreal result, addend;
    QList<cluster*> objects = c->getObjects();

    // Clusters compactness of object is equal to 0
    if(c->size() == 1)
        return 0;

    /*
     * As similarity value of objects is count by
     * getClustersSimilarityValue(cluster *c1, cluster *c2)
     * which requires 2 cluster_pointers a dirty trick was
     * used (created temp cluster* with representatives
     * attributes;
     */

    cluster* temp = new cluster();

    temp->representativeAttributesValues = c->representativeAttributesValues;
    temp->attributesValues = c->representativeAttributesValues;

    for(int i = 0; i < objects.length(); ++i)
    {
        addend = getClustersSimilarityValue(temp, objects.at(i));

        // Punish joining nonsimilar clusters
        // TODO: consider adding weight? Changing it? Maybe divide?
        if(addend == 0)
            addend = -1;

        result += addend;
    }

    result /= objects.length();

    return result;
}

void groupingThread::deleteClusterSimilarityData(unsigned int clusterId, std::vector<simData> *simMatrix)
{
    /*
     * First deleting data from each column that had similarity with clusterId.
     * This happened to be all columns with id > than clusterId.
    */

    for(unsigned int i = 0; i < simMatrix->size(); ++i)
    {
        if(simMatrix->at(i)->size() > clusterId)
            simMatrix->at(i)->erase(simMatrix->at(i)->begin()+(clusterId));
    }

    /*
     * Then deleting a column with clusterSimData.
     *
     * Swapping with empty vector is required to
     * call vectors destructor and freeing the memory.
    */

    simData().swap(simMatrix->at(clusterId));
    simMatrix->erase(simMatrix->begin()+clusterId);
}

// TODO: Consides some kind of cluster validator in other file.

void groupingThread::countMDI(int clustersNum)
{
    /* MDI == -1 means that two clusters with 0 similarity
     * were joined. There's no smaller similarity value than
     * 0 hence it'd return -1 anyway.
     */

    if(MDI == -1) return;

    qreal minIntraClusterSim, maxInterClusterSim;

    minIntraClusterSim = getMinIntraClusterSimilarity(clustersNum);

    maxInterClusterSim = getMaxInterClusterSimilarity(clustersNum);

    //TODO: Reconsider what to do in this case.
    if(minIntraClusterSim == 0)
    {
        MDI = -1;
        return;
    }

    MDI = qreal(maxInterClusterSim/minIntraClusterSim);
}

qreal groupingThread::getMaxInterClusterSimilarity(int clustersNum)
{
    // Return 0 if only 1 objects is left.
    if(clustersNum == 1)
        return 0;

    qreal maxInterSim, potentialMax;

    maxInterSim = getClustersSimilarityValue(clusters[0], clusters[1]);

    for(int i = 0; i < clustersNum; ++i)
    {
        for(int j = 0; j < i; ++j)
        {
            potentialMax = getClustersSimilarityValue(clusters[i], clusters[j]);

            if(potentialMax > maxInterSim)
                maxInterSim = potentialMax;
        }
    }

    return maxInterSim;
}

qreal groupingThread::getMinIntraClusterSimilarity(int clustersNum)
{
    // Return 1 so MDI is not negative if all clusters are objects.
    if(clustersNum == settings->objectsNumber)
        return 1;

    qreal minIntraSim = -1, potentialMin;
    QList<cluster*> objects;

    for(int i = 0; i < clustersNum; ++i)
    {
        objects = clusters[i]->getObjects();

        for(int j = 0; j < objects.length(); ++j)
        {
            for(int k = 0; k < j; ++k)
            {
                potentialMin = getClustersSimilarityValue(objects.at(j), objects.at(k));

                if((potentialMin < minIntraSim) || (minIntraSim == -1))
                    minIntraSim = potentialMin;
            }
        }
    }

    return minIntraSim;
}

// TODO: Sometime it is < 0. Investigate.

void groupingThread::countMDBI(int clustersNum)
{
    MDBI = 0;

    if(settings->objectsNumber == clustersNum)
        return;

    qreal addend, clustersSim;

    for(int i = 0; i < clustersNum; ++i)
    {
        for(int j = 0; j < i; ++j)
        {
            addend = clusters[i]->compactness + clusters[j]->compactness;
            clustersSim = getClustersSimilarityValue(clusters[i], clusters[j]);

            if(clustersSim == 0)
                clustersSim = 1;

            MDBI += addend/clustersSim;
        }
    }

    MDBI = qreal(MDBI/clustersNum);
}

void groupingThread::continueGrouping()
{
  for(int i = settings->stopCondition; i < settings->clusters->size(); ++i)
  {
    joinMostSimilarClusters(&simMatrix);

    updateSimMatrix(&simMatrix);
  }

  settings->clusters->clear();

  for(int i = 0; i < settings->stopCondition; ++i)
    settings->clusters->push_back(clusters[i]);
}
