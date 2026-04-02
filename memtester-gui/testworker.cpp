#include "testworker.h"
#include "../tests.h"
#include "../sizes.h"

#include <sys/types.h>

// External variables required by tests.c
int use_phys = 0;
off_t physaddrbase = 0;

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <QProcess>
#include <QFile>

TestWorker::TestWorker(const TestSettings &settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_stopped(0)
{
    m_pageSize = sysconf(_SC_PAGESIZE);
}

void TestWorker::stop()
{
    m_stopped.storeRelaxed(1);
}

void TestWorker::run()
{
    bool success = true;

    emit progress(QString("Memtester 版本 4.7.1 (%1位)").arg(UL_LEN));
    emit progress("开始内存测试...");
    emit progress(QString("内存: %1 字节").arg(m_settings.memoryBytes));
    emit progress(QString("页面大小: %1 字节").arg(m_pageSize));

    // Allocate memory
    size_t wantbytes = static_cast<size_t>(m_settings.memoryBytes);
    void volatile *buf = nullptr;
    void volatile *aligned = nullptr;
    size_t bufsize = 0;
    bool do_mlock = !m_settings.unlockedMemory;

    if (m_stopped.loadRelaxed()) {
        emit finished(false);
        return;
    }

    // Try to allocate memory
    while (!buf && wantbytes > 0) {
        buf = (void volatile *) malloc(wantbytes);
        if (!buf) {
            wantbytes -= m_pageSize;
        }
    }

    if (!buf) {
        emit progress("错误: 内存分配失败");
        emit finished(false);
        return;
    }

    bufsize = wantbytes;
    emit progress(QString("已分配 %1 MB").arg(wantbytes >> 20));

    // Align to page boundary
    ptrdiff_t pagesizemask = (ptrdiff_t) ~(m_pageSize - 1);
    if ((size_t) buf % m_pageSize) {
        aligned = (void volatile *) (((size_t) buf & pagesizemask) + m_pageSize);
        bufsize -= ((size_t) aligned - (size_t) buf);
    } else {
        aligned = buf;
    }

    // Try to lock memory
    if (do_mlock) {
        if (mlock((void *) aligned, bufsize) < 0) {
            emit progress("警告: 内存锁定失败，将继续使用未锁定内存");
            do_mlock = false;
        } else {
            emit progress("内存锁定成功");
        }
    }

    // Prepare buffers
    size_t halflen = bufsize / 2;
    size_t count = halflen / sizeof(ul);
    ulv *bufa = (ulv *) aligned;
    ulv *bufb = (ulv *) ((size_t) aligned + halflen);

    // Test definitions matching CLI order
    struct TestDef {
        const char *name;
        int (*fp)(ulv *, ulv *, size_t);
        quint32 mask;
    };

    TestDef tests[] = {
        { "随机值测试", test_random_value, 0x0001 },
        { "异或比较", test_xor_comparison, 0x0002 },
        { "减法比较", test_sub_comparison, 0x0004 },
        { "乘法比较", test_mul_comparison, 0x0008 },
        { "除法比较", test_div_comparison, 0x0010 },
        { "或运算比较", test_or_comparison, 0x0020 },
        { "与运算比较", test_and_comparison, 0x0040 },
        { "顺序递增", test_seqinc_comparison, 0x0080 },
        { "固定位测试", test_solidbits_comparison, 0x0100 },
        { "块顺序测试", test_blockseq_comparison, 0x0200 },
        { "棋盘格测试", test_checkerboard_comparison, 0x0400 },
        { "位扩散测试", test_bitspread_comparison, 0x0800 },
        { "位翻转测试", test_bitflip_comparison, 0x1000 },
        { "行走1测试", test_walkbits1_comparison, 0x2000 },
        { "行走0测试", test_walkbits0_comparison, 0x4000 },
        { nullptr, nullptr, 0 }
    };

    // Run tests in loops
    int loopNum = 1;
    while ((m_settings.loopCount == 0 || loopNum <= m_settings.loopCount) && !m_stopped.loadRelaxed()) {
        emit progress(QString("\n=== 第 %1 轮 ===").arg(loopNum));

        // Stuck Address test
        if (!m_stopped.loadRelaxed()) {
            emit progress("正在运行地址卡住测试...");
            int result = runStuckAddressTest((ulv*)aligned, bufsize / sizeof(ul));
            emit testResult("地址卡住测试", result == 0);
            if (result != 0) success = false;
        }

        // Other tests
        for (int i = 0; tests[i].name != nullptr && !m_stopped.loadRelaxed(); ++i) {
            if (m_settings.testMask & tests[i].mask) {
                emit progress(QString("正在运行 %1...").arg(tests[i].name));
                int result = runTest(tests[i].name, tests[i].fp, bufa, bufb, count);
                emit testResult(tests[i].name, result == 0);
                if (result != 0) success = false;

                // Clear buffer between tests
                memset((void *) buf, 0xFF, wantbytes);
            }
        }

        loopNum++;
    }

    // Cleanup
    if (do_mlock) {
        munlock((void *) aligned, bufsize);
    }
    free((void *) buf);

    emit progress("\n测试完成。");
    emit finished(success);
}

int TestWorker::runStuckAddressTest(ulv *bufa, size_t count)
{
    return test_stuck_address(bufa, count);
}

int TestWorker::runTest(const QString &name, int (*test_func)(ulv *, ulv *, size_t),
                        ulv *bufa, ulv *bufb, size_t count)
{
    Q_UNUSED(name)
    return test_func(bufa, bufb, count);
}

void TestWorker::clearBuffer(ulv *buf, size_t bytes)
{
    memset((void *) buf, 0, bytes);
}
