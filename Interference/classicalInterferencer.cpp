#include "classicalInterferencer.h"

#include <iostream>
#include <fstream>
#include <ctime>
#include <math.h>
#include <chrono>

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
  using namespace std::chrono;

  std::vector<rule> workingRules = rules;
  bool wasRuleFired = true;
  //fillFacts();
  facts = getInitialFacts();
  findIndexesOfInitiallyAvailableRules();
  numberOfStructuresSearchedDuringInterference = 0;
  numberOfRulesFired = 0;
  numberOfIterations = 0;
  newFacts.clear();

  high_resolution_clock::time_point start = high_resolution_clock::now();

  while(wasRuleFired)
  {
    wasRuleFired = false;
    ++numberOfIterations;

    for(int i = workingRules.size() - 1; i >= 0; --i)
    {
      ++ numberOfStructuresSearchedDuringInterference;

      if(canRuleBeFired(workingRules[i]))
      {
        wasRuleFired = true;
        fireRule(&workingRules, i);
        ++numberOfRulesFired;
        break;
      }
    }
  }

  high_resolution_clock::time_point end = high_resolution_clock::now();
  duration<double> timeSpan = duration_cast<duration<double>>(end - start);
  interferenceTime = timeSpan.count();

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

std::string classicalInterferencer::getInterferentionType()
{
  return "Classical interference";
}

/** classicalInterferencer::getFirstRuleThatCouldInitiallyBeFired
 *
 * @brief Returns first rule idx from initially available rules.
 *
 * @return First rule that could initially be fired.
 */

int classicalInterferencer::getFirstRuleThatCouldInitiallyBeFired()
{
  if(initiallyAvailableRuleIndexes.size() == 0)
    return -1;

  return initiallyAvailableRuleIndexes.at(0);
}

/** classicalInterferencer::getLastRuleThatCouldInitiallyBeFired
 *
 * @brief Returns last rule idx from initially available rules.
 *
 * @return Last rule that could initially be fired.
 */

int classicalInterferencer::getLastRuleThatCouldInitiallyBeFired()
{
  if(initiallyAvailableRuleIndexes.size() == 0)
    return -1;

  return initiallyAvailableRuleIndexes.at(initiallyAvailableRuleIndexes.size() - 1);
}

/** classicalInterferencer::wasAnyRuleFired
 *
 * If any rule was fired, it should return true, else false. Rule firing is
 * determined by checking new facts size. If anything was fired it should be
 * greater than zero.
 *
 * @brief Checks if any rule was fired.
 * @return Boolean value describing if rule was fired.
 */

bool classicalInterferencer::wasAnyRuleFired()
{
  return newFacts.size();
}

/** classicalInterferencer::wasInterferenceTargetInitiallyConfirmable()
 *
 * Checks if initially fireable rules conclusions confirms the target.
 *
 * @brief Checks if initially fireable rules conclusions confirms the target.
 *
 * @return If target was initially achieveable.
 */

bool classicalInterferencer::wasInterferenceTargetInitiallyConfirmable()
{
  if(!wasTargetSet()) return false;

  std::unordered_map<std::string, std::unordered_set<std::string>>
      initiallyObtainableFacts;

  initiallyObtainableFacts = getInitialFacts();

  std::string attributeName;
  std::unordered_set<std::string> attributesValues;

  for(int ruleIndex : initiallyAvailableRuleIndexes)
  {
    for(auto kv : rules[ruleIndex].conclusions)
    {
      attributeName = kv.first;
      attributesValues = kv.second;

      for(std::string attributesValue : attributesValues)
        initiallyObtainableFacts[attributeName].insert(attributesValue);
    }
  }

  return wasTargetAchieved(&initiallyObtainableFacts);
}

/** classicalInterferencer::getNumberOfNewFacts
 * @brief Counts and returns new facts.
 *
 * As facts can have many values for one attribute, this method cannot
 * simply return size of new facts. Size of each value list has to be
 * added to the result.
 *
 * @return Number of new facts generated during interference.
 */

int classicalInterferencer::getNumberOfNewFacts()
{
  int numberOfNewFacts = 0;

  for(const auto& attrVal : newFacts)
    numberOfNewFacts += attrVal.second.size();

  return numberOfNewFacts;
}

/** classicalInterferencer::getNumberOfStructuresSearched
 *
 * @brief Getter for numberOfStructuresSearchedDuringInterference.
 *
 * @return Number of rules searched during interference.
 */

long classicalInterferencer::getNumberOfStructuresSearched()
{
  return numberOfStructuresSearchedDuringInterference;
}

/** classicalInterferencer::getInterferenceTime
 *
 * @brief Getter for interferenceTime.
 *
 * @return Interference time.
 */

double classicalInterferencer::getInterferenceTime()
{
  return interferenceTime;
}

/** classicalInterferencer::getInitialNumberOfFacts
 *
 * @brief Getter for initialNumberOfFacts;
 *
 * @return initialNumberOfFacts
 */

int classicalInterferencer::getInitialNumberOfFacts()
{
  return initialNumberOfFacts;
}

/** classicalInterferencer::getNumberOfRulesThatCouldInitiallyBeFired
 *
 * Check initiallyAvailableRuleIndexes vector and returns it's size.
 *
 * @brief Counts and returns number of rules that could initially be fired.
 *
 * @return Number of rules that could initially be fired.
 */

int classicalInterferencer::getNumberOfRulesThatCouldInitiallyBeFired()
{
  return initiallyAvailableRuleIndexes.size();
}

/** classicalInterferencer::wasTargetSet
 *
 * Checks if target was set. If it was, thet interferenceTarget size should
 * be higher than 0 as it contains it's attribute data.
 *
 * @brief Checks if target was set.
 *
 * @return True if target was set, false otherwise.
 */

bool classicalInterferencer::wasTargetSet()
{
  return interferenceTarget.size() != 0;
}

/** classicalInterferencer::isTargetAchiveable
 *
 * Checks if target is achiveable. To do so, workingFacts storage containing
 * every rule decision and facts is created and then check if wasTargetAchieved
 * is performed.
 *
 * @brief Checks if target is achiveable.
 * @return
 */

bool classicalInterferencer::isTargetAchiveable()
{
  std::unordered_map<std::string, std::unordered_set<std::string>> workingFacts
      = getInitialFacts();

  std::string attributeName;
  std::unordered_set<std::string> attributesValues;

  for(rule r : rules)
  {
    for(auto kv : r.conclusions)
    {
      attributeName = kv.first;
      attributesValues = kv.second;

      for(std::string attributesValue : attributesValues)
        workingFacts[attributeName].insert(attributesValue);
    }
  }

  return wasTargetAchieved(&workingFacts);
}

/** classicalInterferencer::wasTargetAchieved
 *
 * Checks if interference target was achieved. To do this, each value of each
 * attribute of the target is compared with contained facts. If any value is
 * missing from the facts storage, then returns false.
 *
 * @brief Checks if interference target was achieved.
 *
 * @return True if was achieved, otherwise false.
 */

bool classicalInterferencer::wasTargetAchieved(
    std::unordered_map<std::string, std::unordered_set<std::string>> *consideredFacts)
{
  // First consider situation with no target.
  if(! wasTargetSet())
  {
    return getNumberOfNewFacts() > 0;
  }

  // Proceed with classic check.
  std::unordered_map<std::string, std::unordered_set<std::string>> *factsForCheck;

  if(consideredFacts == nullptr)
    factsForCheck = &facts;
  else
    factsForCheck = consideredFacts;

  for(auto attrVal : interferenceTarget)
  {
    for(auto val : attrVal.second)
    {
      if((*factsForCheck)[attrVal.first].find(val) == (*factsForCheck)[attrVal.first].end())
      {
        return false;
      }
    }
  }

  return true;
}

std::string classicalInterferencer::targetAchieved()
{
    if(numberOfRulesFired > 0)
        return "True";
    return "False";
}

/** classicalInterferencer::getRulesThatCouldInitiallyBeFired
 *
 * @brief Constructs and returns string of rule indexes that could initially be
 * fired separated by specified separator
 *
 * @return String of rule indexes that could initially be fired separated by
 * specified separator.
 */

std::string classicalInterferencer::getRulesThatCouldInitiallyBeFired()
{
  if(initiallyAvailableRuleIndexes.size() == 0)
    return "";

  std::string result = "";
  std::string separator = " & ";
  int i = 0;

  for(i = 0; i < initiallyAvailableRuleIndexes.size() - 1; ++i)
  {
    result += std::to_string(initiallyAvailableRuleIndexes[i]);
    result += separator;
  }

  result += std::to_string(initiallyAvailableRuleIndexes[i]);

  return result;
}

/** classicalInterferencer::getNumberOfRulesFired
 *
 * @brief Getter for number of rules fired.
 *
 * @return numberOfRulesFired
 */

int classicalInterferencer::getNumberOfRulesFired()
{
  return numberOfRulesFired;
}

/** classicalInterferencer::getNumberOfIterations
 *
 * @brief Getter for numberOfIterations.
 *
 * @return numberOfIterations
 */

int classicalInterferencer::getNumberOfIterations()
{
  return numberOfIterations;
}

/** classicalInterferencer::whyWasntTargetConfirmed
 *
 * Returns possible reasons why target wasn't confirmed. If target was
 * confirmed it returns empty string.
 *
 * @brief Returns possible reasons why target wasn't confirmed.
 *
 * @return failure reasons
 */
std::string classicalInterferencer::whyWasntTargetConfirmed()
{
  if(wasTargetAchieved(nullptr)) return "";

  std::string failureReasons = "";

  failureReasons +=
      wasAnyRuleFired() ? "" : "rule wasnt fired; ";

  failureReasons +=
      wasTargetSet() ? "" : "target not set; ";

  failureReasons = failureReasons.empty() ? "other case" : failureReasons;

  return failureReasons;
}

/** classicalInterferencer::setFactsBasePercent
 *
 * @brief Setter for factsBasePercent.
 *
 * @param newFactsBasePercent
 */

void classicalInterferencer::setFactsBasePercent(double newFactsBasePercent)
{
  factsBasePercent = newFactsBasePercent;
}

/** classicalInterferencer::setRules
 *
 * @brief Setter for rules.
 *
 * @param newRules
 */

void classicalInterferencer::setRules(std::vector<rule> newRules)
{
  rules = newRules;
}

/** classicalInterferencer::fillFacts
 *
 * In order for this method to work properly facts should be given in following
 * form: attribute=value. It'd then fill facts with the percent of facts from
 * allFacts vector.
 *
 * @brief Filling facts with given (by percentage) number of facts from all
 * facts.
 *
 * @return Number of facts filled.
 */

int classicalInterferencer::fillFacts()
{
  facts.clear();

  initialNumberOfFacts = ceil(allFacts.size() * factsBasePercent / 100.0);
  std::string delimiter = "=";
  std::string attribute, value;
  int delimiterPosition = 0;

  for(int i = 0; i < initialNumberOfFacts; ++i)
  {
    delimiterPosition = allFacts[i].find(delimiter);
    attribute = allFacts[i].substr(0, delimiterPosition);
    value = allFacts[i].substr(delimiterPosition+1, allFacts[i].length() - 1);
    facts[attribute].insert(value);
  }

  return initialNumberOfFacts;
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

std::unordered_map<std::string, std::unordered_set<std::string>>
  classicalInterferencer::getInitialFacts()
{
  std::unordered_map<std::string, std::unordered_set<std::string> > initialFacts;

  initialNumberOfFacts = ceil(allFacts.size() * factsBasePercent / 100.0);
  std::string delimiter = "=";
  std::string attribute, value;
  int delimiterPosition = 0;

  for(int i = 0; i < initialNumberOfFacts; ++i)
  {
    delimiterPosition = allFacts[i].find(delimiter);
    attribute = allFacts[i].substr(0, delimiterPosition);
    value = allFacts[i].substr(delimiterPosition+1, allFacts[i].length() - 1);
    initialFacts[attribute].insert(value);
  }

  return initialFacts;
}

/** classicalInterferencer::findIndexesOfInitiallyAvailableRules
 *
 * Counts and return number of rules that could be fired from initial facts base.
 * It requires filled facts base to work properly, otherwise may return rubbish.
 *
 * @brief Returns number of rules that could be fired with initial facts base.
 *
 * @return Number of initially available rules.
 */

int classicalInterferencer::findIndexesOfInitiallyAvailableRules()
{
  initiallyAvailableRuleIndexes.clear();

  for(int i = 0; i < rules.size(); ++i)
  {
    if(canRuleBeFired(rules[i]))
      initiallyAvailableRuleIndexes.push_back(i);
  }

  return initiallyAvailableRuleIndexes.size();
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

  interferenceTarget[attributeName].insert(attributeValue);

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
