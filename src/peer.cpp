#include "peer.h"

Peer::Peer(QObject *parent)
    : QObject(parent)
{
}

const QString &Peer::id() const
{
    return mData.id;
}
const QString &Peer::publicKey() const
{
    return mData.publicKey;
}
const QString &Peer::hostName() const
{
    return mData.hostName;
}
const QString &Peer::dnsName() const
{
    return mData.dnsName;
}
const QString &Peer::os() const
{
    return mData.os;
}
const QStringList &Peer::tailscaleIps() const
{
    return mData.tailscaleIps;
}
const QString &Peer::relay() const
{
    return mData.relay;
}
long Peer::rxBytes() const
{
    return mData.rxBytes;
}
long Peer::txBytes() const
{
    return mData.txBytes;
}
const QString &Peer::created() const
{
    return mData.created;
}
const QString &Peer::lastSeen() const
{
    return mData.lastSeen;
}
bool Peer::isOnline() const
{
    return mData.isOnline;
}
bool Peer::isActive() const
{
    return mData.isActive;
}
bool Peer::isCurrentExitNode() const
{
    return mData.isCurrentExitNode;
}
bool Peer::isExitNode() const
{
    return mData.isExitNode;
}
const QStringList &Peer::sshHostKeys() const
{
    return mData.sshHostKeys;
}
bool Peer::isRunningSSH() const
{
    return !mData.sshHostKeys.empty();
}
QString Peer::sshCommand() const
{
    if (!isRunningSSH()) {
        return {""};
    }
    return QString("tailscale ssh %1").arg(mData.dnsName);
}
const QStringList &Peer::tags() const
{
    return mData.tags;
}
bool Peer::isMullvad() const
{
    return mData.isMullvad;
}
Location *Peer::location() const
{
    return mLocation;
}

void Peer::update(PeerData &newData)
{
    if (newData.id != mData.id) {
        mData.id.swap(newData.id);
        emit idChanged(mData.id);
    }
    if (newData.publicKey != mData.publicKey) {
        mData.publicKey.swap(newData.publicKey);
        emit publicKeyChanged(mData.publicKey);
    }
    if (newData.hostName != mData.hostName) {
        mData.hostName.swap(newData.hostName);
        emit hostNameChanged(mData.hostName);
    }
    if (newData.dnsName != mData.dnsName) {
        mData.dnsName.swap(newData.dnsName);
        emit dnsNameChanged(mData.dnsName);
    }
    if (newData.os != mData.os) {
        mData.os.swap(newData.os);
        emit osChanged(mData.os);
    }
    if (newData.tailscaleIps != mData.tailscaleIps) {
        mData.tailscaleIps.swap(newData.tailscaleIps);
        emit tailscaleIpsChanged(mData.tailscaleIps);
        emit sshCommandChanged(sshCommand());
    }
    if (newData.relay != mData.relay) {
        mData.relay.swap(newData.relay);
        emit relayChanged(mData.relay);
    }
    if (newData.rxBytes != mData.rxBytes) {
        mData.rxBytes = newData.rxBytes;
        emit rxBytesChanged(mData.rxBytes);
    }
    if (newData.txBytes != mData.txBytes) {
        mData.txBytes = newData.txBytes;
        emit txBytesChanged(mData.txBytes);
    }
    if (newData.created != mData.created) {
        mData.created.swap(newData.created);
        emit createdChanged(mData.created);
    }
    if (newData.lastSeen != mData.lastSeen) {
        mData.lastSeen.swap(newData.lastSeen);
        emit lastSeenChanged(mData.lastSeen);
    }
    if (newData.isOnline != mData.isOnline) {
        mData.isOnline = newData.isOnline;
        emit isOnlineChanged(mData.isOnline);
    }
    if (newData.isActive != mData.isActive) {
        mData.isActive = newData.isActive;
        emit isActiveChanged(mData.isActive);
    }
    if (newData.isCurrentExitNode != mData.isCurrentExitNode) {
        mData.isCurrentExitNode = newData.isCurrentExitNode;
        emit isCurrentExitNodeChanged(mData.isCurrentExitNode);
    }
    if (newData.isExitNode != mData.isExitNode) {
        mData.isExitNode = newData.isExitNode;
        emit isExitNodeChanged(mData.isExitNode);
    }
    if (newData.sshHostKeys != mData.sshHostKeys) {
        if (newData.sshHostKeys.empty() ^ mData.sshHostKeys.empty()) {
            emit isRunningSSHChanged(!newData.sshHostKeys.empty());
            emit sshCommandChanged(sshCommand());
        }
        mData.sshHostKeys.swap(newData.sshHostKeys);
        emit sshHostKeysChanged(mData.sshHostKeys);
    }
    if (newData.tags != mData.tags) {
        mData.tags.swap(newData.tags);
        emit tagsChanged(mData.tags);
    }
    if (newData.isMullvad != mData.isMullvad) {
        mData.isMullvad = newData.isMullvad;
        emit isMullvadChanged(mData.isMullvad);
    }
    if (mLocation == nullptr) {
        if (newData.location.has_value()) {
            mLocation = new Location(this);
            mLocation->update(newData.location.value());
            emit locationChanged(mLocation);
        }
    } else {
        if (newData.location.has_value()) {
            mLocation->update(newData.location.value());
        } else {
            // mLocation->deleteLater();
            mLocation = nullptr;
            // emit locationChanged(nullptr);
        }
    }
}
