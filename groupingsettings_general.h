#ifndef GROUPINGSETTINGS_GENERAL_H
#define GROUPINGSETTINGS_GENERAL_H

#include <QFileInfo>

#include "clusters.h"

class groupingSettings_General
{
public:
    //Members
        //Constans
            //Algorithms ID

    const static int CLASSIC_AHC_ID = 0;

            //Interobject Distance Measure ID

    const static int GOWERS_MEASURE_ID = 0;
    const static int SIMPLE_SIMILARITY_ID = 1;
    const static int WEIGHTED_SIMILARITY_ID = 2;

            //Intercluster Dinstance Measure ID

    const static int SINGLE_LINK_ID = 0;
    const static int COMPLETE_LINK_ID = 1;
    const static int AVERAGE_LINK_ID = 2;
    const static int CENTROID_LINK_ID = 3;

        //Variables

    QFileInfo objectBaseInfo;

    int groupingAlgorithmID;
    int interobjectDistanceMeasureID;
    int interclusterDistanceMeasureID;
    int attributesNumber;
    int coverSum;

    //Methods

    groupingSettings_General();
    ~groupingSettings_General();
};

#endif // GROUPINGSETTINGS_GENERAL_H
