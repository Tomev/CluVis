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

    grpSet->attributesNumber = 0;

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!line.startsWith(" "))
            line = in.readLine();

        while(line.startsWith(" "))
        {
            /*
             * RSES Knowledge bases stores attributes data in lines like:
             * <spacebar>atr_name<spacebar>atr_type
             * Hence line split(" ") will produce QStringList list like following:
             * lines(0) = "", lines(1) = atr_name, lines(2) = atr_type
             */

            ++(grpSet->attributesNumber);

            atrData = line.split(" ");
            atrName = atrData.at(1);

            if(atrData.at(2) == "numeric")
                attributes->insert(atrData.at(1), new numericAttributeData());
            else
                attributes->insert(atrData.at(1), new categoricalAttributeData());

            attributes->value(atrName)->name = atrData.at(1);
            attributes->value(atrName)->type = atrData.at(2);

            line = in.readLine();
        }
    }
}

void groupingDataPreparator_RSESRules::clusterObjects(cluster **clusters, QHash<QString, attributeData*> *attributes)
{
    int i = 0;

    QString line;

    QFile KB(grpSet->objectBaseInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        //Rewind to rules.
        while(!line.startsWith("RULES"))
            line = in.readLine();

        while(!in.atEnd())
        {
            line = in.readLine();
            clusters[i] = new ruleCluster(i);
            clusterRule(static_cast<ruleCluster*>(clusters[i]), line, attributes);
            ++i;
        }
    }

}

void groupingDataPreparator_RSESRules::clusterRule(ruleCluster* c, QString rule, QHash<QString, attributeData *> *attributes)
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

    // Conclusions are at(1), conditions at(0)
    conditionsConclusions = rule.split(")=>(");

    //Calling QString consturctor to no longer work on constant, enabling use of .remove method
    attributesValues = QString(conditionsConclusions.at(0))
        .remove(0, 1)   // Remove parentheses at the begining
        .split(")&(");  // Split into nice "name=value" strings;

    c->setDecisionGroupingInfo(dGrpSet->groupedPartID == 1);

    // Working on: "atr=val[sup]) sup"
    c->support = conditionsConclusions.at(1).split(" ").at(1).toInt();
    c->compactness = 0;

    // Conclusion attribute looks like "atr=val[sup]) sup", so split at "[" returns whats needed
    attributesValues.append(QString(conditionsConclusions.at(1)).split("[").at(0));

    for(int i = 0; i < attributesValues.length(); ++i)
    {
        attributeValue = attributesValues.at(i).split("=");
        c->attributesValues.insert(attributeValue.at(0), attributeValue.at(1));
        c->attributes.insert(attributeValue.at(0), attributes->value(attributeValue.at(0)));

        //Last attribute is decisional, hence:
        if(i == attributesValues.length()-1)
            c->decisionAttributes.insert(attributeValue.at(0));
        else
            c->premiseAttributes.insert(attributeValue.at(0));
    }

    c->fillRepresentativesAttributesValues(grpSet->repCreationStrategyID ,grpSet->repTreshold);
}

void groupingDataPreparator_RSESRules::fillAttributesValues(QHash<QString, attributeData*> *attributes, cluster** clusters)
{
    QStringList keys;
    QString atrName, atrMaxVal, atrMinVal;

    for(int i = 0; i < genSet->objectsNumber; ++i)
    {
        keys = clusters[i]->attributesValues.keys();

        for(int j = 0; j < keys.length(); ++j)
        {
            atrName = keys.at(j);

            if(attributes->value(atrName)->type == "symbolic")
            {
                categoricalAttributeData* currentAttribute =
                        static_cast<categoricalAttributeData*>(attributes->value(atrName));

                QString attributesValue = clusters[i]->attributesValues.value(atrName);

                if(currentAttribute->valuesFrequency.contains(attributesValue))
                    currentAttribute->valuesFrequency[attributesValue] += 1;
                else
                    currentAttribute->valuesFrequency.insert(attributesValue, 1);

                ++(currentAttribute->numberOfRulesWithGivenAttribute);
            }
            else
            {
                atrMaxVal = static_cast<numericAttributeData*>(attributes->value(atrName))->maxValue;

                // If atrMaxValue = "" then so does minValue. Set them and continue.
                if(atrMaxVal == "")
                {
                    QString newValue = clusters[i]->attributesValues.value(atrName);

                    // TODO: Reconsider what to do if numeric value is "MISSING"
                    if(newValue == "MISSING")
                        newValue = "0";

                    static_cast<numericAttributeData*>(attributes->value(atrName))
                        ->setMaxValue(newValue);

                    static_cast<numericAttributeData*>(attributes->value(atrName))
                        ->setMinValue(newValue);

                    continue;
                }

                // It's not near max to save time in case no values were set to attribute.
                atrMinVal = static_cast<numericAttributeData*>(attributes->value(atrName))->minValue;

                if(atrMaxVal.toDouble() < clusters[i]->attributesValues.value(atrName).toDouble())
                {
                    static_cast<numericAttributeData*>(attributes->value(atrName))->maxValue =
                        clusters[i]->attributesValues.value(atrName);
                }

                if(atrMinVal.toDouble() > clusters[i]->attributesValues.value(atrName).toDouble())
                {
                    static_cast<numericAttributeData*>(attributes->value(atrName))->minValue =
                        clusters[i]->attributesValues.value(atrName);
                }
            }
        }
    }
}
