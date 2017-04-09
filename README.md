# QObjectListModel
A Collection of classes to easily create models with QObject and G_GADGET classes for widgets and quick

## Features
- A QAbstractListModel for QObjects
	- Maps properties to display roles
	- Supports dynamic properties (if declared)
	- Forwards property changes as model changes
	- Can be used in QML ListView
- Generic version for easier use in code
- A QAbstractListModel for Q_GADGET classes
	- Simplefied version for gadgets
	- Works just like the QObject-Version, but gadgets have neither signals nor dynamic properties and are value types
- Proxy Model to map roles to columns
	- Works with any QAbstractListModel (item models with 1 column and no children)
	- Allows you to use the model for QAbstractItemView (e.g. QTreeView or QTableView)
	- Maps any role to a column + role (e.g. name role to {column 0, display role})
	- Custom header names
	- Correctly forwards propert changes
	- Editing does *not* work
	
One of the main advantages of this library is that you can use one model for both, a role-based QML ListView, and a column-based widgets QAbstractItemView, and that all property changes trigger data changes in the model and the views.

## Installation
The package is providet as qpm package, [`de.skycoder42.qobjectlistmodel`](https://www.qpm.io/packages/de.skycoder42.qobjectlistmodel/index.html). To install:

1. Install qpm (See [GitHub - Installing](https://github.com/Cutehacks/qpm/blob/master/README.md#installing))
2. In your projects root directory, run `qpm install de.skycoder42.qobjectlistmodel`
3. Include qpm to your project by adding `include(vendor/vendor.pri)` to your `.pro` file

Check their [GitHub - Usage for App Developers](https://github.com/Cutehacks/qpm/blob/master/README.md#usage-for-app-developers) to learn more about qpm.

## Usage
Thanks to the generic version, usage is pretty straight forward. The model has simple add/insert/remove methods to easily add objects, and can take ownership of it's objects. The following code shows how to create a qobject list model as well as the proxy.
```cpp
// create and use the model
auto model = new QGenericListModel<MyObject>(true, this);//true to be owner of objects
myView->setModel(model);

// add items
model->addObject(new MyObject(...));

// use the proxy
auto proxy = new QObjectProxyModel({"Name", "Info"}, this);// The headers. In this case 2 colums with the given headers are created
proxy->setSourceModel(model);// IMPORTANT! Must be done before setting up the mappings
proxy->addMapping(0, Qt::DisplayRole, "name");// Maps the display role of column 0 to the "name" role of the source model
proxy->addMapping(1, Qt::DisplayRole, "text");// Maps the display role of column 1 to the "text" role of the source model
myView2->setModel(proxy);
```

To use the model in qml, all you need to do is create a custom delegate and assign the roles:
```qml
ListView {
	model: listModel //model from c++, e.g. as a context property

	delegate: ItemDelegate {
		width: parent.width
		text: name
		ToolTip.text: model.text //use model.* if the role name is reserved
	}
}

```

Check the `ModelTest` Project for a full example.