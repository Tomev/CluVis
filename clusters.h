#ifndef CLUSTERS
#define CLUSTERS

#include <QString>
#include <QStringList>
#include <QSet>
#include <QDebug>

#include "enum_interclustersimilaritymeasures.h"

#include "attributedata.h"

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

        virtual QHash<QString, QString> getAttributesForSimilarityCount(int methodId)
        {
        if(methodId == CentroidLinkId)
            return representativeAttributesValues;

        return attributesValues;
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
    //Members

    //TODO: Change to QHash
    QString rule, longestRule, shortestRule;

    int support;

    bool areDecisionsGrouped;

    QSet<QString> decisionAttributes;
    QSet<QString> premiseAttributes;

    //Methods
        //Constructors

    ruleCluster(int id = 0, QString r = "")
    {
        clusterID = id;

        rule =  longestRule = shortestRule = r;
    }

        //Operators

    inline ruleCluster operator = (ruleCluster c)
    {
        clusterID = c.clusterID;

        rule = c.rule;
        longestRule = c.longestRule;
        shortestRule = c.shortestRule;

        leftNode = c.leftNode;
        rightNode = c.rightNode;

        decisionAttributes = c.decisionAttributes;
        premiseAttributes = c.premiseAttributes;

        support = c.support;

        return c;
    }

        //Other

    QString getMostCommonDecision()
    {
        QString result = "Most common decision";

        return result;
    }

    QString getLeastCommonDecision()
    {
        QString result = "Least common decision";

        return result;
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
};

#endif // CLUSTERS

