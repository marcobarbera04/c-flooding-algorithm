#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define MAX_NODES 6
#define MSG_SIZE 100

typedef struct {
    int id;
    char msg[MSG_SIZE];
    bool received;
    int parent;
} NodeData;

char fifo_names[MAX_NODES][12];
pthread_t threads[MAX_NODES];
NodeData nodes_datas[MAX_NODES];
int fd_write[MAX_NODES];
int fd_read[MAX_NODES];

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int adj_matrix[MAX_NODES][MAX_NODES] = {
    {0, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0}
};

void *node_flooding(void *arg){
    NodeData *node_data = (NodeData*)arg;
    int id = node_data->id;
    char msg[MSG_SIZE];

    pthread_mutex_lock(&lock);
    bool received = node_data->received;
    pthread_mutex_unlock(&lock);
    if(!received){
        while(true){
            int n = read(fd_read[id], msg, MSG_SIZE);
            if(n > 0){
                strcpy(node_data->msg, msg);
                node_data->received = true;
                break;
            }else
            if(n == 0 || (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))){
                usleep(50000);
                continue;
            }else{
                perror("read");
                break;
            }
        }
    }

    if(node_data->received && strlen(node_data->msg) > 0){
        usleep(50000);
        for(int i = 0; i < MAX_NODES; i++){
            if(adj_matrix[id][i] == 1){
                pthread_mutex_lock(&lock);
                if(!nodes_datas[i].received){
                    write(fd_write[i], node_data->msg, strlen(node_data->msg) + 1);     // send the message to receiver
                    nodes_datas[i].parent = id;                                         // set the parent of the receiver
                    nodes_datas[i].received = true;                                     // set the received flag of the receiver
                    printf("[NODE %d] Forwarding to NODE %d\n", id, i);
                }
                pthread_mutex_unlock(&lock);
            }
        }
    }

    pthread_exit(NULL);
}

int main() {
    memset(fd_write, 0, sizeof(fd_write));

    // create fifos and init nodes
    for (int i = 0; i < MAX_NODES; i++) {
        sprintf(fifo_names[i], "fifo%d", i);
        mkfifo(fifo_names[i], 0666);
        nodes_datas[i].id = i;
        nodes_datas[i].parent = -1;
        nodes_datas[i].received = false;
        nodes_datas[i].msg[0] = '\0';
    }

    // open all fd write and read
    for (int i = 0; i < MAX_NODES; i++) {
        fd_read[i] = open(fifo_names[i], O_RDWR | O_NONBLOCK);
        if(fd_read[i] < 0){
            perror("[ERROR] opening read only FIFO");
            exit(EXIT_FAILURE);
        }
        fd_write[i] = fd_read[i];
    }

    sleep(1);

    // send initial message to node 0
    char init_msg[MSG_SIZE] = "flooding";
    write(fd_write[0], init_msg, strlen(init_msg) + 1);
    nodes_datas[0].received = true;
    strcpy(nodes_datas[0].msg, init_msg);
    printf("First msg sent to NODE 0\n");

    sleep(2);

    for (int i = 0; i < MAX_NODES; i++) {
        pthread_create(&threads[i], NULL, node_flooding, &nodes_datas[i]);
    }

    sleep(2);

    for (int i = 0; i < MAX_NODES; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n");
    for(int i = 0; i < MAX_NODES; i++){
        int id = nodes_datas[i].id;
        int parent = nodes_datas[i].parent;
        printf("[NODE %d] Received message from NODE %d: %s\n", id, parent,nodes_datas[i].msg);
    }

    printf("\nBFS tree:\n");
    for (int i = 0; i < MAX_NODES; i++) {
        printf("node %d: parent = %d\n", nodes_datas[i].id, nodes_datas[i].parent);
    }

    for (int i = 0; i < MAX_NODES; i++) {
        close(fd_write[i]);
        close(fd_read[i]);
        unlink(fifo_names[i]);
    }

    return 0;
}