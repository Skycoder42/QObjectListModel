#ifndef OBJECTSIGNALHELPER_H
#define OBJECTSIGNALHELPER_H

#include "qobjectlistmodel.h"

#include <QMetaProperty>
#include <QObject>

class ObjectSignalHelper : public QObject
{
	Q_OBJECT
public:
	explicit ObjectSignalHelper(int role, const QMetaMethod &signal, QObjectListModel *parent = nullptr);

	void addObject(QObject *object);
	void removeObject(QObject *object);

private slots:
	void propertyChanged();

private:
	QObjectListModel *_model;
	const int _role;
	const QMetaMethod _signal;
};

#endif // OBJECTSIGNALHELPER_H
