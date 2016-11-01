#ifndef ATTRIBUTEDATA
#define ATTRIBUTEDATA

#include <QString>
#include <QHash>

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

    bool areMinMaxEqual()
    {
        return this->maxValue == this->minValue;
    }

    qreal getMaxMinAbsDiff()
    {
        qreal result;

        result = this->maxValue.toDouble() - this->minValue.toDouble();

        return qAbs(result);
    }
};

struct categoricalAttributeData: attributeData
{
    categoricalAttributeData(){}

    categoricalAttributeData(attributeData data)
    {
        this->name = data.name;
        this->type = data.type;
    }

    QHash<QString, unsigned int> valuesFrequency;

    unsigned int numberOfRulesWithGivenAttribute = 0;

    unsigned int getValuesFrequency(QString value)
    {
        if(valuesFrequency.contains(value))
            return valuesFrequency.value(value);
        else
            return 0;
    }

};

#endif // ATTRIBUTEDATA

