#ifndef CLUSTERCOVERAGEINFERER_H
#define CLUSTERCOVERAGEINFERER_H

#include "interferencer.h"

class clusterCoverageInferer : public interferencer
{
  public:
    clusterCoverageInferer();

    int _numberOfComparisons = 0;

    int loadFactsFromPath(QString path);
    void infere(std::vector<std::shared_ptr<cluster> > clusters);
    std::string getInterferenceType();
    void setInterferenceType(int newInterferentionType);
    double getInterferenceTime();
    void getInitialFacts();
    int getNumberOfNewFacts();
    int getNumberOfInitialFacts();
    int getNumberOfIterations();
    int numberOfRulesFired();

  protected:
    double _inferenceTime = -1.0;
    double _factsBasePercent = 100;
    double _threshold = 1e-5;

    int _iterationsNumber = 0;

    int _numberOfFiredRules = 0;
    int _initialNumberOfFacts = 0;

    bool _greedyInference = false;
    bool _wasRuleFiredThisIteration = false;

    QStringList _clustersToOmit = {};

    void resetInferer();
    bool canInferenceBePerformed(const std::vector<std::shared_ptr<cluster> > &clusters);
    void tryToFireMostAccurateRule(
        std::vector<std::shared_ptr<cluster> > clusters);
    int findIndexOfClusterMostProbableToContainFireableRule(
            const std::vector<std::shared_ptr<cluster> > &clusters);
    int findIndexOfBestCoveredCluster(
            const std::vector<std::shared_ptr<cluster> > &clusters);
    double findClusterCoverageValue(const std::shared_ptr<cluster> &c);
    int findIndexOfClusterWithMostUnfiredRules(
            const std::vector<std::shared_ptr<cluster> > &clusters);
    int countNumberOfUnfiredRulesInCluster(const std::shared_ptr<cluster> &c);
    cluster_ptr findRuleToFireInCluster(const std::shared_ptr<cluster> &c);
    int getNumberOfAvailableSubclusters(const std::shared_ptr<cluster> &c);
    std::vector<cluster_ptr> getAvailableSubclusters(
            const std::shared_ptr<cluster> &c);
    void tryToFire(const std::shared_ptr<cluster> &rule);
};

enum coverageInfererType{
  CC_GREEDY = 0,
  CC_EXHAUSTIVE = 1
};

#endif // CLUSTERCOVERAGEINFERER_H
