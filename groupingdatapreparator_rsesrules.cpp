#include "groupingdatapreparator_rsesrules.h"

#include <QTextStream>

groupingDataPreparator_RSESRules::groupingDataPreparator_RSESRules(groupingSettings* settings)
: groupingDataPreparator(settings->genSet, settings->grpSet)
{
    this->dGrpSet = static_cast<groupingSettings_RSESRules*>(settings->dGrpSet);
}

void groupingDataPreparator_RSESRules::fillAttributesData(QHash<QString, attributeData> *attributes)
{
    QString line;
    QStringList atrData;
    attributeData newAttribute;
    QFile KB(grpSet->objectBaseInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!line.startsWith("ATTRIBUTES"))
            line = in.readLine();

        while(line.startsWith(" "))
        {
            /*
             * RSES Knowledge bases stores attributes data in lines like:
             * <spacebar>atr_name<spacebar>atr_type
             * Hence line split(" ") will produce QStringList list like following:
             * lines(0) = "", lines(1) = atr_name, lines(2) atr_type
             */

            atrData = line.split(" ");
            newAttribute.name = atrData.at(1);
            newAttribute.type = atrData.at(2);

            attributes->insert(newAttribute.name, newAttribute);

            line = in.readLine();
        }
    }
}

void groupingDataPreparator_RSESRules::clusterObjects(cluster **clusters)
{
    clusters = new cluster*[genSet->objectsNumber];
    int i = 0;

    QString line;

    QFile KB(grpSet->objectBaseInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!line.startsWith("RULES"))
            line = in.readLine();

        while(!in.atEnd())
        {
            clusterRule(clusters, i, line);
            ++i;
            line = in.readLine();
        }

    }
}

void groupingDataPreparator_RSESRules::fillNumericAttributesValues(QHash<QString, attributeData> *attributes)
{

}

void groupingDataPreparator_RSESRules::clusterRule(cluster **clusters, int i, QString rule)
{
    /*
     *  Rules in RSES knowledge bases are stored in following format:
     *  (attribute=value)&(atr=val)&(...)=>(decisionAtr=value[sup]) sup,
     *  hence I can use split to split them by ")&(" and ")=>(", thus
     *  removing some of the parentheses I don't want.
     *
     *  In all the KBs so far only one-decision rules were seen.
     *  I'll take that for granted and change it in the future.
     */

    QStringList conditionsConclusions, attributesValues, attributeValue;

    //TODO change into shared_ptr
    ruleCluster* temp = new ruleCluster(i, rule);

    // Conclusions are at(1), conditions at(0)
    conditionsConclusions = rule.split(")=>(");
    // Each value is unprepared attribute (with "(" at the begining.
    attributesValues = conditionsConclusions.at(0).split(")&(");

    temp->areDecisionsGrouped = dGrpSet->groupedPartID == 1;

    // Working on: "atr=val[sup]) sup"
    temp->support = conditionsConclusions.at(1).split(" ").at(1).toInt();

    for(int i = 0; i < attributesValues.length(); ++i)
        prepareAttribute(&attributesValues.at(i), false);

    attributesValues.append(prepareAttribute(conditionsConclusions.at(1), true));


    clusters[i] = temp;
}

void groupingDataPreparator_RSESRules::prepareAttribute(QString* unprepAtr, bool isDecisionAttribute)
{
    if(isDecisionAttribute)         // looks like atr=val[sup]) sup
        unprepAtr->split("[").at(1);
    else                            // looks like (atr=val
        unprepAtr->remove(0, 1);
}
