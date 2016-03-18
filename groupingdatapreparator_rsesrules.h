#ifndef GROUPINGDATAPREPARATOR_RSESRULES_H
#define GROUPINGDATAPREPARATOR_RSESRULES_H

#include <QObject>
#include <boost/shared_ptr.hpp>

#include "groupingdatapreparator.h"
#include "groupingsettings.h"
#include "groupingsettings_rsesrules.h"



typedef boost::shared_ptr<ruleCluster> ruleCluster_ptr;

class groupingDataPreparator_RSESRules : public QObject, public groupingDataPreparator
{
    Q_OBJECT
    Q_INTERFACES(groupingDataPreparator)

public:
    groupingDataPreparator_RSESRules(groupingSettings* settings);

    void clusterObjects(cluster** clusters);
    void fillAttributesData(QHash<QString, attributeData>* attributes);
    void fillNumericAttributesValues(QHash<QString, attributeData>* attributes, cluster **clusters);

private:

    groupingSettings_RSESRules* dGrpSet;

    void clusterRule(cluster **clusters, int i, QString rule);
    QString prepareAttribute(QString unprepAtr, bool isDecisionAttribute);
};

#endif // GROUPINGDATAPREPARATOR_RSESRULES_H
