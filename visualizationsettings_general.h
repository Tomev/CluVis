#ifndef VISUALIZATIONSETTINGS_GENERAL_H
#define VISUALIZATIONSETTINGS_GENERAL_H

#include <QRect>

#include "enum_visualizationalgorithms.h"

class visualizationSettings_general
{
public:
    //Members
        //Variables

    int visualizationAlgorithmID;

    bool visualizeAllHierarchyLevels;

    QRect* sceneRect;

    //Methods

    visualizationSettings_general();
    ~visualizationSettings_general();
};

#endif // VISUALIZATIONSETTINGS_GENERAL_H
