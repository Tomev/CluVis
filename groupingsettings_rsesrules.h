#ifndef GROUPINGSETTINGS_RSESRULES_H
#define GROUPINGSETTINGS_RSESRULES_H

#include <QFileInfo>

struct RSESAttribute
{
    QString name;
    QString type;
    QString value;

    qreal maxValue;
    qreal minValue;

};

class groupingSettings_RSESRules
{
public:
    //Members
        //Constant
            //Grouping Part IDs
    static const int CONDITION_ID = 0;
    static const int CONCLUSION_ID = 1;
        //Variables

    int groupingPartID;

    //Methods

    int getRSESRulesNumber(QFileInfo KBInfo);
    int getRSESRulesAttributeNumber(QFileInfo KBInfo);

    groupingSettings_RSESRules();
    ~groupingSettings_RSESRules();

};

#endif // GROUPINGSETTINGS_RSESRULES_H
