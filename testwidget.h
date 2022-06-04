#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class TestWidget; }
QT_END_NAMESPACE

class TestWidgetPrivate;

/*!
 * \brief The TestWidget class
 * Просто минимальный интерфейс для теста.
 * Как проверялось:
 * 0. Открывается бразуер, там запрашивается авторизация. Токен после протухания придётся сбрасывать в
 * https://myaccount.google.com/permissions, потому что он там остаётся даже через 3599 сек, но для теста хватает.
 * 1. Choose -- выбрать сколько-то файлов
 * 2. Двойной щелчок по файлу в списке удаляет его оттуда
 * 3. Upload -- грузит
 * 4. List Files -- посмотреть, что файл появился
 */
class TestWidget : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TestWidget)

public:
    TestWidget(QWidget *parent = nullptr);
    ~TestWidget();

private:
    const QScopedPointer<TestWidgetPrivate> d_ptr;
};
