import QtQuick 2.8
import QtQuick.Controls 2.1

Item {
	ListView {
		anchors.fill: parent
		ScrollBar.vertical: ScrollBar {}

		model: listmodel

		delegate: CheckDelegate {
			width: parent.width
			tristate: true
			text: "<b>%1:</b> %3 <i>(%2)</i>"
					.arg(name)
					.arg(info)
					.arg(count)
			checkState: model.active

			onCheckStateChanged: model.active = checkState
		}
	}
}
