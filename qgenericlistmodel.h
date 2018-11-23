#ifndef QGENERICLISTMODEL_H
#define QGENERICLISTMODEL_H

#include <type_traits>
#include "qmetaobjectmodel.h"
#include "qobjectlistmodel.h"

template <typename TData, typename = void>
class QGenericListModel;

template <typename TObject>
using QGenericListModel_QObject_SFINAE = typename std::enable_if<std::is_base_of<QObject, TObject>::value>::type;

template <typename TObject>
class QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>> : public QObjectListModel
{
public:
	explicit QGenericListModel(QObject *parent = nullptr);
	explicit QGenericListModel(bool objectOwner, QObject *parent = nullptr);

	QList<TObject*> objects() const;
	TObject *object(const QModelIndex &index) const;
	TObject *object(int index) const;
	TObject *takeObject(const QModelIndex &index);
	TObject *takeObject(int index);

	void addObject(TObject *object);
	void insertObject(const QModelIndex &index, TObject *object);
	void insertObject(int index, TObject *object);
	void resetModel(const QList<TObject*> &objects);
};

template <typename TGadget>
class QGenericListModel<TGadget, typename TGadget::QtGadgetHelper> : public QMetaObjectModel
{
public:
	explicit QGenericListModel(QObject *parent = nullptr);

	QList<TGadget> gadgets() const;
	TGadget gadget(const QModelIndex &index) const;
	TGadget gadget(int index) const;
	void addGadget(const TGadget &gadget);
	void insertGadget(const QModelIndex &index, const TGadget &gadget);
	void insertGadget(int index, const TGadget &gadget);
	void removeGadget(const QModelIndex &index);
	void removeGadget(int index);
	TGadget takeGadget(const QModelIndex &index);
	TGadget takeGadget(int index);
	void resetModel(QList<TGadget> gadgets);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	using QMetaObjectModel::index;
	QModelIndex index(const TGadget &gadget) const;

protected:
	QVariant readProperty(int row, const QMetaProperty &property) const override;
	bool writeProperty(int row, const QMetaProperty &property, const QVariant &data) override;

private:
	QList<TGadget> _gadgets;
};

// ------------- Generic Implementation

template <typename TObject>
QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::QGenericListModel(QObject *parent) :
	QObjectListModel{&TObject::staticMetaObject, true, parent}
{}

template <typename TObject>
QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::QGenericListModel(bool objectOwner, QObject *parent) :
	QObjectListModel{&TObject::staticMetaObject, objectOwner, parent}
{}

template <typename TObject>
QList<TObject*> QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::objects() const
{
	const auto objs = this->QObjectListModel::objects();
	QList<TObject*> list;
	list.reserve(objs.size());
	for(auto obj : objs)
		list.append(qobject_cast<TObject*>(obj));
	return list;
}

template <typename TObject>
TObject *QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::object(const QModelIndex &index) const
{
	return qobject_cast<TObject*>(this->QObjectListModel::object(index));
}

template <typename TObject>
TObject *QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::object(int index) const
{
	return qobject_cast<TObject*>(this->QObjectListModel::object(index));
}

template <typename TObject>
TObject *QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::takeObject(const QModelIndex &index)
{
	return qobject_cast<TObject*>(this->QObjectListModel::takeObject(index));
}

template <typename TObject>
TObject *QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::takeObject(int index)
{
	return qobject_cast<TObject*>(this->QObjectListModel::takeObject(index));
}

template <typename TObject>
void QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::addObject(TObject *object)
{
	this->QObjectListModel::addObject(object);
}

template <typename TObject>
void QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::insertObject(const QModelIndex &index, TObject *object)
{
	this->QObjectListModel::insertObject(index, object);
}

template <typename TObject>
void QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::insertObject(int index, TObject *object)
{
	this->QObjectListModel::insertObject(index, object);
}

template <typename TObject>
void QGenericListModel<TObject, QGenericListModel_QObject_SFINAE<TObject>>::resetModel(const QList<TObject*> &objects)
{
	QObjectList list;
	list.reserve(objects.size());
	for(auto obj : objects)
		list.append(obj);
	this->QObjectListModel::resetModel(std::move(list));
}



template<typename TGadget>
QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::QGenericListModel(QObject *parent) :
	QMetaObjectModel{&TGadget::staticMetaObject, parent}
{}

template<typename TGadget>
QList<TGadget> QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::gadgets() const
{
	return _gadgets;
}

template<typename TGadget>
TGadget QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::gadget(const QModelIndex &index) const
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	return gadget(index.row());
}

template<typename TGadget>
TGadget QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::gadget(int index) const
{
	return _gadgets[index];
}

template<typename TGadget>
void QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::addGadget(const TGadget &gadget)
{
	beginInsertRows(QModelIndex{}, _gadgets.size(), _gadgets.size());
	_gadgets.append(gadget);
	endInsertRows();
}

template<typename TGadget>
void QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::insertGadget(const QModelIndex &index, const TGadget &gadget)
{
	if(index.isValid()) {
		Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
		insertGadget(index.row(), gadget);
	} else
		addGadget(gadget);
}

template<typename TGadget>
void QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::insertGadget(int index, const TGadget &gadget)
{
	beginInsertRows(QModelIndex{}, _gadgets.size(), _gadgets.size());
	_gadgets.insert(index, gadget);
	endInsertRows();
}

template<typename TGadget>
void QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::removeGadget(const QModelIndex &index)
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	removeGadget(index.row());
}

template<typename TGadget>
void QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::removeGadget(int index)
{
	beginRemoveRows(QModelIndex{}, index, index);
	_gadgets.removeAt(index);
	endRemoveRows();
}

template<typename TGadget>
TGadget QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::takeGadget(const QModelIndex &index)
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	return takeGadget(index.row());
}

template<typename TGadget>
TGadget QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::takeGadget(int index)
{
	beginRemoveRows(QModelIndex{}, index, index);
	const auto gadget = _gadgets.takeAt(index);
	endRemoveRows();
	return gadget;
}

template<typename TGadget>
void QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::resetModel(QList<TGadget> gadgets)
{
	beginResetModel();
	_gadgets = std::move(gadgets);
	endResetModel();
}

template<typename TGadget>
int QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::rowCount(const QModelIndex &parent) const
{
	Q_ASSERT(this->checkIndex(parent, QAbstractItemModel::CheckIndexOption::DoNotUseParent));
	if (parent.isValid())
		return 0;
	else
		return _gadgets.size();
}

template<typename TGadget>
QModelIndex QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::index(const TGadget &gadget) const
{
	auto row = _gadgets.indexOf(gadget);
	if(row != -1)
		return index(row, 0);
	else
		return {};
}

template<typename TGadget>
QVariant QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::readProperty(int row, const QMetaProperty &property) const
{
	return property.readOnGadget(&(_gadgets[row]));
}

template<typename TGadget>
bool QGenericListModel<TGadget, typename TGadget::QtGadgetHelper>::writeProperty(int row, const QMetaProperty &property, const QVariant &data)
{
	return property.writeOnGadget(&(_gadgets[row]), data);
}

#endif // QGENERICLISTMODEL_H
