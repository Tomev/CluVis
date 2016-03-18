#ifndef ATTRIBUTEDATA
#define ATTRIBUTEDATA

#include <QString>

struct attributeData
{
    attributeData(){}

    attributeData(QString name, QString type)
    {
        this->name = name;
        this->type = type;
    }

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

    void setMaxValue(QString newVal)
    {
        this->maxValue = newVal;
    }

    void setMinValue(QString newVal)
    {
        this->minValue = newVal;
    }
};

#endif // ATTRIBUTEDATA

