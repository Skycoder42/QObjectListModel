#include "qobjectlistmodel.h"
#include <QEvent>
#include <QDynamicPropertyChangeEvent>
#include <QMetaProperty>
#include "qobjectsignalhelper.h"

QObjectListModel::QObjectListModel(const QMetaObject *objectType, bool objectOwner, QObject *parent) :
	QMetaObjectModel{objectType, parent},
	_objectOwner{objectOwner}
{}

QObjectList QObjectListModel::objects() const
{
	return _objects;
}

QObject *QObjectListModel::object(const QModelIndex &index) const
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	return object(index.row());
}

QObject *QObjectListModel::object(int index) const
{
	return _objects[index];
}

QObject *QObjectListModel::takeObject(const QModelIndex &index)
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	return takeObject(index.row());
}

QObject *QObjectListModel::takeObject(int index)
{
	beginRemoveRows(QModelIndex{}, index, index);
	auto obj = _objects.takeAt(index);
	disconnectPropertyChanges(obj);
	if(_objectOwner && obj->parent() == this)
		obj->setParent(nullptr);
	endRemoveRows();
	return obj;
}

QObject *QObjectListModel::replaceObject(const QModelIndex &index, QObject *object)
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	return replaceObject(index.row(), object);
}

QObject *QObjectListModel::replaceObject(int index, QObject *object)
{
	// remove old object
	auto oldObj = _objects.at(index);
	disconnectPropertyChanges(oldObj);
	if(_objectOwner && oldObj->parent() == this)
		oldObj->setParent(nullptr);

	// add new object
	_objects[index] = object;
	if(_objectOwner)
		object->setParent(this);
	connectPropertyChanges(object);
	emitDataChanged(this->index(index, 0), this->index(index, 0), {});

	// return old object
	return oldObj;
}

int QObjectListModel::rowCount(const QModelIndex &parent) const
{
	Q_ASSERT(this->checkIndex(parent, QAbstractItemModel::CheckIndexOption::DoNotUseParent));
	if (parent.isValid())
		return 0;
	else
		return _objects.size();
}

QModelIndex QObjectListModel::index(QObject *object) const
{
	auto row = _objects.indexOf(object);
	if(row != -1)
		return index(row, 0);
	else
		return {};
}

void QObjectListModel::addObject(QObject *object)
{
	beginInsertRows(QModelIndex{}, _objects.size(), _objects.size());
	_objects.append(object);
	if(_objectOwner)
		object->setParent(this);
	connectPropertyChanges(object);
	endInsertRows();
}

void QObjectListModel::insertObject(const QModelIndex &index, QObject *object)
{
	if(index.isValid()) {
		Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
		insertObject(index.row(), object);
	} else
		addObject(object);
}

void QObjectListModel::insertObject(int index, QObject *object)
{
	beginInsertRows(QModelIndex{}, index, index);
	_objects.insert(index, object);
	if(_objectOwner)
		object->setParent(this);
	connectPropertyChanges(object);
	endInsertRows();
}

void QObjectListModel::removeObject(const QModelIndex &index)
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	removeObject(index.row());
}

void QObjectListModel::removeObject(int index)
{
	beginRemoveRows(QModelIndex{}, index, index);
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
	for(auto obj : qAsConst(_objects)) {
		if(_objectOwner &&
		   obj->parent() == this &&
		   !objects.contains(obj))
			obj->deleteLater();
		else
			disconnectPropertyChanges(obj);
	}

	_objects = std::move(objects);
	for(auto obj : qAsConst(_objects))
		connectPropertyChanges(obj);
	endResetModel();
}

QVariant QObjectListModel::readProperty(int row, const QMetaProperty &property) const
{
	return property.read(_objects[row]);
}

bool QObjectListModel::writeProperty(int row, const QMetaProperty &property, const QVariant &data)
{
	return property.write(_objects[row], data);
}

void QObjectListModel::registerSignalHelper(int role, const QMetaMethod &signal)
{
	_propertyHelpers.insert(new QObjectSignalHelper{role, signal, this});
}

void QObjectListModel::connectPropertyChanges(QObject *object)
{
	for(auto helper : qAsConst(_propertyHelpers))
		helper->addObject(object);
}

void QObjectListModel::disconnectPropertyChanges(QObject *object)
{
	for(auto helper : qAsConst(_propertyHelpers))
		helper->removeObject(object);
}

void QObjectListModel::triggerDataChange(QObject *object, int role)
{
	auto mIndex = index(object);
	if(mIndex.isValid())
		emitDataChanged(mIndex, mIndex, {role});
}
