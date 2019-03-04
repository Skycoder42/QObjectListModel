#ifndef QOBJECTLISTMODEL_H
#define QOBJECTLISTMODEL_H

#include "qmetaobjectmodel.h"

class QObjectSignalHelper;
class Q_QOBJECT_LIST_MODEL_EXPORT QObjectListModel : public QMetaObjectModel
{
	Q_OBJECT

public:
	explicit QObjectListModel(const QMetaObject *objectType,
							  bool objectOwner = true,
							  QObject *parent = nullptr);

	Q_INVOKABLE QObjectList objects() const;
	QObject *object(const QModelIndex &index) const;
	Q_INVOKABLE QObject *object(int index) const;
	QObject *takeObject(const QModelIndex &index);
	Q_INVOKABLE QObject *takeObject(int index);
	QObject *replaceObject(const QModelIndex &index, QObject *object);
	Q_INVOKABLE QObject *replaceObject(int index, QObject *object);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	using QMetaObjectModel::index;
	QModelIndex index(QObject *object) const;

public Q_SLOTS:
	void addObject(QObject *object);
	void insertObject(const QModelIndex &index, QObject *object);
	void insertObject(int index, QObject *object);
	void removeObject(const QModelIndex &index);
	void removeObject(int index);
	void resetModel(QObjectList objects = {});

protected:
	QVariant readProperty(int row, const QMetaProperty &property) const override;
	bool writeProperty(int row, const QMetaProperty &property, const QVariant &data) override;
	void registerSignalHelper(int role, const QMetaMethod &signal) override;

private:
	friend class QObjectSignalHelper;
	bool _objectOwner;
	QObjectList _objects;
	QSet<QObjectSignalHelper*> _propertyHelpers;

	void connectPropertyChanges(QObject *object);
	void disconnectPropertyChanges(QObject *object);
	void triggerDataChange(QObject *object, int role);
};

Q_DECLARE_METATYPE(QObjectListModel*)

#endif // QOBJECTLISTMODEL_H
