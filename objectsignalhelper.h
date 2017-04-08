#ifndef OBJECTSIGNALHELPER_H
#define OBJECTSIGNALHELPER_H

#include "objectlistmodel.h"

#include <QMetaProperty>
#include <QObject>

class ObjectSignalHelper : public QObject
{
	Q_OBJECT
public:
	explicit ObjectSignalHelper(int role, const QMetaMethod &signal, ObjectListModel *parent = nullptr);

	void addObject(QObject *object);
	void removeObject(QObject *object);

private slots:
	void propertyChanged();

private:
	ObjectListModel *model;
	const int role;
	const QMetaMethod signal;
};

#endif // OBJECTSIGNALHELPER_H
