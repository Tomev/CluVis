#ifndef GROUPINGSETTINGS_H
#define GROUPINGSETTINGS_H

#include <QObject>

#include "groupingsettings_general.h"
#include "groupingsettings_detailed.h"
#include "generalsettings.h"

class groupingSettings : public QObject
{
    Q_OBJECT
public:
    explicit groupingSettings();

    generalSettings* genSet;
    groupingSettings_General* grpSet;
    groupingSettings_Detailed* dGrpSet;
};

#endif // GROUPINGSETTINGS_H
