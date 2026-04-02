#ifndef TESTWORKER_H
#define TESTWORKER_H

#include <QObject>
#include <QAtomicInt>
#include <cstddef>

// Test function types from original memtester
typedef unsigned long ul;
typedef unsigned long volatile ulv;

struct TestSettings {
    quint64 memoryBytes;
    int loopCount;
    quint32 testMask;
    bool usePhysicalAddr;
    QString physicalAddr;
    bool unlockedMemory;
};

class TestWorker : public QObject
{
    Q_OBJECT

public:
    explicit TestWorker(const TestSettings &settings, QObject *parent = nullptr);
    void stop();

public slots:
    void run();

signals:
    void progress(const QString &message);
    void testResult(const QString &testName, bool passed);
    void finished(bool success);

private:
    int runStuckAddressTest(ulv *bufa, size_t count);
    int runTest(const QString &name, int (*test_func)(ulv *, ulv *, size_t), ulv *bufa, ulv *bufb, size_t count);
    void clearBuffer(ulv *buf, size_t bytes);

    TestSettings m_settings;
    QAtomicInt m_stopped;
    int m_pageSize;
};

// Test functions (from tests.c)
extern "C" {
    int test_stuck_address(unsigned long volatile *bufa, size_t count);
    int test_random_value(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_xor_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_sub_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_mul_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_div_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_or_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_and_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_seqinc_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_solidbits_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_blockseq_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_checkerboard_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_walkbits0_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_walkbits1_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_bitspread_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
    int test_bitflip_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
}

#endif // TESTWORKER_H
