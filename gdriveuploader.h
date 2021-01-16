#pragma once

#include <QObject>
#include <QNetworkReply>

class GDriveUploaderPrivate;

/*!
 * \brief The GDriveUploader class это
 * простенький загрузчик файлов, использует "resumableUpload", но *не* разбивает файл на фрагменты (Осторожнее с RAM!).
 * QNetWorkAccessManager под капотом умеет, AFAIK, в 6 соединений одновременно, потому файлы грузятся немножко параллельно.
 *
 * Чтобы было совсем production-ready, сюда следует добавить:
 * 1. Обработку NetworkError, ошибок 4xx и прочего -- надо навтыкать перезапусков
 * 2. Возможность грузить действительно большие файлы, технически это просто обработка Content-Range и повторная
 * посылка PUT, из структуры моего кода видно, что я бы взял обработчик &QNetworkReply::finished и вызывал бы досылку (возможно, рекурсия).
 * 3. (??) Возможно, чем раз открывать файл и делать seek(), стоило бы оставлять его открытым, чтобы не делать seek() много раз
 * 4. Также Content-Range + обработка ошибок позволяют делать докачку
 * 5. Очень хочется сделать AuthProvider синглтоном, ибо в данном примере он выступает в таком качестве, но вообще авторизация
 * может быть гораздо более сложной, в разных scope, потому я не могу сейчас сказать, что в этой части что-то стоит менять
 * 6. Перезапрос авторизации по истечению тоже должен быть в обработке ошибок, отдельно отмечу
 * 7. Возможно, кому-то хотелось бы "более параллельную загрузку", тогда это libcurl какой-нибудь уже, ну или QTcpSocket в потоки пихать
 *
 * Кстати, изначально я хотел взять QOAuth2AuthorizationCodeFlow и использовать его QNetWorkAccessManager, т.к. ожидал, что у него не придётся
 * вручную настраивать заголовки. Ну да, ну да...
 *
 * Вместо логгера сделан сигнал, хотя на самом деле это был бы логгер.
 */
class GDriveUploader : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GDriveUploader)
public:
    explicit GDriveUploader(QObject *parent = nullptr);

    void dealWithAuth();

public slots:
    void requestDriveFilesList() const;
    void addFilesForUpload(const QStringList& fileNames);

signals:
    void authObtained();
    void fileListReceived(const QStringList& fileNames) const;
    void networkError(QNetworkReply::NetworkError code);

    // Скорее для удобства демонстрации,
    // всё это можно сделать лучше, (неточный + NetworkError всё испортит),
    // но считаем, что это не важно
    void reportProgress(int percentage);

    // debug purposes
    void displayLogMsg(const QString& msg) const;

private:
    const QScopedPointer<GDriveUploaderPrivate> d_ptr;
};
