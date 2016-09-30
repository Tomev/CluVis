#ifndef OBJECTSCLUSTERER_RSESRULES_H
#define OBJECTSCLUSTERER_RSESRULES_H

#include <QObject>

#include "objectsclusterer.h"

class objectsClusterer_RSESRules : public QObject, public objectsClusterer
{
    Q_OBJECT
    Q_INTERFACES(objectsClusterer)

public:
    explicit objectsClusterer_RSESRules(QObject *parent = 0);

private:

};

#endif // OBJECTSCLUSTERER_RSESRULES_H
