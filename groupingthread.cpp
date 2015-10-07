#include "groupingthread.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStringList>
#include <QMessageBox>

#include "generalsettings.h"

groupingThread::groupingThread(){}

groupingThread::groupingThread(groupingSettings_RSESRules *RSESSettings,
                               groupingSettings_General *groupingSettings,
                               generalSettings* settings)
{

    this->settings = settings;
    this->groupingSettings = groupingSettings;
    this->RSESSettings = RSESSettings;

    wasGroupingCanceled = false;


    groupingProgress = new QProgressDialog("Grupowanie reguł...","Anuluj",1,
                                           settings->objectsNumber,0,0);
    groupingProgress->setValue(0);
    groupingProgress->setWindowTitle("Grupowanie reguł");
    groupingProgress->setFixedSize(groupingProgress->sizeHint());
    groupingProgress->setWindowFlags(groupingProgress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    groupingProgress->setWindowModality(Qt::WindowModal);
    groupingProgress->move(QApplication::desktop()->availableGeometry().center()
                           - groupingProgress->rect().center()
                           - QPoint(groupingProgress->width()/2, 0));

    creatingSimMatrixProgress  = new QProgressDialog("Tworzenie macierzy podobieństwa...","Anuluj",1,
            settings->objectsNumber,0,0);
    creatingSimMatrixProgress->setValue(0);
    creatingSimMatrixProgress->setWindowTitle("Tworzenie macierzy podobieństwa");
    creatingSimMatrixProgress->setFixedSize(creatingSimMatrixProgress->sizeHint());
    creatingSimMatrixProgress->setWindowFlags(creatingSimMatrixProgress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    creatingSimMatrixProgress->setWindowModality(Qt::WindowModal);
    creatingSimMatrixProgress->move(QApplication::desktop()->availableGeometry().center()
                                    - creatingSimMatrixProgress->rect().center()
                                    + QPoint(20 + groupingProgress->width()/2,0));

}

groupingThread::~groupingThread()
{
}

void groupingThread::run()
{
     groupObjects();
}

void groupingThread::groupObjects()
{
    QString logText;

    switch(settings->dataTypeID)
    {
        case settings->RSES_RULES_ID:

            logText = "Rozpoczynam grupowanie dla RSES Rules...";

            emit passLogMsg(logText);

            groupRSESRules();
            break;
        default:

            logText = "Nieznany typ obiektów. Grupowanie nie rozpocznie się.";

            emit passLogMsg(logText);
            ;
    }
}

void groupingThread::groupRSESRules()
{
    QString logText = "Zbieram dane dotyczące atrybutów reguł...";
    emit passLogMsg(logText);

    //qDebug() << "Atr;";

    fillAttributesData();

    //qDebug() << "SimMatrix;";

    qreal** simMatrix;
    int simMatrixSize = settings->objectsNumber;

    int i = settings->objectsNumber - settings->stopCondition;

    logText = "Rozmieszczam reguły do skupień...";
    emit passLogMsg(logText);

    //qDebug() << "Cluster;";

    clusterRules();

    logText = "Rozpoczynam proces grupowania...";

    emit passLogMsg(logText);

    //qDebug() << "Okna;";

    groupingProgress->show();
    creatingSimMatrixProgress->show();

    //qDebug() << "Grupowanie;";

    while(i != 0)
    {
        //qDebug() << i << endl;

        QApplication::processEvents();

        simMatrix = createSimMatrix(simMatrixSize);

        if(groupingProgress->wasCanceled())
            wasGroupingCanceled = true;

        if(wasGroupingCanceled)
        {
            groupingProgress->close();
            creatingSimMatrixProgress->close();
            stopGrouping();
            return;
        }

        qreal highestSim = findHighestSimilarity(simMatrix, simMatrixSize);

        joinMostSimilarClusters(simMatrix, simMatrixSize, highestSim);

        simMatrixSize--;
        i--;

        groupingProgress->setValue(settings->objectsNumber-simMatrixSize+settings->stopCondition);
    }

    groupingProgress->close();
    creatingSimMatrixProgress->close();

    countMDI();
    countMDBI();

    logText = "Liczba skupień: " + QString::number(settings->stopCondition) + ". Wskaźnik MDI grupowania: " + QString::number(MDI) +".";
    emit passLogMsg(logText);

    logText = "Grupowanie zakończone. Przesyłam otrzymane struktury...";
    emit passLogMsg(logText);

    emit passMDI(MDI);
    emit passMDBI(MDBI);
    emit passRules(rules);
    emit passJoinedRules(joinedRules);
    emit passClusteredRules(clusteredRules);
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
    rules = new ruleCluster[settings->objectsNumber];
    clusteredRules = new ruleCluster*[settings->objectsNumber];
    joinedRules = new ruleCluster[settings->objectsNumber];

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
                rules[i].rule = line.split("[")[0];
                rules[i].rule += ")";
                rules[i].clusterID = i;
                rules[i].longestRule = rules[i].rule;
                rules[i].shortestRule = rules[i].rule;

                QStringList splitedRule = line.split("=>");

                rules[i].support = splitedRule[1].split(" ")[1].toInt();

                for(int j = 0; j < groupingSettings->attributesNumber; j++)
                {
                    QString aName = attributes[j].name;

                    if(splitedRule[0].contains(aName))
                        rules[i].conclusionAttributes << aName;
                    if(splitedRule[1].contains(aName))
                        rules[i].decisionAttributes << aName;
                }

                rules[i].representative = rules[i].rule;
                        //createAverageRule(&rules[i]);

                clusteredRules[i] = &rules[i];

                i++;
                continue;
            }

            if(line.startsWith("RULES "))
                startClustering = true;
        }
    }
}

qreal **groupingThread::createSimMatrix(int simMatrixSize)
{
    qreal **simMatrix = new qreal*[simMatrixSize];

    //qDebug() << "Matrix size:" << simMatrixSize;

    for(int i = 0; i < simMatrixSize; i++)
        simMatrix[i] = new qreal[simMatrixSize];

    for(int i =0 ; i < simMatrixSize; i++)
    {
        if(creatingSimMatrixProgress->wasCanceled())
            wasGroupingCanceled = true;

        if(wasGroupingCanceled)
            break;

        simMatrix[simMatrixSize-1][simMatrixSize-1] = 3;
         creatingSimMatrixProgress->setValue(i);
        for(int j = 0; j <= i; j++)
        {
            if(i==j)
                simMatrix[i][j] = 0;
            else
            {
                //qDebug() << "i = " << i << " j = " << j << endl;
                qreal simValue = (double) countRSESClustersSimilarityValue(clusteredRules[i],clusteredRules[j]);
                simMatrix[i][j] = simMatrix[j][i] = simValue;
            }
        }
    }

    //qDebug() << "Wychodzę";

    creatingSimMatrixProgress->setMaximum(simMatrixSize);

    //qDebug() << "Zwracam";

    return simMatrix;
}

qreal groupingThread::countRSESClustersSimilarityValue(ruleCluster *c1, ruleCluster *c2)
{
    if(groupingSettings->interclusterDistanceMeasureID == groupingSettings->CENTROID_LINK_ID)
    {
        qreal result = (double) countRSESRulesSimilarityValue(c1->representative,c2->representative);

        return  result;
    }

    if(c1->rule != "")
        return countRSESClusterRuleSimilarityValue(c1->rule, c2);

    if(c2->rule != "")
        return countRSESClusterRuleSimilarityValue(c2->rule, c1);

    if(groupingSettings->interclusterDistanceMeasureID == groupingSettings->AVERAGE_LINK_ID)
    {
        QStringList c1Rules = c1->getRules();
        QStringList c2Rules = c2->getRules();

        qreal similaritySum = 0;
        int denumerator = c1->size() * c2->size();

        for(int i = 0; i < c1Rules.size(); i++)
        {
            for(int j = 0; j < c2Rules.size(); j++)
            {
                similaritySum += countRSESRulesSimilarityValue(c1Rules[i],c2Rules[j]);
            }
        }

        return (double) similaritySum / denumerator;
    }

    if(groupingSettings->interclusterDistanceMeasureID == groupingSettings->SINGLE_LINK_ID)
        return qMax(countRSESClustersSimilarityValue(c1->leftNode,c2),
                countRSESClustersSimilarityValue(c1->rightNode,c2));


    if(groupingSettings->interclusterDistanceMeasureID == groupingSettings->COMPLETE_LINK_ID)
        return qMin(countRSESClustersSimilarityValue(c1->leftNode,c2),
                countRSESClustersSimilarityValue(c1->rightNode,c2));

    return -1;
}

qreal groupingThread::countRSESClusterRuleSimilarityValue(QString r, ruleCluster *c)
{
    if(c->rule !="")
        return countRSESRulesSimilarityValue(r, c->rule);

    if(groupingSettings->interclusterDistanceMeasureID == groupingSettings->SINGLE_LINK_ID)
        return qMax(countRSESClusterRuleSimilarityValue(r,c->leftNode),
                    countRSESClusterRuleSimilarityValue(r,c->rightNode));

    if(groupingSettings->interclusterDistanceMeasureID == groupingSettings->COMPLETE_LINK_ID)
        return qMin(countRSESClusterRuleSimilarityValue(r,c->leftNode),
                    countRSESClusterRuleSimilarityValue(r,c->rightNode));

    if(groupingSettings->interclusterDistanceMeasureID == groupingSettings->AVERAGE_LINK_ID)
        return countRSESClusterRuleSimilarityValue(r,c->leftNode) +
                countRSESClusterRuleSimilarityValue(r,c->rightNode);

    return -1;
}

qreal groupingThread::countRSESRulesSimilarityValue(QString r1, QString r2)
{
    switch(groupingSettings->interobjectDistanceMeasureID)
    {
        case(groupingSettings->GOWERS_MEASURE_ID):
            return countRSESRulesGowersMeasureValue(r1,r2);

        case(groupingSettings->SIMPLE_SIMILARITY_ID):
            return countRSESRulesSimpleSimilarityValue(r1,r2);

        case(groupingSettings->WEIGHTED_SIMILARITY_ID):
            return countRSESRulesWeightedSimilarityValue(r1,r2);

        default:
            break;
    }
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
                qreal r1Value = (double)prepR1Argument.at(1).toDouble(&isNumber);

                if(isNumber)
                {
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
                        val = (double) val/denumerator;
                        val = (double)1 - val;
                    }

                    sim += (double) val;
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

qreal groupingThread::countRSESRulesSimpleSimilarityValue(QString r1, QString r2)
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

    return (double)(countRSESRulesSimpleSimilarityValue(r1,r2)/argumentsConsidered.size());

}

QStringList groupingThread::getRuleGroupedPart(QString r)
{
    //qDebug() << "Tu";

    QStringList ruleParts = r.split("=>");

    //qDebug() << r;

    if(RSESSettings->groupingPartID == RSESSettings->CONCLUSION_ID)
    {
        QStringList conclusion;
        //qDebug() << "Problem?";
        conclusion.append(ruleParts.at(1));
        //qDebug() << "Tam";
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

    //qDebug() << "Atr: " + a;

    if(a.at(0) == " ")
        attribute.remove(0,1);


    attribute.remove(0,1);
    attribute.remove(attribute.length()-1,1);

    //qDebug() << "Remove:" + attribute;

    return attribute;
}

qreal groupingThread::findHighestSimilarity(qreal **simMatrix, int simMatrixSize)
{
    qreal result = 0;

    for(int i = 0; i < simMatrixSize; i++)
    {
        for(int j = 0; j < i; j++)
        {
            if(simMatrix[i][j] > result)
                result = (double)simMatrix[i][j];
        }
    }

    return result;
}

void groupingThread::joinMostSimilarClusters(qreal **simMatrix, int simMatrixSize, qreal highestSim)
{
    for(int i = 0; i < simMatrixSize; i++)
    {
        for(int j = 0; j < i; j++)
        {

            if(simMatrix[i][j] == highestSim)
            {

                int k = 0;

                while(joinedRules[k].leftNode != NULL)
                    k++;

                joinedRules[k].clusterID = k;
                joinedRules[k].leftNode = clusteredRules[i];
                joinedRules[k].rightNode = clusteredRules[j];

                joinedRules[k].longestRule = getLongerRule(clusteredRules[i]->longestRule,
                                                        clusteredRules[j]->longestRule);
                joinedRules[k].shortestRule = getShorterRule(clusteredRules[i]->shortestRule,
                                                          clusteredRules[j]->shortestRule);
                joinedRules[k].decisionAttributes
                        .unite(clusteredRules[i]->decisionAttributes);
                joinedRules[k].decisionAttributes
                        .unite(clusteredRules[j]->decisionAttributes);

                joinedRules[k].conclusionAttributes
                        .unite(clusteredRules[i]->conclusionAttributes);
                joinedRules[k].conclusionAttributes
                        .unite(clusteredRules[j]->conclusionAttributes);

                joinedRules[k].representative =
                        createAverageRule(&joinedRules[k]);

                joinedRules[k].dispersion =
                        countClusterDispersion(&joinedRules[k], joinedRules[k].representative, false);

                clusteredRules[i] = &joinedRules[k];
                clusteredRules[j] = clusteredRules[simMatrixSize-1];

                return;
            }
        }
    }
}

QString groupingThread::getLongerRule(QString r1, QString r2)
{
    if(getRuleAttributesNumber(r1) > getRuleAttributesNumber(r2))
            return r1;

    if(getRuleAttributesNumber(r1) < getRuleAttributesNumber(r2))
            return r2;

    if(getRuleAttributesNumber(r1) == getRuleAttributesNumber(r2))
        if(r1.length() > r2.length())
           return r1;
        else
            return r2;

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
        if(r1.length() > r2.length())
           return r2;
        else
            return r1;

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

    //qDebug() << symbolicAtrNumber;


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

    //qDebug() << "Badam listy.";

    for(int i = 0; i < numericAtrNumber; i++)
        averageNumericAtrValues[i] /= clusterSize;

    for(int i = 0; i < symbolicAtrNumber; i++)
    {
        //qDebug() << i;

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
            atr = " & ";
        else
            atr = "";

        if(i == groupingSettings->attributesNumber-1)
            atr = " => ";

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

qreal groupingThread::countClusterDispersion(ruleCluster *c, QString aRule, bool recursion)
{
    if(c->hasBothNodes())
        return countClusterDispersion(c->leftNode, aRule, true)+ countClusterDispersion(c->rightNode, aRule, true);

    if(c->size()==1 && !recursion)
    {
        qDebug() << "Zwracam 0";
        return 0;
    }

    else
        return countRSESClusterRuleSimilarityValue(aRule, c);
}

void groupingThread::stopGrouping()
{
    QString msgBoxText = "Grupowanie przerwane.\n";
    msgBoxText += "Wizualizacja będzie niemożliwa.";

    QMessageBox::information(0,
                            "Grupowanie przerwane",
                            msgBoxText,
                            QMessageBox::Ok);

    QString logText = "Grupowanie przerwane na życzenie użytkownika.";
    logText += "Wizualizacja nie będzie możliwa do czasu pełnego pogrupowania.";
    emit passLogMsg(logText);
}

void groupingThread::countMDI()
{
    qreal minInsideSimilarity = -1;
    qreal maxOutsideSimilarity = 0;
    qreal currentSimilarity = 0;

    for(int i = 0; i < settings->stopCondition; i++)
    {
        for(int j = i+1; j < settings->stopCondition; j++)
        {
            currentSimilarity = (double) countRSESClustersSimilarityValue(clusteredRules[i],clusteredRules[j]);

            if(currentSimilarity > maxOutsideSimilarity)
                maxOutsideSimilarity = currentSimilarity;
        }
    }

    for(int i = 0; i < settings->stopCondition; i++)
    {
        if(clusteredRules[i]->size() == 1)
            continue;

        currentSimilarity = (double) countLowestRSESInterclusterSimilarity(
                    clusteredRules[i]->leftNode, clusteredRules[i]->rightNode);

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

    MDI = (double)(maxOutsideSimilarity / minInsideSimilarity);
}

qreal groupingThread::countLowestRSESInterclusterSimilarity(ruleCluster *c1, ruleCluster *c2)
{
    if(c1->rule != "")
        return countLowestRSESClusterRuleSimilarityValue(c1->rule, c2);

    return qMin(countLowestRSESInterclusterSimilarity(c1->leftNode,c2),
                countLowestRSESInterclusterSimilarity(c1->rightNode,c2));

    return -1;
}

qreal groupingThread::countLowestRSESClusterRuleSimilarityValue(QString r, ruleCluster *c)
{
    if(c->rule !="")
        return (double) countRSESRulesSimilarityValue(r, c->rule);    

    return qMin(countLowestRSESClusterRuleSimilarityValue(r,c->leftNode),
                countLowestRSESClusterRuleSimilarityValue(r,c->rightNode));

    return -1;
}

void groupingThread::countMDBI()
{
    qreal similaritySum = countSimilaritySum();
    MDBI = (qreal)(similaritySum/settings->stopCondition);
}

qreal groupingThread::countSimilaritySum()
{
    qreal sum;

    for(int i = 0; i < settings->stopCondition; i++)
    {
        for(int j = i+1; j < settings->stopCondition; j++)
        {
            qreal clustersSim =
                    countRSESClustersSimilarityValue(clusteredRules[i],clusteredRules[j]);

            if(clustersSim == 0)
                continue;

            qreal sumPart = clusteredRules[i]->dispersion + clusteredRules[j]->dispersion;
            sumPart /= clustersSim;

            sum += sumPart;
        }
    }

    return sum;
}
