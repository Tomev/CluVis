#include "categoricalattribute.h"

categoricalAttribute::categoricalAttribute(QString name, QString type) : attribute(name, type){}

void categoricalAttribute::increaseNumberOfRulesWithGivenAttribute()
{
    ++numberOfRulesWithGivenAttribute;
}

int categoricalAttribute::getNumberOfRulesWithGivenAttribute()
{
    return numberOfRulesWithGivenAttribute;
}

void categoricalAttribute::incrementValuesFrequency(QString key)
{
    // Check if attribute already has this value. Add it if not.
    if(! valuesFrequency.keys().contains(key)) addNewValueToAttribute(key);

    ++valuesFrequency[key];
}

void categoricalAttribute::addNewValueToAttribute(QString newValue)
{
    valuesFrequency.insert(newValue, 0);
}

int categoricalAttribute::getValueFreqency(QString key)
{
    // Check if value is in dictionary. If so return it's frequency.
    if(valuesFrequency.keys().contains(key)) return valuesFrequency.value(key);

    // Return -1 otherwise
    return -1;
}
