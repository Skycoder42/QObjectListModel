#ifndef OBJECTPROXYMODEL_H
#define OBJECTPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QObject>
#include "objectlistmodel.h"

class ObjectProxyModel : public QIdentityProxyModel
{
	Q_OBJECT

public:
	explicit ObjectProxyModel(QStringList headers, QObject *parent = nullptr);

	void appendColumn(const QString &text);
	void insertColumn(int index, const QString &text);
	void addMapping(int column, int role, int sourceRole);
	bool addMapping(int column, int role, const char *sourceRoleName);
	void setRoleName(int role, const QByteArray &name);

public:
	QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
	QModelIndex parent(const QModelIndex &) const override;
	int columnCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	void setSourceModel(ObjectListModel *sourceModel);
	ObjectListModel *sourceModel() const;
	QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

private:
	QStringList _headers;
	QHash<QPair<int, int>, int> _roleMapping;
	QHash<int, QByteArray> _extraRoles;

	void setSourceModel(QAbstractItemModel *sourceModel) override;
	static QByteArray defaultRoleName(int role);
	void reloadRoles();
};

#endif // OBJECTPROXYMODEL_H
