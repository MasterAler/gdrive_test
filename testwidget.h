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
 * 0. Открывается бразуер, там запрашивается авторизация. Токен перед каждым запуском придётся сбрасывать,
 * потому что
 * 1. Choose -- выбрать сколько-то файлов
 * 2. Двойной щелчок по файлу в списке удаляет его оттуда
 * 3. Upload -- грузит
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
