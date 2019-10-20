import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    id: root
    property real value : 0
    property real maxValue : 100
    property real minValue : 0
    property real needleAng: {
        if(value>maxValue)
            return 45
        else
            if(value<minValue)
                return -225
            else 100/(root.maxValue-root.minValue)*root.value*2.75 - 223
    }
    property color textColor: "steelblue"


    width: 450;
    height: 450;

    Image {
        width: parent.width
        height: parent.height
        smooth: true
        source: "qrc:/img/dial.png"
        fillMode: Image.PreserveAspectFit
    }

    Image {
        id: needle
        width: parent.width*0.49
        height: parent.height*40/300
        x: parent.width*0.45
        y: parent.height*0.45
        smooth: true
        source: "qrc:/img/arrow.png"
        transform: Rotation {
            id: needleRotation
            origin.x: needle.height/2
            origin.y: needle.height/2
            angle: needleAng
            Behavior on angle {
                SpringAnimation {
                    spring: 1.4
                    damping: .09
                }
            }
        }
    }

    Label{
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height*0.07
        width: parent.width*0.3
        height: parent.height*0.1
        text: root.value

        font.family: "Courier"
        font.pointSize: 16
        font.bold: true
        color: textColor
    }
}
