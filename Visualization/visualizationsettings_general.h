#ifndef VISUALIZATIONSETTINGS_GENERAL_H
#define VISUALIZATIONSETTINGS_GENERAL_H


#include "../Clustering/clusters.h"

#include <memory>

#include <QRect>

class visualizationSettings_general
{
public:
    //Members
        //Variables

    int visualizationAlgorithmID;

    bool visualizeAllHierarchyLevels;

    QRect* sceneRect;

    std::vector<std::shared_ptr<cluster>> *clusters;

    //Methods

    visualizationSettings_general();
    ~visualizationSettings_general();
};

#endif // VISUALIZATIONSETTINGS_GENERAL_H
