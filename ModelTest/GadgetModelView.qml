import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
	ListView {
		anchors.fill: parent
		ScrollBar.vertical: ScrollBar {}

		model: gadgetmodel

		delegate: ItemDelegate {
			width: parent.width
			text: "%1 (%2)".arg(model.name).arg(model.id)
		}
	}
}
