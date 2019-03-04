#ifndef QMODELALIASADAPTER_H
#define QMODELALIASADAPTER_H

#include <type_traits>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QAbstractTableModel>
#include <QtCore/QHash>
#include <QtCore/QDebug>

template <typename TModel = QAbstractTableModel>
class QModelAliasBaseAdapter : public TModel
{
	static_assert(std::is_base_of<QAbstractItemModel, TModel>::value, "TModel must inherit from QAbstractItemModel");

public:
	enum class Convert {
		Read,
		Write
	};

	using ConverterFunc = std::function<QVariant(Convert, QVariant)>;

	explicit QModelAliasBaseAdapter(QObject *parent = nullptr);

	int columnCount(const QModelIndex &parent = {}) const final;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::DisplayRole) final;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	int addColumn(QString text);
	int addColumn(QString text, int originalRole);
	inline int addColumn(QString text, const char *originalRoleName);
	void insertColumn(int column, QString text);
	void insertColumn(int column, QString text, int originalRole);
	inline void insertColumn(int column, const QString &text, const char *originalRoleName);

	void addRole(int column, int aliasRole, int originalRole);
	inline void addRole(int column, int aliasRole, const char *originalRoleName);

	void addSrcConverter(int originalRole, const ConverterFunc &converter);
	void addAliasConverter(int column, int aliasRole, const ConverterFunc &converter);

	void setExtraFlags(int column, Qt::ItemFlags addFlags, Qt::ItemFlags removeFlags = Qt::NoItemFlags);

	void clearColumns();
	QString columnTitle(int column) const;
	int resolveRole(int column, int aliasRole) const;

protected:
	virtual QString defaultHeaderData() const;
	virtual QVariant originalData(const QModelIndex &index, int role) const = 0;
	virtual bool setOriginalData(const QModelIndex &index, const QVariant &value, int role) = 0;

	void applyExtraFlags(int column, Qt::ItemFlags &flags) const;
	void emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

	// dummy to make private
	inline void dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &){}

private:
	using RoleMapping = QHash<int, int>;
	using RoleConverters = QHash<int, ConverterFunc>;
	struct ColumnInfo {
		QString name;
		RoleMapping roles;
		RoleConverters converters;
		Qt::ItemFlags addFlags = Qt::NoItemFlags;
		Qt::ItemFlags removeFlags = Qt::NoItemFlags;

		inline ColumnInfo() = default;
		inline ColumnInfo(QString &&name) :
			name{std::move(name)}
		{}
	};
	QList<ColumnInfo> _columns;
	RoleConverters _origRoleConverters;

	QVariant dataImpl(const QModelIndex &index, int originalRole, const ColumnInfo &info, int aliasRole) const;
	bool setDataImpl(const QModelIndex &index, QVariant value, int originalRole, const ColumnInfo &info, int aliasRole);
};


template <typename TModel>
class QModelAliasAdapter : public QModelAliasBaseAdapter<TModel>
{
public:
	explicit QModelAliasAdapter(QObject *parent = nullptr);

protected:
	QVariant originalData(const QModelIndex &index, int role) const override;
	bool setOriginalData(const QModelIndex &index, const QVariant &value, int role) override;
};

// ------------- Generic Implementation -------------

template<typename TModel>
QModelAliasBaseAdapter<TModel>::QModelAliasBaseAdapter(QObject *parent) :
	TModel{parent}
{}

template<typename TModel>
int QModelAliasBaseAdapter<TModel>::columnCount(const QModelIndex &parent) const
{
	Q_ASSERT(this->checkIndex(parent, QAbstractItemModel::CheckIndexOption::DoNotUseParent));
	if(this->flags(parent).testFlag(Qt::ItemNeverHasChildren) || parent.column() > 0)
		return 0;
	else
		return _columns.isEmpty() ? 1 : _columns.size();
}

template<typename TModel>
QVariant QModelAliasBaseAdapter<TModel>::data(const QModelIndex &index, int role) const
{
	Q_ASSERT(this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
	if(_columns.isEmpty())
		return dataImpl(index, role, {}, role);

	// mappings do exist -> check for one
	const auto origRole = _columns[index.column()].roles.value(role, -1);
	if(origRole == -1) { // no role -> -> no mapping -> forward for column 0, no data for all others
		if(index.column() == 0)
			return dataImpl(index, role, {}, role);
		else
			return {};
	} else // has role -> return data of original role at column 0
		return dataImpl(index.siblingAtColumn(0), origRole, _columns[index.column()], role);
}

template<typename TModel>
bool QModelAliasBaseAdapter<TModel>::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_ASSERT(this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
	if(_columns.isEmpty())
		return setDataImpl(index, value, role, {}, role);

	// mappings do exist -> check for one
	const auto origRole = _columns[index.column()].roles.value(role, -1);
	if(origRole == -1) { // no role -> forward for column 0, no data for all others
		if(index.column() == 0)
			return setDataImpl(index, value, role, {}, role);
		else
			return false;
	} else // has role -> return data of original role at column 0
		return setDataImpl(index.siblingAtColumn(0), value, origRole, _columns[index.column()], role);
}

template<typename TModel>
QVariant QModelAliasBaseAdapter<TModel>::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(_columns.isEmpty()) {
		if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return defaultHeaderData();
		else
			return this->TModel::headerData(section, orientation, role);
	} else if(orientation == Qt::Horizontal &&
	   role == Qt::DisplayRole &&
	   section < _columns.size())
		return columnTitle(section);
	else
		return {};
}

template<typename TModel>
Qt::ItemFlags QModelAliasBaseAdapter<TModel>::flags(const QModelIndex &index) const
{
	Q_ASSERT(this->checkIndex(index, QAbstractItemModel::CheckIndexOption::NoOption));
	auto flags = this->TModel::flags(index);
	applyExtraFlags(index.column(), flags);
	return flags;
}

template<typename TModel>
int QModelAliasBaseAdapter<TModel>::addColumn(QString text)
{
	const auto index = _columns.size();
	this->beginInsertColumns(QModelIndex{}, index, index);
	_columns.append(std::move(text));
	this->endInsertColumns();
	emit this->headerDataChanged(Qt::Horizontal, index, index);
	return index;
}

template<typename TModel>
int QModelAliasBaseAdapter<TModel>::addColumn(QString text, int originalRole)
{
	const auto column = addColumn(std::move(text));
	addRole(column, Qt::DisplayRole, originalRole);
	return column;
}

template<typename TModel>
inline int QModelAliasBaseAdapter<TModel>::addColumn(QString text, const char *originalRoleName)
{
	return addColumn(std::move(text), this->roleNames().key(originalRoleName));
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::insertColumn(int column, QString text)
{
	this->beginInsertColumns(QModelIndex{}, column, column);
	_columns.insert(column, {std::move(text), {}});
	this->endInsertColumns();
	emit this->headerDataChanged(Qt::Horizontal, column, _columns.size() - 1);
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::insertColumn(int column, QString text, int originalRole)
{
	insertColumn(column, std::move(text));
	addRole(column, Qt::DisplayRole, originalRole);
}

template<typename TModel>
inline void QModelAliasBaseAdapter<TModel>::insertColumn(int column, const QString &text, const char *originalRoleName)
{
	insertColumn(column, text, this->roleNames().key(originalRoleName));
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::addRole(int column, int aliasRole, int originalRole)
{
	Q_ASSERT_X(column < _columns.size(), Q_FUNC_INFO, "Cannot add role to non existant column!");
	if(column == 0 && this->roleNames().contains(aliasRole)) {
		qWarning() << "QModelAliasBaseAdapter::addRole: Registering a alias for a named role on column 0 make the role inaccessible from QML!"
				   << "Triggered for alias role" << aliasRole << ", named" << this->roleNames().value(aliasRole);
	}

	_columns[column].roles.insert(aliasRole, originalRole);
	const auto rows = this->rowCount();
	if(rows > 0)
		emit this->dataChanged(this->index(0, column), this->index(rows - 1, column), {aliasRole});
}

template<typename TModel>
inline void QModelAliasBaseAdapter<TModel>::addRole(int column, int aliasRole, const char *originalRoleName)
{
	addRole(column, aliasRole, this->roleNames().key(originalRoleName));
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::addSrcConverter(int originalRole, const ConverterFunc &converter)
{
	_origRoleConverters.insert(originalRole, converter);
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::addAliasConverter(int column, int aliasRole, const ConverterFunc &converter)
{
	Q_ASSERT_X(column < _columns.size(), Q_FUNC_INFO, "Cannot add role converter to non existant column!");
	_columns[column].converters.insert(aliasRole, converter);
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::setExtraFlags(int column, Qt::ItemFlags addFlags, Qt::ItemFlags removeFlags)
{
	Q_ASSERT_X(column < _columns.size(), Q_FUNC_INFO, "Cannot add flags to non existant column!");
	this->beginResetModel();
	_columns[column].addFlags = addFlags;
	_columns[column].removeFlags = removeFlags;
	this->endResetModel();
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::clearColumns()
{
	this->beginRemoveColumns(QModelIndex{}, 0, _columns.size() - 1);
	_columns.clear();
	this->endRemoveColumns();
}

template<typename TModel>
QString QModelAliasBaseAdapter<TModel>::columnTitle(int column) const
{
	Q_ASSERT_X(column < _columns.size(), Q_FUNC_INFO, "Cannot return the name of a non existant column!");
	return _columns.value(column).name;
}

template<typename TModel>
int QModelAliasBaseAdapter<TModel>::resolveRole(int column, int aliasRole) const
{
	Q_ASSERT_X(column < _columns.size(), Q_FUNC_INFO, "Cannot return the original role of a non existant column!");
	return _columns[column].roles.value(aliasRole, -1);
}

template<typename TModel>
QString QModelAliasBaseAdapter<TModel>::defaultHeaderData() const
{
	const auto self = static_cast<const TModel*>(this);
	return self->TModel::headerData(0, Qt::Horizontal, Qt::DisplayRole).toString();
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::applyExtraFlags(int column, Qt::ItemFlags &flags) const
{
	if(!_columns.isEmpty()) {
		flags |= _columns[column].addFlags;
		flags &= ~(_columns[column].removeFlags);
	}
}

template<typename TModel>
void QModelAliasBaseAdapter<TModel>::emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
	Q_ASSERT(this->checkIndex(topLeft, QAbstractItemModel::CheckIndexOption::IndexIsValid));
	Q_ASSERT(this->checkIndex(bottomRight, QAbstractItemModel::CheckIndexOption::IndexIsValid));
	Q_ASSERT_X(topLeft.column() == 0 && bottomRight.column() == 0, Q_FUNC_INFO, "emitDataChanged can only operate on indexes with column 0");

	if(_columns.isEmpty()) {
		emit this->TModel::dataChanged(topLeft, bottomRight, roles);
		return;
	}

	auto allRoles = roles;
	auto lastColumn = 0;
	auto cCounter = 0;
	for(const auto &columnInfo : qAsConst(_columns)) {
		for(auto role : roles) {
			const auto aliasRoles = columnInfo.roles.keys(role);
			for(const auto &aliasRole : aliasRoles) {
				lastColumn = std::max(lastColumn, cCounter);
				if(!allRoles.contains(aliasRole))
					allRoles.append(aliasRole);
			}
		}
		++cCounter;
	}

	emit this->TModel::dataChanged(topLeft, bottomRight.siblingAtColumn(lastColumn), allRoles);
}

template<typename TModel>
QVariant QModelAliasBaseAdapter<TModel>::dataImpl(const QModelIndex &index, int originalRole, const ColumnInfo &info, int aliasRole) const
{
	auto data = originalData(index, originalRole);
	const auto &origConv = _origRoleConverters[originalRole];
	if(origConv)
		data = origConv(Convert::Read, data);
	const auto &aliasConv = info.converters[aliasRole];
	if(aliasConv)
		data = aliasConv(Convert::Read, data);
	return data;
}

template<typename TModel>
bool QModelAliasBaseAdapter<TModel>::setDataImpl(const QModelIndex &index, QVariant value, int originalRole, const ColumnInfo &info, int aliasRole)
{
	const auto &aliasConv = info.converters[aliasRole];
	if(aliasConv)
		value = aliasConv(Convert::Write, value);
	const auto &origConv = qAsConst(_origRoleConverters)[originalRole];
	if(origConv)
		value = origConv(Convert::Write, value);
	return setOriginalData(index, value, originalRole);
}



template<typename TModel>
QModelAliasAdapter<TModel>::QModelAliasAdapter(QObject *parent) :
	TModel{parent}
{}

template<typename TModel>
QVariant QModelAliasAdapter<TModel>::originalData(const QModelIndex &index, int role) const
{
	const auto self = static_cast<const TModel*>(this);
	return self->TModel::data(index, role);
}

template<typename TModel>
bool QModelAliasAdapter<TModel>::setOriginalData(const QModelIndex &index, const QVariant &value, int role)
{
	const auto self = static_cast<TModel*>(this);
	return self->TModel::setData(index, value, role);
}


#endif // QMODELALIASADAPTER_H
