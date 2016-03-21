#ifndef GROUPINGSETTINGS_RSESRULES_H
#define GROUPINGSETTINGS_RSESRULES_H

#include <QFileInfo>

#include "groupingsettings_detailed.h"

class groupingSettings_RSESRules : public groupingSettings_Detailed
{
public:
    //Members
        //Variables

    int groupedPartID;

    //Methods

    static int getRSESRulesNumber(QFileInfo KBInfo);
    static int getRSESRulesAttributeNumber(QFileInfo KBInfo);

    groupingSettings_RSESRules();
    ~groupingSettings_RSESRules();

};

#endif // GROUPINGSETTINGS_RSESRULES_H
