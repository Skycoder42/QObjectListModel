#ifndef QOBJECTPROXYMODEL_H
#define QOBJECTPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QObject>
#include "qobjectlistmodel.h"

class QObjectProxyModel : public QAbstractProxyModel
{
	Q_OBJECT

public:
	explicit QObjectProxyModel(QStringList headers, QObject *parent = nullptr);

	void appendColumn(const QString &text);
	void insertColumn(int index, const QString &text);
	void replaceColumn(int index, const QString &text);
	void removeColumn(int index);

	void addMapping(int column, int role, int sourceRole);
	bool addMapping(int column, int role, const char *sourceRoleName);
	void setRoleName(int role, const QByteArray &name);
	void setExtraFlags(int column, Qt::ItemFlags extraFlags);

public:
	QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
	QModelIndex parent(const QModelIndex &) const override;
	int columnCount(const QModelIndex &parent = {}) const override;
	int rowCount(const QModelIndex& parent = {}) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	QModelIndex sibling(int row, int column, const QModelIndex &index) const override;

	QItemSelection mapSelectionFromSource(const QItemSelection& selection) const override;
	QItemSelection mapSelectionToSource(const QItemSelection& selection) const override;
	QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const override;
	void setSourceModel(QAbstractListModel *sourceModel);
	QAbstractListModel *sourceModel() const;
	QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

private:
	QStringList _headers;
	QHash<QPair<int, int>, int> _roleMapping;
	QHash<int, Qt::ItemFlags> _extraFlags;
	QHash<int, QByteArray> _extraRoles;

	void setSourceModel(QAbstractItemModel *sourceModel) override;
	static QByteArray defaultRoleName(int role);
	void reloadRoles();
};

#endif // QOBJECTPROXYMODEL_H
