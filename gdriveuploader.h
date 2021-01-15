#pragma once

#include <QObject>

class GDriveUploaderPrivate;

class GDriveUploader : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GDriveUploader)
public:
    explicit GDriveUploader(QObject *parent = nullptr);

    void grantAuth();

    void addFilesForUpload(const QStringList& fileNames);

    void cancelUploads();

signals:
    void grantFinished();
    void reportProgress(int percentage);

private:
    const QScopedPointer<GDriveUploaderPrivate> d_ptr;
};
