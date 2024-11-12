#include <iostream>
#include <vector>
#include <thread>

using namespace std;

void genRandChar(int threadId, int countSimv) {
    for (int i = 0; i < countSimv; ++i) {
        char random_char = 32 + rand() % 95;
        cout << "Поток " << threadId << ": " << random_char << endl;
    }
    
}

int main() {
    srand(time(0));

    int countThreads, countSimv;
    cout << "Введите кол-во потоков: ";
    cin >> countThreads;
    cout << "Введите кол-во символов на поток: ";
    cin >> countSimv;

    for (int i = 0; i < countThreads; ++i) {
        thread th(genRandChar, i, countSimv);
        th.detach();
    }
    
    cout << "Главный поток завершился" << endl;

    return 0;
}