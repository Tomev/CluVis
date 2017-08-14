#include "groupingdatapreparator_rsesrules.h"

#include <QTextStream>
#include <QDebug>

groupingDataPreparator_RSESRules::groupingDataPreparator_RSESRules(groupingSettings* settings)
: groupingDataPreparator(settings->genSet, settings->grpSet)
{
    this->dGrpSet =
      static_cast<groupingSettings_RSESRules*>(settings->dGrpSet);
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

void groupingDataPreparator_RSESRules::clusterObjects(
    QVector<cluster*> *clusters, QHash<QString, attributeData*> *attributes)
{
  clusters->clear();
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
      clusters->push_back(new ruleCluster(i++));
      clusterRule(static_cast<ruleCluster*>(clusters->last()), line, attributes);
    }
  }
}

void groupingDataPreparator_RSESRules::clusterRule(
    ruleCluster* c, QString rule, QHash<QString, attributeData *> *attributes)
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

    // Calling QString consturctor to no longer work on constant, enabling use
    // of .remove method
    attributesValues = QString(conditionsConclusions.at(0))
        .remove(0, 1)   // Remove parentheses at the begining
        .split(")&(");  // Split into nice "name=value" strings;

    c->setDecisionGroupingInfo(dGrpSet->groupedPartID == 1);

    // Working on: "atr=val[sup]) sup"
    c->support = conditionsConclusions.at(1).split(" ").at(1).toInt();
    c->compactness = 0;

    // Conclusion attribute looks like "atr=val[sup]) sup", so split at "[" returns what's needed
    attributesValues.append(QString(conditionsConclusions.at(1)).split("[").at(0));

    for(int i = 0; i < attributesValues.length(); ++i)
    {
        attributeValue = attributesValues.at(i).split("=");

        // Check if attribute isn't on attributes values list
        if(!c->attributesValues.keys().contains(attributeValue.at(0)))
        {
            // If so add it with empty QStringList
            c->attributesValues.insert(attributeValue.at(0), new QStringList());
        }

        c->attributesValues.value(attributeValue.at(0))->append(attributeValue.at(1));
        c->attributes.insert(attributeValue.at(0), attributes->value(attributeValue.at(0)));

        //Last attribute is decisional, hence:
        if(i == attributesValues.length()-1)
            c->decisionAttributes.insert(attributeValue.at(0));
        else
            c->premiseAttributes.insert(attributeValue.at(0));
    }

    c->fillRepresentativesAttributesValues(grpSet->repCreationStrategyID ,grpSet->repTreshold);
}

void groupingDataPreparator_RSESRules::fillAttributesValues(QHash<QString, attributeData*> *attributes, QVector<cluster *> *clusters)
{
    QStringList keys;
    QString atrName, atrMaxVal, atrMinVal;

    for(int i = 0; i < genSet->objectsNumber; ++i)
    {
        keys = clusters->at(i)->attributesValues.keys();

        for(int j = 0; j < keys.length(); ++j)
        {
            atrName = keys.at(j);

            if(attributes->value(atrName)->type == "symbolic")
            {
                categoricalAttributeData* currentAttribute =
                        static_cast<categoricalAttributeData*>(attributes->value(atrName));

                QStringList* attributesValues = clusters->at(i)->attributesValues.value(atrName);

                foreach(const QString value, *attributesValues)
                {
                    if(currentAttribute->valuesFrequency.contains(value))
                        currentAttribute->valuesFrequency[value] += 1;
                    else
                        currentAttribute->valuesFrequency.insert(value, 1);
                }

                ++(currentAttribute->numberOfRulesWithGivenAttribute);
            }
            else
            {
                numericAttributeData* atrData =
                  static_cast<numericAttributeData*>(attributes->value(atrName));

                atrMaxVal = atrData->maxValue;

                // If atrMaxValue is equal to "" or "MISSING" then
                // so does minValue. Set them and continue.
                if(atrMaxVal == "" || atrMaxVal == "MISSING")
                {
                    QStringList* newValues =
                        clusters->at(i)->attributesValues.value(atrName);

                    // Set initial values
                    atrData->setMaxValue(newValues->at(0));
                    atrData->setMinValue(newValues->at(0));

                    // For each value in values set
                    foreach(const QString value, *newValues)
                    {
                        atrData->valuesInBase.append(value);

                        // Check if value is missing
                        if(value == "MISSING")
                        {
                            // If so continue
                            continue;
                        }

                        // Check if maxValue (on thus also minValue) is MISSING
                        if(atrData->maxValue == "MISSING")
                        {
                            // If so set value as new min and max value
                            atrData->setMaxValue(value);
                            atrData->setMinValue(value);
                        }
                        else
                        {
                            // If their values check if new value is maximal of minimal
                            if(atrData->maxValue.toDouble() < value.toDouble())
                            {
                                atrData->setMaxValue(value);
                            }

                            if(atrData->minValue.toDouble() > value.toDouble())
                            {
                                atrData->setMinValue(value);
                            }
                        }
                    }

                    continue;
                }

                // When they're not MISSING nor empty then check for new
                // maximum or minimum

                atrMinVal = atrData->minValue;

                QStringList* newValues = clusters->at(i)->attributesValues.value(atrName);

                foreach(const QString value, *newValues)
                {
                    atrData->valuesInBase.append(value);

                    if(value != "MISSING")
                    {
                        if(atrMaxVal.toDouble() < value.toDouble())
                        {
                            atrData->setMaxValue(value);
                        }

                        if(atrMinVal.toDouble() > value.toDouble())
                        {
                            atrData->setMinValue(value);
                        }
                    }
                }

                atrData->valuesInBase = atrData->valuesInBase.toSet().toList();
            }
        }
    }
}
