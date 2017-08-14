#ifndef NUMERICATTRIBUTE_H
#define NUMERICATTRIBUTE_H

#include "attribute.h"

class numericAttribute : public attribute
{
    public:

        numericAttribute(QString name, QString type);

        void setMinValue(QString newMinValue);
        void setMaxValue(QString newMaxValue);

        bool areMinMaxEqual();
        qreal getMinMaxAbsDifference();

    private:

        QString minValue, maxValue;
        QStringList valuesInBase;
};

#endif // NUMERICATTRIBUTE_H
