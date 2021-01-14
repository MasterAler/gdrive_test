#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class TestWidget; }
QT_END_NAMESPACE

class TestWidgetPrivate;

class TestWidget : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TestWidget)

public:
    TestWidget(QWidget *parent = nullptr);
    ~TestWidget();

private:
    QScopedPointer<TestWidgetPrivate> d_ptr;
};
