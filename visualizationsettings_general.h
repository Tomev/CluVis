#ifndef VISUALIZATIONSETTINGS_GENERAL_H
#define VISUALIZATIONSETTINGS_GENERAL_H

#include <QRect>

class visualizationSettings_general
{
public:
    //Members
        //Constans
            //Visualization Algorithm ID

    static const int RT_SLICE_AND_DICE_ID = 0;
    static const int CIRCULAR_TREEMAP_ID = 1;

        //Variables

    int visualizationAlgorithmID = 0;

    bool visualizeAllHierarchyLevels = false;

    QRect* sceneRect = new QRect(0,0,0,0);

    //Methods

    visualizationSettings_general();
    ~visualizationSettings_general();

};

#endif // VISUALIZATIONSETTINGS_GENERAL_H
