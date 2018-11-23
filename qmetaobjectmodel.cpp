#include "qmetaobjectmodel.h"

QMetaObjectModel::QMetaObjectModel(const QMetaObject *metaType, QObject *parent) :
	QModelAliasBaseAdapter(parent),
	_metaObject{metaType}
{
	auto roleIndex = Qt::UserRole + 1;
	for(auto i = 0; i < _metaObject->propertyCount(); i++) {
		auto prop = _metaObject->property(i);
		if(prop.hasNotifySignal())
			registerSignalHelper(roleIndex, prop.notifySignal());
		_roleNames.insert(roleIndex++, prop.name());
	}
}

Qt::ItemFlags QMetaObjectModel::flags(const QModelIndex &index) const
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::NoOption));

	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	auto flags = Qt::ItemIsEnabled |
				 Qt::ItemIsSelectable |
				 Qt::ItemIsDragEnabled |
				 Qt::ItemNeverHasChildren;
	if(_editable)
		flags |= Qt::ItemIsEditable;

	applyExtraFlags(index.column(), flags);

	return flags;
}

QHash<int, QByteArray> QMetaObjectModel::roleNames() const
{
	return _roleNames;
}

bool QMetaObjectModel::editable() const
{
	return _editable;
}

void QMetaObjectModel::setEditable(bool editable)
{
	if (_editable == editable)
		return;

	beginResetModel();
	_editable = editable;
	emit editableChanged(_editable, {});
	endResetModel();
}

QString QMetaObjectModel::defaultHeaderData() const
{
	return QString::fromUtf8(_metaObject->className());
}

QVariant QMetaObjectModel::originalData(const QModelIndex &index, int role) const
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	Q_ASSERT_X(index.column() == 0, Q_FUNC_INFO, "originalData must always be called only with an index for column 0");

	const auto propIndex = _metaObject->indexOfProperty(roleNames().value(role).constData());
	if(propIndex < 0)
		return {};
	else
		return readProperty(index.row(), _metaObject->property(propIndex));
}

bool QMetaObjectModel::setOriginalData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_ASSERT(checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid));
	Q_ASSERT_X(index.column() == 0, Q_FUNC_INFO, "setOriginalData must always be called only with an index for column 0");

	if(!_editable)
		return false;

	const auto propIndex = _metaObject->indexOfProperty(roleNames().value(role).constData());
	if(propIndex < 0)
		return false;
	else if(writeProperty(index.row(), _metaObject->property(propIndex), value)) {
		emitDataChanged(index, index, {role});
		return true;
	} else
		return false;
}

void QMetaObjectModel::registerSignalHelper(int role, const QMetaMethod &signal)
{
	Q_UNUSED(role);
	Q_UNUSED(signal);
}
