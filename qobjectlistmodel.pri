HEADERS += \
	$$PWD/qobjectlistmodel.h \
	$$PWD/qobjectproxymodel.h \
	$$PWD/qobjectsignalhelper.h \
	$$PWD/qgadgetlistmodel.h \
	$$PWD/qmodelaliasadapter.h \
	$$PWD/qmetaobjectmodel.h \
	$$PWD/qgenericlistmodel.h

SOURCES += \
	$$PWD/qobjectlistmodel.cpp \
	$$PWD/qobjectproxymodel.cpp \
	$$PWD/qobjectsignalhelper.cpp \
	$$PWD/qmetaobjectmodel.cpp

INCLUDEPATH += $$PWD

QDEP_PACKAGE_EXPORTS += Q_QOBJECT_LIST_MODEL_EXPORT
!qdep_build: DEFINES += "Q_QOBJECT_LIST_MODEL_EXPORT="
