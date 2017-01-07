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
int readedFrom;
pthread_mutex_t lock;
pthread_mutex_t writeLock;
pthread_mutex_t lockNextClient;
char readed[BUF_SIZE];

struct thread_data_t {
    int socket_descriptor;
    char readedThread[BUF_SIZE];
};


void *receiver(void *t_data) {
    //struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    while (1) {
        if (read(((struct thread_data_t*)t_data)->socket_descriptor, ((struct thread_data_t*)t_data)->readedThread, BUF_SIZE) == -1) {
            printf("Błąd przy próbie CZYTANIA socketu, kod błędu");
            exit(-1);
        } else {
            pthread_mutex_lock(&writeLock); //aby nie nadpisac wiadomosci ktora nie zostala odczytana i wyslana przez serwer
            readedFrom = ((struct thread_data_t*)t_data)->socket_descriptor;
            strncpy(readed, ((struct thread_data_t*)t_data)->readedThread,  BUF_SIZE);
            if (strcmp(readed, "close0") == 0) {
                for(int i = 0; i<clients.size() ; i++) {
                    if(clients[i] == readedFrom) {
                        std::cout<<"USUWAM"<<std::endl;
                        clients.erase(clients.begin() + i);
                        //write(clients[i], "close0", BUF_SIZE);
                    }
                }
            }
        }
    }
}

void *sender(void *t_data) {
    while (1) {
        if (strcmp(readed, "") != 0 && strcmp(readed, "close0") != 0) {
            std::cout<<"User: "<<readedFrom<<" says: "<<readed<<std::endl;
            for (int k = 0; k < clients.size(); k++) {
                if(clients[k] == readedFrom) {
                    continue;
                } else {
                    std::cout<<"Sending: "<<readed<<std::endl;
                    ssize_t write_result = write(clients[k], readed, BUF_SIZE);
                    if (write_result == -1) {
                        printf("Błąd przy próbie pisania do socketa, kod błędu: %d\n", (int) write_result);
                        exit(-1);
                    }
                }
            }
            sprintf(readed, "%c", '\0');
            pthread_mutex_unlock(&writeLock);
            //memset(readed, 0, 255)
        }
    }

}

int main(int argc, char* argv[])
{
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
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

    bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
    if (bind_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
        exit(1);
    }

    listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
    if (listen_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
        exit(1);
    }


    pthread_t thread_receiver;
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

    if (pthread_create(&thread_sender, NULL, sender, NULL) < 0) {
        printf("Błąd przy próbie utworzenia wątku, kod błędu");
        exit(-1);
    }


    while(1) {
        connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);

        if(clients.size() >= 3) {
            write(connection_socket_descriptor, "close0", BUF_SIZE);
            close(connection_socket_descriptor);
        } else {
            clients.push_back(connection_socket_descriptor);

            thread_data_t* t_data = new thread_data_t();
            t_data->socket_descriptor = connection_socket_descriptor;

            if (pthread_create(&thread_receiver, NULL, receiver, (void *) t_data) < 0) {
                perror("could not create thread");
                return 1;
            }
        }

    }

    close(server_socket_descriptor);
    return(0);
}