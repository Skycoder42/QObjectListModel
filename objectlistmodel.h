#ifndef OBJECTLISTMODEL_H
#define OBJECTLISTMODEL_H

#include <QAbstractListModel>

class ObjectSignalHelper;
class ObjectListModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editableChanged)

public:
	explicit ObjectListModel(const QMetaObject *objectType, bool objectOwner, QObject *parent = nullptr);

	QObjectList objects() const;
	QObject *object(const QModelIndex &index) const;
	QObject *object(int index) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	using QAbstractListModel::index;
	QModelIndex index(QObject *object) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value,
				 int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QHash<int, QByteArray> roleNames() const override;

	bool editable() const;

public slots:
	void addObject(QObject *object);
	void insertObject(const QModelIndex &index, QObject *object);
	void insertObject(int index, QObject *object);
	void removeObject(const QModelIndex &index);
	void removeObject(int index);
	void resetModel(QObjectList objects);

	void setEditable(bool editable);

signals:
	void editableChanged(bool editable);

protected:
	bool testValid(const QModelIndex &index, int role = -1) const;

private slots:
	void objectPropertyChanged();

private:
	bool _objectOwner;
	const QMetaObject *_metaObject;
	QHash<int, QByteArray> _roleNames;
	bool _editable;
	QObjectList _objects;
	QHash<int, ObjectSignalHelper*> _propertyHelpers;

	void connectPropertyChanges(QObject *object);
	void disconnectPropertyChanges(QObject *object);
};

Q_DECLARE_METATYPE(ObjectListModel*)

template <typename T>
class GenericListModel : public ObjectListModel
{
	static_assert(std::is_base_of<QObject, T>::value, "T must inherit QObject!");
public:
	explicit GenericListModel(bool objectOwner, QObject *parent = nullptr);

	QList<T*> objects() const;
	T *object(const QModelIndex &index) const;
	T *object(int index) const;

	void addObject(T *object);
	void insertObject(const QModelIndex &index, T *object);
	void insertObject(int index, T *object);
	void resetModel(QList<T*> objects);
};

// ------------- Generic Implementation

template<typename T>
GenericListModel<T>::GenericListModel(bool objectOwner, QObject *parent) :
	ObjectListModel(&T::staticMetaObject, objectOwner, parent)
{}

template<typename T>
QList<T*> GenericListModel<T>::objects() const
{
	QList<T*> list;
	foreach(auto obj, ObjectListModel::objects())
		list.append(qobject_cast<T*>(obj));
	return list;
}

template<typename T>
T *GenericListModel<T>::object(const QModelIndex &index) const
{
	return qobject_cast<T*>(ObjectListModel::object(index));
}

template<typename T>
T *GenericListModel<T>::object(int index) const
{
	return qobject_cast<T*>(ObjectListModel::object(index));
}

template<typename T>
void GenericListModel<T>::addObject(T *object)
{
	ObjectListModel::addObject(object);
}

template<typename T>
void GenericListModel<T>::insertObject(const QModelIndex &index, T *object)
{
	ObjectListModel::insertObject(index, object);
}

template<typename T>
void GenericListModel<T>::insertObject(int index, T *object)
{
	ObjectListModel::insertObject(index, object);
}

template<typename T>
void GenericListModel<T>::resetModel(QList<T*> objects)
{
	QObjectList list;
	foreach(auto obj, objects)
		list.append(obj);
	ObjectListModel::resetModel(list);
}

#endif // OBJECTLISTMODEL_H
