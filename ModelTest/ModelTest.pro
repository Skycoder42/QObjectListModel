TEMPLATE = app

QT  += core gui widgets quickwidgets

DEFINES += QT_DEPRECATED_WARNINGS

TARGET = ModelTest

SOURCES += main.cpp\
	mainwindow.cpp \
	testobject.cpp \
	testgadget.cpp

HEADERS += mainwindow.h \
	testobject.h \
	testgadget.h

FORMS += \
	mainwindow.ui

RESOURCES += \
	qml.qrc

include(../qobjectlistmodel.pri)
