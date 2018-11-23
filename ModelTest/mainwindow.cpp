#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlContext>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	model(new QGenericListModel<TestObject>(true, this)),
	gModel(new QGenericListModel<TestGadget>(this))
{
	ui->setupUi(this);
	model->setEditable(true);

	for(auto i = 0; i < 10; i++) {
		//list
		auto t = new TestObject();
		t->setObjectName(QStringLiteral("Test %1").arg(i));
		t->setInfo(QStringLiteral("This is object no. %1").arg(i));
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
	ui->quickWidget->rootContext()->setContextProperty(QStringLiteral("listmodel"), model);

	//table proxy
	model->addColumn(QStringLiteral("Name"));
	model->addColumn(QStringLiteral("Info"));
	model->addColumn(QStringLiteral("Count"));
	model->addColumn(QStringLiteral("transformed"));
	model->addRole(0, Qt::DisplayRole, "objectName");
	model->addRole(0, Qt::EditRole, "objectName");
	model->addRole(0, Qt::CheckStateRole, "active");
	model->setExtraFlags(0, Qt::ItemIsUserCheckable | Qt::ItemIsUserTristate | Qt::ItemIsTristate);
	model->addRole(1, Qt::DisplayRole, "info");
	model->addRole(1, Qt::EditRole, "info");
	model->addRole(2, Qt::DisplayRole, "count");
	model->addRole(2, Qt::EditRole, "count");
	model->addRole(3, Qt::DisplayRole, "objectName");
	model->setExtraFlags(3, Qt::NoItemFlags, Qt::ItemIsEditable);
	model->addAliasConverter(3, Qt::DisplayRole, [](QGenericListModel<TestObject>::Convert c, const QVariant &v) {
		if(c == QGenericListModel<TestObject>::Convert::Read)
			return QVariant{v.toString() + v.toString()};
		else
			return v;
	});
	ui->treeView->setModel(model);

	//gadget model
	ui->quickWidget_2->rootContext()->setContextProperty(QStringLiteral("gadgetmodel"), gModel);
}

MainWindow::~MainWindow()
{
	delete ui;
}
