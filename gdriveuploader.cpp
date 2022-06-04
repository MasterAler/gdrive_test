#include "gdriveuploader.h"

#include <QFile>
#include <QThread>
#include <QNetworkReply>

#include "authprovider.h"

class GDriveUploaderPrivate
{
    Q_DECLARE_PUBLIC(GDriveUploader)
    GDriveUploaderPrivate(GDriveUploader *ownerPtr)
        : q_ptr(ownerPtr)
        , m_authProvider(new AuthProvider(ownerPtr))
    {

        QObject::connect(m_authProvider, &AuthProvider::tokenReceived, [this](const QString& token){
            m_authToken = token;
            m_networkManager = m_authProvider->getNetworkManager();
            emit q_ptr->grantFinished();
        });
    }

private:
    GDriveUploader * const q_ptr;
    AuthProvider* m_authProvider;

    QNetworkAccessManager* m_networkManager = nullptr;
    QString m_authToken;
};

/*************************************************************************************************************************/

GDriveUploader::GDriveUploader(QObject *parent)
    : QObject(parent)
    , d_ptr{new GDriveUploaderPrivate(this)}
{ }

void GDriveUploader::grantAuth()
{
    Q_D(GDriveUploader);
    d->m_authProvider->grantAuth();
}

void GDriveUploader::addFilesForUpload(const QStringList &fileNames)
{
    Q_D(GDriveUploader);
    if (!d->m_networkManager)
        return;

    QNetworkRequest request(QUrl("https://www.googleapis.com/drive/v3/files"));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(d->m_authToken).toUtf8());

    auto reply = d->m_networkManager->get(request);

    connect(reply, &QNetworkReply::readyRead, [reply]{
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Request error!";
        }
        qWarning() << QString::fromUtf8(reply->readAll());
        reply->deleteLater();
    });
}

void GDriveUploader::cancelUploads()
{

}
