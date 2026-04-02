#include "mainwindow.h"
#include "testworker.h"

#include <unistd.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QProcess>
#include <QFont>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_totalTests(0)
    , m_completedTests(0)
    , m_passedTests(0)
    , m_failedTests(0)
{
    setupUI();
    setupStyles();
    updateSystemInfo();
}

MainWindow::~MainWindow()
{
    if (m_workerThread && m_workerThread->isRunning()) {
        m_worker->stop();
        m_workerThread->quit();
        m_workerThread->wait(5000);
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("Memtester - 内存测试工具");
    setMinimumSize(800, 700);
    resize(960, 850);

    // Central widget with light background
    auto *centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #F2F2F7;");

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: #F2F2F7; border: none; }");

    auto *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background: #F2F2F7;");
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(16);

    // Title
    auto *titleLabel = new QLabel("内存测试工具", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setWeight(QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #1C1C1E; background: transparent;");
    contentLayout->addWidget(titleLabel);

    auto *subtitleLabel = new QLabel("Memtester GUI v4.7.1", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #8E8E93; font-size: 14px; background: transparent; margin-bottom: 10px;");
    contentLayout->addWidget(subtitleLabel);

    // System Info Card
    m_systemInfoLabel = new QLabel(this);
    m_systemInfoLabel->setAlignment(Qt::AlignLeft);
    m_systemInfoLabel->setWordWrap(true);
    auto *sysInfoCard = createCardWidget(m_systemInfoLabel, "#E3F2FD");
    contentLayout->addWidget(sysInfoCard);

    // Memory Settings Card
    auto *memorySettingsWidget = new QWidget();
    memorySettingsWidget->setStyleSheet("background: transparent;");
    auto *memLayout = new QVBoxLayout(memorySettingsWidget);
    memLayout->setContentsMargins(0, 0, 0, 0);
    memLayout->setSpacing(16);

    auto *memTitle = new QLabel("内存设置", this);
    memTitle->setStyleSheet("color: #1C1C1E; font-size: 18px; font-weight: 600; background: transparent;");
    memLayout->addWidget(memTitle);

    auto *sizeLayout = new QHBoxLayout();
    sizeLayout->setSpacing(10);

    auto *sizeLabel = new QLabel("测试内存大小:", this);
    sizeLabel->setStyleSheet("color: #3C3C43; font-size: 15px; background: transparent;");
    sizeLabel->setMinimumWidth(100);
    sizeLayout->addWidget(sizeLabel);

    m_memorySizeSpin = new QSpinBox(this);
    m_memorySizeSpin->setRange(1, 99999);
    m_memorySizeSpin->setValue(100);
    m_memorySizeSpin->setFixedHeight(36);
    m_memorySizeSpin->setStyleSheet(
        "QSpinBox {"
        "  background: #FFFFFF;"
        "  border: 1px solid #C7C7CC;"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "  font-size: 15px;"
        "  color: #1C1C1E;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button { width: 20px; }"
    );
    sizeLayout->addWidget(m_memorySizeSpin);

    m_memoryUnitCombo = new QComboBox(this);
    m_memoryUnitCombo->addItems({"MB", "KB", "GB", "B"});
    m_memoryUnitCombo->setCurrentIndex(0);
    m_memoryUnitCombo->setFixedHeight(36);
    m_memoryUnitCombo->setFixedWidth(80);
    m_memoryUnitCombo->setStyleSheet(
        "QComboBox {"
        "  background: #FFFFFF;"
        "  border: 1px solid #C7C7CC;"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "  font-size: 15px;"
        "  color: #1C1C1E;"
        "}"
    );
    sizeLayout->addWidget(m_memoryUnitCombo);
    sizeLayout->addStretch();
    memLayout->addLayout(sizeLayout);

    // Loop count
    auto *loopLayout = new QHBoxLayout();
    auto *loopLabel = new QLabel("循环次数:", this);
    loopLabel->setStyleSheet("color: #3C3C43; font-size: 15px; background: transparent;");
    loopLabel->setMinimumWidth(100);
    loopLayout->addWidget(loopLabel);

    m_loopCountSpin = new QSpinBox(this);
    m_loopCountSpin->setRange(0, 9999);
    m_loopCountSpin->setValue(1);
    m_loopCountSpin->setSpecialValueText("无限");
    m_loopCountSpin->setFixedHeight(36);
    m_loopCountSpin->setMinimumWidth(100);
    m_loopCountSpin->setStyleSheet(m_memorySizeSpin->styleSheet());
    loopLayout->addWidget(m_loopCountSpin);
    loopLayout->addStretch();
    memLayout->addLayout(loopLayout);

    contentLayout->addWidget(createCardWidget(memorySettingsWidget));

    // Advanced Settings Card
    auto *advancedWidget = new QWidget();
    advancedWidget->setStyleSheet("background: transparent;");
    auto *advLayout = new QVBoxLayout(advancedWidget);
    advLayout->setContentsMargins(0, 0, 0, 0);
    advLayout->setSpacing(12);

    auto *advTitle = new QLabel("高级设置", this);
    advTitle->setStyleSheet("color: #1C1C1E; font-size: 18px; font-weight: 600; background: transparent;");
    advLayout->addWidget(advTitle);

    m_usePhysicalAddrCheck = new QCheckBox("使用物理内存地址映射", this);
    m_usePhysicalAddrCheck->setStyleSheet(
        "QCheckBox { font-size: 15px; color: #1C1C1E; spacing: 10px; background: transparent; }"
        "QCheckBox::indicator { width: 48px; height: 28px; border-radius: 14px; }"
        "QCheckBox::indicator:unchecked { background: #E5E5EA; border: none; }"
        "QCheckBox::indicator:checked { background: #34C759; border: none; }"
    );
    advLayout->addWidget(m_usePhysicalAddrCheck);

    m_physicalAddrEdit = new QLineEdit(this);
    m_physicalAddrEdit->setPlaceholderText("0x00000000 (十六进制地址)");
    m_physicalAddrEdit->setEnabled(false);
    m_physicalAddrEdit->setFixedHeight(36);
    m_physicalAddrEdit->setStyleSheet(
        "QLineEdit {"
        "  background: #FFFFFF;"
        "  border: 1px solid #C7C7CC;"
        "  border-radius: 8px;"
        "  padding: 6px 10px;"
        "  font-size: 14px;"
        "  color: #1C1C1E;"
        "}"
        "QLineEdit:disabled { background: #E5E5EA; color: #8E8E93; }"
    );
    advLayout->addWidget(m_physicalAddrEdit);

    m_unlockedMemoryCheck = new QCheckBox("不使用内存锁定 (mlock) - 测试更快但可靠性较低", this);
    m_unlockedMemoryCheck->setStyleSheet(
        "QCheckBox { font-size: 15px; color: #1C1C1E; spacing: 10px; background: transparent; }"
        "QCheckBox::indicator { width: 48px; height: 28px; border-radius: 14px; }"
        "QCheckBox::indicator:unchecked { background: #E5E5EA; border: none; }"
        "QCheckBox::indicator:checked { background: #FF9500; border: none; }"
    );
    advLayout->addWidget(m_unlockedMemoryCheck);

    contentLayout->addWidget(createCardWidget(advancedWidget));

    // Test Selection Card
    auto *testWidget = new QWidget();
    testWidget->setStyleSheet("background: transparent;");
    auto *testLayout = new QVBoxLayout(testWidget);
    testLayout->setContentsMargins(0, 0, 0, 0);
    testLayout->setSpacing(12);

    auto *testTitle = new QLabel("选择测试项目", this);
    testTitle->setStyleSheet("color: #1C1C1E; font-size: 18px; font-weight: 600; background: transparent;");
    testLayout->addWidget(testTitle);

    // Test selection buttons
    auto *testBtnLayout = new QHBoxLayout();
    m_selectAllButton = createSmallButton("全部选择", "#007AFF");
    m_deselectAllButton = createSmallButton("全部取消", "#8E8E93");
    testBtnLayout->addWidget(m_selectAllButton);
    testBtnLayout->addWidget(m_deselectAllButton);
    testBtnLayout->addStretch();
    testLayout->addLayout(testBtnLayout);

    // Test switches
    QStringList testNames = {
        "随机值测试", "异或比较", "减法比较", "乘法比较", "除法比较",
        "或运算比较", "与运算比较", "顺序递增", "固定位测试", "块顺序测试",
        "棋盘格测试", "位扩散测试", "位翻转测试", "行走1测试", "行走0测试"
    };

    auto *testGrid = new QGridLayout();
    testGrid->setSpacing(6);
    testGrid->setHorizontalSpacing(16);
    int row = 0, col = 0;
    for (const QString &name : testNames) {
        auto *checkBox = new QCheckBox(name, this);
        checkBox->setChecked(true);
        checkBox->setStyleSheet(
            "QCheckBox { font-size: 13px; color: #1C1C1E; spacing: 6px; background: transparent; }"
            "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; }"
            "QCheckBox::indicator:unchecked { border: 1.5px solid #C7C7CC; background: white; }"
            "QCheckBox::indicator:checked { border: 1.5px solid #007AFF; background: #007AFF; }"
        );
        m_testCheckBoxes.append(checkBox);
        testGrid->addWidget(checkBox, row, col);
        if (++col > 1) { col = 0; row++; }
    }
    testLayout->addLayout(testGrid);

    contentLayout->addWidget(createCardWidget(testWidget));

    // Control buttons
    auto *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(12);

    m_startButton = createActionButton("开始测试", "#007AFF", true);
    m_stopButton = createActionButton("停止测试", "#FF3B30", false);
    m_stopButton->setEnabled(false);

    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);
    contentLayout->addLayout(controlLayout);

    // Progress
    auto *progressLabel = new QLabel("测试进度", this);
    progressLabel->setStyleSheet("color: #8E8E93; font-size: 13px; background: transparent;");
    contentLayout->addWidget(progressLabel);

    m_overallProgress = new QProgressBar(this);
    m_overallProgress->setRange(0, 100);
    m_overallProgress->setValue(0);
    m_overallProgress->setTextVisible(true);
    m_overallProgress->setFixedHeight(20);
    m_overallProgress->setStyleSheet(
        "QProgressBar {"
        "  background: #E5E5EA;"
        "  border: none;"
        "  border-radius: 10px;"
        "  text-align: center;"
        "  font-size: 12px;"
        "  color: #1C1C1E;"
        "}"
        "QProgressBar::chunk {"
        "  background: #34C759;"
        "  border-radius: 10px;"
        "}"
    );
    contentLayout->addWidget(m_overallProgress);

    // Results Card
    auto *resultsCardWidget = new QWidget();
    resultsCardWidget->setStyleSheet("background: transparent;");
    auto *resultsCardLayout = new QVBoxLayout(resultsCardWidget);
    resultsCardLayout->setContentsMargins(0, 0, 0, 0);
    resultsCardLayout->setSpacing(10);

    auto *resultsTitle = new QLabel("实时测试结果", this);
    resultsTitle->setStyleSheet("color: #1C1C1E; font-size: 18px; font-weight: 600; background: transparent;");
    resultsCardLayout->addWidget(resultsTitle);

    m_resultsWidget = new QWidget();
    m_resultsWidget->setStyleSheet("background: transparent;");
    m_resultsLayout = new QVBoxLayout(m_resultsWidget);
    m_resultsLayout->setContentsMargins(0, 0, 0, 0);
    m_resultsLayout->setSpacing(6);
    m_resultsLayout->addStretch();
    resultsCardLayout->addWidget(m_resultsWidget);

    contentLayout->addWidget(createCardWidget(resultsCardWidget));

    // Log output
    auto *logTitle = new QLabel("详细日志输出", this);
    logTitle->setStyleSheet("color: #8E8E93; font-size: 13px; background: transparent;");
    contentLayout->addWidget(logTitle);

    m_logOutput = new QTextEdit(this);
    m_logOutput->setReadOnly(true);
    m_logOutput->setMinimumHeight(150);
    m_logOutput->setStyleSheet(
        "QTextEdit {"
        "  background: #1C1C1E;"
        "  border: none;"
        "  border-radius: 12px;"
        "  padding: 12px;"
        "  font-family: monospace;"
        "  font-size: 13px;"
        "  color: #34C759;"
        "}"
    );
    contentLayout->addWidget(m_logOutput);

    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    setCentralWidget(centralWidget);

    // Connections
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_selectAllButton, &QPushButton::clicked, this, &MainWindow::onSelectAllTests);
    connect(m_deselectAllButton, &QPushButton::clicked, this, &MainWindow::onDeselectAllTests);
    connect(m_usePhysicalAddrCheck, &QCheckBox::toggled, m_physicalAddrEdit, &QLineEdit::setEnabled);
}

QWidget* MainWindow::createCardWidget(QWidget *content, const QString &bgColor)
{
    auto *frame = new QFrame(this);
    frame->setFrameShape(QFrame::NoFrame);

    QString bg = bgColor.isEmpty() ? "#FFFFFF" : bgColor;
    frame->setStyleSheet(
        QString("QFrame { background: %1; border-radius: 12px; border: none; }").arg(bg)
    );

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(16);
    shadow->setColor(QColor(0, 0, 0, 20));
    shadow->setOffset(0, 2);
    frame->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->addWidget(content);

    return frame;
}

QPushButton* MainWindow::createSmallButton(const QString &text, const QString &color)
{
    auto *btn = new QPushButton(text, this);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(32);
    btn->setStyleSheet(
        QString("QPushButton {"
        "  background: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 16px;"
        "  padding: 6px 16px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: %2; }"
        "QPushButton:pressed { background: %3; }")
        .arg(color)
        .arg(color == "#8E8E93" ? "#636366" : adjustColor(color, -20))
        .arg(color == "#8E8E93" ? "#48484A" : adjustColor(color, -40))
    );
    return btn;
}

QPushButton* MainWindow::createActionButton(const QString &text, const QString &color, bool isPrimary)
{
    Q_UNUSED(isPrimary)
    auto *btn = new QPushButton(text, this);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(44);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setStyleSheet(
        QString("QPushButton {"
        "  background: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 10px 24px;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: %2; }"
        "QPushButton:pressed { background: %3; }"
        "QPushButton:disabled { background: #C7C7CC; }")
        .arg(color).arg(adjustColor(color, -15)).arg(adjustColor(color, -30))
    );
    return btn;
}

QString MainWindow::adjustColor(const QString &color, int delta)
{
    QColor c(color);
    int h, s, v;
    c.getHsv(&h, &s, &v);
    v = qBound(0, v + delta, 255);
    c.setHsv(h, s, v);
    return c.name();
}

void MainWindow::setupStyles()
{
    setStyleSheet(
        "QMainWindow { background: #F2F2F7; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #C7C7CC; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #8E8E93; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );
}

void MainWindow::updateSystemInfo()
{
    QProcess process;
    process.start("uname", QStringList() << "-m");
    process.waitForFinished();
    QString arch = QString(process.readAllStandardOutput()).trimmed();

    process.start("nproc");
    process.waitForFinished();
    QString cpus = QString(process.readAllStandardOutput()).trimmed();

    long pagesize = sysconf(_SC_PAGESIZE);
    long totalPages = sysconf(_SC_PHYS_PAGES);
    long long totalMem = (long long)pagesize * totalPages;
    int totalMB = totalMem / (1024 * 1024);
    double totalGB = totalMB / 1024.0;

    QString info = QString(
        "<table style='line-height: 1.6; font-size: 14px;'>"
        "<tr><td style='color:#666; width:70px;'>系统架构</td><td style='color:#1a1a1a; font-weight:600;'>%1</td></tr>"
        "<tr><td style='color:#666;'>CPU核心</td><td style='color:#1a1a1a; font-weight:600;'>%2 核</td></tr>"
        "<tr><td style='color:#666;'>页面大小</td><td style='color:#1a1a1a; font-weight:600;'>%3 KB</td></tr>"
        "<tr><td style='color:#666;'>总内存</td><td style='color:#1a1a1a; font-weight:600;'>%4 GB (%5 MB)</td></tr>"
        "</table>"
    ).arg(arch).arg(cpus).arg(pagesize / 1024).arg(QString::number(totalGB, 'f', 2)).arg(totalMB);

    m_systemInfoLabel->setText(info);
    m_systemInfoLabel->setStyleSheet("background: transparent;");
}

void MainWindow::onStartClicked()
{
    m_logOutput->clear();

    QLayoutItem *item;
    while ((item = m_resultsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }
    m_resultsLayout->addStretch();

    quint32 testMask = 0;
    for (int i = 0; i < m_testCheckBoxes.size(); ++i) {
        if (m_testCheckBoxes[i]->isChecked()) testMask |= (1u << i);
    }

    if (testMask == 0) {
        QMessageBox::warning(this, "未选择测试", "请至少选择一项测试运行。");
        return;
    }

    quint64 memorySize = m_memorySizeSpin->value();
    QString unit = m_memoryUnitCombo->currentText();
    int shift = (unit == "KB") ? 10 : (unit == "MB") ? 20 : (unit == "GB") ? 30 : 0;
    memorySize <<= shift;

    if (memorySize < 4096) {
        QMessageBox::warning(this, "内存太小", "内存大小必须至少为4KB。");
        return;
    }

    TestSettings settings;
    settings.memoryBytes = memorySize;
    settings.loopCount = m_loopCountSpin->value();
    settings.testMask = testMask;
    settings.usePhysicalAddr = m_usePhysicalAddrCheck->isChecked();
    settings.physicalAddr = m_physicalAddrEdit->text();
    settings.unlockedMemory = m_unlockedMemoryCheck->isChecked();

    m_totalTests = 0;
    for (int i = 0; i < m_testCheckBoxes.size(); ++i) {
        if (m_testCheckBoxes[i]->isChecked()) m_totalTests++;
    }
    m_totalTests *= (settings.loopCount == 0) ? 100 : settings.loopCount;
    m_completedTests = m_passedTests = m_failedTests = 0;

    m_overallProgress->setValue(0);
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);

    m_workerThread = new QThread(this);
    m_worker = new TestWorker(settings);
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, &TestWorker::run);
    connect(m_worker, &TestWorker::progress, this, &MainWindow::onTestProgress);
    connect(m_worker, &TestWorker::testResult, this, &MainWindow::onTestResult);
    connect(m_worker, &TestWorker::finished, this, &MainWindow::onTestComplete);
    connect(m_worker, &TestWorker::finished, m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished, m_worker, &TestWorker::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QThread::deleteLater);

    m_workerThread->start();
}

void MainWindow::onStopClicked()
{
    if (m_worker) {
        m_worker->stop();
        m_logOutput->append("正在停止测试...");
    }
}

void MainWindow::onTestProgress(const QString &message)
{
    m_logOutput->append(message);
    QScrollBar *sb = m_logOutput->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::onTestResult(const QString &testName, bool passed)
{
    m_completedTests++;
    if (passed) m_passedTests++; else m_failedTests++;

    int progress = (m_completedTests * 100) / qMax(m_totalTests, 1);
    m_overallProgress->setValue(progress);

    auto *resultLabel = new QLabel(QString("%1 %2").arg(passed ? "✓" : "✗").arg(testName));
    resultLabel->setStyleSheet(
        QString("QLabel { font-size: 14px; color: %1; padding: 4px 10px; background: %2; border-radius: 6px; }")
            .arg(passed ? "#1a8a2e" : "#c41e16").arg(passed ? "#e8f5e9" : "#ffebee")
    );
    m_resultsLayout->insertWidget(m_resultsLayout->count() - 1, resultLabel);
}

void MainWindow::onTestComplete(bool success)
{
    Q_UNUSED(success)
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);

    QString summary = (m_failedTests == 0)
        ? QString("✓ 所有测试通过！(共 %1 项)").arg(m_passedTests)
        : QString("⚠ 完成: %1 通过, %2 失败").arg(m_passedTests).arg(m_failedTests);
    m_logOutput->append("\n" + summary);
    m_overallProgress->setValue(100);

    m_worker = nullptr;
    m_workerThread = nullptr;
}

void MainWindow::onMemorySizeChanged(int value)
{
    Q_UNUSED(value)
}

void MainWindow::onSelectAllTests()
{
    for (auto *cb : m_testCheckBoxes) cb->setChecked(true);
}

void MainWindow::onDeselectAllTests()
{
    for (auto *cb : m_testCheckBoxes) cb->setChecked(false);
}

QFrame* MainWindow::createSeparator()
{
    auto *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #E5E5EA;");
    return line;
}

QPushButton* MainWindow::createIOSButton(const QString &text, bool isPrimary)
{
    Q_UNUSED(isPrimary)
    auto *btn = new QPushButton(text, this);
    btn->setMinimumHeight(44);
    return btn;
}

void MainWindow::createTestSwitches()
{
}
