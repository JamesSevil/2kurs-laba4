#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

using namespace std;

class WriterAndReader {
public:
    WriterAndReader(bool check) : countWriter(0), countReader(0), priority(check) {}

    void startWriting() {
        unique_lock<mutex> lock(mx);
        if (priority) {
            while (countWriter > 0) {
                cv.wait(lock); // ждём других писателей
            }
            countWriter++;
        } else {
            while (countReader > 0 || countWriter > 0) {
                cv.wait(lock); // ждём читателей и других писателей
            }
            countWriter++;
        }
    }

    void stopWriting() {
        unique_lock<mutex> lock(mx);
        countWriter--;
        if (countWriter == 0) {
            cv.notify_all(); // уведомляем всех, что можно
        }
    }

    void startReading() {
        unique_lock<mutex> lock(mx);
        if (!priority) {
            countReader++;
        } else {
            while (countWriter > 0) {
                cv.wait(lock); // ожидаем писателей
            }
            countReader++;
        }
    }

    void stopReading() {
        unique_lock<mutex> lock(mx);
        countReader--;
        cv.notify_all(); // уведомляем писателей, что можно
    }

    void replacePriority(bool smena) {
        priority = smena;
    }
private:
    mutex mx;
    condition_variable cv;
    int countWriter; // кол-во активных писателей
    int countReader; // кол-во активный читателей
    bool priority; // приоритет (писатели - true, читатели - false)
};

void writer(WriterAndReader& rw, int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        rw.startWriting();
        cout << "Писатель " << id << " пишет" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000)); // Симуляция записи
        rw.stopWriting();
    }
}

void reader(WriterAndReader& rw, int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        rw.startReading();
        cout << "Читатель " << id << " читает" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000)); // Симуляция чтения
        rw.stopReading();
    }
}

int main() {

    WriterAndReader rw(true);
    int iterations = 1;
    int countWriter = 2;
    int countReader = 6;

    cout << endl << "Приоритет писателей: " << endl;
    vector<thread> threadsWriter(countWriter);
    vector<thread> threadsReader(countReader);
    for (int i = 0; i < countWriter; ++i) {
        threadsWriter[i] = thread(writer, ref(rw), i+1, iterations);
    }
    this_thread::sleep_for(chrono::milliseconds(2000));
    for (int i = 0; i < countReader; ++i) {
        threadsReader[i] = thread(reader, ref(rw), i+1, iterations);
    }
    for (auto& th : threadsWriter) {
        th.join();
    }
    for (auto& th : threadsReader) {
        th.join();
    }

    rw.replacePriority(false);

    cout << endl << "Приоритет читателей: " << endl;
    threadsWriter.clear();
    threadsReader.clear();
    for (int i = 0; i < countWriter; ++i) {
        threadsWriter[i] = thread(writer, ref(rw), i+1, iterations);
    }
    for (int i = 0; i < countReader; ++i) {
        threadsReader[i] = thread(reader, ref(rw), i+1, iterations);
    }
    for (auto& th : threadsWriter) {
        th.join();
    }
    for (auto& th : threadsReader) {
        th.join();
    }


    return 0;
}