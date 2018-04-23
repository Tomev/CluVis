#ifndef CLASSICALINTERFERENCER_H
#define CLASSICALINTERFERENCER_H

#include "interferencer.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

struct rule
{
    std::unordered_map< std::string,
                        std::unordered_set<std::string>> conclusions;

    std::unordered_map< std::string,
                        std::unordered_set<std::string>> premises;
};

class classicalInterferencer
{
  public:
    classicalInterferencer();

    int interfere();
    int loadFactsFromPath(std::string path);

    std::string getInterferentionType();

    int getFirstRuleThatCouldInitiallyBeFired();
    int getLastRuleThatCouldInitiallyBeFired();
    bool wasAnyRuleFired();
    int getNumberOfNewFacts();
    long getNumberOfStructuresSearched();
    double getInterferenceTime();
    int getInitialNumberOfFacts();
    int getNumberOfRulesThatCouldInitiallyBeFired();
    bool wasTargetSet();
    bool isTargetAchiveable();
    bool wasTargetAchieved(std::unordered_map<std::string, std::unordered_set<std::string>>* consideredFacts);
    std::string getRulesThatCouldInitiallyBeFired();
    int getNumberOfRulesFired();
    int getNumberOfIterations();

    void setFactsBasePercent(double newFactsBasePercent);
    void setRules(std::vector<rule> newRules);

  protected:

    std::vector<int> initiallyAvailableRuleIndexes;
    long numberOfStructuresSearchedDuringInterference = -1;
    double interferenceTime = -1.0;
    double factsBasePercent = 100;
    int initialNumberOfFacts = 0;
    int numberOfRulesFired = 0;
    int numberOfIterations = 0;

    std::vector<std::string> allFacts;
    std::unordered_map<std::string, std::unordered_set<std::string>> target;

    std::unordered_map<std::string, std::unordered_set<std::string>> facts;
    std::unordered_map<std::string, std::unordered_set<std::string>> newFacts;
    std::unordered_map<std::string, std::unordered_set<std::string>> interferenceTarget;
    std::vector<rule> rules;

    int fillFacts();
    int findIndexesOfInitiallyAvailableRules();

    int addFactFromLine(std::string line);
    int addTargetFromLine(std::string line);

    bool canRuleBeFired(rule r);
      std::vector<std::string> getRulePermiseAttributesNamesVector(rule r);
    int fireRule(std::vector<rule> *ruleSet, unsigned int ruleIndex);
      std::vector<std::string> getRuleConclusionAttributesNamesVector(rule r);




};

#endif // CLASSICALINTERFERENCER_H
