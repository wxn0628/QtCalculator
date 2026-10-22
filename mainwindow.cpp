#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QStyle>
#include <cmath>

namespace {

// 一个操作数最多允许输入的位数，防止显示区被撑爆
const int kMaxDigits = 16;

// 判断某个数是"0"，用于除零判断
bool isZero(double value)
{
    return std::fabs(value) < 1e-12;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("计算器-吴湘楠-2024414290333"));

    // 显示框只读，保证输入只能来自按键
    ui->editDisplay->setReadOnly(true);

    bindButtons();
    clearAll();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindButtons()
{
    // 数字键地位相同，共用一个槽函数，靠 sender() 区分是谁按的
    const QList<QPushButton *> digitKeys = {
        ui->btnNum0, ui->btnNum1, ui->btnNum2, ui->btnNum3, ui->btnNum4,
        ui->btnNum5, ui->btnNum6, ui->btnNum7, ui->btnNum8, ui->btnNum9
    };
    for (QPushButton *btn : digitKeys)
        connect(btn, &QPushButton::clicked, this, &MainWindow::onDigitClicked);

    m_operatorButtons = { ui->btnAdd, ui->btnSub, ui->btnMul, ui->btnDiv };
    for (QPushButton *btn : m_operatorButtons)
        connect(btn, &QPushButton::clicked, this, &MainWindow::onOperatorClicked);

    connect(ui->btnDot,        &QPushButton::clicked, this, &MainWindow::onDotClicked);
    connect(ui->btnEqual,      &QPushButton::clicked, this, &MainWindow::onEqualClicked);
    connect(ui->btnBackspace,  &QPushButton::clicked, this, &MainWindow::onBackspaceClicked);
    connect(ui->btnClearAll,   &QPushButton::clicked, this, &MainWindow::onClearAllClicked);
    connect(ui->btnClearEntry, &QPushButton::clicked, this, &MainWindow::onClearEntryClicked);
    connect(ui->btnSign,       &QPushButton::clicked, this, &MainWindow::onToggleSignClicked);
    connect(ui->btnPercent,    &QPushButton::clicked, this, &MainWindow::onPercentClicked);
    connect(ui->btnSquare,     &QPushButton::clicked, this, &MainWindow::onSquareClicked);
    connect(ui->btnSqrt,       &QPushButton::clicked, this, &MainWindow::onSqrtClicked);
    connect(ui->btnReciprocal, &QPushButton::clicked, this, &MainWindow::onReciprocalClicked);
}

/* ------------------------------ 数字与小数点 ------------------------------ */

void MainWindow::onDigitClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;

    if (m_error || m_resultShown)
        startNewInput();

    appendDigit(btn->text());
    m_operatorJustPressed = false;
    updateExpression();
}

void MainWindow::appendDigit(const QString &digit)
{
    if (m_waitingForOperand) {
        // 输入新数字，直接覆盖显示区（也就顺手解决了前导零）
        setDisplay(digit);
        m_waitingForOperand = false;
        return;
    }

    QString current = displayText();
    if (current == QLatin1String("0"))
        current.clear();
    else if (current == QLatin1String("-0"))
        current = QStringLiteral("-");

    int digits = 0;
    for (const QChar &ch : current) {
        if (ch.isDigit())
            ++digits;
    }
    if (digits >= kMaxDigits)
        return;

    setDisplay(current + digit);
}

void MainWindow::onDotClicked()
{
    if (m_error || m_resultShown)
        startNewInput();

    if (m_waitingForOperand) {
        // 直接按小数点时补一个前导 0，避免出现 ".5"
        setDisplay(QStringLiteral("0."));
        m_waitingForOperand = false;
    } else if (!displayText().contains(QLatin1Char('.'))) {
        // 已经有一个小数点就直接忽略，杜绝 5..6
        setDisplay(displayText() + QLatin1Char('.'));
    }

    m_operatorJustPressed = false;
    updateExpression();
}

/* ------------------------------ 运算符与等号 ------------------------------ */

void MainWindow::onOperatorClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;
    inputOperator(btn->text());
}

void MainWindow::inputOperator(const QString &op)
{
    if (m_error)
        return;

    // 连按运算符：只替换，不重复计算，也不把当前值当成第二操作数
    if (!m_pendingOp.isEmpty() && m_operatorJustPressed) {
        m_pendingOp = op;
        highlightOperator(op);
        updateExpression();
        return;
    }

    // 已经有待执行的运算，说明是连续运算，先把前一步算出来
    if (!m_pendingOp.isEmpty()) {
        calculate();
        if (m_error)
            return;
    }

    m_operand1 = displayText().toDouble();
    m_pendingOp = op;
    m_waitingForOperand = true;
    m_operatorJustPressed = true;
    m_resultShown = false;

    highlightOperator(op);
    updateExpression();
}

void MainWindow::onEqualClicked()
{
    if (m_error)
        return;

    if (m_pendingOp.isEmpty()) {
        // 没有待执行的运算，= 不改变显示，只是确认当前值
        m_resultShown = true;
        return;
    }
    calculate();
}

void MainWindow::calculate()
{
    if (m_pendingOp.isEmpty())
        return;

    const double left  = m_operand1;
    const double right = displayText().toDouble();   // 刚按完运算符时显示区仍是左操作数
    const QString op = m_pendingOp;

    double result = 0.0;
    if (op == QStringLiteral("+")) {
        result = left + right;
    } else if (op == QStringLiteral("−")) {
        result = left - right;
    } else if (op == QStringLiteral("×")) {
        result = left * right;
    } else if (op == QStringLiteral("÷")) {
        if (isZero(right)) {
            showError(QStringLiteral("除数不能为 0"));
            return;
        }
        result = left / right;
    }

    ui->lblExpression->setText(QStringLiteral("%1 %2 %3 =")
                                   .arg(formatNumber(left))
                                   .arg(op)
                                   .arg(formatNumber(right)));
    setDisplay(formatNumber(result));

    m_operand1 = result;          // 结果作为下一次运算的左操作数，支持连续运算
    m_pendingOp.clear();
    m_waitingForOperand = true;
    m_operatorJustPressed = false;
    m_resultShown = true;

    highlightOperator(QString());
}

/* -------------------------------- 单操作数 -------------------------------- */

void MainWindow::onToggleSignClicked()
{
    if (m_error) {
        clearAll();
        return;
    }
    applyUnary(-displayText().toDouble());
}

void MainWindow::onPercentClicked()
{
    if (m_error)
        return;
    applyUnary(displayText().toDouble() / 100.0);
}

void MainWindow::onSquareClicked()
{
    if (m_error)
        return;
    const double value = displayText().toDouble();
    applyUnary(value * value);
}

void MainWindow::onSqrtClicked()
{
    if (m_error)
        return;
    const double value = displayText().toDouble();
    if (value < 0.0) {
        showError(QStringLiteral("负数不能开平方"));
        return;
    }
    applyUnary(std::sqrt(value));
}

void MainWindow::onReciprocalClicked()
{
    if (m_error)
        return;
    const double value = displayText().toDouble();
    if (isZero(value)) {
        showError(QStringLiteral("0 没有倒数"));
        return;
    }
    applyUnary(1.0 / value);
}

void MainWindow::applyUnary(double value)
{
    setDisplay(formatNumber(value));

    // 单操作数的结果直接成为当前操作数：下一次按数字会重新开始，
    // 按运算符 / 等号则继续参与后面的计算
    m_waitingForOperand = true;
    m_resultShown = false;
    m_operatorJustPressed = false;

    if (m_pendingOp.isEmpty())
        ui->lblExpression->clear();
    else
        ui->lblExpression->setText(QStringLiteral("%1 %2 %3")
                                       .arg(formatNumber(m_operand1))
                                       .arg(m_pendingOp)
                                       .arg(formatNumber(value)));
}

/* -------------------------------- 清除与退格 ------------------------------- */

void MainWindow::onBackspaceClicked()
{
    if (m_error) {
        clearAll();
        return;
    }

    if (m_resultShown) {
        // 结果状态下退格等于重新输入
        setDisplay(QStringLiteral("0"));
        m_resultShown = false;
        m_waitingForOperand = false;
        return;
    }

    if (m_waitingForOperand)
        return;                       // 刚按完运算符，没有正在输入的数字可删

    QString current = displayText();
    current.chop(1);
    if (current.isEmpty() || current == QLatin1String("-"))
        current = QStringLiteral("0");
    setDisplay(current);

    m_operatorJustPressed = false;
    updateExpression();
}

void MainWindow::onClearEntryClicked()
{
    if (m_error) {
        clearAll();
        return;
    }
    // 只清当前输入，保留第一个操作数和运算符
    setDisplay(QStringLiteral("0"));
    m_waitingForOperand = false;
    m_resultShown = false;
    m_operatorJustPressed = false;
    updateExpression();
}

void MainWindow::onClearAllClicked()
{
    clearAll();
}

/* --------------------------------- 辅助函数 -------------------------------- */

void MainWindow::startNewInput()
{
    if (m_error) {
        clearAll();
        return;
    }
    m_operand1 = 0.0;
    m_pendingOp.clear();
    m_waitingForOperand = true;
    m_operatorJustPressed = false;
    m_resultShown = false;
    setDisplay(QStringLiteral("0"));
    ui->lblExpression->clear();
    highlightOperator(QString());
}

void MainWindow::clearAll()
{
    m_operand1 = 0.0;
    m_pendingOp.clear();
    m_waitingForOperand = true;
    m_operatorJustPressed = false;
    m_error = false;
    m_resultShown = false;

    setDisplay(QStringLiteral("0"));
    ui->lblExpression->clear();
    highlightOperator(QString());
}

void MainWindow::setDisplay(const QString &text)
{
    ui->editDisplay->setText(text);
}

QString MainWindow::displayText() const
{
    return ui->editDisplay->text();
}

void MainWindow::updateExpression()
{
    if (m_pendingOp.isEmpty()) {
        ui->lblExpression->clear();
        return;
    }
    QString expr = QStringLiteral("%1 %2").arg(formatNumber(m_operand1)).arg(m_pendingOp);
    if (!m_waitingForOperand)
        expr += QLatin1Char(' ') + displayText();
    ui->lblExpression->setText(expr);
}

void MainWindow::highlightOperator(const QString &op)
{
    // 正在使用的运算符高亮，样式表靠 active 动态属性区分
    for (QPushButton *btn : m_operatorButtons) {
        const bool active = !op.isEmpty() && btn->text() == op;
        if (btn->property("active").toBool() == active)
            continue;
        btn->setProperty("active", active);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

void MainWindow::showError(const QString &reason)
{
    setDisplay(QStringLiteral("错误"));
    ui->lblExpression->setText(reason);

    // 出错后把运算状态清干净，避免后续输入被拼到提示文字后面
    m_operand1 = 0.0;
    m_pendingOp.clear();
    m_waitingForOperand = true;
    m_operatorJustPressed = false;
    m_error = true;
    m_resultShown = false;

    highlightOperator(QString());
}

QString MainWindow::formatNumber(double value)
{
    if (std::isnan(value) || std::isinf(value))
        return QStringLiteral("错误");

    QString text = QString::number(value, 'g', 12);
    if (text == QLatin1String("-0"))
        text = QStringLiteral("0");
    return text;
}
