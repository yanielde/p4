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



typedef struct {
    void* end_ptr;
    void* imap_arr[256];
} Checkpoint;

void* map[4096];
int num_inodes;
int main(int argc, char *argv[]) {
    int port = 15000;
    struct sockaddr_in addr;
    int sd = UDP_Open(port);
    if(sd>0){
		UDP_FillSockAddr(&addr,"localhost",port+1);
	}
    assert(sd > -1);

    Checkpoint checkpoint;
    checkpoint.end_ptr = (void*)sizeof(Checkpoint);
    int fd = open("image", O_RDWR | O_CREAT, 0664);
    write(fd, &checkpoint, sizeof(Checkpoint));
    Inode baseInode;
    baseInode.size = MFS_BLOCK_SIZE;
    baseInode.type = MFS_DIRECTORY;
 
    //printf("basedir[0]: %p", checkpoint);
    MFS_DirEnt_t basedir;
    strcpy(basedir.name, ".");
    basedir.inum = 0;
    write(fd, &basedir, sizeof(basedir));

    MFS_DirEnt_t basedirparent;
    strcpy(basedirparent.name, "..");
    basedirparent.inum = 0;
    write(fd, &basedirparent, sizeof(basedirparent));

    baseInode.ptrs[0] = checkpoint.end_ptr;

    int padding_size = MFS_BLOCK_SIZE-2*sizeof(basedir);
    char padding[padding_size];
    memset(padding,0,padding_size);
    write(fd, &padding, padding_size);
    checkpoint.end_ptr += MFS_BLOCK_SIZE;

    write(fd, &baseInode, sizeof(Inode));

    void* imap[16];
    imap[0] = checkpoint.end_ptr;
    map[0] = checkpoint.end_ptr;

    checkpoint.end_ptr+= sizeof(Inode);

    write(fd,&imap,sizeof(imap));
    checkpoint.imap_arr[0] = checkpoint.end_ptr;
    checkpoint.end_ptr += sizeof(imap);




    num_inodes = 1;
    
    char buffer[sizeof(Message)];

    while(1){
        printf("server:: waiting...\n");
    
        int rc = UDP_Read(sd, &addr,buffer, sizeof(Message));
        if(rc<=0){
            continue;
        }

        Message msg;
        memcpy(&msg, (Message*)buffer, sizeof(Message));

        if(msg.MFS_type==LOOKUP){
            printf("size: %d, type: %d, pinum: %d name:%s \n",rc, msg.MFS_type, msg.pinum , msg.name);
            msg.ret = -1;
            Inode node;
            lseek(fd, (off_t)map[msg.pinum], SEEK_SET);
            read(fd, &node, sizeof(Inode));
            if (node.type==MFS_DIRECTORY){
                MFS_DirEnt_t dir;
                lseek(fd, (off_t)node.ptrs[msg.pinum], SEEK_SET);
                do {
                    read(fd, &dir, sizeof(MFS_DIRECTORY));
                    if(strcmp(dir.name, msg.name)==0){
                        msg.ret = dir.inum;
                        break;
                    }
                }
                while(dir.name[0]!=0);
                }
            printf("msg: %d\n", msg.ret);
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));
            break;
        }
        else if(msg.MFS_type == STAT){
            msg.ret = 0;
            if(msg.inum>num_inodes || msg.inum<0){
                msg.ret = -1;
            }
            Inode node;
            lseek(fd, (off_t)map[msg.inum], SEEK_SET);
            read(fd, &node,sizeof(Inode));
            int type = node.type;
            int size = node.size;
            MFS_Stat_t stat;
            stat.type = type;
            stat.size = size;
            msg.m = stat;
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd,&addr,buffer,sizeof(Message));
            break;
            
        }
        else if(msg.MFS_type == WRITE){
            msg.ret = 0;
            if(msg.inum>14||msg.inum<0||msg.block<0||msg.block>14){
                msg.ret = -1;
            }
            else if (map[msg.inum]==0){
                msg.ret = -1;
            }
            else{
                Inode node;
                lseek(fd, (off_t)map[msg.inum], SEEK_SET);
                read(fd, &node, sizeof(Inode));
                if (node.type == MFS_REGULAR_FILE){
                    
                    lseek(fd, (off_t)node.ptrs[msg.block], SEEK_SET);
                    write(fd, &msg.buffer, MFS_BLOCK_SIZE);
                }
            }
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));      
            
        }
        else if(msg.MFS_type == READ){
            printf("size: %d, type: %d, inum: %d name:%d \n",rc, msg.MFS_type, msg.inum , msg.block);
            Inode node;
            lseek(fd, (off_t)map[msg.inum], SEEK_SET);
            read(fd, &node, sizeof(Inode));
            msg.ret = -1;
            if(node.type==MFS_DIRECTORY){
                lseek(fd, (off_t)node.ptrs[msg.block], SEEK_SET);
                read(fd, &msg.buffer, sizeof(MFS_DIRECTORY));
                msg.ret = 0;
            }else if(node.type==MFS_REGULAR_FILE){
                lseek(fd, (off_t)node.ptrs[msg.block], SEEK_SET);
                read(fd, &msg.buffer, MFS_BLOCK_SIZE);
                msg.ret = 0;
            }
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));
            //return

        }
        else if(msg.MFS_type == CREAT){
            
        }
        else if(msg.MFS_type == UNLINK){

        }
        else if(msg.MFS_type == SHUTDOWN){
            fsync(fd);
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));
            exit(1);
        } 
            
    }
    return 0;
}



