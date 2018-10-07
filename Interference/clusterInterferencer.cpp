#include "clusterInterferencer.h"

#include <QFile>
#include <QDebug>
#include <ctime>
#include <utility>

#include "../Clustering/clusters.h"

/* This class is diabolicaly ugly, but it was made in hurry, so I
 * don't really care that much. It shall be rewritten one day. */

clusterInterferencer::clusterInterferencer()
{
  grpThread = 0;
  factsBasePercent = 100;
}

void clusterInterferencer::setGroupingThread(groupingThread *newGrpThread)
{
  this->grpThread = newGrpThread;
}

int clusterInterferencer::getNumberOfRulesThatCanBeFired()
{
  if(availableRuleIndexes.at(0) == "-1") return 0;

  return this->availableRuleIndexes.size();
}

int clusterInterferencer::getNumberOfFacts()
{
  int result = 0 ;

  for(QString attribute : facts.keys())
  {
    result += facts[attribute].values().size();
  }

  return result;
}

int clusterInterferencer::getNumberOfNewFacts()
{
  long numberOfNewFacts = 0;

  for(QSet<QString> attributesValues : newFacts)
    numberOfNewFacts += attributesValues.size();

  return numberOfNewFacts;
}

int clusterInterferencer::getInitialNumberOfFacts()
{
  long initialNumberOfFacts = 0;

  for(QSet<QString> attributesValues : initialFacts)
    initialNumberOfFacts += attributesValues.size();

  return initialNumberOfFacts;
}

int clusterInterferencer::getNumberOfIterations()
{
  if(interferenceType == GREEDY)
    return 1;

  if(interferenceType == EXHAUSTIVE)
    return numberOfIterations;

  return -1;
}

double clusterInterferencer::getInterferenceTime()
{
  return interferenceTime;
}

std::string clusterInterferencer::whyWasntTargetConfirmed()
{
  if(wasTargetAchieved()) return "";

  std::string failureReasons = "";

  failureReasons +=
      wasRuleFired() ? "" : "rule wasnt fired; ";

  failureReasons +=
      target.keys().size() < 1 ? "target not set; " : "";

  failureReasons +=
      zeroRepresentativeOccured ? "zero representative occurence; " : "";

  failureReasons = failureReasons.empty() ? "other case" : failureReasons;

  return failureReasons;
}

int clusterInterferencer::interfereGreedy()
{
  //qDebug() << "Greedy stuff.";

  zeroRepresentativeOccured = false;

  //qDebug() << "Occurence";

  rulesFiredDuringInterference.clear();
  currentStep = 1;

  //qDebug() << "Filling facts.";

  //qDebug() << "Number of facts: " <<
  fillFacts(factsBasePercent);

  //qDebug() << "Filling available rules.";

  fillAvailableRuleIndexes();
  initiallyFireableRuleIndexes = availableRuleIndexes;
  if(availableRuleIndexes[0].endsWith("-1"))
    initiallyFireableRuleIndexes.clear();

  //qDebug() << "Creating fact rule.";

  // Cluster facts so implemented similarity measures can be used.
  ruleCluster factRule = createFactRule();

  //qDebug() << "Finding most similar rule.";

  int mostSimiliarClusterIdx = findMostSimiliarClusterToFactRule(&factRule);

  numberOfClustersSearched = 0;
  fireableRules.clear();

  //qDebug() << "Finding rules to fire.";

  findRulesToFireInCluster(&factRule,
                             grpThread->settings
                             ->clusters->at(mostSimiliarClusterIdx).get());

  //qDebug() << "Checking if target can be achieved.";

  canTargetBeAchieved();

  numberOfRulesFired = fireableRules.size();

  //qDebug() << "Firing rules.";

  for(auto rCluster : fireableRules)
  {
    fireRule(static_cast<ruleCluster*>(rCluster));
    rulesFiredDuringInterference.insert(static_cast<ruleCluster*>(rCluster)->name().toStdString());
  }

  //qDebug() << "Was target achived.";

  wasTargetAchieved();

  return 0;
}

int clusterInterferencer::interfereExhaustive()
{
  //qDebug() << "Exhaustive.";

  //qDebug() << "Number of facts: " <<

  //qDebug() << "Filling facts.";

  fillFacts(factsBasePercent);

  zeroRepresentativeOccured = false;
  zeroRepresentativeOccurenceStep = 0;
  currentStep = 0;
  numberOfClustersSearched = 0;
  numberOfRulesFired = 0;
  numberOfIterations = 0;

  fireableRules.clear();

  //qDebug() << "Filling avi rules.";

  fillAvailableRuleIndexes();
  initiallyFireableRuleIndexes = availableRuleIndexes;
  if(availableRuleIndexes[0].endsWith("-1"))
    initiallyFireableRuleIndexes.clear();

  //qDebug() << "Can target be achieved.";

  canTargetBeAchieved();
  rulesFiredDuringInterference.clear();

  bool canAnyRuleBeFired = true;
  ruleCluster factRule;
  ruleCluster* ruleToFire = nullptr;

  mostSimilarRule = nullptr;

  while(canAnyRuleBeFired && !wasTargetAchieved())
  {
    ++numberOfIterations;
    ++currentStep;

    //qDebug() << "Creating fact rule.";
    factRule = createFactRule();

    //qDebug() << "Filling clusters order.";
    fillOrderOfClustersToSearchExhaustively(&factRule);

    //qDebug() << "Looking for rule to fire.";
    ruleToFire = exhaustivelySearchForRuleToFire(&factRule);

    if(ruleToFire == nullptr)
    {
      //qDebug() << "No rules to fire.";
      canAnyRuleBeFired = false;
    }
    else
    {
      //qDebug() << "Firing rule.";
      if(mostSimilarRule == nullptr) mostSimilarRule = ruleToFire;

      fireRule(ruleToFire);
      ++numberOfRulesFired;
    }
  }

  //qDebug() << "Checking if target was achived.";
  wasTargetAchieved();

  // Emergency finding for most similar rule
  if(mostSimilarRule == nullptr)
  {
    findMostSimilarRule(&factRule,
                        grpThread->clusters[orderedClustersIndexesForExhaustiveSearch[0]].get());
  }

  return numberOfRulesFired;
}

int clusterInterferencer::fillOrderOfClustersToSearchExhaustively(ruleCluster *factRule)
{
  orderedClustersIndexesForExhaustiveSearch.clear();
  std::vector<std::pair<int, double>> clustersSimilarityValues;

  double similarityValue = 0.0;

  //qDebug() << "First for.";
  for(int index = 0; index < grpThread->clusters.size(); ++index)
  {
    similarityValue =
      grpThread->getClustersSimilarityValue(factRule, grpThread->clusters.at(index).get());

    clustersSimilarityValues.push_back(std::pair<int, double>(index, similarityValue));
  }

  double highestSimValue = -1;
  int highestSimClusterIdx = 0;
  int eraseOffset = 0;

  //qDebug() << "While.";
  while(clustersSimilarityValues.size() != 0)
  {
    highestSimValue = -1;
    highestSimClusterIdx = 0;
    eraseOffset = 0;

    //qDebug() << "Inner for.";
    // Find highest similarity cluster
    for(int i = 0; i < clustersSimilarityValues.size(); ++ i)
    {
      if(clustersSimilarityValues[i].second > highestSimValue)
      {
        highestSimValue = clustersSimilarityValues[i].second;
        highestSimClusterIdx = clustersSimilarityValues[i].first;
        eraseOffset = i;
      }
    }

    // Add it to target vector and remove from working vector
    orderedClustersIndexesForExhaustiveSearch.push_back(highestSimClusterIdx);
    clustersSimilarityValues.erase(clustersSimilarityValues.begin() + eraseOffset);
  }

  return orderedClustersIndexesForExhaustiveSearch.size();
}

ruleCluster *clusterInterferencer::exhaustivelySearchForRuleToFire(ruleCluster *factRule)
{
  ++ numberOfClustersSearched;

  ruleCluster *ruleToFire = nullptr;
  cluster *consideredCluster = nullptr;

  for(int clusterIndex : orderedClustersIndexesForExhaustiveSearch)
  {
    //qDebug() << orderedClustersIndexesForExhaustiveSearch;

    consideredCluster = grpThread->clusters[clusterIndex].get();

    //qDebug() << "After get";

    ruleToFire = exhaustivelySearchForRuleToFireInCluster(consideredCluster, factRule);

    //qDebug() << "Found rule to fire.";

    if(ruleToFire != nullptr)
    {
      // Add it to the list of unavailable rules and break the loop
      rulesFiredDuringInterference.insert(ruleToFire->name().toStdString());
      break;
    }
  }

  return ruleToFire;
}

ruleCluster *clusterInterferencer::exhaustivelySearchForRuleToFireInCluster(cluster *clusterToSearch, ruleCluster *factRule)
{
  if(clusterToSearch->representativeAttributesValues.keys().size() == 0)
  {
    zeroRepresentativeOccured = true;
    ++zeroRepresentativeOccurenceNumber;
  }

  ++ numberOfClustersSearched;

  ruleCluster *consideredCluster =
    static_cast<ruleCluster*>(clusterToSearch);

  ruleCluster *ruleFound = nullptr;

  std::vector<std::shared_ptr<cluster>> subclustersConsiderationOrder;
  double leftNodeSimilarity = 0.0;
  double rightNodeSimilarity = 0.0;

  if(consideredCluster->size() == 1)
  {
    if(canRuleBeFired(consideredCluster) &&
       !wasRuleFiredDuringInterference(consideredCluster->name().toStdString()))
    {
      ruleFound = consideredCluster;
    }
  }
  else // if it's a cluster not a rule
  {
    // determine which cluster should be looked up first
    leftNodeSimilarity = grpThread->getClustersSimilarityValue(
         consideredCluster->leftNode.get(), factRule);
    rightNodeSimilarity = grpThread->getClustersSimilarityValue(
         consideredCluster->rightNode.get(), factRule);

    if(rightNodeSimilarity > leftNodeSimilarity)
    {
      subclustersConsiderationOrder.push_back(consideredCluster->rightNode);
      subclustersConsiderationOrder.push_back(consideredCluster->leftNode);
    }
    else
    {
      subclustersConsiderationOrder.push_back(consideredCluster->leftNode);
      subclustersConsiderationOrder.push_back(consideredCluster->rightNode);
    }

    //qDebug() << subclustersConsiderationOrder.size();

    ruleFound =
      exhaustivelySearchForRuleToFireInCluster(subclustersConsiderationOrder[0].get(), factRule);

    //qDebug() << "After rule found.";

    if(ruleFound == nullptr)
      ruleFound =
        exhaustivelySearchForRuleToFireInCluster(subclustersConsiderationOrder[1].get(), factRule);
      //qDebug() << "Second rule found.";
  }

  //qDebug() << "Leaving exhaustivelySearchForRuleToFireInCluster.";

  return ruleFound;
}

bool clusterInterferencer::wasRuleFiredDuringInterference(std::string clusterName)
{
  if(rulesFiredDuringInterference.find(clusterName) != rulesFiredDuringInterference.end())
    return true;

  return false;
}

int clusterInterferencer::fireRule(ruleCluster *ruleToFire)
{
  for(QString decisionAttribute : ruleToFire->decisionAttributes)
  {
    for(QString value : *(ruleToFire->attributesValues[decisionAttribute]))
    {
      facts[decisionAttribute].insert(value);
      newFacts[decisionAttribute].insert(value);
    }
  }

  return 0;
}

int clusterInterferencer::generateRandomFactsBase(QString path,
                                                  int desiredNumberOfFacts)
{
  //return saveAllFactsToBase(path);

  if(grpThread == 0) return -1;

  int numberOfPossibleFacts =
      countNumberOfPossibleFacts();

  if(numberOfPossibleFacts <= desiredNumberOfFacts)
    return saveAllFactsToBase(path);

  if(numberOfPossibleFacts > desiredNumberOfFacts)
    return saveRandomNFactsToBase(desiredNumberOfFacts, path);

  return -1;
}

int clusterInterferencer::loadFactsFromPath(QString path)
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

int clusterInterferencer::interfere()
{
  int returnCode = -1;

  clock_t start = clock();

  switch (interferenceType) {
    case GREEDY:
      returnCode =  interfereGreedy();
      break;
    case EXHAUSTIVE:
      returnCode =  interfereExhaustive();
      break;
    default:
      returnCode = -2;
      break;
  }

  interferenceTime = (clock() - start) / (double) CLOCKS_PER_SEC;

  return returnCode;
}

std::string clusterInterferencer::getInterferenceType()
{
  switch (interferenceType)
  {
    case GREEDY:
      return "Greedy cluster interferencer";
      break;
    case EXHAUSTIVE:
      return "Exhaustive cluster interferencer";
    default:
      return "ERROR: Unknown interferenction type.";
      break;
  }

  return "ERROR: Unknown interferention type, after switch.";
}

void clusterInterferencer::setInterferenceType(int newInterferenceType)
{
  interferenceType = newInterferenceType;
}

// Returns indexes of rules that can be fired
int clusterInterferencer::fillAvailableRuleIndexes()
{
  if(grpThread == 0) return -1;

  QList<cluster*> objects = {};

  //qDebug() << "Clus size: " << grpThread->settings->clusters->size();

  for(unsigned int i = 0; i < grpThread->settings->clusters->size(); ++i)
  {
    //qDebug() << i << ". " << grpThread->settings->clusters->at(i)->size();
    objects += grpThread->settings->clusters->at(i)->getObjects();
  }

  //qDebug() << "got objects: " << objects.size();

  availableRuleIndexes.clear();

  for(int i = 0; i < objects.size(); ++i)
  {
    ruleCluster* c =
        static_cast<ruleCluster*>(objects.at(i));

    if(canRuleBeFired(c))
      availableRuleIndexes.push_back(QString::number((i+1)));
  }

  if(availableRuleIndexes.size() < 1)
    availableRuleIndexes.push_back("-1");

  return 0;
}

int clusterInterferencer::canTargetBeAchieved()
{
  targetAchiveable = 0;

  if(target.size() < 1)
  {
    targetAchiveable = 0;
    return 0;
  }
  if(grpThread == nullptr) return -1;

  QHash<QString, QSet<QString>> decisions;

  QList<cluster*> clus = {};

  for(unsigned int i = 0; i < grpThread->settings->clusters->size(); ++i)
    clus += grpThread->settings->clusters->at(i)->getObjects();

  for(cluster* cl : clus)
  {
    ruleCluster *c = static_cast<ruleCluster*>(cl);

    if(canRuleBeFired(c))
    {
      for(QString decAttr : c->decisionAttributes)
      {
        for(QString val : *c->attributesValues[decAttr])
        {
          decisions[decAttr].insert(val);
        }
      }
    }
  }

  for(QString key : target.keys())
  {
    for(QString val : target[key].values())
    {
      if(!decisions.keys().contains(key))
      {
        targetAchiveable = 0;
        return 0;
      }
      if(!decisions[key].contains(val))
      {
        targetAchiveable = 0;
        return 0;
      }
    }
  }

  targetAchiveable = 1;

  return 0;
}

int clusterInterferencer::wasTargetAchieved()
{
  targetAchieved = 0;

  for(QString attributeName : target.keys())
  {
    for(QString attributeValue : target[attributeName])
    {
      if(!facts[attributeName].contains(attributeValue))
        return 0;
    }
  }

  targetAchieved = 1;

  return 1;

  /*
  bool oneOfRulesContainsDescriptor = false;

  if(numberOfRulesFired == 0) return 0;

  for(QString key : target.keys())
  {
    for(QString val : target[key].values())
    {
      oneOfRulesContainsDescriptor = false;

      for(cluster* clus : fireableRules)
      {
        ruleCluster* c = static_cast<ruleCluster*>(clus);

        if(!c->decisionAttributes.contains(key)) continue;

        if(c->attributesValues[key]->contains(val))
          oneOfRulesContainsDescriptor = true;
      }

      if(!oneOfRulesContainsDescriptor)
        return 0;
    }
  }*/
}

int clusterInterferencer::wasRuleFired()
{
  if(numberOfRulesFired > 0) return 1;

  return 0;
}

bool clusterInterferencer::wasMostImportantRuleFired()
{
  return rulesFiredDuringInterference.find(
        mostSimilarRule->name().toStdString()) != rulesFiredDuringInterference.end();
}

ruleCluster clusterInterferencer::createFactRule()
{
  ruleCluster factRule;

  for(QString factAttribute : facts.keys())
  {
    // Check if attribute isn't on attributes values list
    if(!factRule.attributesValues.keys().contains(factAttribute))
    {
        // If so add it with empty QStringList
        factRule.attributesValues.insert(factAttribute, new QStringList());
    }

    factRule.premiseAttributes.insert(factAttribute);
    factRule.attributes.insert(factAttribute,
                               grpThread->attributes.value(factAttribute));
    for(QString value : facts[factAttribute])
      factRule.attributesValues[factAttribute]->push_back(value);
  }

  factRule.fillRepresentativesAttributesValues(grpThread->grpSettings->repCreationStrategyID,
                                               grpThread->grpSettings->repTreshold);

  return factRule;
}

int clusterInterferencer::fillFacts(int basePercent)
{
  facts.clear();

  int numberOfFacts = qCeil(allFacts.size() * basePercent / 100.0);
  QStringList fact;

  for(int i = 0; i < numberOfFacts; ++i)
  {
    fact = QString(allFacts.at(i)).split("=");
    facts[fact.at(0)] += fact.at(1);
  }

  initialFacts = facts;

  return numberOfFacts;
}

int clusterInterferencer::findMostSimiliarClusterToFactRule(cluster* factRule)
{
  int clusterIdx = 0;
  double maxSimilarity = -1.0, sim;

  std::vector<std::shared_ptr<cluster>> *clusters = grpThread->settings->clusters;

  zeroRepresentativeOccurenceNumber = 0;

  for(unsigned int idx = 0; idx < clusters->size(); ++idx)
  {
    sim = grpThread->getClustersSimilarityValue(clusters->at(idx).get(), factRule);

    if(sim > maxSimilarity)
    {
      maxSimilarity = sim;
      clusterIdx = idx;
    }

    if(clusters->at(idx).get()->representativeAttributesValues.keys().size() == 0)
    {
      zeroRepresentativeOccurenceStep =
          zeroRepresentativeOccurenceStep == 0 ? currentStep : zeroRepresentativeOccurenceStep;
      zeroRepresentativeOccured = true;
      ++zeroRepresentativeOccurenceNumber;
    }
  }

  return clusterIdx;
}

int clusterInterferencer::findRulesToFireInCluster(cluster *fc, cluster *c)
{
  ++numberOfClustersSearched;

  if(c->size() == 1)
  {
    if(canRuleBeFired(static_cast<ruleCluster*>(c)))
      fireableRules.push_back(c);

    return 0;
  }

  return  findRulesToFireInCluster(fc, c->leftNode.get()) +
          findRulesToFireInCluster(fc, c->rightNode.get());
}

bool clusterInterferencer::canRuleBeFired(ruleCluster *c)
{
 QSet<QString> keys = c->attributesValues.keys().toSet();

  for(QString dec : c->decisionAttributes)
    keys.remove(dec);

  for(QString key : keys)
  {
    for(QString value : *c->attributesValues[key])
    {
      if(!facts.keys().contains(key))
      {
        return false;
      }
      if(!facts[key].contains(value))
      {
        return false;
      }
    }
  }

  return true;
}

int clusterInterferencer::countNumberOfPossibleFacts()
{
  QSet<QString> factsSet;

  for(int i = 0; i < grpThread->settings->stopCondition; ++i)
  {
    ruleCluster* c =
      static_cast<ruleCluster*>(grpThread->clusters[i].get());
    factsSet += c->getDescriptors(PREMISES);
  }

  return factsSet.size();
}

int clusterInterferencer::saveAllFactsToBase(QString path)
{
  QSet<QString> factsSet;

  for(int i = 0; i < grpThread->settings->stopCondition; ++i)
  {
    ruleCluster* c =
      static_cast<ruleCluster*>(grpThread->clusters[i].get());
    factsSet += c->getDescriptors(PREMISES);
  }

  return insertFactsToBase(&factsSet, path);
}

int clusterInterferencer::saveRandomNFactsToBase(int numOfFacts, QString path)
{
  QSet<QString> factsSet, nFacts;

  for(int i = 0; i < grpThread->settings->stopCondition; ++i)
  {
    ruleCluster* c =
      static_cast<ruleCluster*>(grpThread->clusters[i].get());
    factsSet += c->getDescriptors(PREMISES);
  }

  while(nFacts.size() < numOfFacts)
    nFacts << factsSet.values().at(rand() % factsSet.size());

  return insertFactsToBase(&nFacts, path);
}

int clusterInterferencer::insertFactsToBase(QSet<QString> *factsBase, QString path)
{
  QFile file(path);
  int factIdx;
  QStringList factsList = factsBase->toList();

  if(file.open(QIODevice::ReadWrite))
  {
    QTextStream stream(&file);

    while(factsList.size() > 0)
    {
      factIdx = rand() % factsList.size();

      stream << factsList.at(factIdx) << endl;

      factsList.removeAt(factIdx);
    }

  }
  else return -7;

  file.close();

  return 0;
}

int clusterInterferencer::findMostSimilarRule(cluster* fc, cluster* c)
{
  if(c->size() == 1)
  {
    mostSimilarRule = c;
    return 0;
  }

  if(grpThread->getClustersSimilarityValue(fc, c->leftNode.get()) >
     grpThread->getClustersSimilarityValue(fc, c->rightNode.get()))
    return findMostSimilarRule(fc, c->leftNode.get());
  else
    return findMostSimilarRule(fc, c->rightNode.get());

  return -1;
}
