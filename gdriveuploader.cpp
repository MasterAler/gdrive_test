#include "gdriveuploader.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMimeDatabase>
#include <QMimeType>

#include "authprovider.h"

namespace
{
const QString GDRIVE_API_URI = "https://www.googleapis.com/drive/v3/files";
const QString GDRIVE_UPLOAD_URI = "https://www.googleapis.com/upload/drive/v3/files?uploadType=resumable";
const int OK = 200;
const int CREATED = 200;
}

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
            emit q_ptr->displayLogMsg(QString("token: %1").arg(m_authToken));
        });
    }

    void ListRemoteFiles() const;
    bool InitFileUpload(const QString& path);
    bool LaunchUpload(const QString& path, const QString& uri);
    void ReportUploadAttemptEnded(const QString& path, bool ok);

private:
    GDriveUploader * const q_ptr;
    AuthProvider* m_authProvider;

    QNetworkAccessManager* m_networkManager = nullptr;
    QString m_authToken;

    QHash<QString, QString> m_sessions;
    QMimeDatabase m_mimeDB;
};

void GDriveUploaderPrivate::ListRemoteFiles() const
{
    Q_Q(const GDriveUploader);
    if (!m_networkManager || m_authToken.isEmpty())
        return;

    QNetworkRequest request(GDRIVE_API_URI);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());

    auto reply = m_networkManager->get(request);

    QObject::connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred), q, &GDriveUploader::networkError);
    QObject::connect(reply, &QNetworkReply::finished, [reply, q] {
        if (reply->error() == QNetworkReply::NoError) {
            QStringList result;

            auto response = QJsonDocument::fromJson(reply->readAll()).object();
            auto files = response["files"].toArray();
            for (const auto file: files)
                result << file.toObject()["name"].toString();
            emit q->fileListReceived(result);
        }
        else
            qCritical() << reply->errorString();

        reply->deleteLater();
    });
}

bool GDriveUploaderPrivate::InitFileUpload(const QString &path)
{
    Q_Q(GDriveUploader);
    QNetworkRequest request(GDRIVE_UPLOAD_URI);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "unable to open: " << path << " for upload:" << file.errorString();
        return false;
    }

    auto fileSize = QString::number(file.size()).toUtf8();
    file.close();

    QJsonObject body;
    body["name"] = QFileInfo(path).fileName();
    QByteArray bodyData = QJsonDocument(body).toJson();

    //set headers
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    request.setRawHeader("Content-Length", QString::number(bodyData.size()).toUtf8());
    request.setRawHeader("X-Upload-Content-Type", m_mimeDB.mimeTypeForFile(path).name().toUtf8());
    request.setRawHeader("X-Upload-Content-Type-Length", fileSize);

    auto reply = m_networkManager->post(request, bodyData);

    QObject::connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred), q, &GDriveUploader::networkError);

    QObject::connect(reply, &QNetworkReply::metaDataChanged, [this, path, reply, sz=file.size()]{
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid() && statusCode.toInt() == OK)
            m_sessions[path] = reply->header(QNetworkRequest::LocationHeader).toString();
    });

    QObject::connect(reply, &QNetworkReply::finished, [reply, path, q, this] {
        if (reply->error() == QNetworkReply::NoError)
        {
            QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (statusCode.isValid() && statusCode.toInt() == OK)
            {
                emit q->displayLogMsg(QString("Target %1 -- upload init").arg(path));
                if (!LaunchUpload(path, reply->header(QNetworkRequest::LocationHeader).toString()))
                    ReportUploadAttemptEnded(path, false);
            }
        }
        else
            emit q->displayLogMsg(reply->errorString());

        reply->deleteLater();
    });

    return true;
}

bool GDriveUploaderPrivate::LaunchUpload(const QString& path, const QString& uri)
{
    QFile targetFile(path);
    if (!targetFile.open(QIODevice::ReadOnly))
        return false;

    QNetworkRequest request(uri);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    request.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    request.setRawHeader("Content-Length", QString::number(targetFile.size()).toUtf8());

    auto reply = m_networkManager->put(request, targetFile.readAll());
    targetFile.close();

    Q_Q(GDriveUploader);
    QObject::connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred), q, &GDriveUploader::networkError);

    QObject::connect(reply, &QNetworkReply::finished, [reply, path, this] {
        if (reply->error() == QNetworkReply::NoError)
        {
            QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (statusCode.isValid() && (statusCode.toInt() == OK || statusCode.toInt() == CREATED))
                ReportUploadAttemptEnded(path, true);
        }
        else
            ReportUploadAttemptEnded(path, false);

        reply->deleteLater();
    });

    return true;
}

void GDriveUploaderPrivate::ReportUploadAttemptEnded(const QString& path, bool ok)
{
    Q_Q(GDriveUploader);
    m_sessions.remove(path);
    emit q->displayLogMsg(QString("Upload %1 : %2").arg(ok ? "SUCCESS" : "FAIL").arg(path));
}

/*************************************************************************************************************************/

GDriveUploader::GDriveUploader(QObject *parent)
    : QObject(parent)
    , d_ptr{new GDriveUploaderPrivate(this)}
{ }

void GDriveUploader::grantAuth()
{
    Q_D(GDriveUploader);
    d->m_authToken.clear();
    d->m_authProvider->grantAuth();
}

void GDriveUploader::addFilesForUpload(const QStringList &fileNames)
{
    Q_D(GDriveUploader);
    if (!d->m_networkManager || d->m_authToken.isEmpty())
        return;

    for(const QString& path : fileNames)
    {
        if (!d->InitFileUpload(path))
            emit displayLogMsg(QString("Bad file: %1").arg(path));
    }
}

void GDriveUploader::getDriveFilesList() const
{
    Q_D(const GDriveUploader);
    d->ListRemoteFiles();
}

