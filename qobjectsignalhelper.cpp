#include "qobjectsignalhelper.h"
#include <QVector>

QObjectSignalHelper::QObjectSignalHelper(int role, QMetaMethod signal, QObjectListModel *parent) :
	QObject{parent},
	_model{parent},
	_role{role},
	_signal{std::move(signal)}
{}

void QObjectSignalHelper::addObject(QObject *object)
{
	auto slot = metaObject()->method(metaObject()->indexOfSlot("propertyChanged()"));
	connect(object, _signal, this, slot);
}

void QObjectSignalHelper::removeObject(QObject *object)
{
	object->disconnect(this);
}

void QObjectSignalHelper::propertyChanged()
{
	_model->triggerDataChange(sender(), _role);
}
