#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

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
    void onOperatorClicked();
    void onEqualClicked();
    void onBackspaceClicked();
    void onClearAllClicked();
    void onClearEntryClicked();
    void onToggleSignClicked();
    void onPercentClicked();
    void onSquareClicked();
    void onSqrtClicked();
    void onReciprocalClicked();

private:
    void bindButtons();                       // 批量连接信号与槽
    void appendDigit(const QString &digit);   // 追加一位数字
    void inputOperator(const QString &op);    // 处理 + - × ÷
    void calculate();                         // 执行一次双操作数运算
    void applyUnary(double value);            // 单操作数运算结果的统一收尾
    void startNewInput();                     // 结果/错误状态下重新开始
    void clearAll();                          // C 键：全部复位
    void setDisplay(const QString &text);
    QString displayText() const;
    void updateExpression();
    void highlightOperator(const QString &op);
    void showError(const QString &reason);
    static QString formatNumber(double value);

    Ui::MainWindow *ui;
    QList<QPushButton *> m_operatorButtons;

    double  m_operand1 = 0.0;              // 第一个操作数
    QString m_pendingOp;                   // 待执行的运算符，空串表示没有
    bool    m_waitingForOperand = true;    // true：下一个数字会覆盖显示区
    bool    m_operatorJustPressed = false; // true：刚按过运算符，再按只替换运算符
    bool    m_error = false;               // 是否处于错误状态
    bool    m_resultShown = false;         // 显示区是否是运算结果
};

#endif // MAINWINDOW_H
