#include "tailscale.h"
#include "libktailctl_wrapper.h"

Tailscale::Tailscale(QObject *parent)
    : QObject(parent)
    , mPeerModel(new PeerModel(this))
    , mExitNodeModel(new ExitNodeModel(this))
    , mMullvadNodeModel(new MullvadNodeModel(this))
    , mMullvadCountryModel(new MullvadCountryModel(this))
{
    mExitNodeModel->setSourceModel(mPeerModel);
    mMullvadNodeModel->setSourceModel(mPeerModel);
}

Tailscale *Tailscale::instance()
{
    static Tailscale instance;
    return &instance;
}

PeerModel *Tailscale::peerModel() const
{
    return mPeerModel;
}
ExitNodeModel *Tailscale::exitNodeModel() const
{
    return mExitNodeModel;
}
MullvadNodeModel *Tailscale::mullvadNodeModel() const
{
    return mMullvadNodeModel;
}
MullvadCountryModel *Tailscale::mullvadCountryModel() const
{
    return mMullvadCountryModel;
}
bool Tailscale::success() const
{
    return mSuccess;
}
const QString &Tailscale::version() const
{
    return mVersion;
}
bool Tailscale::isOperator() const
{
    return mIsOperator;
}
const QString &Tailscale::backendState() const
{
    return mBackendState;
}
const PeerData &Tailscale::self() const
{
    return mSelf;
}
bool Tailscale::hasSuggestedExitNode() const
{
    return mHasSuggestedExitNode;
}
bool Tailscale::hasCurrentExitNode() const
{
    return mHasCurrentExitNode;
}
const PeerData &Tailscale::suggestedExitNode() const
{
    return mSuggestedExitNode;
}
const PeerData &Tailscale::currentExitNode() const
{
    return mCurrentExitNode;
}

void Tailscale::up()
{
    tailscale_up();
}
void Tailscale::down()
{
    tailscale_down();
}
void Tailscale::toggle()
{
    if (mBackendState == "Running") {
        down();
    } else {
        up();
    }
}
void Tailscale::refresh()
{
    StatusData data;
    {
        std::unique_ptr<char, decltype(&free)> jsonStr(tailscale_status(), free);
        if (jsonStr == nullptr) {
            if (mSuccess) {
                mSuccess = false;
                emit successChanged(mSuccess);
            }
            return;
        }
    }
    if (!mSuccess) {
        mSuccess = true;
        emit successChanged(mSuccess);
    }
    json::parse(tailscale_status()).get_to(data);
    std::stable_sort(data.peers.begin(), data.peers.end(), [](const PeerData &a, const PeerData &b) {
        return a.mId < b.mId;
    });

    QString suggestedExitNode;
    {
        const std::unique_ptr<char, decltype(&free)> suggestedExitNodePtr(tailscale_suggest_exit_node(), free);
        suggestedExitNode = QString::fromUtf8(suggestedExitNodePtr.get());
        if (suggestedExitNode.isEmpty()) {
            if (mHasSuggestedExitNode) {
                mHasSuggestedExitNode = false;
                emit hasSuggestedExitNodeChanged(mHasSuggestedExitNode);
            }
        } else {
            if (!mHasSuggestedExitNode) {
                mHasSuggestedExitNode = true;
                emit hasSuggestedExitNodeChanged(mHasSuggestedExitNode);
            }
        }
    }

    if (data.version != mVersion) {
        mVersion = data.version;
        emit versionChanged(mVersion);
    }
    if (data.backendState != mBackendState) {
        mBackendState = data.backendState;
        emit backendStateChanged(mBackendState);
    }
    if (data.self != mSelf) {
        mSelf = data.self;
        emit selfChanged(mSelf);
    }

    const bool isOperator = tailscale_is_operator();
    if (isOperator != mIsOperator) {
        mIsOperator = isOperator;
        emit isOperatorChanged(mIsOperator);
    }

    mPeerModel->update(data.peers);

    QMap<QString, QString> countries;
    // TODO: collect this data when bulding peer model?
    bool foundCurrentExitNode = false;
    for (const PeerData &peer : data.peers) {
        if (peer.mIsMullvad) {
            countries.insert(peer.mCountryCode, peer.mCountry);
        }
        if (peer.mId == suggestedExitNode) {
            if (mSuggestedExitNode != peer) {
                mSuggestedExitNode = peer;
                emit suggestedExitNodeChanged(mSuggestedExitNode);
            }
        }
        if (peer.mIsCurrentExitNode) {
            foundCurrentExitNode = true;
            if (!mHasCurrentExitNode) {
                mHasCurrentExitNode = true;
                emit hasCurrentExitNodeChanged(true);
            }
            if (peer != mCurrentExitNode) {
                mCurrentExitNode = peer;
                emit currentExitNodeChanged(mCurrentExitNode);
            }
        }
    }
    if (!foundCurrentExitNode) {
        if (mHasCurrentExitNode) {
            mHasCurrentExitNode = false;
            emit hasCurrentExitNodeChanged(false);
        }
    }

    mMullvadCountryModel->update(countries);

    emit refreshed();
}

void Tailscale::setExitNode(const QString &ip)
{
    const QByteArray bytes = ip.toUtf8();
    GoString tmp{bytes.data(), bytes.size()};
    tailscale_set_exit_node(&tmp);
}
void Tailscale::unsetExitNode()
{
    GoString tmp{nullptr, 0};
    tailscale_set_exit_node(&tmp);
}