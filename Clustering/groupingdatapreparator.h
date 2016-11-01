#ifndef GROUPINGDATAPREPARATOR_H
#define GROUPINGDATAPREPARATOR_H

#include "clusters.h"
#include "attributedata.h"

#include "generalsettings.h"
#include "groupingsettings_general.h"


/*
 *  This class is an interface for all the groupingDataPreparators -
 *  classes that are meant to cluster rules and fill attributes data
 *  (for Gower's measure and representative).
 */

class groupingDataPreparator
{
    public:
        groupingDataPreparator(generalSettings* genSet, groupingSettings_General* grpSet);

        virtual void fillAttributesData(QHash<QString, attributeData*>* attributes) = 0;
        virtual void clusterObjects(cluster** clusters, QHash<QString, attributeData*> *attributes) = 0;
        virtual void fillAttributesValues(QHash<QString, attributeData*>* attributes, cluster **clusters) = 0;

    protected:
        generalSettings* genSet;
        groupingSettings_General* grpSet;
};

Q_DECLARE_INTERFACE(groupingDataPreparator, "groupingDataPreparator")

#endif // GROUPINGDATAPREPARATOR_H
