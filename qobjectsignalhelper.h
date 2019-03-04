#ifndef QOBJECTSIGNALHELPER_H
#define QOBJECTSIGNALHELPER_H

#include "qobjectlistmodel.h"

#include <QMetaProperty>
#include <QObject>

class Q_QOBJECT_LIST_MODEL_EXPORT QObjectSignalHelper : public QObject
{
	Q_OBJECT
public:
	explicit QObjectSignalHelper(int role, QMetaMethod signal, QObjectListModel *parent = nullptr);

	void addObject(QObject *object);
	void removeObject(QObject *object);

private Q_SLOTS:
	void propertyChanged();

private:
	QObjectListModel *_model;
	const int _role;
	const QMetaMethod _signal;
};

#endif // QOBJECTSIGNALHELPER_H
