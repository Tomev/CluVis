#include "interferencer.h"

#include <QFile>
#include <QDebug>

#include "../Clustering/clusters.h"

/* This class is diabolicaly ugly, but it was made in hurry, so I
 * don't really care that much. It shall be rewritten.
 */

interferencer::interferencer()
{

}

void interferencer::setGroupingThread(groupingThread *newGrpThread)
{
  this->grpThread = newGrpThread;
}

void interferencer::generateRandomFactsBase()
{
  // Takes random rule and creates facts base from that.

}

int interferencer::loadFactsFromPath(QString path)
{
  facts.clear();

  QFile factsBase(path);
  QTextStream stream(&factsBase);
  QString line;

  QStringList fact;

  if(factsBase.open(QIODevice::ReadWrite))
  {
    while(!stream.atEnd())
    {
      line = stream.readLine();
      fact = line.split("=");
      facts[fact.at(0)] = fact.at(1);
    }
  }

  factsBase.close();

  return 0;
}

int interferencer::interfere()
{
  // Cluster facts so implemented similarity measures can be used.
  ruleCluster factRule = createFactRule();

  int mostSimiliarClusterIdx = findMostSimiliarClusterToFactRule(&factRule);

  numberOfClustersSearched = 0;

  cluster* rule = findRuleToFireInCluster(&factRule, grpThread->settings->clusters->at(mostSimiliarClusterIdx));

  //qDebug() << static_cast<ruleCluster*>(rule)->rule();

  if(canRuleBeFired(static_cast<ruleCluster*>(rule))) numberOfRulesFired = 1;
  else numberOfRulesFired = 0;

  return 0;
}

ruleCluster interferencer::createFactRule()
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
    factRule.attributes.insert(factAttribute, grpThread->attributes.value(factAttribute));
    factRule.attributesValues[factAttribute]->push_back(facts[factAttribute]);
  }

  factRule.fillRepresentativesAttributesValues(grpThread->grpSettings->repCreationStrategyID,
                                               grpThread->grpSettings->repTreshold);

  return factRule;
}

int interferencer::findMostSimiliarClusterToFactRule(cluster* factRule)
{
  int clusterIdx = -1;
  double maxSimilarity = -1.0, sim;

  QVector<cluster*>* clusters = grpThread->settings->clusters;

  for(int idx = 0; idx < clusters->size(); ++idx)
  {
    sim = grpThread->getClustersSimilarityValue(clusters->at(idx), factRule);

    if(sim > maxSimilarity)
    {
      maxSimilarity = sim;
      clusterIdx = idx;
    }
  }

  return clusterIdx;
}

cluster* interferencer::findRuleToFireInCluster(cluster *fc, cluster *c)
{
  ++numberOfClustersSearched;
  if(c->size() == 1) return c;

  if(grpThread->getClustersSimilarityValue(fc, c->leftNode) >
     grpThread->getClustersSimilarityValue(fc, c->rightNode))
    return findRuleToFireInCluster(fc, c->leftNode);
  else return findRuleToFireInCluster(fc, c->rightNode);
}


bool interferencer::canRuleBeFired(ruleCluster *c)
{
  for(QString key : c->attributesValues.keys())
  {
    if(facts.keys().contains(key))
      if(!c->attributesValues[key]->contains(facts[key])) return false;
  }

  return true;
}

