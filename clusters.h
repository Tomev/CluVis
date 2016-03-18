#ifndef CLUSTERS
#define CLUSTERS

#include <QString>
#include <QStringList>
#include <QSet>

#include "enum_interclustersimilaritymeasures.h"

struct cluster
{
    cluster(int id = 0)
    {
        clusterID = id;
    }

    //Members
    QHash<QString, QString> attributesValues;
    QHash<QString, QString> representativeAttributesValues;

    int clusterID;
    qreal dispersion;

    QString representative;

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
        if(attributesValues.empty())
        {
            QList<cluster*> result;
            result.append(this);
            return result;
        }
        else
            return this->leftNode->getObjects() + this->rightNode->getObjects();
    }

    QHash<QString, QString> getAttributesForSimilarityCount(int methodId)
    {
        if(methodId == CentroidLinkId)
            return representativeAttributesValues;

        return attributesValues;
    }

    QString getClusterRepresentativeString() {return "";}

    QString toString(){return "To string.";}
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

        rule =  longestRule = shortestRule =
                representative = r;
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

        representative = c.representative;

        support = c.support;

        return c;
    }

        //Other

    QString getMostCommonDecision()
    {
        QString result = "Most common decision";

        QStringList rules = getRules();
        QStringList decisions;
        QList<int> decisionOccurences;

        for(int i = 0; i < rules.length(); i++)
        {
            QString decision = rules[i].split("=>")[1];

            if(decisions.contains(decision))
            {
                decisionOccurences[decisions.indexOf(decision)] ++;
            }
            else
            {
                decisions.append(decision);
                decisionOccurences.append(1);
            }
        }

        int highestValue = 1;
        int highestValueIndex = 0;

        for(int i = 0; i < decisionOccurences.length(); i++)
        {
            if(decisionOccurences[i] > highestValue)
            {
                highestValue = decisionOccurences[i];
                highestValueIndex = i;
            }
        }

        result = decisions[highestValueIndex];

        return result;
    }

    QString getLeastCommonDecision()
    {
        QString result = "Least common decision";

        QStringList rules = getRules();
        QStringList decisions;
        QList<int> decisionOccurences;

        for(int i = 0; i < rules.length(); i++)
        {
            QString decision = rules[i].split("=>")[1];

            if(decisions.contains(decision))
            {
                decisionOccurences[decisions.indexOf(decision)] ++;
            }
            else
            {
                decisions.append(decision);
                decisionOccurences.append(1);
            }
        }

        int lowestValue = 1;
        int lowestValueIndex = 0;

        for(int i = 0; i < decisionOccurences.length(); i++)
        {
            if(decisionOccurences[i] < lowestValue)
            {
                lowestValue = decisionOccurences[i];
                lowestValueIndex = i;
            }
        }

        result = decisions[lowestValueIndex];

        return result;
    }

    QString getClusterRepresentativeString()
    {        
        return this->representative;
    }

    QStringList getRules()
    {
        if(rule=="")
            return ((ruleCluster*) leftNode)->getRules() + ((ruleCluster*) rightNode)->getRules();

        return QStringList(rule);
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
            result.erase(std::find(result.begin(), result.end(), *i));

        return result;
    }

};

#endif // CLUSTERS

