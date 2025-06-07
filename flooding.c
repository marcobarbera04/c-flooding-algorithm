#include "flooding.h"

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