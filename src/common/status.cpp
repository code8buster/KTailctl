#include "status.h"
#include "libktailctl_wrapper.h"

#include <algorithm>

Status::Status(QObject *parent)
    : QObject(parent)
    , mData()
    , mCurrentExitNode(nullptr)
    , mNewData()
{
}

bool Status::success() const
{
    return mSuccess;
}
bool Status::isOperator() const
{
    return mIsOperator;
}
const QString &Status::version() const
{
    return mData.version;
}
bool Status::isTun() const
{
    return mData.isTun;
}
const QString &Status::backendState() const
{
    return mData.backendState;
}
Peer *Status::self() const
{
    return mSelf;
}
QVector<Peer *> &Status::peers()
{
    return mPeers;
}
const QVector<Peer *> &Status::peers() const
{
    return mPeers;
}
Peer *Status::currentExitNode()
{
    return mCurrentExitNode;
}
const StatusData &Status::statusData() const
{
    return mData;
}
Peer *Status::suggestedExitNode() const
{
    return mSuggestedExitNode;
}

std::tuple<QList<Peer *>, QList<Peer *>> Status::exitNodes() const
{
    QList<Peer *> exit_nodes;
    QList<Peer *> mullvad_nodes;
    for (const Peer *peer : mPeers) {
        if (peer->isMullvad()) {
            mullvad_nodes.append(const_cast<Peer *>(peer));
        } else if (peer->isExitNode()) {
            exit_nodes.append(const_cast<Peer *>(peer));
        }
    }
    std::stable_sort(exit_nodes.begin(), exit_nodes.end(), [](const Peer *peer_a, const Peer *peer_b) {
        return peer_a->hostName() < peer_b->hostName();
    });
    std::stable_sort(mullvad_nodes.begin(), mullvad_nodes.end(), [](const Peer *peer_a, const Peer *peer_b) {
        if (peer_a->location() == nullptr || peer_b->location() == nullptr) {
            return peer_a->hostName() < peer_b->hostName();
        }
        if (peer_a->location()->countryCode() == peer_b->location()->countryCode()) {
            return peer_a->hostName() < peer_b->hostName();
        }
        return peer_a->location()->countryCode() < peer_b->location()->countryCode();
    });
    return {exit_nodes, mullvad_nodes};
}

void Status::setExitNode(const Peer *node)
{
    if (node == nullptr) {
        unsetExitNode();
        return;
    }

    GoUint8 false_ = 0;
    tailscale_set_advertise_exit_node(&false_);

    const QByteArray bytes = node->tailscaleIps().front().toUtf8();
    GoString tmp{bytes.data(), bytes.size()};
    tailscale_set_exit_node(&tmp);
}
void Status::setExitNodeFromIP(const QString &ip)
{
    // TODO: get rid of this
    GoUint8 false_ = 0;
    tailscale_set_advertise_exit_node(&false_);

    const QByteArray bytes = ip.toUtf8();
    GoString tmp{bytes.data(), bytes.size()};
    tailscale_set_exit_node(&tmp);
}
void Status::unsetExitNode()
{
    GoString tmp{nullptr, 0};
    tailscale_set_exit_node(&tmp);
}

void Status::update(StatusData &newData)
{
    if (newData.version != mData.version) {
        mData.version.swap(newData.version);
        emit versionChanged(mData.version);
    }
    if (newData.isTun != mData.isTun) {
        mData.isTun = newData.isTun;
        emit isTunChanged(mData.isTun);
    }
    if (newData.backendState != mData.backendState) {
        mData.backendState.swap(newData.backendState);
        emit backendStateChanged(mData.backendState);
    }
    if (mSelf == nullptr) {
        mSelf = new Peer(this);
    }
    mSelf->update(newData.self);

    // remove extra elements
    bool peerVectorChanged = false;
    if (!mPeers.isEmpty()) {
        std::for_each(mPeers.begin() + newData.peers.size(), mPeers.end(), [&peerVectorChanged](Peer *peer) {
            peer->deleteLater();
            peerVectorChanged = true;
        });
        mPeers.erase(mPeers.begin() + newData.peers.size(), mPeers.end());
    }

    // add elements to match size of newData.peers
    while (mPeers.size() < newData.peers.size()) {
        mPeers.append(new Peer(this));
        peerVectorChanged = true;
    }

    // update elements
    Peer *newExitNode = nullptr;
    for (int i = 0; i < newData.peers.size(); ++i) {
        mPeers[i]->update(newData.peers[i]);

        if (mPeers[i]->isCurrentExitNode()) {
            newExitNode = mPeers[i];
        }
    }
    if (newExitNode != mCurrentExitNode) {
        mCurrentExitNode = newExitNode;
        emit currentExitNodeChanged(mCurrentExitNode);
    }

    mData.peers.swap(newData.peers);

    if (peerVectorChanged) {
        emit peersChanged(mPeers);
    }
}
void Status::refresh()
{
    char *jsonStr = tailscale_status();
    const bool statusSuccess = jsonStr != nullptr;
    if (statusSuccess) {
        const std::unique_ptr<char, decltype(&free)> jsonStrPtr(jsonStr, free);
        json::parse(jsonStr).get_to<StatusData>(mNewData);
        update(mNewData);
        emit refreshed(*this);

        bool const newIsOperator = tailscale_is_operator() != 0U;
        if (newIsOperator != mIsOperator) {
            mIsOperator = newIsOperator;
            emit isOperatorChanged(mIsOperator);
        }

        const std::unique_ptr<char, decltype(&free)> suggestedExitNode(tailscale_suggest_exit_node(), free);
        const QString suggestedExitNodeStr = QString::fromUtf8(suggestedExitNode.get());
        if ((mSuggestedExitNode == nullptr) || (suggestedExitNodeStr != mSuggestedExitNode->id())) {
            mSuggestedExitNode = nullptr;
            for (Peer *peer : mPeers) {
                if (peer->id() == suggestedExitNodeStr) {
                    mSuggestedExitNode = peer;
                    break;
                }
            }
            emit suggestedExitNodeChanged(mSuggestedExitNode);
        }
    }
    if (mSuccess != statusSuccess) {
        mSuccess = statusSuccess;
        emit successChanged(mSuccess);
    }
}
