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
                Interference/clusterCoverageInferer.cpp \
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
                generalsettings.cpp \
                Object/attribute.cpp \
                Object/numericattribute.cpp \
                Object/categoricalattribute.cpp \
                Validators/rsesrulebasevalidator.cpp \
                Interference/clusterInterferencer.cpp \
                Interference/classicalInterferencer.cpp


HEADERS  +=     Forms/about.h \
                Forms/clusterinfo_rsesrules.h \
                Forms/mainwindow.h \
                Interference/clusterCoverageInferer.h \
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
                Object/categoricalattribute.h \
                Validators/validator.h \
                Validators/rsesrulebasevalidator.h \
                Interference/clusterInterferencer.h \
                Interference/classicalInterferencer.h

FORMS    +=     Forms/about.ui \
                Forms/clusterinfo_rsesrules.ui \
                Forms/mainwindow.ui

TRANSLATIONS = polish.ts\
               english.ts
