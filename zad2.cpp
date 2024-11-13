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
        while (true) {
            Training training;
            cout << endl << "Введите дату тренировки, формат ввода: dd-mm-yyyy(Для завершения - exit): ";
            cin >> training.date;
            if (training.date == "exit") {
                cout << endl;
                return;
            }
            while (!checkDate(training.date)) {
                cout << "Нарушены правила записи даты, повторите ввод: ";
                cin >> training.date;
                if (training.date == "exit") break;
                
            }
            cout << "Введите время тренировки, формат ввода: hh:mm: ";
            cin >> training.time;
            while (!checkTime(training.time)) {
                cout << "Нарушены правила записи времени, повторите ввод: ";
                cin >> training.time;
            }
            cout << "Введите имя тренера: ";
            cin >> training.coach;

            trainings.push_back(training);
        }
    }

    void getDayWeek(int& dayweek) {
        lock_guard<mutex> lock(mx);
        for (int i = 0; i < trainings.size(); ++i) {
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
                cout << "Дата: " << trainings[i].date << " | " << "Время: " << trainings[i].time << " | " << "Тренер: " << trainings[i].coach << endl;
            }
        }
    }

    
private:
    struct Training {
        string date; // 
        string time; // hh:mm
        string coach;
    };
    vector<Training> trainings;
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
            int countTread;
            cout << "Введите кол-во потоков: ";
            cin >> countTread;
            cout << endl;
            {
                Timer t;
                cout << "Однопоточная обработка: " << endl;
                zal.getDayWeek(dayweek);
            }
    
            {
                Timer t;
                cout << endl << "Многопоточная обработка: " << endl;
                vector<thread> threads(countTread);
                for (int i = 0; i < countTread; ++i) {
                    threads[i] = thread([&zal, &dayweek] () { zal.getDayWeek(dayweek); });
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