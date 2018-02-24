#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include "Clustering/clusters.h"

enum dataTypesId
{
  RSESRulesId
};

class generalSettings
{
public:
    //Members
        //Variables

    int dataTypeID;
    int objectsNumber;
    int stopCondition;
    std::vector<std::shared_ptr<cluster>> *clusters;

    //Methods

    generalSettings();
    ~generalSettings();
};

#endif // GENERALSETTINGS_H
