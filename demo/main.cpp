// Copyright 2022 Shevelyova Darya photodoshfy@gmail.com

#include "picosha2.h"
#include "hash.hpp"
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <thread>

// Флаги
std::atomic<bool> continueProcess = true;
std::atomic<bool> addToJson = false;

// Сигнал - нажатие Ctri+C
void stopProcess(int param) {
    if (param == SIGINT) {
        continueProcess = false;
    }
}

// Генератор хэш-функции + уровень логгирования
void hashGenerator(JsonFiler& j) {
    while (continueProcess) {
        std::string genstring = std::to_string(std::rand());
        // Генерация шестнадцатеричной строки хэша SHA256 из std::string
        std::string hash = picosha2::hash256_hex_string(genstring);
        // Фиксируем время
        std::time_t timestamp(std::time(nullptr));
        // Возвращаем последние 4 символа хэша
        std::string lastChar = hash.substr(hash.size() - NumZeroes);

        // Проверяем, это хэш с 4-мя нулями в конце?
        if (lastChar == Ending) {
            // Уровень логирования для данного события info - нашли нужный хэш
            BOOST_LOG_TRIVIAL(info) << "Necessary ending found in hash |" << hash
                                    << "| generarted from string |" << genstring << "|";
            // Создаём элемент json-файла (сохраним хэш-функцию)
            if (addToJson) {
                j.NewElement(genstring, hash, timestamp);
            }
        } else {
            // Уровень логирования для данного события trace - хэш не подходит
            BOOST_LOG_TRIVIAL(trace)
                    << "Hash |" << hash << "| generated from string|" << genstring << "|";
        }
    }
}
void StartProcess(const int& argc, char **argv){
    // Количество потоков
    unsigned int numberOfThreads;
    std::string json_path;
    std::srand(time(nullptr));
    switch (argc) {
        case 1:
            numberOfThreads = std::thread::hardware_concurrency();
            break;
        case 2:
            numberOfThreads = std::atoi(argv[1]);
            if (numberOfThreads == 0 ||
                numberOfThreads > std::thread::hardware_concurrency()) {
                throw std::out_of_range(" Invalid number of threads!!!");
            }
            break;
        case 3:
            numberOfThreads = std::atoi(argv[1]);
            if (numberOfThreads == 0 ||
                numberOfThreads > std::thread::hardware_concurrency()) {
                throw std::out_of_range(" Invalid number of threads!!!");
            }
            json_path = argv[2];
            addToJson = true;
            break;
        default:
            throw std::out_of_range("Invalid number of arguments!!!");
    }
    SetUpLogging();
    // Ведение журнала логгирования
    boost::log::add_common_attributes();
    // Инициализация потоков
    std::vector<std::thread> threads;
    // Json-объект
    JsonFiler json_obj;
    threads.reserve(numberOfThreads);
    // Обработка Ctrl+C
    std::signal(SIGINT, stopProcess);

    for (size_t i = 0; i < numberOfThreads; i++) {
        // Передаём в потоки хэши
        threads.emplace_back(hashGenerator, std::ref(json_obj));
    }
    for (auto& thread : threads) {
        // Ожидаем завершения каждого потока
        thread.join();
    }
    // Добавляем найденные хэши в json-файл
    if (addToJson) {
        std::ofstream fout{json_path};
        fout << json_obj;
    }
}

int main(int argc, char* argv[]) {
   StartProcess(argc,argv);
   return 0;
}
//int main() {
//    std::cout << std::thread::hardware_concurrency();
//    return 0;
//}

