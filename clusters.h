#ifndef CLUSTERS
#define CLUSTERS

#include <QString>
#include <QStringList>
#include <QSet>

struct cluster
{
    cluster(int id = 0)
    {
        clusterID = id;
    }

    //Members

    int clusterID = 0;
    qreal dispersion = 0;

    cluster* leftNode = NULL;
    cluster* rightNode = NULL;

    //Methods

    int size()
    {
        if(hasBothNodes())
            return leftNode->size() + rightNode->size();

        return 1;
    }

    bool hasBothNodes()
    {
        if(leftNode == NULL || rightNode == NULL)
            return false;

        return true;
    }

    QString getId() {return "";}
    QString getClusterRepresentativeString() {return "";}
    QString toString(){return "To string.";}
};

struct ruleCluster : cluster
{
    //Members

    QString rule, longestRule, shortestRule, representative;

    int support = 0;

    QSet<QString> decisionAttributes;
    QSet<QString> conclusionAttributes;

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
        conclusionAttributes = c.conclusionAttributes;

        representative = c.representative;

        support = c.support;

        return c;
    }

        //Other

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
            return ((ruleCluster*) leftNode)->getRules() + ((ruleCluster*) rightNode)->getRules();

        return QStringList(rule);
    }

};

#endif // CLUSTERS

