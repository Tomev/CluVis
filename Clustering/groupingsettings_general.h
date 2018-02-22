#ifndef GROUPINGSETTINGS_GENERAL_H
#define GROUPINGSETTINGS_GENERAL_H

#include <QFileInfo>

#include "clusters.h"

enum visualizationAlgorithmsId
{
    SliceAndDiceRTId = 0,
    CircularTreemapId = 1
};

enum representativeCreationStrategy
{
    THRESHOLD,
    LOWER_ESTIMATE,
    HIGHER_ESTIMATE,
    WEIGHTED_ESTIMATE
};

enum groupingAlgorithmsId
{
    ClassicAHCId = 0
};

class groupingSettings_General
{
public:

    QFileInfo objectBaseInfo;

    int groupingAlgorithmID;
    int interObjectSimMeasureID;
    int interClusterSimMeasureID;
    int attributesNumber;
    int attributesFrequencyPercent;
    int coverSum;
    unsigned int repCreationStrategyID;
    int repTreshold;

    int zeroRepresentativeClusterOccurence;
    unsigned int zeroRepresentativesNumber = 0;

    double giniIndex;
    double bonferroniIndex;
    double inequityThreshold;

    bool findBestClustering;

    //Methods

    groupingSettings_General();
    ~groupingSettings_General();
};

#endif // GROUPINGSETTINGS_GENERAL_H
