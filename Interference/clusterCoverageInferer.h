#ifndef CLUSTERCOVERAGEINFERER_H
#define CLUSTERCOVERAGEINFERER_H

#include "interferencer.h"

class clusterCoverageInferer : public interferencer
{
  public:
    clusterCoverageInferer();

    int loadFactsFromPath(QString path);

    void infere(std::vector<std::shared_ptr<cluster> > clusters);


  protected:
    double _inferenceTime = -1.0;
    double _factsBasePercent = 100;
    double _threshold = 1e-5;

    int _iterationsNumber = 0;
    int _numberOfComparisons = 0;
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

    std::string getInterferenceType() = 0;
    void setInterferenceType(int newInterferentionType) = 0;
    double getInterferenceTime() = 0;
    void getInitialFacts();
    int getNumberOfNewFacts();




};

enum coverageInfererType{
  GREEDY = 0,
  EXHAUSTIVE = 1
};

#endif // CLUSTERCOVERAGEINFERER_H
