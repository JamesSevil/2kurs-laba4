#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <thread>
#include <mutex>
#include "timer/include/timer.h"

using namespace std;

class Zal {
public:
    void addTraining() {
        int n;
        cout << "Введите кол-во тренировок: ";
        cin >> n;

        for (int i = 0; i < n; ++i) {
            Training training;

            // дата
            while (true) {
                int day = 1 + rand() % 30;
                int month = 1 + rand() % 11;
                int year = 2000 + rand() % 24;
                string date = to_string(day) + '-' + to_string(month) + '-' + to_string(year);
                if (checkDate(date)) {
                    training.date = date;
                    break;
                }
            }

            // время
            while (true) {
                int hours = 0 + rand() % 23;
                int minute = 0 + rand() % 60;
                string time = to_string(hours) + ':' + to_string(minute);
                if (checkTime(time)) {
                    training.time = time;
                    break;
                }
            }
            
            // тренер
            char coach = 32 + rand() % 95;
            training.coach = coach;

            trainings.push_back(training);
        }
    }

    void getDayWeek(int& dayweek, int start, int end) {
        for (int i = start; i < end; ++i) {
            int day, month, year;
            stringstream stream(trainings[i].date);
            string token;
            getline(stream, token, '-');
            day = stoi(token);
            getline(stream, token, '-');
            month = stoi(token);
            getline(stream, token, '-');
            year = stoi(token);
            tm date;
            date.tm_year = year - 1900;
            date.tm_mon = month - 1;
            date.tm_mday = day;

            mktime(&date); // получаем инфо о дате
            int weekday = date.tm_wday; // определяем день недели
            
            if (dayweek == weekday) {
                lock_guard<mutex> lock(mx);
                cout << "Дата: " << trainings[i].date << " | " << "Время: " << trainings[i].time << " | " << "Тренер: " << trainings[i].coach << endl;
            }
        }
           
    }

    struct Training {
        string date; // dd-mm-yyyy
        string time; // hh:mm
        string coach;
    };

    vector<Training> trainings;
    
private:
    mutex mx;

    bool checkDate(string& date) {
        stringstream stream(date);
        string token;
        getline(stream, token, '-');
        if (stoi(token) < 1 || stoi(token) > 31) return false; // проверка дня
        string day = token; // сохраняем день

        getline(stream, token, '-');
        if (stoi(token) < 1 || stoi(token) > 12) return false; // проверка месяца
        else if ((stoi(token) % 2 == 0) && (stoi(day) > 30) && (stoi(token) != 8)) return false;// соблюдение правил даты(30 или 31 день и август)
        else if ((stoi(token) == 2) && (stoi(day) > 29)) return false; // соблюдение правил даты(февраль)

        return true;
    }

    bool checkTime(string& time) {
        stringstream stream(time);
        string token;
        getline(stream, token, ':');
        if (stoi(token) < 0 || stoi(token) > 23) return false; // проверка часов
        
        getline(stream, token, ':');
        if (stoi(token) < 0 || stoi(token) > 59) return false; // проверка минут

        return true;
    }
};


int main() {
    srand(time(0));

    Zal zal;

    cout << "Выберите действие: 1-добавить тренировки, 2-найти тренировки, 3-выход" << endl;
    while (true) {
        string choice;
        cout << "Ваш выбор: ";
        cin >> choice;

        if (choice == "1") zal.addTraining();
        
        else if (choice == "2") {
            int dayweek;
            cout << endl << "Введите день недели для поиска тренировок(0 - воскресенье, 1 - понедельник, ..., 6 - суббота): ";
            cin >> dayweek;
            cout << endl;
            {
                Timer t;
                cout << "Однопоточная обработка: " << endl;
                zal.getDayWeek(dayweek, 0, zal.trainings.size());
            }

            int countTread;
            cout << "Введите кол-во потоков: ";
            cin >> countTread;
            int size = zal.trainings.size();
            int chunkSize = size / countTread;
            {
                Timer t;
                cout << endl << "Многопоточная обработка: " << endl;
                vector<thread> threads(countTread);
                for (int i = 0; i < countTread; ++i) {
                    int start = i * chunkSize;
                    int end = (i == countTread - 1) ? size : start + chunkSize; // Обработка последнего потока
                    threads[i] = thread([&zal, &dayweek, &start, &end] () { zal.getDayWeek(dayweek, start, end); });
                }
                for (int i = 0; i < countTread; ++i) {
                    threads[i].join();
                }
            }

        } else if (choice == "3") {
            cout << "Выход..." << endl;
            break;
        }
        
        else cout << "Ошибка, нет такого действия!" << endl;
    }

    return 0;
}