#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlContext>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	model(new QGenericListModel<TestObject>(true, this)),
	proxy(new QObjectProxyModel({"Name", "Info", "Count"}, this))
{
	ui->setupUi(this);

	for(auto i = 0; i < 10; i++) {
		auto t = new TestObject();
		t->setObjectName(QStringLiteral("Test %1").arg(i));
		t->setInfo("This is an object");
		t->setCount(i*i);
		model->addObject(t);
	}

	//simple list
	ui->listView->setModel(model);

	//qml list
	ui->quickWidget->rootContext()->setContextProperty("listmodel", model);

	//table proxy
	proxy->setSourceModel(model);
	proxy->addMapping(0, Qt::DisplayRole, "objectName");
	proxy->addMapping(0, Qt::CheckStateRole, "active");
	proxy->addMapping(1, Qt::DisplayRole, "info");
	proxy->addMapping(2, Qt::DisplayRole, "count");
	ui->tableView->setModel(proxy);
}

MainWindow::~MainWindow()
{
	delete ui;
}
