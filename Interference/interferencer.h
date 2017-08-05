#ifndef INTERFERENCER_H
#define INTERFERENCER_H

#include <QHash>

#include "../Clustering/groupingthread.h"

class interferencer
{
  public:

    interferencer();

    void generateRandomFactsBase();
    int loadFactsFromPath(QString path);
    int interfere();

    void setGroupingThread(groupingThread* newGrpThread);

    int numberOfClustersSearched = 0;
    int numberOfRulesFired = 0;

  private:

    QHash<QString, QString> facts;
    groupingThread* grpThread;

    ruleCluster createFactRule();

    int findMostSimiliarClusterToFactRule(cluster* factRule);
    cluster *findRuleToFireInCluster(cluster* fc, cluster* c);

    bool canRuleBeFired(ruleCluster *c);

};

#endif // INTERFERENCER_H
