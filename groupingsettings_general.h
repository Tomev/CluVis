#ifndef GROUPINGSETTINGS_GENERAL_H
#define GROUPINGSETTINGS_GENERAL_H

#include <QFileInfo>

#include "clusters.h"
#include "enum_interclustersimilaritymeasures.h"
#include "enum_interobjectsimilaritymeasure.h"

class groupingSettings_General
{
public:
    //Members
        //Constans
            //Algorithms ID

    const static int CLASSIC_AHC_ID = 0;

        //Variables

    QFileInfo objectBaseInfo;

    int groupingAlgorithmID;
    int interObjectSimMeasureID;
    int interClusterSimMeasureID;
    int attributesNumber;
    int attributesFrequencyPercent;
    int coverSum;

    bool findBestClustering;

    //Methods

    groupingSettings_General();
    ~groupingSettings_General();
};

#endif // GROUPINGSETTINGS_GENERAL_H
