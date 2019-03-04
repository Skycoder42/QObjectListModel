#ifndef QOBJECTPROXYMODEL_H
#define QOBJECTPROXYMODEL_H

#include <QIdentityProxyModel>
#include "qmodelaliasadapter.h"

class Q_QOBJECT_LIST_MODEL_EXPORT QObjectProxyModel : public QModelAliasBaseAdapter<QIdentityProxyModel>
{
	Q_OBJECT

public:
	explicit QObjectProxyModel(QObject *parent = nullptr);
	explicit QObjectProxyModel(const QStringList &headers, QObject *parent = nullptr);

public:
	QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
	QModelIndex parent(const QModelIndex &) const override;
	int rowCount(const QModelIndex& parent = {}) const override;
	QHash<int, QByteArray> roleNames() const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	QModelIndex sibling(int row, int column, const QModelIndex &index) const override;

	QItemSelection mapSelectionFromSource(const QItemSelection& selection) const override;
	QItemSelection mapSelectionToSource(const QItemSelection& selection) const override;
	QModelIndexList match(const QModelIndex& start, int role, const QVariant& value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const override;
	void setSourceModel(QAbstractItemModel *sourceModel) override;
	QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

protected:
	QVariant originalData(const QModelIndex &index, int role) const override;
	bool setOriginalData(const QModelIndex &index, const QVariant &value, int role) override;

private Q_SLOTS:
	void extendDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
};

#endif // QOBJECTPROXYMODEL_H
