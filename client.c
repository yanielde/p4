#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (1000)
//export LD_LIBRARY_PATH=.
// client code
int main(int argc, char *argv[]) {
    // struct sockaddr_in addrSnd, addrRcv;

    // int sd = UDP_Open(20000);
    // int rc = UDP_FillSockAddr(&addrSnd, "localhost", 10000);

    // char message[BUFFER_SIZE];
    // sprintf(message, "hello world");

    // printf("client:: send message [%s]\n", message);
    // rc = UDP_Write(sd, &addrSnd, message, BUFFER_SIZE);
    // if (rc < 0) {
	// printf("client:: failed to send\n");
	// exit(1);
    // }

    // printf("client:: wait for reply...\n");
    // rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
    // printf("client:: got reply [size:%d contents:(%s)\n", rc, message);
    // return 0;


    MFS_Init("localhost",15000);
    // char buffer[MFS_BLOCK_SIZE];
    // printf("%s","works\n");
    // int ret = MFS_Read(0,buffer,0);
    // printf("lookup return\n: %d",ret);
    MFS_Stat_t stat;
    MFS_Stat_t *m = &stat;
    int rc = MFS_Stat(0,m);
    printf("stat return: %d",rc);
    
    // Message msg;
    // MFS_Stat_t stat;
    // MFS_Stat_t *m = &stat;
    // msg.m = m;
    // msg.m -> type = 5;
    // printf("%d",msg.m -> type);

}