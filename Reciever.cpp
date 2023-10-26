
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

const int MAX_MESSAGE_LENGTH = 20;
const int MAX_SENDERS = 10;
const char FILE_NAME[] = "message_file.bin";

// Структура для семафоров
struct sembuf sem_wait = {0, -1, 0};
struct sembuf sem_signal = {0, 1, 0};

// Функция для создания бинарного файла для сообщений
void createMessageFile(int numMessages) {
    std::ofstream messageFile(FILE_NAME, std::ios::out | std::ios::binary);
    for (int i = 0; i < numMessages; i++) {
        char emptyMessage[MAX_MESSAGE_LENGTH] = {0};
        messageFile.write(emptyMessage, MAX_MESSAGE_LENGTH);
    }
    messageFile.close();
}

int main() {
    int numSenders;
    std::cout << "Введите количество записей в бинарном файле: ";
    int numMessages;
    std::cin >> numMessages;
    createMessageFile(numMessages);

    std::cout << "Введите количество процессов Sender: ";
    std::cin >> numSenders;

    int semId = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    semop(semId, &sem_signal, 1); // Инициализация семафора

    // Создание процессов Sender
    for (int i = 0; i < numSenders; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Ошибка при создании процесса Sender." << std::endl;
            return 1;
        } else if (pid == 0) { // Процесс Sender
            std::string senderName = "Sender " + std::to_string(i + 1);
            char* args[] = {const_cast<char*>(senderName.c_str()), nullptr};
            execv("./sender", args);
        }
    }

    // Процесс Receiver
    char command;
    while (true) {
        std::cout << "Введите команду (r - читать сообщение, q - завершить): ";
        std::cin >> command;

        if (command == 'q') {
            // Завершение работы Receiver и ожидание завершения процессов Sender
            for (int i = 0; i < numSenders; i++) {
                wait(nullptr);
            }
            semctl(semId, 0, IPC_RMID); // Удаление семафора
            break;
        }

        if (command == 'r') {
            semop(semId, &sem_wait, 1); // Ожидание готовности Sender
            std::ifstream messageFile(FILE_NAME, std::ios::in | std::ios::binary);
            char message[MAX_MESSAGE_LENGTH];
            messageFile.read(message, MAX_MESSAGE_LENGTH);
            messageFile.close();
            std::cout << "Прочитано сообщение: " << message << std::endl;
            semop(semId, &sem_signal, 1); // Сигнал Sender, что файл доступен для записи
        }
    }

    return 0;
}
