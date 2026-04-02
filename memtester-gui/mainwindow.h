#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include "testworker.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QProgressBar;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QTextEdit;
class QGroupBox;
class QScrollArea;
class QWidget;
class QFrame;
class QVBoxLayout;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onTestProgress(const QString &message);
    void onTestResult(const QString &testName, bool passed);
    void onTestComplete(bool success);
    void onMemorySizeChanged(int value);
    void onSelectAllTests();
    void onDeselectAllTests();
    void updateSystemInfo();

private:
    void setupUI();
    void setupStyles();
    void createTestSwitches();
    QWidget *createCardWidget(QWidget *content, const QString &bgColor = QString());
    QFrame *createSeparator();
    QPushButton *createIOSButton(const QString &text, bool isPrimary = false);
    QPushButton *createSmallButton(const QString &text, const QString &color);
    QPushButton *createActionButton(const QString &text, const QString &color, bool isPrimary);
    QString adjustColor(const QString &color, int delta);

    // System info
    QLabel *m_systemInfoLabel;

    // Memory settings
    QSpinBox *m_memorySizeSpin;
    QComboBox *m_memoryUnitCombo;
    QSpinBox *m_loopCountSpin;

    // Advanced settings
    QCheckBox *m_usePhysicalAddrCheck;
    QLineEdit *m_physicalAddrEdit;
    QCheckBox *m_unlockedMemoryCheck;

    // Test switches
    QVector<QCheckBox*> m_testCheckBoxes;

    // Control buttons
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_selectAllButton;
    QPushButton *m_deselectAllButton;

    // Progress and results
    QProgressBar *m_overallProgress;
    QTextEdit *m_logOutput;
    QWidget *m_resultsWidget;
    QVBoxLayout *m_resultsLayout;

    // Worker thread
    QThread *m_workerThread;
    TestWorker *m_worker;
    int m_totalTests;
    int m_completedTests;
    int m_passedTests;
    int m_failedTests;
};

#endif // MAINWINDOW_H
