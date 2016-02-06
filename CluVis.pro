#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T19:25:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DaVis
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    about.cpp \
    customgraphicsrectobject.cpp \
    groupingthread.cpp \
    groupingsettings_general.cpp \
    visualizationthread.cpp \
    groupingsettings_rsesrules.cpp \
    visualizationsettings_general.cpp \
    generalsettings.cpp \
    visualizationsettings_rsesrules.cpp \
    clusterinfo_rsesrules.cpp \
    customgraphicellipseobject.cpp

TRANSLATIONS += polish.ts\
                english.ts

HEADERS  += mainwindow.h \
    about.h \
    customgraphicsrectobject.h \
    groupingthread.h \
    groupingsettings_general.h \
    groupingsettingsincludes.h \
    visualizationthread.h \
    groupingsettings_rsesrules.h \
    visualizationsettings_general.h \
    clusters.h \
    generalsettings.h \
    visualizationincludes.h \
    generalincludes.h \
    visualizationsettings_rsesrules.h \
    clusterinfo_rsesrules.h \
    customgraphicellipseobject.h \
    clusterinfoincludes.h

FORMS    += mainwindow.ui \
    about.ui \
    clusterinfo_rsesrules.ui
