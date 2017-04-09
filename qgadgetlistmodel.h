#ifndef QGADGETLISTMODEL_H
#define QGADGETLISTMODEL_H

#include <QAbstractListModel>
#include <QMetaProperty>

template <typename T>
class QGadgetListModel : public QAbstractListModel
{
	static_assert(std::is_void<typename T::QtGadgetHelper>::value, "T must have the Q_GADGET macro!");

public:
	explicit QGadgetListModel(QObject *parent = nullptr);

	QList<T> gadgets() const;
	T gadget(const QModelIndex &index) const;
	T gadget(int index) const;
	void addGadget(T gadget);
	void insertGadget(const QModelIndex &index, T gadget);
	void insertGadget(int index, T gadget);
	void removeGadget(const QModelIndex &index);
	void removeGadget(int index);
	T takeGadget(const QModelIndex &index);
	T takeGadget(int index);
	void resetModel(QList<T> gadgets);

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	using QAbstractListModel::index;
	QModelIndex index(T gadget) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value,
				 int role = Qt::EditRole) override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QHash<int, QByteArray> roleNames() const override;

	bool editable() const;
	void setEditable(bool editable);

protected:
	bool testValid(const QModelIndex &index, int role = -1) const;

private:
	QHash<int, QByteArray> _roleNames;
	bool _editable;
	QList<T> _gadgets;
};

template<typename T>
QGadgetListModel<T>::QGadgetListModel(QObject *parent) :
	QAbstractListModel(parent),
	_roleNames(),
	_editable(false),
	_gadgets()
{
	auto roleIndex = Qt::UserRole + 1;
	for(auto i = 0; i < T::staticMetaObject.propertyCount(); i++)
		_roleNames.insert(roleIndex++, T::staticMetaObject.property(i).name());
}

template<typename T>
QList<T> QGadgetListModel<T>::gadgets() const
{
	return _gadgets;
}

template<typename T>
T QGadgetListModel<T>::gadget(const QModelIndex &index) const
{
	return gadget(index.row());
}

template<typename T>
T QGadgetListModel<T>::gadget(int index) const
{
	return _gadgets[index];
}

template<typename T>
void QGadgetListModel<T>::addGadget(T gadget)
{
	beginInsertRows(QModelIndex(), _gadgets.size(), _gadgets.size());
	_gadgets.append(gadget);
	endInsertRows();
}

template<typename T>
void QGadgetListModel<T>::insertGadget(const QModelIndex &index, T gadget)
{
	return insertGadget(index.row(), gadget);
}

template<typename T>
void QGadgetListModel<T>::insertGadget(int index, T gadget)
{
	beginInsertRows(QModelIndex(), index, index);
	_gadgets.insert(index, gadget);
	endInsertRows();
}

template<typename T>
void QGadgetListModel<T>::removeGadget(const QModelIndex &index)
{
	removeGadget(index.row());
}

template<typename T>
void QGadgetListModel<T>::removeGadget(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	_gadgets.removeAt(index);
	endRemoveRows();
}

template<typename T>
T QGadgetListModel<T>::takeGadget(const QModelIndex &index)
{
	return takeGadget(index.row());
}

template<typename T>
T QGadgetListModel<T>::takeGadget(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	auto gadget = _gadgets.takeAt(index);
	endRemoveRows();
	return gadget;
}

template<typename T>
void QGadgetListModel<T>::resetModel(QList<T> gadgets)
{
	beginResetModel();
	_gadgets = gadgets;
	endResetModel();
}

template<typename T>
QVariant QGadgetListModel<T>::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return QString::fromUtf8(T::staticMetaObject.className());
	else
		return {};
}

template<typename T>
int QGadgetListModel<T>::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return _gadgets.size();
}

template<typename T>
QModelIndex QGadgetListModel<T>::index(T gadget) const
{
	auto idx = _gadgets.indexOf(gadget);
	if(idx != -1)
		return index(idx);
	else
		return {};
}

template<typename T>
QVariant QGadgetListModel<T>::data(const QModelIndex &index, int role) const
{
	if (!testValid(index, role))
		return {};

	auto pIndex = T::staticMetaObject.indexOfProperty(_roleNames[role].constData());
	if(pIndex == -1)
		return {};
	else
		return T::staticMetaObject.property(pIndex).readOnGadget(&_gadgets[index.row()]);
}

template<typename T>
bool QGadgetListModel<T>::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!testValid(index, role))
	  return false;

	if (data(index, role) != value) {
		auto pIndex = T::staticMetaObject.indexOfProperty(_roleNames[role].constData());
		if(pIndex != -1) {
			auto ok = T::staticMetaObject.property(pIndex).writeOnGadget(&_gadgets[index.row()], value);
			if(ok)
				emit dataChanged(index, index, {role});
			return ok;
		}
	}

	return false;
}

template<typename T>
Qt::ItemFlags QGadgetListModel<T>::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	auto flags = Qt::ItemIsEnabled |
				 Qt::ItemIsSelectable |
				 Qt::ItemIsDragEnabled |
				 Qt::ItemNeverHasChildren;
	if(_editable)
		flags |= Qt::ItemIsEditable;
	return flags;
}

template<typename T>
QHash<int, QByteArray> QGadgetListModel<T>::roleNames() const
{
	return _roleNames;
}

template<typename T>
bool QGadgetListModel<T>::editable() const
{
	return _editable;
}

template<typename T>
void QGadgetListModel<T>::setEditable(bool editable)
{
	_editable = editable;
}

template<typename T>
bool QGadgetListModel<T>::testValid(const QModelIndex &index, int role) const
{
	return index.isValid() &&
			index.column() == 0 &&
			index.row() < _gadgets.size() &&
			(role < 0 || _roleNames.contains(role));
}

#endif // QGADGETLISTMODEL_H
