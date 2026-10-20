#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

namespace {

// 一个操作数最多允许输入的位数，防止显示区被撑爆
const int kMaxDigits = 16;

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

    connect(ui->btnDot,        &QPushButton::clicked, this, &MainWindow::onDotClicked);
    connect(ui->btnBackspace,  &QPushButton::clicked, this, &MainWindow::onBackspaceClicked);
    connect(ui->btnClearAll,   &QPushButton::clicked, this, &MainWindow::onClearAllClicked);
    connect(ui->btnClearEntry, &QPushButton::clicked, this, &MainWindow::onClearEntryClicked);
}

void MainWindow::onDigitClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;
    appendDigit(btn->text());
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
    if (m_waitingForOperand) {
        // 直接按小数点时补一个前导 0，避免出现 ".5"
        setDisplay(QStringLiteral("0."));
        m_waitingForOperand = false;
    } else if (!displayText().contains(QLatin1Char('.'))) {
        // 已经有一个小数点就直接忽略，杜绝 5..6
        setDisplay(displayText() + QLatin1Char('.'));
    }
}

void MainWindow::onBackspaceClicked()
{
    if (m_waitingForOperand)
        return;                       // 没有正在输入的数字可删

    QString current = displayText();
    current.chop(1);
    if (current.isEmpty() || current == QLatin1String("-"))
        current = QStringLiteral("0");
    setDisplay(current);
}

void MainWindow::onClearEntryClicked()
{
    // 只清当前输入
    setDisplay(QStringLiteral("0"));
    m_waitingForOperand = false;
}

void MainWindow::onClearAllClicked()
{
    clearAll();
}

void MainWindow::clearAll()
{
    m_waitingForOperand = true;
    setDisplay(QStringLiteral("0"));
}

void MainWindow::setDisplay(const QString &text)
{
    ui->editDisplay->setText(text);
}

QString MainWindow::displayText() const
{
    return ui->editDisplay->text();
}

QString MainWindow::formatNumber(double value)
{
    QString text = QString::number(value, 'g', 12);
    if (text == QLatin1String("-0"))
        text = QStringLiteral("0");
    return text;
}
