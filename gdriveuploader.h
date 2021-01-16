#pragma once

#include <QObject>
#include <QNetworkReply>

class GDriveUploaderPrivate;

class GDriveUploader : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GDriveUploader)
public:
    explicit GDriveUploader(QObject *parent = nullptr);

    void grantAuth();

public slots:
    void getDriveFilesList() const;
    void addFilesForUpload(const QStringList& fileNames);

signals:
    void grantFinished();
    void fileListReceived(const QStringList& fileNames) const;
    void networkError(QNetworkReply::NetworkError code);
    void reportProgress(int percentage);

    // debug purposes
    void displayLogMsg(const QString& msg);

private:
    const QScopedPointer<GDriveUploaderPrivate> d_ptr;
};
