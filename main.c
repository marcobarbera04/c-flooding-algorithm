#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "flooding.h"

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