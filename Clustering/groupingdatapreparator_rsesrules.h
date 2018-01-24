#ifndef GROUPINGDATAPREPARATOR_RSESRULES_H
#define GROUPINGDATAPREPARATOR_RSESRULES_H

#include <QObject>
#include <memory>

#include "groupingdatapreparator.h"
#include "groupingsettings.h"
#include "groupingsettings_rsesrules.h"

typedef std::shared_ptr<ruleCluster> ruleCluster_ptr;

class groupingDataPreparator_RSESRules : public QObject, public groupingDataPreparator
{
    Q_OBJECT
    Q_INTERFACES(groupingDataPreparator)

public:
    groupingDataPreparator_RSESRules(groupingSettings* settings);

    void fillAttributesData(QHash<QString, attributeData *> *attributes);
    void clusterObjects(std::vector<std::shared_ptr<cluster>> *clusters, QHash<QString, attributeData *> *attributes);
    void fillAttributesValues(QHash<QString, attributeData*>* attributes, std::vector<std::shared_ptr<cluster>> *clusters);

private:

    groupingSettings_RSESRules* dGrpSet;

    void clusterRule(ruleCluster* c, QString rule, QHash<QString, attributeData *> *attributes);
};

#endif // GROUPINGDATAPREPARATOR_RSESRULES_H
