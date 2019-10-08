import QtQuick 2.11

import QtQuick.Window 2.11
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
//import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0
import QtDataVisualization 1.0
import QtCharts 2.2

import my_appController 1.0

import "my_components" as MyComponents

ApplicationWindow {
    minimumWidth: 800
    minimumHeight: 600
    width: 800
    height: 600
    visible: true

    property int i: 0
    property int impPerS: 2000

    RadialGradient{
        anchors.fill: parent
        horizontalRadius: parent.width/2
        verticalRadius: parent.height/2
        GradientStop {
            position: 0.0
            color: "#F0F0F0"
        }
        GradientStop {
            position: 0.5
            color: "#000000"
        }
        GradientStop {
            position: 1.0
            color: "#F0F0F0"
        }
    }

    MyComponents.ScopeView {
        id: scopeView
        anchors.top: mainBar.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: settingsItem.left

        axisXMin: appController.xMin
        axisXMax: appController.xMax
        axisYMin: appController.yMin
        axisYMax: appController.yMax

    }

    AppController{
        id: appController
    }

    MyComponents.Dial{
        id: dial
        anchors.top: mainBar.bottom
        anchors.left: settingsItem.visible ? settingsItem.right : parent.left
        anchors.margins: 15
        width: 200
        height: width
        visible: showDialBtn.checked

        minValue: 0
        maxValue: timeStepSpinBox.value*impPerS

        value: appController.dialValue

    }

    Rectangle{
        id: mainBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: 40
        visible: true
        color: "#77666666"

        Button{
            id: settingsButton
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.left: mainBar.left
            anchors.margins: 5
            checkable: true
            text: qsTr("Settings")
            font.pointSize: 13
        }

        Button {
            id: connectToSerial
            anchors.left: settingsButton.right
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.margins: 5

            checkable: true
            checked: appController.serialConnected
            text: checked ? qsTr("Disconnect") : qsTr("Connect")
            font.pointSize: 13
            onClicked: {
                if(connectToSerial.checked)
                    appController.openSerialPort()
                else
                    appController.closeSerialPort()
            }
        }

        ComboBox {
            id: portSelect
            anchors.left: connectToSerial.right
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.margins: 5
            editable: false
            model: appController.portList
            visible: !appController.serialConnected
            onActivated: {
                appController.selectPort(index)
            }
        }

        Button{
            id: startButton
            anchors.left: connectToSerial.right
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.margins: 5
            checkable: true
            text: checked ? qsTr("STOP") : qsTr("START")
            font.pointSize: 13
            visible: appController.serialConnected
            onClicked: {
                appController.startExchange(startButton.checked)
                if(!startButton.checked)
                    appController.setDialValue(0)
            }
        }

        Button{
            id: saveButton
            anchors.left: portSelect.visible ? portSelect.right : startButton.right
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.margins: 5
            text: qsTr("Save")
            font.pointSize: 13
            visible: !startButton.checked
            onClicked: {
                appController.saveData()
            }
        }

        Button{
            id: clearButton
            anchors.left: saveButton.visible ? saveButton.right : startButton.right
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.margins: 5
            text: qsTr("Clear data")
            font.pointSize: 13
            onClicked: {
                appController.clearData()
            }
        }
        Rectangle{
            id: serialConnected
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.right: ioData.left
            anchors.margins: 5
            width: height
            color: appController.serialConnected ? "green" : "red"
            opacity: appController.serialConnected ? 0.5 : 0.9
        }

        Rectangle{
            id: ioData
            anchors.top: mainBar.top
            anchors.bottom: mainBar.bottom
            anchors.right: mainBar.right
            anchors.margins: 5
            width: height
            color: appController.ioData ? "green" : "red"
            opacity: appController.serialConnected ? 0.5 : 0.9
        }
    }


    Rectangle {
        id: settingsItem
        anchors.top: mainBar.bottom
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        visible: settingsButton.checked
        width: (parent.width*0.3<270) ? 270 : parent.width*0.3
        color: "#DD111111"

        Label{
            id: markTime
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 5

            text: "Time step:"
            font.family: "Courier"
            font.pointSize: 16
            font.bold: true
            color: "#CCCCCCCC"
            width: 52
        }

        SpinBox {
            id: timeStepSpinBox
            anchors.top: markTime.bottom
            anchors.left: parent.left
            anchors.right: rangeSliderComplex.left
            anchors.margins: 5

            from: 1
            to: 20*100
            value: appController.timeStep
            validator: DoubleValidator {
                bottom: timeStepSpinBox.from
                top:  timeStepSpinBox.to
            }
            textFromValue: function(value, locale) {
                return Number(value / 10).toLocaleString(locale, 'f', timeStepSpinBox.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 10
            }

            editable: true
            property int decimals: 1
            property real realValue: value / 10.

            onValueChanged: {
                appController.setTimeStep(value)
            }
        }

        Button{
            id: getSpectrBtn
            anchors.top: timeStepSpinBox.bottom
            anchors.left: parent.left
            anchors.margins: 5
            text: qsTr("Get spectr")
            checkable: true
            checked: appController.isGetSpectrState
            font.pointSize: 13
            onClicked: {
                appController.getSpectr(getSpectrBtn.checked)
                startButton.checked = false;
            }
        }

        Button{
            id: showDialBtn
            anchors.bottom: markTheme.top
            anchors.left: parent.left
            anchors.margins: 5
            text: checked ? qsTr("Hide meter") : qsTr("Show meter")
            font.pointSize: 13
            checkable: true
            checked: true
        }

        Label{
            id: markTheme
            anchors.left: parent.left
            anchors.bottom: themeSelector.top
            anchors.margins: 5

            text: "Scope theme:"
            font.family: "Courier"
            font.pointSize: 16
            font.bold: true
            color: "#CCCCCCCC"
            width: 52
        }

        SpinBox {
            id: themeSelector
            anchors.left: parent.left
            anchors.right: rangeSliderComplex.left
            anchors.bottom: parent.bottom
            anchors.margins: 5

            from: 0
            to: items.length - 1
            value: 1 // "Medium"

            property var items: ["Light","BlueCerulean","Dark","BrownSand",
                "BlueNcs","HighContrast","BlueIcy","Qt"]

            textFromValue: function(value) {
                return items[value];
            }

            valueFromText: function(text) {
                for (var i = 0; i < items.length; ++i) {
                    if (items[i].toLowerCase().indexOf(text.toLowerCase()) === 0)
                        return i
                }
                return themeSelector.value
            }
            onValueChanged: scopeView.theme = value
         }

        Item{
            id: rangeSliderComplex
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: mark.width+descriminatorSlider.width

            RangeSlider {
                id: descriminatorSlider
                width: 40
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                orientation: Qt.Vertical
                from: 1
                to: 4095
                first.value: appController.lowBorder
                second.value: appController.highBorder
                first.onValueChanged: {
                    appController.setLowBorder(first.value)
                }
                second.onValueChanged: {
                    appController.setHighBorder(second.value)
                }
            }

            Label{
                id: mark
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.left: descriminatorSlider.right

                text: descriminatorSlider.second.value.toFixed(0)
                font.family: "Courier"
                font.pointSize: 16
                font.bold: true
                color: "#CCCCCCCC"
                width: 52
            }
            Label{
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.left: descriminatorSlider.right

                text: descriminatorSlider.first.value.toFixed(0)
                font.family: "Courier"
                font.pointSize: 16
                font.bold: true
                color: "#CCCCCCCC"
                width: mark.width
            }
        }
    }


}
