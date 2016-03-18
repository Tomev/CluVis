#ifndef ATTRIBUTEDATA
#define ATTRIBUTEDATA

#include <QString>

struct attributeData
{
    QString name;
    QString type;
};

struct numericAttributeData : attributeData
{
    numericAttributeData(){}

    numericAttributeData(attributeData data)
    {
        this->name = data.name;
        this->type = data.type;
    }

    QString maxValue;
    QString minValue;
};

#endif // ATTRIBUTEDATA

