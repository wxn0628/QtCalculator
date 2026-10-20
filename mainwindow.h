#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QPushButton;
class QKeyEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDigitClicked();
    void onDotClicked();
    void onBackspaceClicked();
    void onClearAllClicked();
    void onClearEntryClicked();

private:
    void bindButtons();                      // 批量连接信号与槽
    void appendDigit(const QString &digit);  // 追加一位数字
    void clearAll();                         // C 键：全部复位
    void setDisplay(const QString &text);
    QString displayText() const;
    static QString formatNumber(double value);

    Ui::MainWindow *ui;
    bool m_waitingForOperand = true;         // true：下一个数字会覆盖显示区
};

#endif // MAINWINDOW_H
