#include "qobjectsignalhelper.h"
#include <QVector>

QObjectSignalHelper::QObjectSignalHelper(int role, const QMetaMethod &signal, QObjectListModel *parent) :
	QObject(parent),
	_model(parent),
	_role(role),
	_signal(signal)
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
	auto index = _model->index(sender());
	if(index.isValid())
		emit _model->dataChanged(index, index, {_role});
}
