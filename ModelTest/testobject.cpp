#include "testobject.h"

TestObject::TestObject(QObject *parent) :
	QObject(parent),
	_info(),
	_count(0),
	_active(Qt::Unchecked)
{}

QString TestObject::info() const
{
	return _info;
}

int TestObject::count() const
{
	return _count;
}

Qt::CheckState TestObject::active() const
{
	return _active;
}

void TestObject::setInfo(QString info)
{
	if (_info == info)
		return;

	_info = info;
	emit infoChanged(info);
}

void TestObject::setCount(int count)
{
	if (_count == count)
		return;

	_count = count;
	emit countChanged(count);
}

void TestObject::setActive(Qt::CheckState active)
{
	if (_active == active)
		return;

	_active = active;
	emit activeChanged(active);
}
