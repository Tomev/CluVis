#ifndef CLUSTERINTERFERENCER_H
#define CLUSTERINTERFERENCER_H

#include "interferencer.h"

class clusterInterferencer : public interferencer
{
  public:

    clusterInterferencer();

    int generateRandomFactsBase(QString path, int desiredNumberOfFacts);
    int loadFactsFromPath(QString path);
    int interfere();

    int fillAvailableRuleIndexes();
    int canTargetBeAchieved();
    int wasTargetAchieved();
    int wasRuleFired();

    void setGroupingThread(groupingThread* newGrpThread);

    int numberOfClustersSearched = 0;
    int numberOfRulesFired = 0;

    int getNumberOfRulesThatCanBeFired();
    int getNumberOfFacts();
    int getNumberOfNewFacts();


  protected:

    groupingThread* grpThread;

    ruleCluster createFactRule();
    QVector<cluster*> fireableRules;

    int fillFacts(int basePercent);

    int findMostSimiliarClusterToFactRule(cluster* factRule);
    int findRulesToFireInCluster(cluster* fc, cluster* c);

    int countNumberOfPossibleFacts();
    int saveAllFactsToBase(QString path);
    int saveRandomNFactsToBase(int numOfFacts, QString path);
      int insertFactsToBase(QSet<QString> *factsBase, QString path);

    int findMostSimilarRule(cluster *fc, cluster *c);

    bool canRuleBeFired(ruleCluster *c);
};

#endif // CLUSTERINTERFERENCER_H
