#include "objectsignalhelper.h"
#include <QVector>

ObjectSignalHelper::ObjectSignalHelper(int role, const QMetaMethod &signal, ObjectListModel *parent) :
	QObject(parent),
	model(parent),
	role(role),
	signal(signal)
{}

void ObjectSignalHelper::addObject(QObject *object)
{
	auto slot = metaObject()->method(metaObject()->indexOfSlot("propertyChanged()"));
	connect(object, signal, this, slot);
}

void ObjectSignalHelper::removeObject(QObject *object)
{
	object->disconnect(this);
}

void ObjectSignalHelper::propertyChanged()
{
	auto index = model->index(sender());
	if(index.isValid())
		emit model->dataChanged(index, index, {role});
}
