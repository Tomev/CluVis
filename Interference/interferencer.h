#ifndef INTERFERENCER_H
#define INTERFERENCER_H

#include <QHash>

#include "../Clustering/groupingthread.h"

class interferencer
{
  public:

    interferencer();

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

    QHash<QString, QSet<QString>> facts;
    QStringList allFacts;
    QHash<QString, QSet<QString>> target;

    QStringList availableRuleIndexes;

    int targetAchiveable;
    int targetAchieved;

    cluster* mostSimilarRule;

    QList<int> factsBasePercents;

    bool zeroRepresentativeOccured = false;
    long zeroRepresentativeOccurenceNumber = 0;

  private:

    groupingThread* grpThread;

    ruleCluster createFactRule();
    QVector<cluster*> fireableRules;

    int fillFacts(int basePercent);

    int findMostSimiliarClusterToFactRule(cluster* factRule);
    int findRulesToFireInCluster(cluster* fc, cluster* c);

    bool canRuleBeFired(ruleCluster *c);

    int countNumberOfPossibleFacts();
    int saveAllFactsToBase(QString path);
    int saveRandomNFactsToBase(int numOfFacts, QString path);
      int insertFactsToBase(QSet<QString> *factsBase, QString path);

    int findMostSimilarRule(cluster *fc, cluster *c);

};

#endif // INTERFERENCER_H
