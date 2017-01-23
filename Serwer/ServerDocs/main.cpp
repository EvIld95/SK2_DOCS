#include <sys/types.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <vector>

#define SERVER_PORT 1251
#define QUEUE_SIZE 5
#define BUF_SIZE 1024

std::vector<int> clients;
std::vector<char *> messages;
std::vector<char *> docs;
std::vector<int> senders;
std::vector<pthread_t *> threads;

pthread_mutex_t lock;
pthread_mutex_t writeLock;
pthread_mutex_t lockNextClient;

struct thread_data_t {
    int socket_descriptor;
    char readedThread[BUF_SIZE];
    pthread_t *thredID;
};

void *senderBuf(void *t_data) {

    while (1) {
        if (messages.size() > 0 && senders.size() > 0) {

            char *message = messages[0];
            messages.erase(messages.begin());
            int sender = senders[0];
            senders.erase(senders.begin());
            pthread_t *threadID = threads[0];
            threads.erase(threads.begin());

            if (strcmp(message, "b:newUser:-") == 0) {
                if (clients.size() > 1) {
                    std::cout << "newUser" << std::endl;
                    if (write(clients[0], "b:c:-", 5) == -1) {
                        std::cout << "ERROR" << std::endl;
                    } //sending to oldest client
                }
            } else if (strcmp(message, "b:close:-") == 0) {
                int toRemove = 0;
                for (int k = 0; k < clients.size(); k++) {
                    if (clients[k] == sender) {
                        toRemove = k;
                        break;
                    }
                }

                if (write(sender, "b:f:-", 5) == -1) {
                    std::cout << "ERROR" << std::endl;
                }
                clients.erase(clients.begin() + toRemove);
                std::cout << "Closing..." << std::endl;
                pthread_kill(*threadID, 0);

            } else {
                for (int k = 0; k < clients.size(); k++) {
                    if (clients[k] == sender) {
                        continue;
                    } else {

                        ssize_t write_result = write(clients[k], message, BUF_SIZE);
                        if (write_result == -1) {
                            printf("Błąd przy próbie pisania do socketa, kod błędu: %d\n", (int) write_result);
                            exit(-1);
                        } else {
                            std::cout << "Sending: " << message << " " << "Result: " << write_result << std::endl;
                        }
                    }
                }
            }
            //delete [] message;
        }
    }
}

void *receiver2(void *t_data) {
    while (1) {
        char *message = new char[BUF_SIZE];
        int alreadyRead = 0;

        while (1) {
            int size = read(((struct thread_data_t *) t_data)->socket_descriptor,
                            ((struct thread_data_t *) t_data)->readedThread, 1);
            if (size != -1) {
                message[alreadyRead] = ((struct thread_data_t *) t_data)->readedThread[0];
                if (message[0] == 'b') {
                    //std::cout << message[alreadyRead] << std::endl;
                    if (message[alreadyRead] == '-') {
                        messages.push_back(message);
                        senders.push_back(((struct thread_data_t *) t_data)->socket_descriptor);
                        threads.push_back(((struct thread_data_t *) t_data)->thredID);
                        std::cout << "Loading to buffer:" << message << std::endl;

                        break;
                    }
                    alreadyRead += size;
                } else {
                    if (message != nullptr) {
                        delete[] message;
                    }
                    break;
                }
            } else {
                printf("Błąd przy próbie CZYTANIA socketu, kod błędu2");
                exit(-1);
            }
        }

    }
}

int main(int argc, char *argv[]) {
    int server_socket_descriptor;
    int connection_socket_descriptor;
    int bind_result;
    int listen_result;
    char reuse_addr_val = 1;
    struct sockaddr_in server_address;


    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0) {
        fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
        exit(1);
    }
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse_addr_val, sizeof(reuse_addr_val));

    bind_result = bind(server_socket_descriptor, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
    if (bind_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
        exit(1);
    }

    listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
    if (listen_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
        exit(1);
    }


    pthread_t thread_sender;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_mutex_init(&writeLock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }
    if (pthread_mutex_init(&lockNextClient, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_create(&thread_sender, NULL, senderBuf, NULL) < 0) {
        printf("Błąd przy próbie utworzenia wątku, kod błędu");
        exit(-1);
    }


    while (1) {
        connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);

        if (clients.size() >= 9) {
            write(connection_socket_descriptor, "b:f:-", 5);
            std::cout << "To much users!" << std::endl;
        } else {
            pthread_t *thread_receiver = new pthread_t;
            clients.push_back(connection_socket_descriptor);

            thread_data_t *t_data = new thread_data_t();
            t_data->socket_descriptor = connection_socket_descriptor;

            if (pthread_create(thread_receiver, NULL, receiver2, (void *) t_data) < 0) {
                perror("Could not create thread");
                return 1;
            }
            t_data->thredID = thread_receiver;

        }

    }

}