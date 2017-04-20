#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T19:25:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic

TARGET = CluVis

TEMPLATE = app

SOURCES +=      Forms/about.cpp \
                Forms/clusterinfo_rsesrules.cpp \
                Forms/mainwindow.cpp \
                Visualization/visualizationthread.cpp \
                Visualization/visualizationsettings_general.cpp \
                Visualization/customgraphicellipseobject.cpp \
                Visualization/customgraphicsrectobject.cpp \
                Clustering/groupingthread.cpp \
                Clustering/groupingdatapreparator.cpp \
                Clustering/groupingdatapreparator_rsesrules.cpp \
                Clustering/groupingsettings.cpp \
                Clustering/groupingsettings_general.cpp \
                Clustering/groupingsettings_rsesrules.cpp \
                main.cpp \
                enum_interobjectsimilaritymeasures.cpp \
                generalsettings.cpp \
    Interference/interferencer.cpp \
    Object/attribute.cpp \
    Object/numericattribute.cpp \
    Object/categoricalattribute.cpp


HEADERS  +=     Forms/about.h \
                Forms/clusterinfo_rsesrules.h \
                Forms/mainwindow.h \
                enum_interclustersimilaritymeasures.h \
                enum_interobjectsimilaritymeasure.h \
                enum_visualizationalgorithms.h \
                enum_rulesparts.h \
                enum_reporttypes.h \
                enum_languages.h \
                enum_datatype.h \
                enum_groupingalgorithms.h \
                Visualization/customgraphicsrectobject.h \
                Visualization/visualizationthread.h \
                Visualization/visualizationsettings_general.h \
                Visualization/customgraphicellipseobject.h \
                Visualization/visualizationincludes.h \
                Clustering/groupingthread.h \
                Clustering/groupingsettings_general.h \
                Clustering/groupingsettingsincludes.h \
                Clustering/attributedata.h \
                Clustering/clusterinfoincludes.h \
                Clustering/clusters.h \
                Clustering/groupingdatapreparator.h \
                Clustering/groupingdatapreparator_rsesrules.h \
                Clustering/groupingsettings.h \
                Clustering/groupingsettings_detailed.h \
                Clustering/groupingsettings_rsesrules.h \
                generalsettings.h \
                generalincludes.h \
    Interference/interferencer.h \
    Object/attribute.h \
    Object/numericattribute.h \
    Object/categoricalattribute.h

FORMS    +=     Forms/about.ui \
                Forms/clusterinfo_rsesrules.ui \
                Forms/mainwindow.ui

TRANSLATIONS += polish.ts\
                english.ts
