#ifndef CLUSTERS
#define CLUSTERS

#include <QString>
#include <QStringList>
#include <QSet>
#include <QDebug>

#include <memory>

#include "attributedata.h"

enum interClusterSimilarityMeasureId
{
    SingleLinkId = 0,
    CompleteLinkId = 1,
    AverageLinkId = 2,
    CentroidLinkId = 3,
    GenieGiniId = 4,
    GenieBonferroniId = 5
};

enum rulePart
{
  PREMISES = 0,
  DECISIONS = 1,
  BOTH = 2
};

struct descriptor
{
    QString attribute;
    QString value;

    descriptor(QString attribute, QString value)
    {
        this->attribute = attribute;
        this->value = value;
    }

    bool equals(descriptor comparator) const
    {
        return (this->attribute == comparator.attribute &&
                this->value     == comparator.value);
    }

    QString toString() const
    {
        return "Attribute: " + this->attribute + ", Value: " + this->value;
    }
};

struct cluster
{
    public:

        cluster(int id = 0)
        {
            this->clusterID = id;
        }

    //Members
        QHash<QString, QStringList*> attributesValues;
        QHash<QString, QStringList*> representativeAttributesValues;

        // Cosists of attributes that are present in cluster.
        QHash<QString, attributeData*> attributes;

        /*
         * Compactness requires knowledge about similarity measures
         * hence should be filled in grouping thread.
         */
        qreal compactness;

        std::shared_ptr<cluster> leftNode;
        std::shared_ptr<cluster> rightNode;

        //Methods
        QString name()
    {
        QString result = "";

        if(hasBothNodes())
            result+="J";
        else
            result+="O";

        result += QString::number(clusterID+1);

        return result;
    }

        int size()
    {
        if(hasBothNodes())
            return leftNode->size() + rightNode->size();

        return 1;
    }

        int nodesNumber()
    {
        return 2*size()-1;
    }

        bool hasBothNodes()
    {
        if(leftNode.use_count() == 0 || rightNode.use_count() == 0)
            return false;

        return true;
    }

        QList<cluster*> getObjects()
        {
            if(this->hasBothNodes())
                return this->leftNode->getObjects() + this->rightNode->getObjects();

            QList<cluster*> result;
            result.append(this);

            return result;
        }

        // TODO: It's written horribly. To be rewritten as soon as work is done.

        void fillRepresentativesAttributesValues(unsigned int strategyID, unsigned int threshold)
        {
          if(size() == 1)
          {
            representativeAttributesValues = attributesValues;
            return;
          }

            switch(strategyID)
            {
                case 0: //Threshold
                    fillThresholdRepresentativesAttributesValues(threshold);
                    break;
                case 1:
                    fillLowerEstimationRepresentativeAttributesValues();
                    break;
                case 2:
                    fillHigherEstimationRepresentativeAttributesValues();
                    break;
                case 3:
                    fillThresholdEstimationRepresentativeAttributesValues(threshold);
                    break;
                default:
                    fillThresholdRepresentativesAttributesValues(30); // Default settings
            }
        }

        void fillThresholdRepresentativesAttributesValues(int treshold)
        {
            QStringList repAttributes = getRepresentativesAttributesList(treshold);
            QString atrName, avgValue;

            // For each representative attribute
            for(int i = 0; i < repAttributes.length(); ++i)
            {
                atrName = repAttributes.at(i);

                // Check if this attribute is already in representative
                if(representativeAttributesValues.keys().contains(atrName))
                {
                    // Do nothing if so
                    continue;
                }
                else
                {
                    // If not add it with empty QStringList
                    representativeAttributesValues.insert(atrName, new QStringList());
                }

                // Add average value of the attribute into the QList of that attribute
                avgValue = getAttributesAverageValue(atrName);

                representativeAttributesValues.value(atrName)->append(avgValue);
            }
        }

        void fillLowerEstimationRepresentativeAttributesValues()
        {
            QList<descriptor> representativeCandidate;

            // Get all objects within given cluster
            QList<cluster*> objects = this->getObjects();

            // For first object descriptors
            foreach(const QString key, objects.at(0)->attributesValues.keys())
            {
                foreach(const QString value, *(objects.at(0)->attributesValues.value(key)))
                {
                    // Add descriptor to descriptors list
                    representativeCandidate.append(descriptor(key,value));
                }
            }

            // Remove first object
            objects.removeAt(0);

            // For all remaining objects
            foreach(const cluster* object, objects)
            {
                // For all descriptors in potential representative
                for(int descriptorNumber = representativeCandidate.size()-1; descriptorNumber >= 0 ; --descriptorNumber)
                {
                    // Check if it's attribute is in set
                    if(object->attributesValues.keys().contains(representativeCandidate.at(descriptorNumber).attribute))
                    {
                        // If so, check if value is the same
                        if(object->attributesValues
                                .value(representativeCandidate
                                .at(descriptorNumber).attribute)
                                ->contains(representativeCandidate.at(descriptorNumber).value))
                        {
                            // If so do nothing
                            continue;
                        }
                        else
                        {
                            // If not then remove it from descriptors list
                            representativeCandidate.removeAt(descriptorNumber);
                            continue;
                        }
                    }
                    else
                    {
                        // If not then remove it from descriptors list
                        representativeCandidate.removeAt(descriptorNumber);
                        continue;
                    }
                }

                // Check if there are any descriptors left
                if(representativeCandidate.size() > 0)
                {
                    // If so do nothing
                    continue;
                }
                else
                {
                    // If not return
                    return;
                }
            }

            // Return result as representative
            foreach(const descriptor desc, representativeCandidate)
            {
                // If attribute is not already in the hash
                if(!representativeAttributesValues.keys().contains(desc.attribute))
                {
                    // Add it with empty QStringList
                    representativeAttributesValues.insert(desc.attribute, new QStringList());
                }

                // Insert value of the descriptor to appropriate
                representativeAttributesValues.value(desc.attribute)->append(desc.value);
            }
        }

        void fillHigherEstimationRepresentativeAttributesValues()
        {
            QList<descriptor> representativeCandidate;

            // Get all objects within given cluster
            QList<cluster*> objects = this->getObjects();

            // For each object
            foreach(const cluster* object, objects)
            {
                // For each descriptor
                foreach(const QString key, object->attributesValues.keys())
                {
                    foreach(const QString value, *(object->attributesValues.value(key)))
                    {
                        // Save it to list
                        representativeCandidate.append(descriptor(key, value));
                    }
                }
            }

            // Save these descriptors to representative
            foreach(const descriptor desc, representativeCandidate)
            {
                // If attribute is not in the hashtable add it with empty QStringList
                if(!representativeAttributesValues.keys().contains(desc.attribute))
                {
                    representativeAttributesValues.insert(desc.attribute, new QStringList());
                }

                // Add descriptors value to the appropriate list
                representativeAttributesValues.value(desc.attribute)->append(desc.value);
            }

            // Remove duplicate values from each QStringList of attribute
            foreach(QStringList* values, representativeAttributesValues)
            {
                values->removeDuplicates();
            }

        }

        void fillThresholdEstimationRepresentativeAttributesValues(int threshold)
        {
            QList<descriptor> representativeCandidate;

            // Check if threshold is < 0 or > 100
            if(threshold > 100 || threshold < 0)
            {
                // If so return
                return;
            }

            // Get all objects within given cluster
            QList<cluster*> objects = this->getObjects();

            // For each object
            foreach(const cluster* object, objects)
            {
                // For each descriptor
                foreach(const QString key, object->attributesValues.keys())
                {
                    foreach(const QString value, *(object->attributesValues.value(key)))
                    {
                        // Save it to list
                        representativeCandidate.append(descriptor(key, value));
                    }
                }
            }

            // For each descriptor pair
            int descriptorCouter, descriptorIndex = 0, comparatorIndex;
            qreal requiredOccurenceNumber = objects.size() * threshold / 100.0;

            while(representativeCandidate.size() > descriptorIndex)
            {
                descriptorCouter = 0;
                comparatorIndex = descriptorIndex;

                while(comparatorIndex < representativeCandidate.size())
                {
                    // Check if descriptors are equal
                    if( representativeCandidate.at(descriptorIndex)
                        .equals(representativeCandidate.at(comparatorIndex)))
                    {
                        // If so increase descriptor counter
                        ++descriptorCouter;
                        
                        // Check if it's the same value
                        if(descriptorIndex != comparatorIndex)
                        {
                            // If not then remove it from the list
                            representativeCandidate.removeAt(comparatorIndex);
                            continue;
                        }
                    }

                    ++comparatorIndex;
                }

                // Check if it occurs on list number of times given by threshold
                if(descriptorCouter >= requiredOccurenceNumber)
                {
                    // If it should be in representative, then increase descriptor index
                    ++descriptorIndex;
                }
                else
                {
                    // If not then remove it from the list
                    representativeCandidate.removeAt(descriptorIndex);
                }

            }

            // Save these descriptors to representative
            foreach(const descriptor desc, representativeCandidate)
            {
                // If attribute is not in the hashtable add it with empty QStringList
                if(!representativeAttributesValues.keys().contains(desc.attribute))
                {
                    representativeAttributesValues.insert(desc.attribute, new QStringList());
                }

                // Add descriptors value to the appropriate list
                representativeAttributesValues.value(desc.attribute)->append(desc.value);
            }

            // Remove duplicate values from each QStringList of attribute
            foreach(QStringList* values, representativeAttributesValues)
            {
                values->removeDuplicates();
            }

        }

        virtual QHash<QString, QStringList*> getAttributesForSimilarityCount(int methodId)
        {
            if(methodId == CentroidLinkId)
                return representativeAttributesValues;

            return attributesValues;
        }

    protected:

        int clusterID;

        QStringList getRepresentativesAttributesList(int treshold)
        {
            /*
             *  If in *treshold* percent rules some attribute occurs
             *  then it should be used to represent this cluster.
             */

            QList<cluster*> objects = this->getObjects();
            int rulesContainingAttribute;
            QStringList result, attributesNames;

            attributesNames = attributes.keys();

            for(int i = 0; i < attributesNames.length(); ++i)
            {
                rulesContainingAttribute = 0;

                for(int j = 0; j < objects.length(); ++j)
                {
                    if(objects.at(j)->attributesValues.keys().contains(attributesNames.at(i)))
                        ++rulesContainingAttribute;
                }

                if(qreal(100*rulesContainingAttribute/this->size()) >= treshold)
                    result.append(attributesNames.at(i));
            }

            return result;
        }

        QString getAttributesAverageValue(QString atrName)
        {
            //  Get modal for symbolic and average for numeric attributes
            QString result;

            if(attributes.value(atrName)->type == "symbolic")
                result = getAttributesModal(atrName);
            else
                result = getNumericAttributesAverage(atrName);

            return result;
        }

        QString getAttributesModal(QString atrName)
        {
            QList<cluster*> objects = this->getObjects();
            QStringList values, uniqueValues;
            QString mostCommonValue = "";

            // For each object
            foreach(const cluster* object, objects)
            {
                // If it contains given attribute add it's values to the lists
                if(object->attributesValues.keys().contains(atrName))
                {
                    foreach(const QString value, *(object->attributesValues.value(atrName)))
                    {
                        values.append(value);
                        uniqueValues.append(value);
                    }
                }
            }

            // Remove duplicates to get unique values
            uniqueValues.removeDuplicates();

            // For each unique value
            foreach(const QString value, uniqueValues)
            {
                // If most common value occurs less often than current value
                if(values.count(mostCommonValue) < values.count(value))
                {
                    // Make current value most common value
                    mostCommonValue = value;
                }
            }

            return mostCommonValue;
        }

        QString getNumericAttributesAverage(QString atrName)
        {
            qreal result = 0;
            int numberOfObjectsWithGivenAttribute = 0;
            QList<cluster*> objects = this->getObjects();

            // For each object within cluster
            foreach(const cluster* object, objects)
            {
                // Check if given object contains examined attribute
                if(object->attributesValues.keys().contains(atrName))
                {
                   // If so add it's average value to the result

                   qreal averageValue = 0.0;
                   ++numberOfObjectsWithGivenAttribute;

                   foreach(const QString value, *(object->attributesValues.value(atrName)))
                   {
                       averageValue += value.toDouble();
                   }

                   result += averageValue / object->attributesValues.value(atrName)->size();
                }
            }

            result /= numberOfObjectsWithGivenAttribute;

            return QString::number(result);
        }
};

struct ruleCluster : cluster
{
    /*
     *  This struct represents RSES generated rules clusters.
     */

    public:

        //Members
        ruleCluster()
        {
            support = 0;
        }

        ruleCluster(int id) : cluster(id)
        {
            support = 0;
        }

        int support;

        QSet<QString> decisionAttributes;
        QSet<QString> premiseAttributes;

        //Methods

        QString rule()
        {
            // If clusters size is higher than 1, then it's not a rule.
            // Empty QString should be then returned.
            if(this->size() != 1)
                return "";

            QString rule = "";

            // For each permise attribute
            foreach(const QString attribute, premiseAttributes)
            {
                // Check if rule contains this attribute
                if(this->attributesValues.keys().contains(attribute))
                {
                    // If so add it's values to the rule as distinct descriptors
                    foreach(const QString value, *(this->attributesValues.value(attribute)))
                    {
                        rule += "(" + attribute + "=" + value + ")&";
                    }
                }
            }

            // Remove descriptors' separator at the end
            rule.remove(rule.length()-1,1);

            // Add rules parts separator
            rule += "=>";

            // For each decision attribute
            foreach(const QString attribute, decisionAttributes)
            {
                // Check if rule contains this attribute
                if(this->attributesValues.contains(attribute))
                {
                    // If so add it's values to the rule
                    foreach(const QString value, *(this->attributesValues.value(attribute)))
                    {
                        rule += "(" + attribute + "=" + value + ")&";
                    }
                }
            }

            // Remove descriptors' separator at the end
            rule.remove(rule.length()-1,1);

            return rule;
        }

        QString representative()
        {
            // If representative is empty return empty QString
            if(representativeAttributesValues.size() == 0)
                return "";

            QString representative = "";

            // For each permise attribute
            foreach(const QString attribute, premiseAttributes)
            {
                // Check if rule contains this attribute
                if(this->representativeAttributesValues.keys().contains(attribute))
                {
                    // If so add it's values to the rule as distinct descriptors
                    foreach(const QString value, *(this->representativeAttributesValues.value(attribute)))
                    {
                        representative += "(" + attribute + "=" + value + ")&";
                    }
                }
            }

            // Remove descriptors' separator at the end
            representative.remove(representative.length()-1,1);

            if(decisionAttributes.size() == 0) return representative;

            // Add rules parts separator
            representative += "=>";

            // For each decision attribute
            foreach(const QString attribute, decisionAttributes)
            {
                // Check if rule contains this attribute
                if(this->representativeAttributesValues.contains(attribute))
                {
                    // If so add it's values to the rule
                    foreach(const QString value, *(this->representativeAttributesValues.value(attribute)))
                    {
                        representative += "(" + attribute + "=" + value + ")&";
                    }
                }
            }

            // Remove descriptors' separator at the end
            representative.remove(representative.length()-1,1);

            return representative;
        }

        QString getMostCommonDecision()
        {
            QStringList decisions, uniqueDecisions;
            int mostCommonDecisionIdx = 0, mostCommonDecisionOccurences;

            decisions = this->getClustersDecisionsList();

            uniqueDecisions = decisions;
            uniqueDecisions.removeDuplicates();

            mostCommonDecisionOccurences = decisions.count(uniqueDecisions.at(0));

            for(int i = 1; i < uniqueDecisions.length(); ++i)
            {
                if(decisions.count(uniqueDecisions.at(i)) > mostCommonDecisionOccurences)
                {
                    mostCommonDecisionIdx = i;
                    mostCommonDecisionOccurences = decisions.count(uniqueDecisions.at(i));
                }
            }

            return uniqueDecisions.at(mostCommonDecisionIdx);
        }

        QString getLeastCommonDecision()
        {
            QStringList decisions, uniqueDecisions;
            int leastCommonDecisionIdx = 0, leastCommonDecisionOccurences;

            decisions = this->getClustersDecisionsList();

            uniqueDecisions = decisions;
            uniqueDecisions.removeDuplicates();

            leastCommonDecisionOccurences = decisions.count(uniqueDecisions.at(0));

            for(int i = 1; i < uniqueDecisions.length(); ++i)
            {
                if(decisions.count(uniqueDecisions.at(i)) < leastCommonDecisionOccurences)
                {
                    leastCommonDecisionIdx = i;
                    leastCommonDecisionOccurences = decisions.count(uniqueDecisions.at(i));
                }
            }

            return uniqueDecisions.at(leastCommonDecisionIdx);
        }

        QString getLongestRule()
        {
            QList<cluster*> rules = this->getObjects();

            QString result = static_cast<ruleCluster*>(rules[0])->rule();
            int longestRuleLength = static_cast<ruleCluster*>(rules[0])->size();;

            for(int i = 1; i < rules.length(); ++i)
            {
                if(static_cast<ruleCluster*>(rules[i])->getRulesLength() > longestRuleLength)
                {
                    longestRuleLength = static_cast<ruleCluster*>(rules[i])->size();
                    result = static_cast<ruleCluster*>(rules[i])->rule();
                }
            }

            return result;
        }

        QString getShortestRule()
        {
            QList<cluster*> rules = this->getObjects();

            QString result = static_cast<ruleCluster*>(rules[0])->rule();
            int shortestRuleLength = static_cast<ruleCluster*>(rules[0])->size();

            for(int i = 1; i < rules.length(); ++i)
            {
                if(static_cast<ruleCluster*>(rules[i])->getRulesLength() < shortestRuleLength)
                {
                    shortestRuleLength = static_cast<ruleCluster*>(rules[i])->size();
                    result = static_cast<ruleCluster*>(rules[i])->rule();
                }
            }

            return result;
        }

        void setDecisionGroupingInfo(bool areDecisionsGrouped)
        {
            this->areDecisionsGrouped = areDecisionsGrouped;
        }

        QHash<QString, QStringList*> getAttributesForSimilarityCount(int methodId)
        {
            QHash<QString, QStringList*> result;
            QSet<QString> attributesToExclude;

            if(methodId == CentroidLinkId)
              result = representativeAttributesValues;
            else result = attributesValues;

            if(!areDecisionsGrouped){attributesToExclude = decisionAttributes;}
            else                    {attributesToExclude = premiseAttributes;}

            // Remove attributes from other rules part
            foreach(const QString attribute, attributesToExclude)
                result.remove(attribute);

            return result;
        }

        QSet<QString> getDescriptors(int part)
        {
          QSet<QString> result;

          if(this->size() > 1) return result;

          QSet<QString> attributesOccured;

          switch (part)
          {
            case PREMISES:
              attributesOccured = premiseAttributes;
              break;
            case DECISIONS:
              attributesOccured = decisionAttributes;
            case BOTH:
            default:
              attributesOccured = premiseAttributes + decisionAttributes;
              break;
          }

          for(QString atrName : attributesOccured)
          {
            for(QString atrValue : *attributesValues[atrName])
            {
              result << atrName + "=" + atrValue;
            }
          }

          return result;
        }


    private:

        bool areDecisionsGrouped;

        int getRulesLength()
        {
            if(this->size() > 1)
                return 0;

            return this->premiseAttributes.size();
        }

        QStringList getClustersDecisionsList()
        {
            QList<cluster*> rules = this->getObjects();
            QStringList decisions;

            // For each rule
            foreach(cluster* object, rules)
            {
                ruleCluster* rule = static_cast<ruleCluster*>(object);

                // For each decisional attribute
                foreach(const QString attribute, rule->decisionAttributes)
                {
                    // For each value of this attribute
                    foreach(const QString value, *(rule->attributesValues.value(attribute)))
                    {
                        // Add it's decision to the list
                        decisions.append(QString("(" + attribute + "=" + value + ")"));
                    }
                }
            }

            // Remove duplicates from decisions list
            decisions.removeDuplicates();

            return decisions;
        }

};

typedef std::shared_ptr<cluster> cluster_ptr;

#endif // CLUSTERS
