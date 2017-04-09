#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlContext>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	model(new QGenericListModel<TestObject>({"baum"}, true, this)),
	proxy(new QObjectProxyModel({"Name", "Info", "Count", "Dynamic Baum"}, this)),
	gModel(new QGadgetListModel<TestGadget>(this))
{
	ui->setupUi(this);
	model->setEditable(true);

	for(auto i = 0; i < 10; i++) {
		//list
		auto t = new TestObject();
		t->setObjectName(QStringLiteral("Test %1").arg(i));
		t->setInfo("This is an object");
		t->setCount(i*i);
		t->setProperty("baum", "42");
		model->addObject(t);

		//gadget
		TestGadget g;
		g.id = i;
		g.name = QStringLiteral("Test - Q_GADGET");
		gModel->addGadget(g);
	}

	//simple list
	ui->listView->setModel(model);

	//qml list
	ui->quickWidget->rootContext()->setContextProperty("listmodel", model);

	//table proxy
	proxy->setSourceModel(model);
	proxy->addMapping(0, Qt::DisplayRole, "objectName");
	proxy->addMapping(0, Qt::CheckStateRole, "active");
	proxy->setExtraFlags(0, Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
	proxy->addMapping(1, Qt::DisplayRole, "info");
	proxy->addMapping(2, Qt::DisplayRole, "count");
	proxy->addMapping(3, Qt::DisplayRole, "baum");
	ui->tableView->setModel(proxy);

	//gadget model
	ui->quickWidget_2->rootContext()->setContextProperty("gadgetmodel", gModel);
}

MainWindow::~MainWindow()
{
	delete ui;
}
