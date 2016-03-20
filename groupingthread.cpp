#include "groupingthread.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStringList>
#include <QMessageBox>
#include <algorithm>

#include "math.h"

#include "generalsettings.h"
#include "enum_interclustersimilaritymeasures.h"
#include "enum_interobjectsimilaritymeasure.h"

#include "objectsclusterer_rsesrules.h"

groupingThread::groupingThread(groupingSettings_Detailed *dGrpSettings,
                               groupingSettings_General *groupingSettings,
                               generalSettings* settings)
{
    nextClusterID = 0;
    maxMDI = maxMDBI = 0.0;
    maxMDBIClustersNumber = maxMDIClustersNumber = 1;

    this->settings = settings;
    this->grpSettings =  groupingSettings;
    this->dGrpSettings = dGrpSettings;

    wasGroupingCanceled = false;

    initializeGroupingProgressbar();
    initializeCreatingMatrixProgressbar();

}

groupingThread::groupingThread(groupingSettings *settings)
{
    nextClusterID = 0;
    maxMDI = maxMDBI = 0.0;
    maxMDBIClustersNumber = maxMDIClustersNumber = 1;

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

void groupingThread::groupObjects()
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
            groupRSESRules();
            
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

void groupingThread::groupRSESRules()
{
    emit passLogMsg(tr("log.gatheringAttributesData"));

    grpDataPrep->fillAttributesData(&attributes);

    // Clusters are initialized here, so they wont be deleted after prep method finishes.
    clusters = new cluster*[settings->objectsNumber];

    emit passLogMsg(tr("log.placingObjectsInClusters"));

    grpDataPrep->clusterObjects(clusters, &attributes);

    grpDataPrep->fillNumericAttributesValues(&attributes, clusters);

    emit passLogMsg(tr("log.creatingSimMatrix"));

    std::vector<simData> simMatrix;
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
            groupingProgress->close();
            creatingSimMatrixProgress->close();
            stopGrouping();
            return;
        }

        if(grpSettings->findBestClustering)
        {
            //Count indexes each time, so we can find best one.
            //countMDI(simMatrixSize);
            //countMDBI(simMatrixSize);

            if(MDI > maxMDI)
            {
                maxMDI = MDI;
                //maxMDIClustersNumber = simMatrixSize;
            }

            if(MDBI > maxMDBI)
            {
                maxMDBI = MDBI;
                //maxMDBIClustersNumber = simMatrixSize;
            }
        }

        joinMostSimilarClusters(&simMatrix);

        updateSimMatrix(&simMatrix);
    }

    if(!grpSettings->findBestClustering)
    {
        ;
        //countMDI(simMatrixSize);
        //countMDBI(simMatrixSize);
    }

    groupingProgress->close();

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

    emit passLogMsg(tr("log.groupingFinished"));
    emit passLogMsg(tr("log.sendingResultatntStructure"));
    emit passMDIData(MDI, maxMDI, maxMDIClustersNumber);
    emit passMDBIData(MDBI, maxMDBI, maxMDBIClustersNumber);
    emit passClusters(clusters);
}

void groupingThread::fillSimMatrix(std::vector<simData> *simMatrix, int simMatrixSize)
{
    creatingSimMatrixProgress->show();

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
                qreal simValue = getClustersSimilarityValue(clusters[i], clusters[j]);//countRSESClustersSimilarityValue(((ruleCluster*)clusters[i]),((ruleCluster*)clusters[j]));
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

qreal groupingThread::getClustersSimilarityValue(cluster *c1, cluster *c2)
{
    //In case average link is selected
    if(grpSettings->interClusterSimMeasureID == AverageLinkId)
        return getClustersAverageLinkValue(c1, c2);

    //In case both clusters are objects or centroid link is selected
    if( grpSettings->interClusterSimMeasureID == CentroidLinkId ||
        (c1->size()+c2->size()) == 2 )
        return getObjectsSimValue(c1, c2);

    //Otherwise
    if(!(c1->hasBothNodes())) std::swap(c1, c2);

    switch(grpSettings->interClusterSimMeasureID)
    {
    case SingleLinkId:
        return qMax(getClustersSimilarityValue(c1->leftNode,c2),
                    getClustersSimilarityValue(c1->rightNode,c2));
        break;
    case CompleteLinkId:
        return qMin(getClustersSimilarityValue(c1->leftNode,c2),
                    getClustersSimilarityValue(c1->rightNode,c2));
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
            break;
        case SMCId:
            return getObjectsSMCValue(c1, c2);
            break;
        case WSMCId:
            return getObjectsWSMCValue(c1, c2);
            break;
        default:
            return -1;
    }
}

qreal groupingThread::getObjectsGowersSimValue(cluster *c1, cluster *c2)
{
    qreal result = 0, denumerator, addend;

    QHash<QString, QString> c1Attributes, c2Attributes;
    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    for(QHash<QString, QString>::iterator i = c1Attributes.begin(); i != c1Attributes.end(); ++i)
    {
        if(c2Attributes.contains(i.key()))
        {
            if(     (attributes.value(i.key())->type == "symbolic"
                 && (c2->attributesValues.value(i.key()) == c1->attributesValues.value(i.key())))
                ||
                (   attributes.value(i.key())->type == "numeric"
                 && static_cast<numericAttributeData*>(attributes[i.key()])->areMinMaxEqual()))
            {
                ++result;
                continue;
            }

            if(attributes.value(i.key())->type == "numeric")
            {
                denumerator = static_cast<numericAttributeData*>(attributes[i.key()])->getMaxMinAbsDiff();
                addend = qAbs(c1->attributesValues.value(i.key()).toDouble() -
                              c2->attributesValues.value(i.key()).toDouble());
                addend = 1 - addend/denumerator;

                result += addend;
            }
        }
    }

    return result;
}

qreal groupingThread::getObjectsSMCValue(cluster *c1, cluster *c2)
{
    qreal result = 0;

    QHash<QString, QString> c1Attributes, c2Attributes;
    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    for(QHash<QString, QString>::iterator i = c1Attributes.begin(); i != c1Attributes.end(); ++i)
        if( c2Attributes.contains(i.key()) && c1Attributes[i.key()] == c2Attributes[i.key()])
            ++result;

    return result;
}

qreal groupingThread::getObjectsWSMCValue(cluster *c1, cluster *c2)
{
    qreal result = 0;
    QList<QString> allAttributes;
    QHash<QString, QString> c1Attributes, c2Attributes;

    c1Attributes =
            c1->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);
    c2Attributes =
            c2->getAttributesForSimilarityCount(grpSettings->interClusterSimMeasureID);

    for(QHash<QString, QString>::iterator i = c1Attributes.begin(); i != c1Attributes.end(); ++i)
        if( c2Attributes.contains(i.key()) && c1Attributes[i.key()] == c2Attributes[i.key()])
            ++result;


    allAttributes = c1Attributes.keys() + c2Attributes.keys();
    allAttributes.removeDuplicates();

    int denumerator = allAttributes.size();

    return result/denumerator;
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
    ruleCluster* temp = new ruleCluster(++nextClusterID);


    temp->leftNode = c1;
    temp->rightNode = c2;

    temp->decisionAttributes
            .unite(((ruleCluster*) c1)->decisionAttributes);
    temp->decisionAttributes
            .unite(((ruleCluster*) c2)->decisionAttributes);

    temp->premiseAttributes
            .unite(((ruleCluster*) c1)->premiseAttributes);
    temp->premiseAttributes
            .unite(((ruleCluster*) c2)->premiseAttributes);

    temp->attributes = c1->attributes;
    atrNames = c2->attributes.keys();
    for(int i = 0; i < atrNames.length(); ++i)
    {
        if(c1->attributes.keys().contains(atrNames.at(i)))
            continue;

        temp->attributes.insert(atrNames.at(i), c2->attributes.value(atrNames.at(i)));
    }

    temp->fillRepresentativesAttributesValues(grpSettings->repTreshold);

    //temp->compactness =
            //countClustersCompactness(temp, temp->representative);

    return temp;
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













QString groupingThread::getLongerRule(QString r1, QString r2)
{
    if(getRuleAttributesNumber(r1) > getRuleAttributesNumber(r2))
            return r1;

    if(getRuleAttributesNumber(r1) < getRuleAttributesNumber(r2))
            return r2;

    if(getRuleAttributesNumber(r1) == getRuleAttributesNumber(r2))
    {
        if(r1.length() > r2.length())
            return r1;
        else
            return r2;
    }

    return "a";
}

int groupingThread::getRuleAttributesNumber(QString r)
{
    return 1 + r.count('&');
}

QString groupingThread::getShorterRule(QString r1, QString r2)
{
    if(getRuleAttributesNumber(r1) > getRuleAttributesNumber(r2))
            return r2;

    if(getRuleAttributesNumber(r1) < getRuleAttributesNumber(r2))
            return r1;

    if(getRuleAttributesNumber(r1) == getRuleAttributesNumber(r2))
    {
        if(r1.length() > r2.length())
           return r2;
        else
            return r1;
    }

    return "a";
}

qreal groupingThread::countClustersCompactness(ruleCluster *c, QString aRule)
{
    /*  TODO: Rename to clusters compactness
     *  Change code
     *
     *
    if(c->hasBothNodes())
    {
        qreal result =
                countClusterDispersion(((ruleCluster*) c->leftNode), aRule)
                +countClusterDispersion(((ruleCluster*) c->rightNode), aRule);

        if(std::abs(result)<1e-4)
            return 0.0;

        return result;
    }

    if((c->rule == aRule))
        return 0;

    */
    return 0;
}

void groupingThread::stopGrouping()
{
    emit passLogMsg(tr("log.groupingCancelled"));
    emit passLogMsg(tr("log.visualizationImpossible"));
}

void groupingThread::countMDI(int size)
{
    qreal minInsideSimilarity = -1;
    qreal maxOutsideSimilarity = 0;
    qreal currentSimilarity = 0;

    for(int i = 0; i < size; i++)
    {
        for(int j = i+1; j < size; j++)
        {
            currentSimilarity = getClustersSimilarityValue(clusters[i], clusters[j]);

            if(currentSimilarity > maxOutsideSimilarity)
                maxOutsideSimilarity = currentSimilarity;
        }
    }

    for(int i = 0; i < size; i++)
    {
        if(clusters[i]->size() == 1)
            continue;

        currentSimilarity = ((qreal) countLowestRSESInterclusterSimilarity(
                    ((ruleCluster*)clusters[i]->leftNode), ((ruleCluster*)clusters[i]->rightNode)));

        if(minInsideSimilarity == -1)
            minInsideSimilarity = currentSimilarity;


        if(currentSimilarity < minInsideSimilarity)
            minInsideSimilarity = currentSimilarity;
    }

    if(minInsideSimilarity == 0)
    {
        MDI = 0;
        return;
    }

    MDI = maxOutsideSimilarity / minInsideSimilarity;
}

qreal groupingThread::countLowestRSESInterclusterSimilarity(ruleCluster *c1, ruleCluster *c2)
{
    if(c1->rule() != "")
        return countLowestRSESClusterRuleSimilarityValue(c1->rule(), c2);

    return qMin(countLowestRSESInterclusterSimilarity(((ruleCluster*) c1->leftNode),c2),
                countLowestRSESInterclusterSimilarity(((ruleCluster*) c1->rightNode),c2));

    return -1;
}

qreal groupingThread::countLowestRSESClusterRuleSimilarityValue(QString r, ruleCluster *c)
{
    if(c->rule() !="")
        return 0;

    return qMin(countLowestRSESClusterRuleSimilarityValue(r,((ruleCluster*) c->leftNode)),
                countLowestRSESClusterRuleSimilarityValue(r,((ruleCluster*) c->rightNode)));

    return -1;
}

void groupingThread::countMDBI(int size)
{
    qreal similaritySum = countSimilaritySum(size);

    if(std::abs(similaritySum)<= 1e-4)
    {
        MDBI = 0.0;
        return;
    }

    MDBI = (qreal)(similaritySum/size);
}

qreal groupingThread::countSimilaritySum(int size)
{
    qreal sum = 0;

    for(int i = 0; i < size; i++)
    {
        for(int j = i+1; j < size; j++)
        {
            qreal clustersSim =
                getClustersSimilarityValue(clusters[i], clusters[j]);

            if(std::abs(clustersSim)<= 1e-4)
                continue;

            qreal sumPart = ((ruleCluster*)clusters[i])->compactness + ((ruleCluster*)clusters[j])->compactness;

            sumPart /= clustersSim;

            sum += sumPart;
        }
    }

    return sum;
}
