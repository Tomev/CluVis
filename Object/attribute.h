#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QObject>
#include <QString>

// TODO TR: Use attribute classes to replace structs.

class attribute
{
    public:

        attribute(QString name, QString type);

        QString getType();
        QString getName();

    protected:

        QString type, name;

};

#endif // ATTRIBUTE_H
