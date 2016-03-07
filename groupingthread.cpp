#include "groupingthread.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStringList>
#include <QMessageBox>
#include <algorithm>
#include "math.h"

#include "generalsettings.h"

groupingThread::groupingThread(){}

groupingThread::groupingThread(groupingSettings_RSESRules *RSESSettings,
                               groupingSettings_General *groupingSettings,
                               generalSettings* settings)
{
    nextClusterID = 0;
    maxMDI = 0.0;
    maxMDBI = 0.0;
    maxMDBIClustersNumber = 1;
    maxMDIClustersNumber = 1;

    this->settings = settings;
    this->groupingSettings = groupingSettings;
    this->RSESSettings = RSESSettings;

    wasGroupingCanceled = false;


    groupingProgress = new QProgressDialog(tr("gThreadDialog.grouping"),tr("gThreadDialog.cancel"),1, settings->objectsNumber,0,0);
    groupingProgress->setValue(0);
    groupingProgress->setWindowTitle(tr("gThreadDialog.grouping"));
    groupingProgress->setFixedSize(groupingProgress->sizeHint());
    groupingProgress->setWindowFlags(groupingProgress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    groupingProgress->setWindowModality(Qt::WindowModal);
    groupingProgress->move(QApplication::desktop()->availableGeometry().center()
                           - groupingProgress->rect().center()
                           - QPoint(groupingProgress->width()/2, 0));


    creatingSimMatrixProgress  = new QProgressDialog(tr("gThreadDialog.creatingSimilarityMatrix")
                                                     ,tr("gThreadDialog.cancel"),1,settings->objectsNumber,0,0);
    // Tworzenie macierzy podobieństwa
    creatingSimMatrixProgress->setValue(0);
    creatingSimMatrixProgress->setWindowTitle(tr("gThreadDialog.creatingSimilarityMatrix"));
    creatingSimMatrixProgress->setFixedSize(creatingSimMatrixProgress->sizeHint());
    creatingSimMatrixProgress->setWindowFlags(creatingSimMatrixProgress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    creatingSimMatrixProgress->setWindowModality(Qt::WindowModal);
    creatingSimMatrixProgress->move(QApplication::desktop()->availableGeometry().center()
                                    - creatingSimMatrixProgress->rect().center()
                                    + QPoint(20 + groupingProgress->width()/2,0));
}

groupingThread::~groupingThread(){}

void groupingThread::run()
{
     groupObjects();
}

void groupingThread::groupObjects()
{
    switch(settings->dataTypeID)
    {
        case generalSettings::RSES_RULES_ID:

            emit passLogMsg(tr("log.rsesGroupingStarted"));
            groupRSESRules();
            
            break;

        default:

            emit passLogMsg(tr("log.unknownObjectsType"));
            emit passLogMsg(tr("log.operationWontStart"));
    }
}

void groupingThread::groupRSESRules()
{
    emit passLogMsg(tr("log.gatheringAttributesData"));

    fillAttributesData();

    int simMatrixSize = settings->objectsNumber;

    std::vector<simData> simMatrix;

    int i = settings->objectsNumber - settings->stopCondition;

    emit passLogMsg(tr("log.placingObjectsInClusters"));

    clusterRules();

    emit passLogMsg(tr("log.groupingProcessStarted"));

    fillSimMatrix(&simMatrix, simMatrixSize);

    groupingProgress->show();

    while(i != 0)
    {
        groupingProgress->setValue(settings->objectsNumber-simMatrixSize+settings->stopCondition);

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

        joinMostSimilarClusters(&simMatrix);

        --simMatrixSize;

        if(groupingSettings->findBestClustering)
        {
            //Count indexes each time, so we can find best one.
            countMDI(simMatrixSize);
            countMDBI(simMatrixSize);

            if(MDI > maxMDI)
            {
                maxMDI = MDI;
                maxMDIClustersNumber = simMatrixSize;
            }

            if(MDBI > maxMDBI)
            {
                maxMDBI = MDBI;
                maxMDBIClustersNumber = simMatrixSize;
            }
        }

        --i;

        if(simMatrix.size() == 0)
            break;
        else
            updateSimMatrix(&simMatrix);
    }

    if(!groupingSettings->findBestClustering)
    {
        countMDI(simMatrixSize);
        countMDBI(simMatrixSize);
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

void groupingThread::fillAttributesData()
{
    groupingSettings->attributesNumber =
        RSESSettings->getRSESRulesAttributeNumber(groupingSettings->objectBaseInfo);

    attributes = new RSESAttribute[groupingSettings->attributesNumber];

    QString line;

    int i = 0;

    bool fillAttributesData = false;
    bool fillAttributesValues = false;

    QFile KB(groupingSettings->objectBaseInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!in.atEnd())
        {
            line = in.readLine();

            if(fillAttributesValues)
            {
                for(i = 0; i < groupingSettings->attributesNumber; i++)
                {
                    if(attributes[i].type != "symbolic")
                    {
                        if(line.contains(attributes[i].name+"="))
                        {
                            QString additionalLine = line;

                            QStringList transformationAssistant =
                                    additionalLine.split(attributes[i].name+"=");

                            additionalLine = transformationAssistant.at(1);

                            additionalLine.resize(additionalLine.indexOf(")"));

                            if(additionalLine.contains("["))
                                additionalLine.resize(additionalLine.indexOf("["));


                            qreal value = additionalLine.toDouble();

                            if(attributes[i].value == "")
                            {
                                attributes[i].value = "set";
                                attributes[i].maxValue = value;
                                attributes[i].minValue = value;
                            }

                            if(attributes[i].maxValue < value)
                                attributes[i].maxValue = value;

                            if(attributes[i].minValue > value)
                                attributes[i].minValue = value;
                        }
                        else
                        {
                        }
                    }
                    else
                    {
                    }

                }
            }

            if(line.startsWith("ATTRIBUTES "))
            {
                fillAttributesData = true;
            }

            if(fillAttributesData)
            {
                line = in.readLine();

                while(line.startsWith(" "))
                {
                    line.remove(0,1);
                    QStringList data = line.split(" ");
                    attributes[i].name = data.at(0);
                    attributes[i].type = data.at(1);
                    attributes[i].value = "";
                    i++;
                    line = in.readLine();
                }

                fillAttributesData = false;
            }

            if(line.startsWith("RULES "))
            {
                fillAttributesValues = true;
            }
        }
    }
}

void groupingThread::clusterRules()
{
    clusters = new cluster*[settings->objectsNumber];

    QString line;

    int i = 0;

    bool startClustering = false;

    QFile KB(groupingSettings->objectBaseInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!in.atEnd())
        {
            line = in.readLine();

            if(startClustering)
            {
                QString rule = line.split("[")[0];
                rule += ")";

                ruleCluster* temp = new ruleCluster(i+1, rule);

                QStringList splitedRule = line.split("=>");

                temp->support = splitedRule[1].split(" ")[1].toInt();

                for(int j = 0; j < groupingSettings->attributesNumber; j++)
                {
                    QString aName = attributes[j].name;

                    if(splitedRule[0].contains(aName))
                        temp->conclusionAttributes << aName;

                    if(splitedRule[1].contains(aName))
                        temp->decisionAttributes << aName;
                }

                temp->dispersion = 0;

                clusters[i] = temp;

                i++;
                continue;
            }

            if(line.startsWith("RULES "))
                startClustering = true;
        }
    }
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
                qreal simValue = countRSESClustersSimilarityValue(((ruleCluster*)clusters[i]),((ruleCluster*)clusters[j]));
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
            qreal simValue = countRSESClustersSimilarityValue(((ruleCluster*)clusters[newClusterIdx]),((ruleCluster*)clusters[i]));
            simMatrix->at(newClusterIdx)->push_back(qreal_ptr(new qreal(simValue)));
        }
    }

    //Then, the row is filled.
    for(unsigned int i = (newClusterIdx + 1); i <= (simMatrix->size() - 1) ; ++i)
    {
        qreal simValue = countRSESClustersSimilarityValue(((ruleCluster*)clusters[newClusterIdx]),((ruleCluster*)clusters[i]));
        simMatrix->at(i)->insert(simMatrix->at(i)->begin()+newClusterIdx, qreal_ptr(new qreal(simValue)));
    }
}

qreal groupingThread::countRSESClustersSimilarityValue(ruleCluster *c1, ruleCluster *c2)
{
    if(groupingSettings->interClusterSimMeasureID == groupingSettings->CENTROID_LINK_ID)
    {
        // For some reason if function is returned the value is 0. Hence variable was used.
        qreal result = countRSESRulesSimilarityValue(c1->representative,c2->representative);

        return  result;
    }

    if(groupingSettings->interClusterSimMeasureID == groupingSettings->AVERAGE_LINK_ID)
    {
        QStringList c1Rules = c1->getRules();
        QStringList c2Rules = c2->getRules();

        qreal similaritySum = 0;

        int denumerator = c1->size() * c2->size();

        for(int i = 0; i < c1Rules.size(); ++i)
        {
            for(int j = 0; j < c2Rules.size(); ++j)
            {
                similaritySum += countRSESRulesSimilarityValue(c1Rules[i],c2Rules[j]);
            }
        }

        // For some reason if function is returned the value is 0. Hence variable was used.
        qreal result = similaritySum / denumerator;

        return result;
    }

    if(c1->rule != "")
        return countRSESClusterRuleSimilarityValue(c1->rule, c2);

    if(c2->rule != "")
        return countRSESClusterRuleSimilarityValue(c2->rule, c1);


    if(groupingSettings->interClusterSimMeasureID == groupingSettings->SINGLE_LINK_ID)
        return qMax(countRSESClustersSimilarityValue(((ruleCluster*) c1->leftNode), c2),
                    countRSESClustersSimilarityValue(((ruleCluster*) c1->rightNode), c2));


    if(groupingSettings->interClusterSimMeasureID == groupingSettings->COMPLETE_LINK_ID)
        return qMin(countRSESClustersSimilarityValue(((ruleCluster*) c1->leftNode), c2),
                    countRSESClustersSimilarityValue(((ruleCluster*) c1->rightNode), c2));

    return -1;
}

qreal groupingThread::countRSESClusterRuleSimilarityValue(QString r, ruleCluster *c)
{
    if(c->rule !="")
        return countRSESRulesSimilarityValue(r, c->rule);

    if(groupingSettings->interClusterSimMeasureID == groupingSettings->SINGLE_LINK_ID)
        return qMax(countRSESClusterRuleSimilarityValue(r,((ruleCluster*) c->leftNode)),
                    countRSESClusterRuleSimilarityValue(r,((ruleCluster*) c->rightNode)));

    if(groupingSettings->interClusterSimMeasureID == groupingSettings->COMPLETE_LINK_ID)
        return qMin(countRSESClusterRuleSimilarityValue(r,((ruleCluster*) c->leftNode)),
                    countRSESClusterRuleSimilarityValue(r,((ruleCluster*) c->rightNode)));

    return -1;
}

qreal groupingThread::countRSESRulesSimilarityValue(QString r1, QString r2)
{
    switch(groupingSettings->interObjectSimMeasureID)
    {
        case groupingSettings_General::GOWERS_MEASURE_ID:
            return countRSESRulesGowersMeasureValue(r1,r2);

        case groupingSettings_General::SIMPLE_SIMILARITY_ID:
            return countRSESRulesSimpleSimilarityValue(r1,r2);

        case groupingSettings_General::WEIGHTED_SIMILARITY_ID:
            return countRSESRulesWeightedSimilarityValue(r1,r2);

        default:
            break;
    }

    return 0;
}

qreal groupingThread::countRSESRulesGowersMeasureValue(QString r1, QString r2)
{
    QStringList r1GroupedPart = getRuleGroupedPart(r1);
    QStringList r2GroupedPart = getRuleGroupedPart(r2);

    QStringList prepR1Argument;
    QStringList prepR2Argument;

    bool isNumber = false;

    int wager = 0;
    qreal sim = 0;

    qreal result = 0;

    for(int i=0; i < r1GroupedPart.length(); i++)
    {
        prepR1Argument = prepareAttribute(r1GroupedPart.at(i));

        for(int j=0; j < r2GroupedPart.length(); j++)
        {
            prepR2Argument = prepareAttribute(r2GroupedPart.at(j));

            if(prepR1Argument.at(0) == prepR2Argument.at(0))
            {
                isNumber = false;

                for(int k = 0; k < groupingSettings->attributesNumber; ++k)
                {
                    if(prepR1Argument.at(0) == attributes[k].name)
                    {
                        if(attributes[k].type == "symbolic")
                            isNumber = false;
                        else
                            isNumber = true;
                        break;
                    }
                }


                if(isNumber)
                {
                    qreal r1Value = (double)prepR1Argument.at(1).toDouble();

                    qreal val = (double) qAbs(r1Value - prepR2Argument.at(1).toDouble());

                    qreal denumerator = 0;

                    for(int k = 0; k < groupingSettings->attributesNumber; k++)
                    {
                        if(attributes[k].name == prepR1Argument.at(0))
                        {
                            denumerator = (double)
                                    attributes[k].maxValue -
                                    attributes[k].minValue;
                            break;
                        }
                    }

                    if(denumerator == 0)
                        val = 1;
                    else
                    {
                        val = ((qreal) val/denumerator);
                        val = ((qreal)1 - val);
                    }


                    sim += val;

                    wager++;
                }
                else
                {
                    if(prepR1Argument.at(0) == prepR2Argument.at(0))
                    {
                        wager++;
                        sim++;
                    }
                }
            }

            if(r1GroupedPart.at(i) == r2GroupedPart.at(j))
            {
                result++;
                break;
            }
        }
    }

    if(wager == 0)
        return 0;

    result = (double) sim/wager;

    return (double)result;
}

int groupingThread::countRSESRulesSimpleSimilarityValue(QString r1, QString r2)
{
    QStringList r1GroupedPart = getRuleGroupedPart(r1);
    QStringList r2GroupedPart = getRuleGroupedPart(r2);

    int similarityValue = 0;

    for(int i = 0; i < r1GroupedPart.length(); i++)
    {
        for(int j = 0; j < r2GroupedPart.length(); j++)
        {
            if(r1GroupedPart[i] == r2GroupedPart[j])
                similarityValue++;
        }
    }

    return similarityValue;
}

qreal groupingThread::countRSESRulesWeightedSimilarityValue(QString r1, QString r2)
{
    QSet<QString> argumentsConsidered;

    QStringList r1GroupedPart = getRuleGroupedPart(r1);
    QStringList r2GroupedPart = getRuleGroupedPart(r2);

    foreach(QString part, r1GroupedPart)
    {
        argumentsConsidered.insert(prepareAttribute(part)[0]);
    }

    foreach(QString part, r2GroupedPart)
    {
        argumentsConsidered.insert(prepareAttribute(part)[0]);
    }

    // For some reason it cannot be returned in one line. Dunno why.
    qreal result = countRSESRulesSimpleSimilarityValue(r1,r2);
    result /= argumentsConsidered.size();

    return result;
}

QStringList groupingThread::getRuleGroupedPart(QString r)
{
    QStringList ruleParts = r.split("=>");

    if(RSESSettings->groupingPartID == RSESSettings->CONCLUSION_ID)
    {
        QStringList conclusion;
        conclusion.append(ruleParts.at(1));
        return conclusion;
    }

    QStringList conditions = ruleParts.at(0).split("&");

    return conditions;
}

QStringList groupingThread::prepareAttribute(QString a)
{
    QString attribute = removeBraces(a);

    return attribute.split("=");
}

QString groupingThread::removeBraces(QString a)
{
    QString attribute = a;

    if(a.at(0) == ' ')
        attribute.remove(0,1);

    attribute.remove(0,1);
    attribute.remove(attribute.length()-1,1);

    return attribute;
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
;
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
    ruleCluster* temp = new ruleCluster();

    temp->leftNode = c1;
    temp->rightNode = c2;

    temp->longestRule =  getLongerRule  (((ruleCluster*) c1)->longestRule,
                                        ((ruleCluster*) c2)->longestRule);
    temp->shortestRule = getShorterRule (((ruleCluster*) c1)->shortestRule,
                                        ((ruleCluster*) c2)->shortestRule);

    temp->decisionAttributes
            .unite(((ruleCluster*) c1)->decisionAttributes);
    temp->decisionAttributes
            .unite(((ruleCluster*) c2)->decisionAttributes);

    temp->conclusionAttributes
            .unite(((ruleCluster*) c1)->conclusionAttributes);
    temp->conclusionAttributes
            .unite(((ruleCluster*) c2)->conclusionAttributes);

    temp->representative = createAverageRule(temp);

    temp->dispersion =
            countClusterDispersion(temp, temp->representative);

    temp->support = ((ruleCluster*) c1)->support + ((ruleCluster*) c2)->support;

    temp->clusterID = ++nextClusterID;

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

QString groupingThread::createAverageRule(ruleCluster *c)
{
    QString result = "average Rule";
    QString splitter;
    QString rule;
    QString ruleValue;
    QString atr;

    QStringList cRules = c->getRules();

    int unusedAtr = 0;
    int numericAtrNumber = 0;
    int symbolicAtrNumber = 0;
    int numericAtrIndex = 0;
    int symbolicAtrIndex = 0;
    int clusterSize = c->size();

    qreal* averageNumericAtrValues;
    QStringList* symbolicAtrValues;
    QString* mostCommonSymbolicAtrValue;
    QStringList symbolicAtrValuesSet;


    for(int i = 0; i < groupingSettings->attributesNumber; i++)
    {
        if(attributes[i].type == "numeric")
            numericAtrNumber++;
        else
            symbolicAtrNumber++;
    }

    averageNumericAtrValues = new qreal[numericAtrNumber];
    symbolicAtrValues = new QStringList[symbolicAtrNumber];
    mostCommonSymbolicAtrValue = new QString[symbolicAtrNumber];

    for(int i = 0; i < numericAtrNumber; i++)
        averageNumericAtrValues[i] = 0;

    for(int i = 0; i < cRules.length(); i++)
    {
        rule = cRules[i];

        symbolicAtrIndex = 0;
        numericAtrIndex = 0;

        for(int j = 0; j < groupingSettings->attributesNumber; j++)
        {                        
            splitter = attributes[j].name + "=";

            if(rule.contains(splitter))
            {
                ruleValue = rule.split(splitter)[1];

                ruleValue = ruleValue.split(")")[0];

                if(ruleValue.contains("["))
                    ruleValue = ruleValue.split("[")[0];

                if(attributes[j].type == "numeric")
                {
                    averageNumericAtrValues[numericAtrIndex]
                            += ruleValue.toFloat();
                }
                else
                    symbolicAtrValues[symbolicAtrIndex]
                            .append(ruleValue);

            }

            if(attributes[j].type == "numeric")
                numericAtrIndex++;
            else
                symbolicAtrIndex++;

        }
    }

    for(int i = 0; i < numericAtrNumber; i++)
        averageNumericAtrValues[i] /= clusterSize;

    for(int i = 0; i < symbolicAtrNumber; i++)
    {
        symbolicAtrValuesSet = symbolicAtrValues[i].toSet().toList();


        if(symbolicAtrValuesSet.isEmpty())
            mostCommonSymbolicAtrValue[i] = "";
        else
            mostCommonSymbolicAtrValue[i] = symbolicAtrValuesSet[0];


        for(int j = 1; j < symbolicAtrValuesSet.size(); j++)
        {
            if(symbolicAtrValues[i].count(symbolicAtrValuesSet[j])
                    > symbolicAtrValues[i].count(mostCommonSymbolicAtrValue[i]))
                mostCommonSymbolicAtrValue[i] = symbolicAtrValuesSet[j];
        }
    }

    symbolicAtrIndex = 0;
    numericAtrIndex = 0;
    result = "";

    for(int i = 0; i < groupingSettings->attributesNumber; i++)
    {
        if(!c->conclusionAttributes.contains(attributes[i].name) &&
           !c->decisionAttributes.contains(attributes[i].name))
        {
            unusedAtr++;


            if(attributes[i].type == "numeric")
                numericAtrIndex++;
            else
                symbolicAtrIndex++;

            continue;
        }

        if(result != "")
            atr = "&";
        else
            atr = "";

        if(i == groupingSettings->attributesNumber-1)
            atr = "=>";

        atr += "(" + attributes[i].name + "=";

        if(attributes[i].type == "numeric")
        {
            atr += QString::number(averageNumericAtrValues[numericAtrIndex]);
            numericAtrIndex++;
        }
        else
        {
            atr += mostCommonSymbolicAtrValue[symbolicAtrIndex];
            symbolicAtrIndex++;
        }

        atr += ")";

        result += atr;
    }

    return result;
}

qreal groupingThread::countClusterDispersion(ruleCluster *c, QString aRule)
{
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

    return countRSESRulesSimilarityValue(aRule, c->rule);
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
            currentSimilarity = ((qreal)countRSESClustersSimilarityValue(((ruleCluster*)clusters[i]),((ruleCluster*)clusters[j])));

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
    if(c1->rule != "")
        return countLowestRSESClusterRuleSimilarityValue(c1->rule, c2);

    return qMin(countLowestRSESInterclusterSimilarity(((ruleCluster*) c1->leftNode),c2),
                countLowestRSESInterclusterSimilarity(((ruleCluster*) c1->rightNode),c2));

    return -1;
}

qreal groupingThread::countLowestRSESClusterRuleSimilarityValue(QString r, ruleCluster *c)
{
    if(c->rule !="")
        return (double) countRSESRulesSimilarityValue(r, c->rule);    

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
                countRSESClustersSimilarityValue(((ruleCluster*)clusters[i]),((ruleCluster*)clusters[j]));

            if(std::abs(clustersSim)<= 1e-4)
                continue;

            qreal sumPart = ((ruleCluster*)clusters[i])->dispersion + ((ruleCluster*)clusters[j])->dispersion;

            sumPart /= clustersSim;

            sum += sumPart;
        }
    }

    return sum;
}
