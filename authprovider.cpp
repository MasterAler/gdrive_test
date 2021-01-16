#include "authprovider.h"

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include <QFile>
#include <QSettings>
#include <QDateTime>

namespace
{
const QString ACCESS_TOKEN_KEY = "access_token";
const QString EXPIRATION_KEY = "expires_in";
}

AuthProvider::AuthProvider(QObject *parent)
    : QObject(parent)
    , m_gdrive(new QOAuth2AuthorizationCodeFlow(this))
    , m_tokenStorage(new QSettings(QSettings::IniFormat, QSettings::UserScope, "Interviewing", "TestApp", this))
{
    QFile clientRes(":/res/client_id.json"); // hardcoded, oops
    if (!clientRes.open(QIODevice::ReadOnly))
        throw std::runtime_error("No resource, magic oO");

    auto authData = QJsonDocument::fromJson(clientRes.readAll());

    const auto object = authData.object();
    const auto settingsObject = object["web"].toObject();
    const QUrl authUri(settingsObject["auth_uri"].toString());
    const auto clientId = settingsObject["client_id"].toString();
    const QUrl tokenUri(settingsObject["token_uri"].toString());
    const auto clientSecret(settingsObject["client_secret"].toString());
    const auto redirectUris = settingsObject["redirect_uris"].toArray();
    const QUrl redirectUri(redirectUris[0].toString()); // Get the first URI
    const auto port = static_cast<quint16>(redirectUri.port()); // Get the port

    m_gdrive->setScope("https://www.googleapis.com/auth/drive");
    QObject::connect(m_gdrive, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

    m_gdrive->setAuthorizationUrl(authUri);
    m_gdrive->setClientIdentifier(clientId);
    m_gdrive->setAccessTokenUrl(tokenUri);
    m_gdrive->setClientIdentifierSharedKey(clientSecret);

    m_replyHandler = new QOAuthHttpServerReplyHandler(port, this);
    m_gdrive->setReplyHandler(m_replyHandler);

    QObject::connect(m_replyHandler, &QOAuthHttpServerReplyHandler::tokensReceived, [this](const QVariantMap& tokens)
    {
        m_tokenStorage->setValue(ACCESS_TOKEN_KEY, tokens[ACCESS_TOKEN_KEY]);
        m_tokenStorage->setValue(EXPIRATION_KEY, QDateTime::currentSecsSinceEpoch() + tokens[EXPIRATION_KEY].toInt());
        m_tokenStorage->sync();

        emit tokenReady(tokens[ACCESS_TOKEN_KEY].toString());
    });

    // ---------

    m_authToken = m_tokenStorage->value(ACCESS_TOKEN_KEY).toString();
    qint64 expirationTime = m_tokenStorage->value(EXPIRATION_KEY).toLongLong();
    if (expirationTime < QDateTime::currentSecsSinceEpoch())
        m_authToken.clear();
}

QString AuthProvider::getToken() const
{
    return m_authToken;
}

void AuthProvider::grantAuth()
{
    m_gdrive->grant();
}
