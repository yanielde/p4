#include <stdio.h>
#include "udp.h"
#include "mfs.h"
#define BUFFER_SIZE (1000)
// typedef struct{
//     int type;
//     char arg1[28];
//     char arg2[28];
//     char arg3[28];
// }Message;

char buffer[4064];

typedef struct {
    int end_ptr;
    void* inode_chkpt_arr[256];
} Checkpoint;


Inode map[4096];
// server code
int main(int argc, char *argv[]) {
    int fd = open(argv[2], O_RDWR);
    int sd = UDP_Open(15000);
    assert(sd > -1);

    Checkpoint checkpoint;
    checkpoint.end_ptr = sizeof(Checkpoint);
    //write checkpoint
    write(fd, &checkpoint, sizeof(Checkpoint));

    //write dir
    struct __MFS_DirEnt_t basedir;
    strcpy(basedir.name, ".");
    basedir.inum = 0;
    write(fd, &basedir, sizeof(struct __MFS_DirEnt_t));
    write(fd, &buffer, MFS_dir_padding);

    //write Inode
    Inode baseInode = {MFS_BLOCK_SIZE, MFS_DIRECTORY};

    baseInode.ptrs[0] = checkpoint.end_ptr;
    checkpoint.end_ptr+= MFS_BLOCK_SIZE;

    write(fd, &baseInode, sizeof(Inode));
    map[0] = &baseInode;
    //write Imap

    int imap[16];
    checkpoint.end_ptr+= sizeof(Inode);
    imap[0] = checkpoint.end_ptr;
    write(fd,&imap,sizeof(imap));
    
    checkpoint.inode_chkpt_arr[0] = &baseInode;
    checkpoint.end_ptr += sizeof(imap);

    while (1) {
        struct sockaddr_in addr;
        //char message[BUFFER_SIZE];
        printf("server:: waiting...\n");
        Message msg;
        int rc = UDP_Read(sd, &addr,(char *) &msg, sizeof(msg));

        //printf("server:: read message [size:%d contents:(%s)]\n", rc, m.arg1);
        if (rc > 0) {
            //     char reply[BUFFER_SIZE];
            //     sprintf(reply, "goodbye world");
            //     rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
            // printf("server:: reply\n");
            if(msg.MFS_type==LOOKUP){
                if(map[msg.pinum].type== MFS_DIRECTORY){
                    __MFS_DirEnt_t dir;
                    lseek(fd, map[msg.pinum].ptrs[0], SEEK_SET);
                    read(fd, &dir, sizeof(dir));
                    int ret = 0;
                    while(dir != 0){
                        if (strcmp(msg.name, dir.name)==0){
                            UDP_Write(fd,&addr,(char*) &msg,sizeof(msg));
                            ret = 1;
                        }
                        lseek(fd, 32, SEEK_SET);
                        read(fd, &dir, sizeof(dir));
                        
                    }

                    
                    
                    
                }
               printf("here"); 
            }
            else if(msg.MFS_type == STAT){

            }
            else if(msg.MFS_type == WRITE){

            }
            else if(msg.MFS_type == READ){

            }
            else if(msg.MFS_type == CREAT){

            }
            else if(msg.MFS_type == UNLINK){

            }
            else if(msg.MFS_type == SHUTDOWN){

            
            } 
        }
    }
    return 0; 
}
    