
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

const char FILE_NAME[] = "message_file.bin";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <Sender Name>" << std::endl;
        return 1;
    }

    const std::string senderName = argv[1];
    int semId = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    struct sembuf sem_wait = {0, -1, 0};
    struct sembuf sem_signal = {0, 1, 0};

    semop(semId, &sem_wait, 1); // Ожидание сигнала готовности к работе от Receiver

    while (true) {
        std::string message;
        std::cout << senderName << " введите сообщение (или 'q' для завершения): ";
        std::cin >> message;

        if (message == "q") {
            break;
        }

        if (message.length() <= 20) {
            std::ofstream messageFile(FILE_NAME, std::ios::in | std::ios::out | std::ios::binary);
            messageFile.seekp(0);
            messageFile.write(message.c_str(), message.length());
            messageFile.close();
            semop(semId, &sem_signal, 1); // Сигнал Receiver о доступности нового сообщения
        } else {
            std::cerr << "Сообщение слишком длинное (максимальная длина 20 символов)." << std::endl;
        }
    }

    return 0;
}
