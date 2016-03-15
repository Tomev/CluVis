#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T19:25:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CluVis

TEMPLATE = app
boost=D:/Boost
boost_include=$$boost/include/boost-1_60
boost_libs = $$boost/lib/libboost_filesystem-vc140-mt-gd-1_60.lib \
$$boost/lib/libboost_date_time-vc140-mt-gd-1_60.lib \
$$boost/lib/libboost_thread-vc140-mt-gd-1_60.lib \
$$boost/lib/libboost_system-vc140-mt-gd-1_60.lib

INCLUDEPATH += . \
$$boost_include

LIBS += $$boost_libs

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
    customgraphicellipseobject.cpp \
    test_clusters.cpp

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
    clusterinfoincludes.h \
    test_clusters.h \
    enum_interclustersimilaritymeasures.h \
    enum_interobjectsimilaritymeasure.h \
    enum_visualizationalgorithms.h

FORMS    += mainwindow.ui \
    about.ui \
    clusterinfo_rsesrules.ui
