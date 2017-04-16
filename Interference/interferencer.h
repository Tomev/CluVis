#ifndef INTERFERENCER_H
#define INTERFERENCER_H

#include <QObject>

class Interferencer : public QObject
{
    Q_OBJECT
public:
    explicit Interferencer(QObject *parent = 0);

signals:

public slots:
};

#endif // INTERFERENCER_H