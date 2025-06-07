#ifndef FLOODING_H
#define FLOODING_H

#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_NODES 6
#define MSG_SIZE 100

typedef struct {
    int id;
    char msg[MSG_SIZE];
    bool received;
    int parent;
} NodeData;

extern char fifo_names[MAX_NODES][12];
extern pthread_t threads[MAX_NODES];
extern NodeData nodes_datas[MAX_NODES];

extern int fd_write[MAX_NODES];
extern int fd_read[MAX_NODES];

extern pthread_mutex_t lock;

extern int adj_matrix[MAX_NODES][MAX_NODES];

void *node_flooding(void *arg);

#endif