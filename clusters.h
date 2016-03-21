#ifndef CLUSTERS
#define CLUSTERS

#include <QString>
#include <QStringList>
#include <QSet>

#include "attributedata.h"

#include "enum_interclustersimilaritymeasures.h"



struct cluster
{
    public:

        cluster(int id = 0)
    {
        this->clusterID = id;
    }

    //Members
        QHash<QString, QString> attributesValues;
        QHash<QString, QString> representativeAttributesValues;

        // Cosists of attributes that are present in cluster.
        QHash<QString, attributeData*> attributes;

        /*
         * Compactness requires knowledge about similarity measures
         * hence should be filled in grouping thread.
         */
        qreal compactness;

        cluster* leftNode = NULL;
        cluster* rightNode = NULL;

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
        if(leftNode == NULL || rightNode == NULL)
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

        void fillRepresentativesAttributesValues(int treshold)
        {
            QStringList repAttributes = getRepresentativesAttributesList(treshold);
            QString atrName;

            for(int i = 0; i < repAttributes.length(); ++i)
            {
                atrName = repAttributes.at(i);
                representativeAttributesValues.insert(atrName, getAttributesAverageValue(atrName));
            }
        }

        virtual QHash<QString, QString> getAttributesForSimilarityCount(int methodId)
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
             *  then it should be used to represent the this cluster.
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
            int mostCommonValueIdx = 0, mostCommonValueOccurences = 0, currentValueOccurences;

            for(int i = 0; i < objects.length(); ++i)
            {
                if(objects.at(i)->attributesValues.keys().contains(atrName))
                {
                    values.append(objects.at(i)->attributesValues.value(atrName));
                    uniqueValues.append(objects.at(i)->attributesValues.value(atrName));
                }
            }

            uniqueValues.removeDuplicates();

            for(int i = 0; i < uniqueValues.length(); ++i)
            {
                currentValueOccurences = values.count(uniqueValues.at(i));

                if(currentValueOccurences > mostCommonValueOccurences)
                {
                    mostCommonValueIdx = i;
                    mostCommonValueOccurences = currentValueOccurences;
                }
            }

            return uniqueValues.at(mostCommonValueIdx);
        }

        QString getNumericAttributesAverage(QString atrName)
        {
            qreal result = 0;
            int denumerator = 0;

            QList<cluster*> objects = this->getObjects();

            for(int i = 0; i < objects.length(); ++i)
            {
                if(objects.at(i)->attributesValues.keys().contains(atrName))
                {
                   result += objects.at(i)->attributesValues.value(atrName).toDouble();
                   ++denumerator;
                }
            }

            result /= denumerator;

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

            //Other

        QString rule()
        {
            if(this->size() != 1)
                return "";

            QString rule = "";
            QStringList ruleAttributes = attributesValues.keys();

            for(QSet<QString>::iterator i = premiseAttributes.begin(); i != premiseAttributes.end(); ++i)
            {
                if(ruleAttributes.contains(*i))
                    rule += "(" + *i + "=" + attributesValues.value(*i) + ")&";
            }

            rule.remove(rule.length()-1,1);

            rule += "=>";

            for(QSet<QString>::iterator i = decisionAttributes.begin(); i != decisionAttributes.end(); ++i)
            {
                if(ruleAttributes.contains(*i))
                    rule += "(" + *i + "=" + attributesValues.value(*i) + ")&";
            }

            rule.remove(rule.length()-1,1);

            return rule;
        }

        QString representative()
        {
            if(representativeAttributesValues.size() == 0)
                return "";

            QString representative = "";
            QStringList representativeAttributes = representativeAttributesValues.keys();

            for(QSet<QString>::iterator i = premiseAttributes.begin(); i != premiseAttributes.end(); ++i)
            {
                if(representativeAttributes.contains(*i))
                    representative += "(" + *i + "=" + representativeAttributesValues.value(*i) + ")&";
            }

            representative.remove(representative.length()-1,1);
            representative += "=>";

            for(QSet<QString>::iterator i = decisionAttributes.begin(); i != decisionAttributes.end(); ++i)
            {
                if(representativeAttributes.contains(*i))
                    representative += "(" + *i + "=" + representativeAttributesValues.value(*i) + ")&";
            }

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

        QHash<QString, QString> getAttributesForSimilarityCount(int methodId)
        {
            QHash<QString, QString> result;
            QSet<QString> attributesToExclude;

            if(methodId == CentroidLinkId){result = representativeAttributesValues;}
            else{result = attributesValues;}

            if(!areDecisionsGrouped){attributesToExclude = decisionAttributes;}
            else                    {attributesToExclude = premiseAttributes;}

            for(QSet<QString>::iterator i = attributesToExclude.begin(); i != attributesToExclude.end(); ++i)
                result.remove(*i);

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
            QString decision;
            QStringList decisions;

            for(int i = 0; i < rules.length(); ++i)
            {
                ruleCluster* ruleClu = static_cast<ruleCluster*>(rules[i]);
                decision = "";

                for(    QSet<QString>::iterator j = ruleClu->decisionAttributes.begin();
                        j != ruleClu->decisionAttributes.end(); ++j)
                {
                    decision += QString("(" + *j + "=" + ruleClu->attributesValues.value(*j) + ")&");
                }

                decision.remove(decision.length()-1,1);
                decisions.append(decision);
            }

            return decisions;
        }

};

#endif // CLUSTERS
