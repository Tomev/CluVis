#include "classicalInterferencer.h"

#include <iostream>
#include <fstream>

enum ruleFiringErrors
{
  ruleIndexBelowZero = -1,
  ruleIndexHigherThanRulesetSize = -2
};

classicalInterferencer::classicalInterferencer()
{

}

/** classicalInterferencer::interfere
 * @brief This is the main functionality of this class. Takes a vector of rules
 *        and basing on given facts.
 *
 *
 * For previously set rules set and facts set, this method performs classical
 * interference, iteratively, until no rule can be fired. Note that a working
 * set of rules is created in order for entered rule set to be left intact.
 *
 *
 * @return Number of new facts.
 */

int classicalInterferencer::interfere()
{
  std::vector<rule> workingRules = rules;
  bool wasRuleFired = true;

  while(wasRuleFired)
  {
    wasRuleFired = false;

    for(int i = workingRules.size() - 1; i >= 0; --i)
    {
      if(canRuleBeFired(workingRules[i]))
      {
        wasRuleFired = true;
        fireRule(&workingRules, i);
      }
    }
  }

  return newFacts.size();
}

/** classicalInterferencer::loadFactsFromPath
 *
 * Finds out if there's a file in the given path. If not it reports. Is so,
 * then it loads
 *
 * @brief Reads facts and target of interference from facts base at given path.
 *
 * @param path Path to facts base.
 *
 * @return Number of facts loaded.
 */

int classicalInterferencer::loadFactsFromPath(std::string path)
{
  allFacts.clear();
  target.clear();

  std::ifstream factsBase(path);
  std::string line;
  int factsCount = 0;

  std::string targetPrefix = "Target:";
  std::string commentPrefix = "#";

  if(factsBase.is_open())
  {
    while(std::getline(factsBase, line))
    {
      if(!line.compare(0, targetPrefix.size(), targetPrefix)) break;
      if(!line.compare(0, commentPrefix.size(), commentPrefix)) continue;
      // # in facts base means comment.
      allFacts.push_back(line);
      /*
       * Facts are stored in a vector instead of parsed structure, as it is
       * easier to retrive their random number.
      */
      ++factsCount;
    }

    while(std::getline(factsBase, line))
    {
      if(!line.compare(0, commentPrefix.size(), commentPrefix)) continue;
      // # in facts base means comment.
      addTargetFromLine(line);
    }
  }

  factsBase.close();

  return factsCount;
}

/** classicalInterferencer::addFactsFromLine
 *
 * Parses line read from facts base into the form that can easily be stored
 * and read by interferencer.
 *
 * @brief From given line adds fact to allFacts.
 *
 * @param line Line that contains
 *
 * @return 0 if success.
 */

int classicalInterferencer::addFactFromLine(std::string line)
{
  std::string delimiter = "=";

  int delimiterPosition = line.find(delimiter);
  std::string attributeName = line.substr(0, delimiterPosition);

  line.erase(0, delimiterPosition + delimiter.length());

  std::string attributeValue = line;

  facts[attributeName].insert(attributeValue);

  return 0;
}

/** classicalInterferencer::addTargetFromLine
 *
 *
 *
 * @brief Adds parsed target from given line.
 *
 * @param line Line read from facts base.
 *
 * @return 0 if finished correctly.
 */

int classicalInterferencer::addTargetFromLine(std::string line)
{
  std::string delimiter = "=";

  int delimiterPosition = line.find(delimiter);
  std::string attributeName = line.substr(0, delimiterPosition);

  line.erase(0, delimiterPosition + delimiter.length());

  std::string attributeValue = line;

  facts[attributeName].insert(attributeValue);

  return 0;
}

/** classicalInterferencer::canRuleBeFired
 *
 * Checks rule permise with current facts base. If all permises are in the facts
 * base, then the rule can be fired.
 *
 * @brief Checks if given rule can be fired according to current facts base.
 *
 * @param r Rule which permise attributes are set.
 *
 * @return Boolean value deciding if all of rule permises are in facts base.
 */

bool classicalInterferencer::canRuleBeFired(rule r)
{
  std::vector<std::string> permiseAttributesNames =
    getRulePermiseAttributesNamesVector(r);

  for(std::string attributeName : permiseAttributesNames)
  {
    for(std::string attributeValue : r.premises[attributeName])
    {
      // Examplary use of std::unordered_set::find from www.cplusplus.com.
      std::unordered_set<std::string>::const_iterator got =
        facts[attributeName].find(attributeValue);

      if(got == facts[attributeName].end()) return false;
    }
  }

  return true;
}

/** classicalInterferencer::getRulePermiseAttributesNamesVector
 * @brief Get permise attributes names from given rule.
 *
 * This method returns vector names of all attributes in rules permise part.
 *
 * @param r Rule which permise attributes names are extracted.
 *
 * @return Vector of permise attributes list of given rule.
 */

std::vector<std::string> classicalInterferencer::getRulePermiseAttributesNamesVector(rule r)
{
  std::vector<std::string> attributesVector;

  for(auto kv: r.premises) attributesVector.push_back(kv.first);

  return attributesVector;
}

/** classicalIntrerferencer::fireRule
 * @brief Fires given rule given by index in given rule set.
 *
 * Fires selected rule, which implies, that it's conclusion attributes and
 * values are pushed to facts base and new facts base, and rule is removed
 * from given rules set. Note that this method doesn't check if rule can be
 * fired, as it's done prior to calling this method. It does, however, check
 * if rule index lies within proper bounds.
 *
 * @param ruleSet It's a pointer to set of rules from which rule should be
 *                fired.
 * @param ruleIndex It's the index of rule in given rule set, that should be
 *                  fired.
 * @return Current number of rules in rule set.
 */

int classicalInterferencer::fireRule(std::vector<rule>* ruleSet, unsigned int ruleIndex)
{
  if(ruleIndex >= ruleSet->size()) return ruleIndexHigherThanRulesetSize;

  rule *ruleToFirePtr = &((*ruleSet)[ruleIndex]);

  std::string attributeName;
  std::unordered_set<std::string> attributesValues;

  for(auto kv : ruleToFirePtr->conclusions)
  {
    attributeName = kv.first;
    attributesValues = kv.second;

    for(std::string attributesValue : attributesValues)
    {
      newFacts[attributeName].insert(attributesValue);
      facts[attributeName].insert(attributesValue);
    }
  }


  ruleSet->erase(ruleSet->begin() + ruleIndex);

  return ruleSet->size();
}

/** classicalInterferencer::getRuleConclusionAttributesNamesVector
 * @brief Get conclusion attributes names from given rule.
 *
 * This method returns vector names of all attributes in rules conclusion part.
 *
 * @param r Rule which conclusion attributes names are extracted.
 *
 * @return Vector of conclusion attributes list of given rule.
 */

std::vector<std::string> classicalInterferencer::getRuleConclusionAttributesNamesVector(rule r)
{
  std::vector<std::string> attributesVector;

  for(auto kv: r.conclusions) attributesVector.push_back(kv.first);

  return attributesVector;
}
