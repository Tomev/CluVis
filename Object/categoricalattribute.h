#ifndef CATEGORICALATTRIBUTE_H
#define CATEGORICALATTRIBUTE_H

#include "attribute.h"

#include <QHash>

class categoricalAttribute : public attribute
{
    public:

        categoricalAttribute(QString name, QString type);

        void increaseNumberOfRulesWithGivenAttribute();
        int getNumberOfRulesWithGivenAttribute();

        void incrementValuesFrequency(QString key);
        int getValueFreqency(QString key);

    private:

        QHash<QString, int> valuesFrequency;
        int numberOfRulesWithGivenAttribute = 0;

        void addNewValueToAttribute(QString newValue);

};

#endif // CATEGORICALATTRIBUTE_H
