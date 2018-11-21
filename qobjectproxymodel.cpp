#include "qobjectproxymodel.h"

#include <QDebug>
#include <QItemSelection>

QObjectProxyModel::QObjectProxyModel(QObject *parent) :
	QModelAliasBaseAdapter{parent}
{}

QObjectProxyModel::QObjectProxyModel(const QStringList &headers, QObject *parent) :
	QModelAliasBaseAdapter{parent}
{
	for(const auto &header : headers)
		addColumn(header);
}

void QObjectProxyModel::setExtraFlags(int column, Qt::ItemFlags extraFlags)
{
	beginResetModel();
	_extraFlags.insert(column, extraFlags);
	endResetModel();
}

QModelIndex QObjectProxyModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_ASSERT(checkIndex(parent, CheckIndexOption::NoOption));
	if(parent.isValid())
		return {};
	else {
		if(row < 0 || row >= rowCount() ||
		   column < 0 || column >= columnCount())
			return {};
		else
			return createIndex(row, column);
	}
}

QModelIndex QObjectProxyModel::parent(const QModelIndex &) const
{
	return {};
}

int QObjectProxyModel::rowCount(const QModelIndex &parent) const
{
	Q_ASSERT(this->checkIndex(parent, QAbstractItemModel::CheckIndexOption::DoNotUseParent));
	if(!sourceModel())
		return 0;
	else
		return sourceModel()->rowCount(mapToSource(parent));
}

QHash<int, QByteArray> QObjectProxyModel::roleNames() const
{
	return sourceModel() ? sourceModel()->roleNames() : QHash<int, QByteArray>{};
}

Qt::ItemFlags QObjectProxyModel::flags(const QModelIndex &index) const
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::NoOption));
	auto flags = QIdentityProxyModel::flags(index);
	flags |= _extraFlags.value(index.column(), 0);
	flags &= ~Qt::ItemIsEditable;//disable editing because it does not work
	return flags;
}

bool QObjectProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_ASSERT(checkIndex(parent, CheckIndexOption::DoNotUseParent));
	Q_UNUSED(column);
	if(sourceModel())
		return sourceModel()->dropMimeData(data, action, row, 0, mapToSource(parent));
	else
		return false;
}

QModelIndex QObjectProxyModel::sibling(int row, int column, const QModelIndex &index) const
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid));
	if(!sourceModel())
		return {};
	auto tIndex = mapToSource(index);
	tIndex = sourceModel()->sibling(row, 0, tIndex);
	tIndex = mapFromSource(tIndex);
	return createIndex(tIndex.row(), column);
}

QItemSelection QObjectProxyModel::mapSelectionFromSource(const QItemSelection &selection) const
{
	QItemSelection proxySelection;

	if (!sourceModel())
		return proxySelection;

	proxySelection.reserve(selection.count());
	for (auto it = selection.constBegin(); it != selection.constEnd(); ++it) {
		const QItemSelectionRange range(mapFromSource(it->topLeft()), mapFromSource(it->bottomRight()));
		proxySelection.append(range);
	}

	return proxySelection;
}

QItemSelection QObjectProxyModel::mapSelectionToSource(const QItemSelection &selection) const
{
	QItemSelection sourceSelection;

	if (!sourceModel())
		return sourceSelection;

	sourceSelection.reserve(selection.count());
	for (auto it = selection.constBegin(); it != selection.constEnd(); ++it) {
		const QItemSelectionRange range(mapToSource(it->topLeft()), mapToSource(it->bottomRight()));
		sourceSelection.append(range);
	}

	return sourceSelection;
}

QModelIndexList QObjectProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
	Q_ASSERT(checkIndex(start, CheckIndexOption::NoOption));

	if (!sourceModel())
		return {};

	const auto sourceList = sourceModel()->match(mapToSource(start), role, value, hits, flags);
	QModelIndexList proxyList;
	proxyList.reserve(sourceList.count());
	for (auto it = sourceList.constBegin(); it != sourceList.constEnd(); ++it)
		proxyList.append(mapFromSource(*it));
	return proxyList;
}

void QObjectProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	if(sourceModel->columnCount() > 0)
		qWarning() << "sourceModel has more than 1 column! This can lead to undefined behaviour!";

	if(this->sourceModel()) {
		disconnect(this->sourceModel(), &QAbstractItemModel::dataChanged,
				   this, &QObjectProxyModel::extendDataChanged);
	}

	QIdentityProxyModel::setSourceModel(sourceModel);

	disconnect(this->sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
			   this, SLOT(_q_sourceDataChanged(QModelIndex,QModelIndex,QVector<int>)));
	connect(this->sourceModel(), &QAbstractItemModel::dataChanged,
			this, &QObjectProxyModel::extendDataChanged);
}

QModelIndex QObjectProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
	Q_ASSERT(checkIndex(proxyIndex, CheckIndexOption::NoOption));
	if(!sourceModel())
		return {};
	if(!proxyIndex.isValid() || proxyIndex.parent().isValid())
		return {};
	else
		return sourceModel()->index(proxyIndex.row(), 0);
}

QModelIndex QObjectProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
	Q_ASSERT(checkIndex(sourceIndex, CheckIndexOption::NoOption));
	if(!sourceModel())
		return {};
	if(!sourceIndex.isValid() || sourceIndex.parent().isValid())
		return {};
	else
		return index(sourceIndex.row(), 0);
}

QVariant QObjectProxyModel::originalData(const QModelIndex &index, int role) const
{
	if(!sourceModel())
		return {};
	auto src = mapToSource(index);
	return sourceModel()->data(src, role);
}

bool QObjectProxyModel::setOriginalData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!sourceModel())
		return false;
	auto src = mapToSource(index);
	return sourceModel()->setData(src, value, role);
}

void QObjectProxyModel::extendDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
	emitDataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}
