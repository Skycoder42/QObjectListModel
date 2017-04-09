#ifndef TESTGADGET_H
#define TESTGADGET_H

#include <QObject>

class TestGadget
{
	Q_GADGET

	Q_PROPERTY(int id MEMBER id)
	Q_PROPERTY(QString name MEMBER name)

public:
	TestGadget();

	int id;
	QString name;
};

#endif // TESTGADGET_H
