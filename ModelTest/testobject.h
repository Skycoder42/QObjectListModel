#ifndef TESTOBJECT_H
#define TESTOBJECT_H

#include <QObject>

class TestObject : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString info READ info WRITE setInfo NOTIFY infoChanged)
	Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
	Q_PROPERTY(Qt::CheckState active READ active WRITE setActive NOTIFY activeChanged)

public:
	explicit TestObject(QObject *parent = nullptr);

	QString info() const;
	int count() const;
	Qt::CheckState active() const;

public slots:
	void setInfo(QString info);
	void setCount(int count);
	void setActive(Qt::CheckState active);

signals:
	void infoChanged(QString info);
	void countChanged(int count);
	void activeChanged(Qt::CheckState active);

private:
	QString _info;
	int _count;
	Qt::CheckState _active;
};

#endif // TESTOBJECT_H
