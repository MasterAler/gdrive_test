#include "testwidget.h"
#include "ui_testwidget.h"

#include <QDialog>
#include <QFileDialog>
#include <QSplashScreen>

#include "gdriveuploader.h"

class TestWidgetPrivate
{
    Q_DECLARE_PUBLIC(TestWidget)
    TestWidgetPrivate(TestWidget *ownerPtr)
        : q_ptr(ownerPtr)
        , ui(new Ui::TestWidget)
        , uploader{new GDriveUploader(ownerPtr)}
    { }

    void configUI()
    {
        QObject::connect(ui->choosePushButton, &QAbstractButton::clicked, [this]{
            auto fileNames = QFileDialog::getOpenFileNames(q_ptr
                                                           , QObject::tr("Выберите файлы для загрузки")
                                                           , QString()
                                                           , "Any file (*.*)");
            if (!fileNames.isEmpty())
                ui->filesListWidget->clear();

            for(const auto& name: fileNames)
                ui->filesListWidget->addItem(name);
        });

        QObject::connect(ui->filesListWidget, &QListWidget::doubleClicked, [this](const QModelIndex& index) {
            ui->filesListWidget->model()->removeRow(index.row());
        });

        QObject::connect(ui->uploadPushButton, &QAbstractButton::clicked, [this]{
            QStringList targets;
            for (int i = 0 ; i < ui->filesListWidget->model()->rowCount(); ++i)
                targets << ui->filesListWidget->item(i)->text();

            uploader->addFilesForUpload(targets);
        });

        QObject::connect(ui->listFilesPushButton, &QAbstractButton::clicked, uploader, &GDriveUploader::getDriveFilesList);

        ui->stackedWidget->setCurrentIndex(1);
        QObject::connect(uploader, &GDriveUploader::grantFinished, [this]{
            ui->stackedWidget->setCurrentIndex(0);
            ui->logTextEdit->setText(ui->logTextEdit->toPlainText() + "ACCESS GRANTED\n");
        });

        QObject::connect(uploader, &GDriveUploader::networkError, [this](QNetworkReply::NetworkError code){
            ui->logTextEdit->setText(ui->logTextEdit->toPlainText() + QString("\nNetwork error: %1").arg(code));
        });

        QObject::connect(uploader, &GDriveUploader::fileListReceived, [this](const QStringList& nameList){
            ui->driveFilesListWidget->clear();
            for(const QString& name : nameList) {
                ui->driveFilesListWidget->addItem(name);
            }
        });

        QObject::connect(uploader, &GDriveUploader::displayLogMsg, [this](const QString& msg){
            ui->logTextEdit->setText(ui->logTextEdit->toPlainText() + QString("\n%1\n").arg(msg));
        });

        uploader->dealWithAuth();
    }

private:
    TestWidget * const q_ptr;
    QScopedPointer<Ui::TestWidget> ui;

    GDriveUploader *uploader;
};

/*************************************************************************************************************************/

TestWidget::TestWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr{new TestWidgetPrivate(this)}
{
    d_ptr->ui->setupUi(this);
    d_ptr->configUI();
}

TestWidget::~TestWidget()
{ }
