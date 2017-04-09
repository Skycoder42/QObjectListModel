#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qobjectlistmodel.h>
#include <qgadgetlistmodel.h>
#include <qobjectproxymodel.h>
#include "testobject.h"
#include "testgadget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow *ui;

	QGenericListModel<TestObject> *model;
	QObjectProxyModel *proxy;

	QGadgetListModel<TestGadget> *gModel;
};

#endif // MAINWINDOW_H
