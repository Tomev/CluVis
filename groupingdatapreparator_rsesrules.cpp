#include "groupingdatapreparator_rsesrules.h"

#include <QTextStream>
#include <QDebug>

groupingDataPreparator_RSESRules::groupingDataPreparator_RSESRules(groupingSettings* settings)
: groupingDataPreparator(settings->genSet, settings->grpSet)
{
    this->dGrpSet = static_cast<groupingSettings_RSESRules*>(settings->dGrpSet);
}

void groupingDataPreparator_RSESRules::fillAttributesData(QHash<QString, attributeData*> *attributes)
{
    QString line, atrName;
    QStringList atrData;
    QFile KB(grpSet->objectBaseInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!line.startsWith("ATTRIBUTES"))
            line = in.readLine();

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
            atrName = atrData.at(1);

            if(atrData.at(2) == "numeric")
                attributes->insert(atrData.at(1), new numericAttributeData());
            else
                attributes->insert(atrData.at(1), new attributeData());

            attributes->value(atrName)->name = atrData.at(1);
            attributes->value(atrName)->type = atrData.at(2);

            line = in.readLine();
        }
    }
}

void groupingDataPreparator_RSESRules::clusterObjects(cluster **clusters)
{
    int i = 0;

    QString line;

    QFile KB(grpSet->objectBaseInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!line.startsWith("RULES"))
            line = in.readLine();

        line = in.readLine();

        while(!in.atEnd())
        {
            clusterRule(clusters, i, line);
            ++i;
            line = in.readLine();
        }
    }
}

void groupingDataPreparator_RSESRules::fillNumericAttributesValues(QHash<QString, attributeData*> *attributes, cluster** clusters)
{
    qDebug() << "Filling.";

    QStringList keys;
    QString atrName, atrMaxVal, atrMinVal;

    for(int i = 0; i < genSet->objectsNumber; ++i)
    {
        qDebug() << "i = " << i;

        keys = clusters[i]->attributesValues.keys();

        for(int j = 0; j < keys.length(); ++j)
        {
            qDebug() << "j = " << j;

            atrName = keys.at(j);

            qDebug() << attributes->keys().length();
            qDebug() << "Keys " << attributes->keys();
            qDebug() << "obj keys" << keys;

            qDebug() << "Key = " << atrName;
            qDebug() << "atrName = " << attributes->value(atrName)->name;
            qDebug() << "Type = " << attributes->value(atrName)->type;

            if(attributes->value(atrName)->type == "symbolic")
                continue;

            atrMaxVal = static_cast<numericAttributeData*>(attributes->value(atrName))->maxValue;

            //If atrMaxValue = "" then so does minValue
            if(atrMaxVal == "")
            {
                qDebug() << "MaxVal == ''";
                qDebug() << "AtrName = " << atrName;
                qDebug() << "new value = " << clusters[i]->attributesValues.value(atrName);

                static_cast<numericAttributeData*>(attributes->value(atrName))
                    ->setMaxValue(clusters[i]->attributesValues.value(atrName));

                qDebug() << "Atr new max value = " << static_cast<numericAttributeData*>(attributes->value(atrName))->maxValue;

                static_cast<numericAttributeData*>(attributes->value(atrName))
                    ->setMinValue(clusters[i]->attributesValues.value(atrName));

                continue;
            }

            atrMinVal = static_cast<numericAttributeData*>(attributes->value(atrName))->minValue;

            if(atrMaxVal.toDouble() < clusters[i]->attributesValues.value(atrName).toDouble())
            {
                //qDebug() << "atr name" << atrName;
                //qDebug() << "Atr max = " << atrMaxVal;
                //qDebug() << "Clusters val" << clusters[i]->attributesValues.value(atrName);

                static_cast<numericAttributeData*>(attributes->value(atrName))->maxValue =
                    clusters[i]->attributesValues.value(atrName);
            }

            if(atrMinVal.toDouble() > clusters[i]->attributesValues.value(atrName).toDouble())
            {
                //qDebug() << "atr name" << atrName;
                //qDebug() << "Atr max = " << atrMaxVal;
                //qDebug() << "Clusters val" << clusters[i]->attributesValues.value(atrName);

                static_cast<numericAttributeData*>(attributes->value(atrName))->minValue =
                    clusters[i]->attributesValues.value(atrName);
            }
        }
    }

    qDebug() << "Fucking done";
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

    clusters[i] = new ruleCluster(i, rule);

    // Conclusions are at(1), conditions at(0)
    conditionsConclusions = rule.split(")=>(");
    //Calling QString consturctor to no longer work on constant, enabling use of .remove method
    attributesValues = QString(conditionsConclusions.at(0))
            .remove(0, 1) // Remove parentheses at the begining
            .split(")&("); // Split into nice "name=value" strings;

    static_cast<ruleCluster*>(clusters[i])->areDecisionsGrouped = dGrpSet->groupedPartID == 1;

    // Working on: "atr=val[sup]) sup"
    static_cast<ruleCluster*>(clusters[i])->support = conditionsConclusions.at(1).split(" ").at(1).toInt();

    // Conclusion attribute looks like "atr=val[sup]) sup", so split at "[" returns whats needed
    attributesValues.append(QString(conditionsConclusions.at(1)).split("[").at(0));

    for(int j = 0; j < attributesValues.length(); ++j)
    {
        attributeValue = attributesValues.at(j).split("=");
        static_cast<ruleCluster*>(clusters[i])->attributesValues.insert(attributeValue.at(0), attributeValue.at(1));
    }
}
