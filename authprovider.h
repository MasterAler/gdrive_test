#pragma once

#include <QObject>

class QOAuth2AuthorizationCodeFlow;
class QOAuthHttpServerReplyHandler;
class QNetworkAccessManager;

/*!
 * \brief The AuthProvider
 * должен обновлять данные о токенах, птому что там есть expires_in,
 * а ещё лучше --  действовать через refresh tokens, но не стал развивать эту тему,
 * особенно потому, что в Qt эти классы очень сырые, надо было бы тащить стороннюю либу тогда
 */
class AuthProvider : public QObject
{
    Q_OBJECT
public:
    explicit AuthProvider(QObject *parent = nullptr);

    void grantAuth();

    QNetworkAccessManager* getNetworkManager() const;

signals:
    void tokenReceived(const QString& token);

private:
    QOAuth2AuthorizationCodeFlow* m_gdrive;
    QOAuthHttpServerReplyHandler* m_replyHandler = nullptr;
};
