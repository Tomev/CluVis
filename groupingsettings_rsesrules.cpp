#include "groupingsettings_rsesrules.h"

#include <QTextStream>

int groupingSettings_RSESRules::getRSESRulesNumber(QFileInfo KBInfo)
{
    int rNumber = 0;

    QString line = "";

    QFile KB(KBInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!in.atEnd())
        {
            line = in.readLine();

            if(line.startsWith("RULES "))
            {
                line.remove("RULES ");
                rNumber = line.toInt();
                break;
            }
        }
    }

    return rNumber;

}

int groupingSettings_RSESRules::getRSESRulesAttributeNumber(QFileInfo KBInfo)
{
    int aNumber = 0;

    QString line = "";

    QFile KB(KBInfo.absoluteFilePath());

    if(KB.open(QIODevice::ReadOnly))
    {
        QTextStream in(&KB);

        while(!in.atEnd())
        {
            line = in.readLine();

            if(line.startsWith("ATTRIBUTES "))
            {
                line.remove("ATTRIBUTES ");
                aNumber = line.toInt();
                break;
            }
        }
    }

    return aNumber;
}

groupingSettings_RSESRules::groupingSettings_RSESRules()
{

}

groupingSettings_RSESRules::~groupingSettings_RSESRules()
{

}
