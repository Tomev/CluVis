#include "numericattribute.h"

numericAttribute::numericAttribute(QString name, QString type) : attribute(name, type){}

void numericAttribute::setMinValue(QString newMinValue)
{
    minValue = newMinValue;
}

void numericAttribute::setMaxValue(QString newMaxValue)
{
    maxValue = newMaxValue;
}

bool numericAttribute::areMinMaxEqual()
{
    if(minValue == maxValue) return true;

    return false;
}

qreal numericAttribute::getMinMaxAbsDifference()
{
    bool areValuesDouble = false;

    qreal min = minValue.toDouble(&areValuesDouble), max = maxValue.toDouble();

    // Check if values are double. If not return -1.;
    if(!areValuesDouble) return -1;

    return qAbs(max-min);
}
