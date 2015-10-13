#ifndef CLUSTERS
#define CLUSTERS

#include <QString>
#include <QStringList>
#include <QSet>

struct cluster
{
    //Members

    int clusterID = 0;
    qreal dispersion = 0;

    //Methods

    int size(){return 0;}
    bool hasBothNodes(){return false;}
    QString getId() {return "";}
    QString getClusterRepresentativeString() {return "";}
};

struct ruleCluster : cluster
{
    //Members

    QString rule = "";
    QString longestRule = "";
    QString shortestRule = "";
    QString representative = "Cluster Representative";

    int support = 0;

    QSet<QString> decisionAttributes;
    QSet<QString> conclusionAttributes;

    ruleCluster* leftNode = NULL;
    ruleCluster* rightNode = NULL;

    //Methods
        //Constructors

    ruleCluster()
    {    }

    ruleCluster(QString rule)
    {
        this->rule = rule;
    }

        //Operator

    inline ruleCluster operator=(ruleCluster c)
    {
        clusterID = c.clusterID;

        rule = c.rule;
        longestRule = c.longestRule;
        shortestRule = c.shortestRule;

        leftNode = c.leftNode;
        rightNode = c.rightNode;

        decisionAttributes = c.decisionAttributes;
        conclusionAttributes = c.conclusionAttributes;

        representative = c.representative;

        support = c.support;

        return c;
    }

        //Other

    int size()
    {
        if(rule!="")
            return 1;
        else
            return this->leftNode->size() + this->rightNode->size();
    }

    bool hasBothNodes()
    {
        if(leftNode == NULL)
            return false;
        if(rightNode == NULL)
            return false;

        return true;
    }

    QString getId()
    {
        QString result = "";

        if(this->size()==1)
            result += "R";
        else
            result += "J";

        result += this->clusterID+1;

        return result;
    }

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
            return leftNode->getRules() + rightNode->getRules();
        else
            return QStringList(rule);
    }

};

#endif // CLUSTERS

