#include "qobjectproxymodel.h"

#include <QItemSelection>

QObjectProxyModel::QObjectProxyModel(QStringList headers, QObject *parent) :
	QAbstractProxyModel(parent),
	_headers(headers),
	_roleMapping(),
	_extraFlags(),
	_extraRoles()
{}

void QObjectProxyModel::appendColumn(const QString &text)
{
	beginInsertColumns(QModelIndex(), _headers.size(), _headers.size());
	_headers.append(text);
	endInsertColumns();
}

void QObjectProxyModel::insertColumn(int index, const QString &text)
{
	beginInsertColumns(QModelIndex(), index, index);
	_headers.insert(index, text);
	endInsertColumns();
}

void QObjectProxyModel::replaceColumn(int index, const QString &text)
{
	_headers[index] = text;
	emit headerDataChanged(Qt::Horizontal, index, index);
}

void QObjectProxyModel::removeColumn(int index)
{
	beginRemoveColumns(QModelIndex(), index, index);
	_headers.removeAt(index);
	endRemoveColumns();
}

void QObjectProxyModel::addMapping(int column, int role, int sourceRole)
{
	beginResetModel();
	Q_ASSERT(column < _headers.size());
	_roleMapping.insert({column, role}, sourceRole);
	if(!_extraRoles.contains(role))
		_extraRoles.insert(role, defaultRoleName(role));
	endResetModel();
}

bool QObjectProxyModel::addMapping(int column, int role, const char *sourceRoleName)
{
	if(!sourceModel())
		return false;

	auto sRole = sourceModel()->roleNames().key(sourceRoleName, -1);
	if(sRole == -1)
		return false;
	else {
		addMapping(column, role, sRole);
		return true;
	}
}

void QObjectProxyModel::setRoleName(int role, const QByteArray &name)
{
	beginResetModel();
	_extraRoles.insert(role, name);
	endResetModel();
}

void QObjectProxyModel::setExtraFlags(int column, Qt::ItemFlags extraFlags)
{
	beginResetModel();
	_extraFlags.insert(column, extraFlags);
	endResetModel();
}

QModelIndex QObjectProxyModel::index(int row, int column, const QModelIndex &parent) const
{
	if(parent.isValid())
		return {};
	else {
		if(row < 0 ||
		   column < 0 ||
		   row >= rowCount() ||
		   column >= columnCount())
			return {};
		else
			return createIndex(row, column);
	}
}

QModelIndex QObjectProxyModel::parent(const QModelIndex &) const
{
	return {};
}

int QObjectProxyModel::columnCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;
	else
		return _headers.size();
}

int QObjectProxyModel::rowCount(const QModelIndex &parent) const
{
	if(!sourceModel())
		return 0;
	else
		return sourceModel()->rowCount(mapToSource(parent));
}

QVariant QObjectProxyModel::data(const QModelIndex &index, int role) const
{
	if(!sourceModel())
		return {};
	auto src = mapToSource(index);
	auto srcRole = _roleMapping.value({index.column(), role}, role);
	return sourceModel()->data(src, srcRole);
}

bool QObjectProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!sourceModel())
		return false;
	auto src = mapToSource(index);
	auto srcRole = _roleMapping.value({index.column(), role}, role);
	return sourceModel()->setData(src, value, srcRole);
}

QVariant QObjectProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(section >= 0 &&
	   section < _headers.size() &&
	   orientation == Qt::Horizontal &&
	   role == Qt::DisplayRole)
		return _headers[section];
	else
		return {};
}

QHash<int, QByteArray> QObjectProxyModel::roleNames() const
{
	auto roles = QAbstractProxyModel::roleNames();
	for(auto it = _extraRoles.constBegin(); it != _extraRoles.constEnd(); it++)
		roles.insert(it.key(), it.value());
	return roles;
}

Qt::ItemFlags QObjectProxyModel::flags(const QModelIndex &index) const
{
	auto flags = QAbstractProxyModel::flags(index);
	flags &= ~Qt::ItemIsEditable;//disable editing because it does not work
	flags |= _extraFlags.value(index.column(), 0);
	return flags;
}

bool QObjectProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(column);
	if(sourceModel())
		return sourceModel()->dropMimeData(data, action, row, 0, mapToSource(parent));
	else
		return false;
}

QModelIndex QObjectProxyModel::sibling(int row, int column, const QModelIndex &index) const
{
	if(!sourceModel())
		return {};
	auto src = mapToSource(index);
	src = sourceModel()->sibling(row, 0, src);
	return this->index(src.row(), column, index.parent());
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
	if (!sourceModel())
		return QModelIndexList();

	const QModelIndexList sourceList = sourceModel()->match(mapToSource(start), role, value, hits, flags);
	QModelIndexList::const_iterator it = sourceList.constBegin();
	const QModelIndexList::const_iterator end = sourceList.constEnd();
	QModelIndexList proxyList;
	proxyList.reserve(sourceList.count());
	for ( ; it != end; ++it)
		proxyList.append(mapFromSource(*it));
	return proxyList;
}

void QObjectProxyModel::setSourceModel(QAbstractListModel *sourceModel)
{
	QAbstractProxyModel::setSourceModel(sourceModel);
}

QAbstractListModel *QObjectProxyModel::sourceModel() const
{
	return qobject_cast<QAbstractListModel*>(QAbstractProxyModel::sourceModel());
}

QModelIndex QObjectProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
	if(!sourceModel())
		return {};
	if(!proxyIndex.isValid() || proxyIndex.parent().isValid())
		return {};
	else
		return sourceModel()->index(proxyIndex.row(), 0);
}

QModelIndex QObjectProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
	if(!sourceModel())
		return {};
	if(!sourceIndex.isValid() || sourceIndex.parent().isValid())
		return {};
	else
		return index(sourceIndex.row(), 0);
}

void QObjectProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	Q_ASSERT(sourceModel->inherits("QAbstractListModel"));
	QAbstractProxyModel::setSourceModel(sourceModel);
}

QByteArray QObjectProxyModel::defaultRoleName(int role)
{
	switch (role) {
	case Qt::DisplayRole:
		return "display";
	case Qt::DecorationRole:
		return "decoration";
	case Qt::EditRole:
		return "edit";
	case Qt::ToolTipRole:
		return "toolTip";
	case Qt::StatusTipRole:
		return "statusTip";
	case Qt::WhatsThisRole:
		return "whatsThis";
	default:
		return "role_" + QByteArray::number(role);
	}
}
