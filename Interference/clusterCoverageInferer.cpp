#include "clusterCoverageInferer.h"

/** clusterCoverageInferer::clusterCoverageInferer
 * @brief Basic constructor.
 */
clusterCoverageInferer::clusterCoverageInferer() {}

/**  clusterCoverageInferer::loadFactsFromPath
 *
 * @brief Fills inferer facts list.
 *
 * @return 0 if everything went ok. There are no other options for now.
 */
int clusterCoverageInferer::loadFactsFromPath(QString path)
{
  allFacts.clear();
  target.clear();

  QFile factsBase(path);
  QTextStream stream(&factsBase);
  QString line;

  QStringList tar;

  if(factsBase.open(QIODevice::ReadWrite))
  {
    while(!stream.atEnd())
    {
      line = stream.readLine();
      if(line.startsWith("Target:")) break;
      if(line.startsWith("#")) continue;
      allFacts.push_back(line);
    }

    while (!stream.atEnd())
    {
      line = stream.readLine();
      if(line.startsWith("#")) continue;
      tar = line.split("=");
      target[tar.at(0)] += tar.at(1);
    }
  }

  factsBase.close();

  return 0;
}

void clusterCoverageInferer::infere(
    std::vector<std::shared_ptr<cluster>> clusters)
{
  // First, reset the inferer.
  resetInferer();

  // Then get (initial) facts basing on used base percent.
  getInitialFacts();

  // Check if inference can be performed
  if(!canInferenceBePerformed(clusters)) return;

  // And then start infering
  clock_t start = clock();

  do {
    ++_iterationsNumber;
    _wasRuleFiredThisIteration = false;
    tryToFireMostAccurateRule(clusters);
  } while(!_greedyInference && _wasRuleFiredThisIteration);

  _inferenceTime = (clock() - start) / (double) CLOCKS_PER_SEC;
}

/** clusterCoverageInferer::resetInferer
 * @brief Resets inferer members to initial state before inference.
 */
void clusterCoverageInferer::resetInferer()
{
  _iterationsNumber = 0;
  _numberOfComparisons = 0;
  newFacts.clear();
  zeroRepresentativeOccured = false;
  _clustersToOmit.clear();
}

/** clusterCoverageInferer::canInferenceBePerformed
 * @brief Checks if it's possible to perform inference.
 * @param clusters -- potential hierarchical structure.
 * @return Information whether inference can be performed.
 */
bool clusterCoverageInferer::canInferenceBePerformed(
    const std::vector<std::shared_ptr<cluster> > &clusters)
{
  if(clusters.size() == 0) return false;
  if(initialFacts.size() == 0) return false;
  return true;
}

/** clusterCoverageInferer::tryToFireMostAccurateRule
 * @brief Tries to fire most accurate rule, where accurate means that
 *  representative path  had best coverage.
 *
 * Note that it's possible to have no rule fired, if the rule at the end of
 * the path is not 100% covered by facts.
 *
 * @param clusters A hierarchical structure in which rule search will be
 *  performed.
 */
void clusterCoverageInferer::tryToFireMostAccurateRule(
    std::vector<std::shared_ptr<cluster> > clusters)
{
  int clusterIndex =
      findIndexOfClusterMostProbableToContainFireableRule(clusters);

  if(clusterIndex == -1 ||
     countNumberOfUnfiredRulesInCluster(clusters[clusterIndex]) < 1) return;

  auto rule = findRuleToFireInCluster(clusters[clusterIndex]);

  tryToFire(rule);
}

/** clusterCoverageInferer::findIndexOfClusterMostProbableToContainFireableRule
 * @brief Finds the index of cluster that is most probable to contain a rule
 * that will be fired this iteration.
 *
 * @param clusters -- clusters to consider during search.
 * @return Index of the cluster that is most probable to contain fireable rule.
 */
int clusterCoverageInferer::findIndexOfClusterMostProbableToContainFireableRule(
    const std::vector<std::shared_ptr<cluster> > &clusters)
{
  int indexOfClusterWithHighestProbability =
      findIndexOfBestCoveredCluster(clusters);

  // Fallback procedure in case zero representatives are present.
  double bestCoveredClusterCoverage =
     findClusterCoverageValue(clusters[indexOfClusterWithHighestProbability]);

  if(bestCoveredClusterCoverage < _threshold)
      indexOfClusterWithHighestProbability =
              findIndexOfClusterWithMostUnfiredRules(clusters);

  return indexOfClusterWithHighestProbability;
}

/** clusterCoverageInferer::findIndexOfBestCoveredCluster
 * @brief Selects index of a cluster which representative is covered best.
 *
 * @param clusters -- clusters to consider during search.
 * @return Index of cluster which representative was best covered.
 */
int clusterCoverageInferer::findIndexOfBestCoveredCluster(
    const std::vector<std::shared_ptr<cluster> > &clusters)
{
  int bestCoveredClusterIndex = -1;
  double highestCoverageValue = 0;

  for(int i = 0; i < clusters.size(); ++ i){
      if(_clustersToOmit.contains(clusters[i]->name())) continue;
      bestCoveredClusterIndex = i;
      highestCoverageValue = findClusterCoverageValue(clusters[i]);
      break;
  }

  for(int i = bestCoveredClusterIndex + 1; i < clusters.size(); ++i){
    if(_clustersToOmit.contains(clusters[i]->name())) continue;
    double currentClusterCoverageValue =
      findClusterCoverageValue(clusters[i]);

    if(currentClusterCoverageValue > highestCoverageValue){
      highestCoverageValue = currentClusterCoverageValue;
      bestCoveredClusterIndex = i;
    }
  }

  return bestCoveredClusterIndex;
}

/** clusterCoverageInferer::findClusterCoverageValue
 * @brief Counts to what extent facts covers clusters c representative.
 *
 * @param c - cluster for which coverage should be evaluated.
 * @return Coverage value of c.
 */
double clusterCoverageInferer::findClusterCoverageValue(
    const std::shared_ptr<cluster> &c)
{
  ++_numberOfComparisons;
  double coverage = 0;
  int numberOfRepresentativesDescriptors = 0;
  for(auto attribute :
      c->getAttributesForSimilarityCount(CentroidLinkId).keys()){

    if(!facts.keys().contains(attribute)){
        numberOfRepresentativesDescriptors +=
                c->representativeAttributesValues[attribute]->size();
        continue;
    }

    for(auto value : *(c->representativeAttributesValues[attribute])){
        ++numberOfRepresentativesDescriptors;
        if(facts[attribute].contains(value)) coverage += 1;
    }
  }

  return coverage / numberOfRepresentativesDescriptors;
}

/** clusterCoverageInferer::findIndexOfClusterWithMostUnfiredRules
 * @brief Returns index of cluster with most unfired rules. If there's no
 * clusters with unfired rules returns -1;
 *
 * @param clusters -- clusters to search.
 * @return Index of cluster with most unfired rules or -1 it there's no such
 * cluster.
 */
int clusterCoverageInferer::findIndexOfClusterWithMostUnfiredRules(
        const std::vector<std::shared_ptr<cluster> > &clusters)
{
  int indexOfClusterWithMostUnfiredRules = -1;
  int mostUnfiredRulesNumberInACluster = -1;

  for(int i = 0; i < clusters.size(); ++ i){
      if(_clustersToOmit.contains(clusters[i]->name())) continue;
      indexOfClusterWithMostUnfiredRules = i;
      mostUnfiredRulesNumberInACluster =
              countNumberOfUnfiredRulesInCluster(clusters[i]);
      break;
  }

  for(int i = 0; i < clusters.size(); ++ i){
    if(_clustersToOmit.contains(clusters[i]->name())) continue;

    int currentClustersNumberOfUnfiredRules =
      countNumberOfUnfiredRulesInCluster(clusters[i]);

    if(currentClustersNumberOfUnfiredRules > mostUnfiredRulesNumberInACluster)
    {
        indexOfClusterWithMostUnfiredRules = i;
        mostUnfiredRulesNumberInACluster = currentClustersNumberOfUnfiredRules;
    }
  }

  return indexOfClusterWithMostUnfiredRules;
}

/** clusterCoverageInferer::findNumberOfUnfiredRulesInCluster
 * @brief Counts number of unfired rules in c.
 * @param c -- cluster to determine number of unfired rules in.
 * @return Number of unfired rules in c.
 */
int clusterCoverageInferer::countNumberOfUnfiredRulesInCluster(
        const std::shared_ptr<cluster> &c)
{
    ++_numberOfComparisons;

    if(_clustersToOmit.contains(c->name())) return 0;

    auto rulesInC = c->getObjects();
    int numberOfUnfiredRulesInC = 0;

    for(auto rule : rulesInC){
        if(!_clustersToOmit.contains(rule->name())) ++numberOfUnfiredRulesInC;
    }

    if(numberOfUnfiredRulesInC == 0) _clustersToOmit.push_back(c->name());

    return numberOfUnfiredRulesInC;
}

/** clusterCoverageInferer::findRuleToFireInCluster
 * @brief Finds and returns best rule to fire.
 *
 * Criteria of deciding whether rule is the best are decided upon most covered
 * representatives path in hierarchical group represented by given cluster.
 *
 * @param c -- cluster to search for rule to fire
 * @return Shared pointer to the rule cluster.
 */
cluster_ptr clusterCoverageInferer::findRuleToFireInCluster(
        const std::shared_ptr<cluster> &c)
{
  auto availableSubclusters = getAvailableSubclusters(c);

  if(availableSubclusters.size() == 1)
    return findRuleToFireInCluster(availableSubclusters[0]);

  // It either have 1 or 2 available subclusters, thus this scenarion focuses
  // on 2 subclusters
  double firstSubclusterCoverage =
          findClusterCoverageValue(availableSubclusters[0]);
  double secondSubclusterCoverage =
          findClusterCoverageValue(availableSubclusters[1]);

  if(firstSubclusterCoverage > secondSubclusterCoverage)
    return findRuleToFireInCluster(availableSubclusters[0]);
  return findRuleToFireInCluster(availableSubclusters[1]);
}

/** clusterCoverageInferer::getAvailableSubclusters
 * @brief Finds available subclusters in given cluster.
 *
 * Subclusters are available if they were not fired.
 *
 * @param c -- cluster for which subclusters are checked for availability.
 * @return Vector of available subclusters.
 */
std::vector<cluster_ptr> clusterCoverageInferer::getAvailableSubclusters(
        const std::shared_ptr<cluster> &c)
{
  std::vector<cluster_ptr> availableClusters = {};

  for(auto node : {c->leftNode, c->rightNode}){
    if(!_clustersToOmit.contains(node->name())){
      if(countNumberOfUnfiredRulesInCluster(node) > 0)
        availableClusters.push_back(node);
    }
  }

  return availableClusters;
}

/** clusterCoverageInferer::tryToFire
 * @brief Try to fire specified rule.
 *
 * If rule can be fired then it's coverage is equal 1. In that case rules
 * decision values should be added to newFacts (and facts) and flag
 * _wasRuleFiredThisIteration should be set to true.
 *
 * @param rule -- rule that inferer will try to fire.
 */
void clusterCoverageInferer::tryToFire(const std::shared_ptr<cluster> &rule)
{
    if(1 - findClusterCoverageValue(rule) > _threshold) return;

    auto r = static_cast<ruleCluster*>(rule.get());

    for(QString decisionAttribute : r->decisionAttributes)
    {
      for(QString value : *(rule->attributesValues[decisionAttribute]))
      {
        facts[decisionAttribute].insert(value);
        newFacts[decisionAttribute].insert(value);
      }
    }

    _wasRuleFiredThisIteration = true;
    _clustersToOmit.append(rule->name());
}

/** clusterCoverageInferer::getInterferenceType
 * @brief Getter for inference type.
 *
 * @return std::string Inference type as std::string.
 */
std::string clusterCoverageInferer::getInterferenceType()
{
  if(_greedyInference) return "Coverage Greedy";
  return "Coverage Exhaustive";
}

/** clusterCoverageInferer::setInterferenceType
 * @brief Changes inferer mode.
 * @param newInterferentionType Type of inference
 */
void clusterCoverageInferer::setInterferenceType(int newInterferentionType)
{
  if(newInterferentionType == CC_GREEDY) _greedyInference = true;
  else _greedyInference = false;
}

/** clusterCoverageInferer::getInterferenceTime
 * @brief Getter for inference time.
 * @return Returns inference time in seconds as double.
 */
double clusterCoverageInferer::getInterferenceTime()
{
  return _inferenceTime;
}

/** classicalInterferencer::getInitialFacts
 *
 * In order for this method to work properly facts should be given in following
 * form: attribute=value. It'd then create facts map with the percent of facts
 * from allFacts vector.
 *
 * @brief Creating map of facts with given (by percentage) number of facts from
 * all facts.
 *
 * @return Unordered map with initial facts attributes and their values.
 */
void clusterCoverageInferer::getInitialFacts()
{
  _initialNumberOfFacts = ceil(allFacts.size() * factsBasePercent / 100.0);
  initialFacts.clear();

  QString delimiter = "=";
  QString attribute, value;

  for(int i = 0; i < _initialNumberOfFacts; ++i)
  {
    attribute = allFacts[i].split(delimiter)[0];
    value = allFacts[i].split(delimiter)[1];
    initialFacts[attribute].insert(value);
    facts[attribute].insert(value);
  }
}

/** clusterCoverageInferer::getNumberOfNewFacts
 * @brief Getter for number of new facts.
 * @return Number of new facts.
 */
int clusterCoverageInferer::getNumberOfNewFacts()
{
  long numberOfNewFacts = 0;

  for(QSet<QString> attributesValues : newFacts)
    numberOfNewFacts += attributesValues.size();

  return numberOfNewFacts;
}

/** clusterCoverageInferer::getNumberOfInitialFacts
 * @brief Getter for number of initial facts.
 * @return Number of initial facts.
 */
int clusterCoverageInferer::getNumberOfInitialFacts()
{
    long numberOfInitialFacts = 0;

    for(QSet<QString> attributesValues : initialFacts)
      numberOfInitialFacts += attributesValues.size();

    return numberOfInitialFacts;
}

/** clusterCoverageInferer::getNumberOfIterations
 * @brief Getter for number of iterations.
 * @return Number of iterations.
 */
int clusterCoverageInferer::getNumberOfIterations()
{
    return _iterationsNumber;
}

/** clusterCoverageInferer::numberOfRulesFired
 * @brief Getter for number of rules fired.
 * @return Number of rules fired.
 */
int clusterCoverageInferer::numberOfRulesFired()
{
   return _numberOfFiredRules;
}
