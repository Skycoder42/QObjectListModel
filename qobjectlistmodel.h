#ifndef QOBJECTLISTMODEL_H
#define QOBJECTLISTMODEL_H

#include <QAbstractListModel>

class QObjectSignalHelper;
class QObjectListModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editableChanged)

public:
	explicit QObjectListModel(const QMetaObject *objectType,
							  bool objectOwner,
							  QObject *parent = nullptr);
	explicit QObjectListModel(const QMetaObject *objectType,
							  const QByteArrayList extraProperties,
							  bool objectOwner,
							  QObject *parent = nullptr);

	QObjectList objects() const;
	QObject *object(const QModelIndex &index) const;
	QObject *object(int index) const;
	QObject *takeObject(const QModelIndex &index);
	QObject *takeObject(int index);

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
	void resetModel(QObjectList objects = {});

	void setEditable(bool editable);

signals:
	void editableChanged(bool editable);

protected:
	bool testValid(const QModelIndex &index, int role = -1) const;

private:
	bool _objectOwner;
	const QMetaObject *_metaObject;
	QHash<int, QByteArray> _roleNames;
	bool _editable;
	QObjectList _objects;
	QHash<int, QObjectSignalHelper*> _propertyHelpers;
	QByteArrayList _extraProperties;

	void connectPropertyChanges(QObject *object);
	void disconnectPropertyChanges(QObject *object);

	bool eventFilter(QObject *watched, QEvent *event) override;
};

Q_DECLARE_METATYPE(QObjectListModel*)

template <typename T>
class QGenericListModel : public QObjectListModel
{
	static_assert(std::is_base_of<QObject, T>::value, "T must inherit QObject!");

public:
	explicit QGenericListModel(bool objectOwner, QObject *parent = nullptr);
	explicit QGenericListModel(const QByteArrayList extraProperties, bool objectOwner, QObject *parent = nullptr);

	QList<T*> objects() const;
	T *object(const QModelIndex &index) const;
	T *object(int index) const;
	T *takeObject(const QModelIndex &index);
	T *takeObject(int index);

	void addObject(T *object);
	void insertObject(const QModelIndex &index, T *object);
	void insertObject(int index, T *object);
	void resetModel(QList<T*> objects);
};

// ------------- Generic Implementation

template<typename T>
QGenericListModel<T>::QGenericListModel(bool objectOwner, QObject *parent) :
	QObjectListModel(&T::staticMetaObject, objectOwner, parent)
{}

template<typename T>
QGenericListModel<T>::QGenericListModel(const QByteArrayList extraProperties, bool objectOwner, QObject *parent) :
	QObjectListModel(&T::staticMetaObject, extraProperties, objectOwner, parent)
{}

template<typename T>
QList<T*> QGenericListModel<T>::objects() const
{
	QList<T*> list;
	foreach(auto obj, QObjectListModel::objects())
		list.append(qobject_cast<T*>(obj));
	return list;
}

template<typename T>
T *QGenericListModel<T>::object(const QModelIndex &index) const
{
	return qobject_cast<T*>(QObjectListModel::object(index));
}

template<typename T>
T *QGenericListModel<T>::object(int index) const
{
	return qobject_cast<T*>(QObjectListModel::object(index));
}

template<typename T>
T *QGenericListModel<T>::takeObject(const QModelIndex &index)
{
	return qobject_cast<T*>(QObjectListModel::takeObject(index));
}

template<typename T>
T *QGenericListModel<T>::takeObject(int index)
{
	return qobject_cast<T*>(QObjectListModel::takeObject(index));
}

template<typename T>
void QGenericListModel<T>::addObject(T *object)
{
	QObjectListModel::addObject(object);
}

template<typename T>
void QGenericListModel<T>::insertObject(const QModelIndex &index, T *object)
{
	QObjectListModel::insertObject(index, object);
}

template<typename T>
void QGenericListModel<T>::insertObject(int index, T *object)
{
	QObjectListModel::insertObject(index, object);
}

template<typename T>
void QGenericListModel<T>::resetModel(QList<T*> objects)
{
	QObjectList list;
	foreach(auto obj, objects)
		list.append(obj);
	QObjectListModel::resetModel(list);
}

#endif // QOBJECTLISTMODEL_H
