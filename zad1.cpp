#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "timer/include/timer.h"

using namespace std;

class Semaphore {
public:
    Semaphore() : count(1) {}

    void wait() {
        unique_lock<mutex> lock(mx);
        while (count <= 0) {
            condition.wait(lock);
        }
        count--;
    }

    void signal() {
        unique_lock<mutex> lock(mx);
        count++;
        condition.notify_one(); // разблокировка ожидающего потока
    }

private:
    mutex mx;
    condition_variable condition;
    int count;
};

class SemaphoreSlim {
public:
    SemaphoreSlim() : count(1) {}

    void wait() {
        unique_lock<mutex> lock(mx);
        while (count <= 0) {
            condition.wait(lock);
        }
        count--;
    }

    void signal() {
        {
            unique_lock<mutex> lock(mx);
            count++;
        }
        condition.notify_one(); // разблокировка ожидающего потока
    }

private:
    mutex mx;
    condition_variable condition;
    int count;
};

class Barrier {
public:
    Barrier(int count) : thread_count(count), waiting(0) {}

    void wait() {
        unique_lock<mutex> lock(mx);
        waiting++;
        if (waiting == thread_count) {
            // Все потоки достигли барьера
            waiting = 0; // Сбросить для следующего использования
            cv.notify_all(); // Разбудить все потоки
        } else {
            // Ожидание других потоков
            cv.wait(lock);
        }
    }

private:
    mutex mx;
    condition_variable cv;
    int thread_count;
    int waiting;
};

class Monitor {
public:
    void enter() {
        mx.lock();
    }

    void exit() {
        mx.unlock();
    }

    void wait() {
        unique_lock<mutex> lock(mx);
        cv.wait(lock); // Ожидание сигнала
    }

    void signal() {
        unique_lock<mutex> lock(mx);
        cv.notify_one(); // Разбудить один поток
    }
private:
    mutex mx;
    condition_variable cv;
};

class genRandChar {
public:
    genRandChar(int threadCount) : barrier(threadCount) {} // для барьера

    void genRandCharMutex(int threadId, int countSimv) {
        lock_guard<mutex> lock(mx);
        for (int i = 0; i < countSimv; ++i) {
            char random_char = 32 + rand() % 95;
            cout << "Поток " << threadId << ": " << random_char << endl;
        }
    }

    void genRandCharSemaphore(int threadId, int countSimv) {
        sema.wait();
        for (int i = 0; i < countSimv; ++i) {
            char random_char = 32 + rand() % 95;
            cout << "Поток " << threadId << ": " << random_char << endl;
        }
        sema.signal();
    }

    void genRandCharSemaphoreSlim(int threadId, int countSimv) {
        semaslim.wait();
        for (int i = 0; i < countSimv; ++i) {
            char random_char = 32 + rand() % 95;
            cout << "Поток " << threadId << ": " << random_char << endl;
        }
        semaslim.signal();
    }

    void genRandCharBarrier(int threadId, int countSimv) {
        for (int i = 0; i < countSimv; ++i) {
            lock_guard<mutex> lock(mx);
            char random_char = 32 + rand() % 95;
            cout << "Поток " << threadId << ": " << random_char << endl;
        }
        barrier.wait();
    }

    void genRandCharSpinLock(int threadId, int countSimv) {
        while(spinlock.test_and_set(memory_order_acquire));
        for (int i = 0; i < countSimv; ++i) {
            char random_char = 32 + rand() % 95;
            cout << "Поток " << threadId << ": " << random_char << endl;
        }
        spinlock.clear(memory_order_acquire);
    }

    void genRandCharSpinWait(int threadId, int countSimv) {
        while(spinlock.test_and_set(memory_order_acquire)) {
            this_thread::yield();
        }
        for (int i = 0; i < countSimv; ++i) {
            char random_char = 32 + rand() % 95;
            cout << "Поток " << threadId << ": " << random_char << endl;
        }
        spinlock.clear(memory_order_acquire);
    }

    void genRandCharMonitor(int threadId, int countSimv) {
        monitor.enter();
        for (int i = 0; i < countSimv; ++i) {
            char random_char = 32 + rand() % 95;
            cout << "Поток " << threadId << ": " << random_char << endl;
        }
        spinlock.clear(memory_order_acquire);
        monitor.exit();
    }
private:
    mutex mx;
    Semaphore sema;
    SemaphoreSlim semaslim;
    Barrier barrier;
    atomic_flag spinlock = ATOMIC_FLAG_INIT;
    Monitor monitor;
};

int main() {
    srand(time(0));

    int countThreads, countSimv;
    cout << "Введите кол-во потоков: ";
    cin >> countThreads;
    cout << "Введите кол-во символов на поток: ";
    cin >> countSimv;

    vector<thread> threads(countThreads);

    cout << "Mutex:" << "\n----------------------------\n";
    {
        Timer t;
        genRandChar generator(countThreads);
        for (int i = 0; i < countThreads; ++i) {
            threads[i] = thread([&] () { generator.genRandCharMutex(i, countSimv); });
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        for (int i = 0; i < countThreads; ++i) {
            threads[i].join();
        }
    }
    cout << "----------------------------" << endl;

    cout << endl << "Semaphore:" << "\n----------------------------\n";
    {
        Timer t;
        genRandChar generator(countThreads);
        for (int i = 0; i < countThreads; ++i) {
            threads[i] = thread([&] () { generator.genRandCharSemaphore(i, countSimv); });
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        for (int i = 0; i < countThreads; ++i) {
            threads[i].join();
        }
    }
    cout << "----------------------------" << endl;

    cout << endl << "SemaphoreSlim:" << "\n----------------------------\n";
    {
        Timer t;
        genRandChar generator(countThreads);
        for (int i = 0; i < countThreads; ++i) {
            threads[i] = thread([&] () { generator.genRandCharSemaphoreSlim(i, countSimv); });
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        for (int i = 0; i < countThreads; ++i) {
            threads[i].join();
        }
    }
    cout << "----------------------------" << endl;

    cout << endl << "Barrier:" << "\n----------------------------\n";
    {
        Timer t;
        genRandChar generator(countThreads);
        for (int i = 0; i < countThreads; ++i) {
            threads[i] = thread([&] () { generator.genRandCharBarrier(i, countSimv); });
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        for (int i = 0; i < countThreads; ++i) {
            threads[i].join();
        }
    }
    cout << "----------------------------" << endl;

    cout << endl << "SpinLock:" << "\n----------------------------\n";
    {
        Timer t;
        genRandChar generator(countThreads);
        for (int i = 0; i < countThreads; ++i) {
            threads[i] = thread([&] () { generator.genRandCharSpinLock(i, countSimv); });
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        for (int i = 0; i < countThreads; ++i) {
            threads[i].join();
        }
    }
    cout << "----------------------------" << endl;

    cout << endl << "SpinWait:" << "\n----------------------------\n";
    {
        Timer t;
        genRandChar generator(countThreads);
        for (int i = 0; i < countThreads; ++i) {
            threads[i] = thread([&] () { generator.genRandCharSpinWait(i, countSimv); });
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        for (int i = 0; i < countThreads; ++i) {
            threads[i].join();
        }
    }
    cout << "----------------------------" << endl;

    cout << endl << "Monitor:" << "\n----------------------------\n";
    {
        Timer t;
        genRandChar generator(countThreads);
        for (int i = 0; i < countThreads; ++i) {
            threads[i] = thread([&] () { generator.genRandCharMonitor(i, countSimv); });
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        for (int i = 0; i < countThreads; ++i) {
            threads[i].join();
        }
    }
    cout << "----------------------------" << endl;
    
    cout << endl << "Главный поток завершился" << endl;

    return 0;
}