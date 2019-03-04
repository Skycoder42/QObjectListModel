#ifndef QMETAOBJECTMODEL_H
#define QMETAOBJECTMODEL_H

#include <QtCore/QMetaProperty>

#include "qmodelaliasadapter.h"

class Q_QOBJECT_LIST_MODEL_EXPORT QMetaObjectModel : public QModelAliasBaseAdapter<>
{
	Q_OBJECT

	Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editableChanged)

public:
	explicit QMetaObjectModel(const QMetaObject *metaType,
							  QObject *parent = nullptr);

	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QHash<int, QByteArray> roleNames() const override;

	bool editable() const;

public Q_SLOTS:
	void setEditable(bool editable);

Q_SIGNALS:
	void editableChanged(bool editable, QPrivateSignal);

protected:
	QString defaultHeaderData() const override;
	QVariant originalData(const QModelIndex &index, int role) const override;
	bool setOriginalData(const QModelIndex &index, const QVariant &value, int role) override;

	virtual QVariant readProperty(int row, const QMetaProperty &property) const = 0;
	virtual bool writeProperty(int row, const QMetaProperty &property, const QVariant &data) = 0;
	virtual void registerSignalHelper(int role, const QMetaMethod &signal);

private:
	const QMetaObject *_metaObject;
	QHash<int, QByteArray> _roleNames;
	bool _editable = false;
};

#endif // QMETAOBJECTMODEL_H
