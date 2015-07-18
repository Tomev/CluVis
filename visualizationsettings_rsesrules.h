#ifndef VISUALIZATIONSETTINGS_RSESRULES_H
#define VISUALIZATIONSETTINGS_RSESRULES_H

#include "clusters.h"

class visualizationSettings_RSESRules
{
public:
    ruleCluster** clusteredRules;

    visualizationSettings_RSESRules();
    ~visualizationSettings_RSESRules();
};

#endif // VISUALIZATIONSETTINGS_RSESRULES_H
