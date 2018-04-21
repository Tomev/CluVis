#ifndef INTERFERENCER_H
#define INTERFERENCER_H

#include <QHash>

#include "../Clustering/groupingthread.h"

class interferencer
{
  public:

    QHash<QString, QSet<QString>> facts;
    QHash<QString, QSet<QString>> target;
    QStringList allFacts;

    int targetAchiveable;
    int targetAchieved;

    cluster* mostSimilarRule;

    int factsBasePercent;

    bool zeroRepresentativeOccured = false;
    long zeroRepresentativeOccurenceNumber = 0;

    QStringList availableRuleIndexes;

    bool canRuleBeFired(ruleCluster *c);

    virtual std::string getInterferenceType() = 0;
    virtual void setInterferenceType(int newInterferentionType) = 0;
    virtual double getInterferenceTime() = 0;

};

#endif // INTERFERENCER_H
