#include "qobjectlistmodel.h"
#include <QEvent>
#include <QDynamicPropertyChangeEvent>
#include <QMetaProperty>
#include "qobjectsignalhelper.h"

QObjectListModel::QObjectListModel(const QMetaObject *objectType, bool objectOwner, QObject *parent) :
	QObjectListModel(objectType, {}, objectOwner, parent)
{}

QObjectListModel::QObjectListModel(const QMetaObject *objectType, const QByteArrayList extraProperties, bool objectOwner, QObject *parent) :
	QAbstractListModel(parent),
	_objectOwner(objectOwner),
	_metaObject(objectType),
	_roleNames(),
	_editable(false),
	_objects(),
	_propertyHelpers(),
	_extraProperties(extraProperties)
{
	_roleNames.insert(Qt::DisplayRole, _metaObject->property(0).name());//property 0 is the objectName property
	_roleNames.insert(Qt::EditRole, _metaObject->property(0).name());//allow editing via simple role
	_propertyHelpers.insert(0, new QObjectSignalHelper(Qt::DisplayRole, _metaObject->property(0).notifySignal(), this));

	auto roleIndex = Qt::UserRole + 1;
	for(auto i = 1; i < _metaObject->propertyCount(); i++) {
		auto prop = _metaObject->property(i);
		if(prop.hasNotifySignal())
			_propertyHelpers.insert(i, new QObjectSignalHelper(roleIndex, prop.notifySignal(), this));
		_roleNames.insert(roleIndex++, prop.name());
	}

	foreach(auto extra, extraProperties)
		_roleNames.insert(roleIndex++, extra);
}

QObjectList QObjectListModel::objects() const
{
	return _objects;
}

QObject *QObjectListModel::object(const QModelIndex &index) const
{
	return object(index.row());
}

QObject *QObjectListModel::object(int index) const
{
	return _objects[index];
}

QObject *QObjectListModel::takeObject(const QModelIndex &index)
{
	return takeObject(index.row());
}

QObject *QObjectListModel::takeObject(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	auto obj = _objects.takeAt(index);
	disconnectPropertyChanges(obj);
	if(_objectOwner && obj->parent() == this)
		obj->setParent(nullptr);
	endRemoveRows();
	return obj;
}

QVariant QObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return QString::fromUtf8(_metaObject->className());
	else
		return {};
}

int QObjectListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return _objects.size();
}

QModelIndex QObjectListModel::index(QObject *object) const
{
	auto idx = _objects.indexOf(object);
	if(idx != -1)
		return index(idx);
	else
		return {};
}

QVariant QObjectListModel::data(const QModelIndex &index, int role) const
{
	if (!testValid(index, role))
		return {};

	return _objects[index.row()]->property(_roleNames[role].constData());
}

bool QObjectListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!_editable || !flags(index).testFlag(Qt::ItemIsEditable))
		return false;
	if (!testValid(index, role))
		return false;

	if (data(index, role) != value)
		return _objects[index.row()]->setProperty(_roleNames[role].constData(), value);

	return false;
}

Qt::ItemFlags QObjectListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	auto flags = Qt::ItemIsEnabled |
				 Qt::ItemIsSelectable |
				 Qt::ItemIsDragEnabled |
				 Qt::ItemNeverHasChildren;
	if(_editable)
		flags |= Qt::ItemIsEditable;
	return flags;
}

QHash<int, QByteArray> QObjectListModel::roleNames() const
{
	return _roleNames;
}

bool QObjectListModel::editable() const
{
	return _editable;
}

void QObjectListModel::addObject(QObject *object)
{
	beginInsertRows(QModelIndex(), _objects.size(), _objects.size());
	_objects.append(object);
	if(_objectOwner)
		object->setParent(this);
	connectPropertyChanges(object);
	endInsertRows();
}

void QObjectListModel::insertObject(const QModelIndex &index, QObject *object)
{
	insertObject(index.row(), object);
}

void QObjectListModel::insertObject(int index, QObject *object)
{
	beginInsertRows(QModelIndex(), index, index);
	_objects.insert(index, object);
	if(_objectOwner)
		object->setParent(this);
	connectPropertyChanges(object);
	endInsertRows();
}

void QObjectListModel::removeObject(const QModelIndex &index)
{
	removeObject(index.row());
}

void QObjectListModel::removeObject(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	auto obj = _objects.takeAt(index);
	if(_objectOwner && obj->parent() == this)
		obj->deleteLater();
	else
		disconnectPropertyChanges(obj);
	endRemoveRows();
}

void QObjectListModel::resetModel(QObjectList objects)
{
	beginResetModel();
	foreach(auto obj, _objects) {
		if(_objectOwner && obj->parent() == this)
			obj->deleteLater();
		else
			disconnectPropertyChanges(obj);
	}

	_objects = objects;
	foreach(auto obj, objects)
		connectPropertyChanges(obj);
	endResetModel();
}

void QObjectListModel::setEditable(bool editable)
{
	if (_editable == editable)
		return;

	_editable = editable;
	emit editableChanged(editable);
}

bool QObjectListModel::testValid(const QModelIndex &index, int role) const
{
	return index.isValid() &&
			index.column() == 0 &&
			index.row() < _objects.size() &&
			(role < 0 || _roleNames.contains(role));
}

void QObjectListModel::connectPropertyChanges(QObject *object)
{
	for(auto i = 1; i < _metaObject->propertyCount(); i++) {
		auto helper = _propertyHelpers.value(i, nullptr);
		if(helper)
			helper->addObject(object);
	}

	if(!_extraProperties.isEmpty())
		object->installEventFilter(this);
}

void QObjectListModel::disconnectPropertyChanges(QObject *object)
{
	for(auto i = 1; i < _metaObject->propertyCount(); i++) {
		auto helper = _propertyHelpers.value(i, nullptr);
		if(helper)
			helper->removeObject(object);
	}

	if(!_extraProperties.isEmpty())
		object->removeEventFilter(this);
}

bool QObjectListModel::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::DynamicPropertyChange) {
		auto ev = static_cast<QDynamicPropertyChangeEvent*>(event);
		if(_extraProperties.contains(ev->propertyName())) {
			auto role = _roleNames.key(ev->propertyName(), -1);
			if(role != -1) {
				auto modelIndex = index(watched);
				if(modelIndex.isValid())
					emit dataChanged(modelIndex, modelIndex, {role});
			}
		}
	}

	//don't consume events, simply use it
	return false;
}
