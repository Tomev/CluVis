#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <QObject>

class visualizer : public QObject
{
    Q_OBJECT
public:
    explicit visualizer(QObject *parent = 0);

signals:

public slots:
};

#endif // VISUALIZER_H
