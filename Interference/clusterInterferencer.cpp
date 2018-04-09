#include "clusterInterferencer.h"

#include <QFile>
#include <QDebug>

#include "../Clustering/clusters.h"

/* This class is diabolicaly ugly, but it was made in hurry, so I
 * don't really care that much. It shall be rewritten.
 */

clusterInterferencer::clusterInterferencer()
{
  grpThread = 0;
  factsBasePercents = {100, 75, 50, 25, 10, 1};
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
  QHash<QString, QSet<QString>> newFacts;

  for(cluster* clus : fireableRules)
  {
    ruleCluster* c = static_cast<ruleCluster*>(clus);

    for(QString dec : c->decisionAttributes)
    {
      for(QString decVal : *c->attributesValues[dec])
      {
        newFacts[dec].insert(decVal);
      }
    }
  }

  int result = 0;

  for(QString fact : newFacts.keys())
    result += newFacts[fact].size();

  return result;
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
  zeroRepresentativeOccured = false;

  for(int basePercent : factsBasePercents)
  {
    qDebug() << "Number of facts: " << fillFacts(basePercent);

    fillAvailableRuleIndexes();

    // Cluster facts so implemented similarity measures can be used.
    ruleCluster factRule = createFactRule();

    int mostSimiliarClusterIdx = findMostSimiliarClusterToFactRule(&factRule);

    numberOfClustersSearched = 0;
    fireableRules.clear();

    findRulesToFireInCluster(&factRule,
                             grpThread->settings
                             ->clusters->at(mostSimiliarClusterIdx).get());

    canTargetBeAchieved();

    numberOfRulesFired = fireableRules.size();

    findMostSimilarRule(&factRule, grpThread->settings
                        ->clusters->at(mostSimiliarClusterIdx).get());

    wasTargetAchieved();
  }

  return 0;
}

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
    targetAchiveable = 1;
    return 0;
  }
  if(grpThread == 0) return -1;

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
  }

  targetAchieved = 1;

  return 0;
}

int clusterInterferencer::wasRuleFired()
{
  if(numberOfRulesFired > 0) return 1;

  return 0;
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
