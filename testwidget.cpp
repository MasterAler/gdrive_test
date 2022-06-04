#include "testwidget.h"
#include "ui_testwidget.h"

#include <QDialog>
#include <QFileDialog>

class TestWidgetPrivate
{
    Q_DECLARE_PUBLIC(TestWidget)
    TestWidgetPrivate(TestWidget *ownerPtr)
        : q_ptr(ownerPtr)
        , ui(new Ui::TestWidget)
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
    }

private:
    TestWidget * const q_ptr;
    QScopedPointer<Ui::TestWidget> ui;
};

/****************************************************************/

TestWidget::TestWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr{new TestWidgetPrivate(this)}
{
    d_ptr->ui->setupUi(this);
    d_ptr->configUI();
}

TestWidget::~TestWidget()
{ }

