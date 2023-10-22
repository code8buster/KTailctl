// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2023 Fabian Köhler <me@fkoehler.org>

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.19 as Kirigami
import org.fkoehler.KTailctl 1.0
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kirigamiaddons.labs.components 1.0 as Components
import org.fkoehler.KTailctl.Components 1.0 as MyComponents

Kirigami.ScrollablePage {
    id: root
    property var peer: null
    property bool isSelf: false

    objectName: "Peer"
    title: "Peer: " + peer.hostName

    header: ColumnLayout {
        Components.Banner {
            width: parent.width
            visible: !Tailscale.status.success
            type: Kirigami.MessageType.Error
            text: i18n("Tailscaled is not running")
        }
        Components.Banner {
            width: parent.width
            visible: (!Tailscale.status.isOperator) && Tailscale.status.success
            type: Kirigami.MessageType.Warning
            text: "KTailctl functionality limited, current user is not the Tailscale operator"
        }
    }

    // DropArea {
    //     anchors.fill: parent
    //     onEntered: {
    //         drag.accept (Qt.LinkAction)
    //     }
    //     onDropped: {
    //         TaildropSender.sendFiles(peer.dnsName, Util.fileUrlsToStrings(drop.urls))
    //     }
    // }

    ColumnLayout {
        FormCard.FormHeader {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            title: i18nc("@title:group", "Information")
        }

        FormCard.FormCard {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 0

                MyComponents.FormCopyLabelDelegate {
                    text: i18nc("@label", "Hostname:")
                    copyData: peer.hostName
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormCopyLabelDelegate {
                    text: i18nc("@label", "DNS name:")
                    copyData: peer.dnsName
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormCopyLabelDelegate {
                    text: i18nc("@label", "Tailscale ID:")
                    copyData: peer.tailscaleID
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormCopyLabelDelegate {
                    text: i18nc("@label", "Created:")
                    copyData: {
                        var duration = Util.formatDurationHumanReadable(peer.created);
                        if(duration == "") {
                            return "now";
                        } else {
                            return duration + " ago";
                        }
                    }
                    onClicked: {
                        Util.setClipboardText(Util.toMSecsSinceEpoch(peer.created));
                    }
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormCopyLabelDelegate {
                    text: i18nc("@label", "Last seen:")
                    copyData: {
                        var duration = Util.formatDurationHumanReadable(peer.lastSeen);
                        if(duration == "") {
                            return "now";
                        } else {
                            return duration + " ago";
                        }
                    }
                    onClicked: {
                        Util.setClipboardText(Util.toMSecsSinceEpoch(peer.lastSeen));
                    }
                }

                FormCard.FormDelegateSeparator { visible: peer.os != "" }

                MyComponents.FormCopyLabelDelegate {
                    text: i18nc("@label", "OS:")
                    copyData: peer.os
                    visible: peer.os != ""
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormCopyChipsDelegate {
                    text: i18nc("@label", "Addresses:")
                    model: peer.tailscaleIps
                }

                FormCard.FormDelegateSeparator {
                    visible: peer.tags.length > 0
                }

                MyComponents.FormCopyChipsDelegate {
                    text: i18nc("@label", "Tags:")
                    visible: peer.tags.length > 0
                    model: peer.tags
                }
            }
        }

        FormCard.FormHeader {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            title: i18nc("@title:group", "Exit node")
        }

        FormCard.FormCard {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 0

                MyComponents.FormLabeledIconDelegate {
                    text: i18nc("@label", "Exit node:")
                    label: peer.isExitNode ? "Yes" : "No"
                    source: peer.isExitNode ? "dialog-ok" : "dialog-cancel"
                }

                FormCard.FormDelegateSeparator {}

                FormCard.FormSwitchDelegate {
                    text: i18nc("@label", "Use this exit node:")
                    checked: peer.isCurrentExitNode
                    enabled: peer.isExitNode && !Tailscale.preferences.advertiseExitNode
                    onToggled: {
                        if(peer.isCurrentExitNode) {
                            Util.unsetExitNode();
                        } else {
                            Util.setExitNode(peer.tailscaleIps[0]);
                        }
                    }
                }
            }
        }

        FormCard.FormHeader {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            title: i18nc("@title:group", "Tailscale SSH")
        }

        FormCard.FormCard {
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 0

                MyComponents.FormLabeledIconDelegate {
                    text: i18nc("@label", "Runs Tailscale SSH:")
                    label: peer.isRunningSSH ? "Yes" : "No"
                    source: peer.isRunningSSH ? "dialog-ok" : "dialog-cancel"
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormCopyLabelDelegate {
                    text: i18nc("@label", "SSH command:")
                    copyData: i18nc("@label", "Copy")
                    enabled: peer.isRunningSSH
                    onClicked: {
                        Util.setClipboardText(peer.sshCommand())
                    }
                }
            }
        }

        FormCard.FormHeader {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: peer.location != null

            title: i18nc("@title:group", "Location")
        }

        FormCard.FormCard {
            Layout.fillWidth: true
            visible: peer.location != null

            ColumnLayout {
                spacing: 0

                MyComponents.FormLabeledIconDelegate {
                    text: i18nc("@label", "Country:")
                    label: peer.location == null ? "" : peer.location.country + " (" + peer.location.countryCode + ")"
                    source: peer.location == null ? "question" : "qrc:/country-flags/" + peer.location.countryCode.toLowerCase() + ".svg"
                    onClicked: {
                        Util.setClipboardText(peer.location.country)
                    }
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormLabelDelegate {
                    text: i18nc("@label", "City:")
                    label: peer.location == null ? "" : peer.location.city + " (" + peer.location.cityCode + ")"
                    onClicked: {
                        Util.setClipboardText(peer.location.city)
                    }
                }
            }
        }

        FormCard.FormHeader {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: !root.isSelf

            title: i18nc("@title:group", "Statistics")
        }

        FormCard.FormCard {
            Layout.fillWidth: true
            visible: !root.isSelf

            ColumnLayout {
                spacing: 0

                MyComponents.FormLabeledIconDelegate {
                    text: i18nc("@label", "Download:")
                    label: root.isSelf ? "" : Util.formatCapacityHumanReadable(peer.rxBytes) + " (" + Util.formatSpeedHumanReadable(Tailscale.statistics.speedDown(peer.tailscaleID).average1Second) + ")"
                    source: "cloud-download"
                    onClicked: {
                        Util.setClipboardText(peer.rxBytes)
                    }
                }

                FormCard.FormDelegateSeparator {}

                MyComponents.FormLabeledIconDelegate {
                    text: i18nc("@label", "Upload:")
                    label: root.isSelf ? "" : Util.formatCapacityHumanReadable(peer.txBytes) + " (" + Util.formatSpeedHumanReadable(Tailscale.statistics.speedUp(peer.tailscaleID).average1Second) + ")"
                    source: "cloud-upload"
                    onClicked: {
                        Util.setClipboardText(peer.txBytes)
                    }
                }
            }
        }
    }

}
