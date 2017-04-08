#include "objectproxymodel.h"

ObjectProxyModel::ObjectProxyModel(QStringList headers, QObject *parent) :
	QIdentityProxyModel(parent),
	_headers(headers),
	_roleMapping(),
	_extraRoles()
{}

void ObjectProxyModel::appendColumn(const QString &text)
{
	beginInsertColumns(QModelIndex(), _headers.size(), _headers.size());
	_headers.append(text);
	endInsertColumns();
}

void ObjectProxyModel::insertColumn(int index, const QString &text)
{
	beginInsertColumns(QModelIndex(), index, index);
	_headers.insert(index, text);
	endInsertColumns();
}

void ObjectProxyModel::addMapping(int column, int role, int sourceRole)
{
	beginResetModel();
	Q_ASSERT(column < _headers.size());
	_roleMapping.insert({column, role}, sourceRole);
	if(!_extraRoles.contains(role))
		_extraRoles.insert(role, defaultRoleName(role));
	endResetModel();
}

bool ObjectProxyModel::addMapping(int column, int role, const char *sourceRoleName)
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

void ObjectProxyModel::setRoleName(int role, const QByteArray &name)
{
	beginResetModel();
	_extraRoles.insert(role, name);
	endResetModel();
}

QModelIndex ObjectProxyModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex ObjectProxyModel::parent(const QModelIndex &) const
{
	return {};
}

int ObjectProxyModel::columnCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;
	else
		return _headers.size();
}

QVariant ObjectProxyModel::data(const QModelIndex &index, int role) const
{
	if(!sourceModel())
		return {};
	auto src = mapToSource(index);
	auto srcRole = _roleMapping.value({index.column(), role}, role);
	return sourceModel()->data(src, srcRole);
}

bool ObjectProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!sourceModel())
		return false;
	auto src = mapToSource(index);
	auto srcRole = _roleMapping.value({index.column(), role}, role);
	return sourceModel()->setData(src, value, srcRole);
}

QVariant ObjectProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(section >= 0 &&
	   section < _headers.size() &&
	   orientation == Qt::Horizontal &&
	   role == Qt::DisplayRole)
		return _headers[section];
	else
		return {};
}

QHash<int, QByteArray> ObjectProxyModel::roleNames() const
{
	auto roles = QIdentityProxyModel::roleNames();
	for(auto it = _extraRoles.constBegin(); it != _extraRoles.constEnd(); it++)
		roles.insert(it.key(), it.value());
	return roles;
}

void ObjectProxyModel::setSourceModel(ObjectListModel *sourceModel)
{
	QIdentityProxyModel::setSourceModel(sourceModel);
}

ObjectListModel *ObjectProxyModel::sourceModel() const
{
	return qobject_cast<ObjectListModel*>(QIdentityProxyModel::sourceModel());
}

QModelIndex ObjectProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
	if(!sourceModel())
		return {};
	if(!proxyIndex.isValid() || proxyIndex.parent().isValid())
		return {};
	else
		return sourceModel()->index(proxyIndex.row(), 0);
}

QModelIndex ObjectProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
	if(!sourceModel())
		return {};
	if(!sourceIndex.isValid() || sourceIndex.parent().isValid())
		return {};
	else
		return index(sourceIndex.row(), 0);
}

void ObjectProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	Q_ASSERT(sourceModel->inherits("ObjectListModel"));
	QIdentityProxyModel::setSourceModel(sourceModel);
}

QByteArray ObjectProxyModel::defaultRoleName(int role)
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
