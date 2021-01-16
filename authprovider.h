#pragma once

#include <QObject>

class QOAuth2AuthorizationCodeFlow;
class QOAuthHttpServerReplyHandler;
class QNetworkAccessManager;
class QSettings;

/*!
 * \brief The AuthProvider class
 * содержит авторизацию, сделано через очень сырые классы из `networkauth`,
 * потому оставлено достаточно тупо. В идеале тут должны быть всякие refresh token хотя бы.
 * Хотя `expires_in` приходит, с перезапросом наблюдаются проблемы, т.к. было указано, что
 * можно с этим не заморачиваться -- следует сбросить у приложения авторизацию если её заклинит.
 */
class AuthProvider : public QObject
{
    Q_OBJECT
public:
    explicit AuthProvider(QObject *parent = nullptr);

    QString getToken() const;

public slots:
    void grantAuth();

signals:
    void tokenReady(const QString& token);

private:
    QOAuth2AuthorizationCodeFlow* m_gdrive;
    QOAuthHttpServerReplyHandler* m_replyHandler = nullptr;
    QSettings* m_tokenStorage;

    QString m_authToken;
};
