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

  protected:

    std::vector<std::string> allFacts;
    std::unordered_map<std::string, std::unordered_set<std::string>> target;

    std::unordered_map<std::string, std::unordered_set<std::string>> facts;
    std::unordered_map<std::string, std::unordered_set<std::string>> newFacts;
    std::vector<rule> rules;

    int addFactFromLine(std::string line);
    int addTargetFromLine(std::string line);

    bool canRuleBeFired(rule r);
      std::vector<std::string> getRulePermiseAttributesNamesVector(rule r);
    int fireRule(std::vector<rule> *ruleSet, unsigned int ruleIndex);
      std::vector<std::string> getRuleConclusionAttributesNamesVector(rule r);




};

#endif // CLASSICALINTERFERENCER_H
