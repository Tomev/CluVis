#ifndef CLUSTERINTERFERENCER_H
#define CLUSTERINTERFERENCER_H

#include "interferencer.h"

#include <unordered_set>
#include <string>
#include <vector>

enum interferenceType
{
  GREEDY      = 0,
  EXHAUSTIVE  = 1
};

class clusterInterferencer : public interferencer
{
  public:

    clusterInterferencer();

    int generateRandomFactsBase(QString path, int desiredNumberOfFacts);
    int loadFactsFromPath(QString path);
    int interfere();

    std::string getInterferenceType();
    void setInterferenceType(int newInterferenceType);

    int fillAvailableRuleIndexes();
    int canTargetBeAchieved();
    int wasTargetAchieved();
    int wasRuleFired();
    bool wasMostImportantRuleFired();

    void setGroupingThread(groupingThread* newGrpThread);

    int numberOfClustersSearched = 0;
    int numberOfRulesFired = 0;

    int getNumberOfRulesThatCanBeFired();
    int getNumberOfFacts();
    int getNumberOfNewFacts();
    int getInitialNumberOfFacts();
    int getNumberOfIterations();

    double getInterferenceTime();

  protected:

    groupingThread* grpThread;

    int interferenceType = 0;
    double interferenceTime = 0;
    int initialNumberOfFacts = 0;
    int numberOfIterations = 0;

    std::unordered_set<std::string> rulesFiredDuringInterference;
    std::vector<int> orderedClustersIndexesForExhaustiveSearch;

    int initializeInterference();
    int interfereGreedy();
    int interfereExhaustive();
      int fillOrderOfClustersToSearchExhaustively(ruleCluster *factRule);
      ruleCluster* exhaustivelySearchForRuleToFire(ruleCluster *factRule);
        ruleCluster* exhaustivelySearchForRuleToFireInCluster(cluster *clusterToSearch, ruleCluster *factRule);
          bool wasRuleFiredDuringInterference(std::string clusterName);
      int fireRule(ruleCluster* ruleToFire);

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
