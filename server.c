#include <stdio.h>
#include "udp.h"
#include "mfs.h"

#define BUFFER_SIZE (1000)

typedef struct {
    void* end_ptr;
    void* imap_arr[256];
} Checkpoint;

void* map[4096];
int num_inodes;
int num_imap_chunks;
int main(int argc, char *argv[]) {
    //int port = 15000;

    int port;
    int fd;
    if(argc == 1){
        port = 15000;
        fd = open("image", O_RDWR | O_CREAT, 0664);
    }else{
        port = atoi(argv[1]);
        fd = open(argv[2], O_RDWR | O_CREAT, 0664);
    }
	// //char filename[60];
	// //strcpy(filename, argv[2]);

	// int sd = UDP_Open(port);
	// struct sockaddr_in addr;
	// UDP_FillSockAddr(&addr, argv[2], port);

	// int fd = open(argv[2], O_CREAT | O_RDWR);
    //int port = argv[];
    struct sockaddr_in addr;
    int sd = UDP_Open(port);
    if(sd>0){
		UDP_FillSockAddr(&addr,"localhost",port+1);
	}
    //int fd = open(argv[2], O_RDWR | O_CREAT, 0664);
    assert(sd > -1);

    Checkpoint checkpoint;
    checkpoint.end_ptr = (void*)sizeof(Checkpoint);
    write(fd, &checkpoint, sizeof(Checkpoint));
    Inode baseInode;
    baseInode.size = MFS_BLOCK_SIZE;
    baseInode.type = MFS_DIRECTORY;
 
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

    void* first_imap[16];
    first_imap[0] = checkpoint.end_ptr;
    map[0] = checkpoint.end_ptr;
    num_inodes = 1;
    num_imap_chunks = 1;
    checkpoint.end_ptr+= sizeof(Inode);

    write(fd,&first_imap,sizeof(first_imap));
    checkpoint.imap_arr[0] = checkpoint.end_ptr;
    checkpoint.end_ptr += sizeof(first_imap);


    char buffer[sizeof(Message)];
    // printf("addr %p", map[0]);
    // Inode node;
    // lseek(fd, (off_t)map[0], SEEK_SET);
    // read(fd, &node, sizeof(Inode));
    // printf("nodesize: %d", node.size);

    while(1){
        //printf("server:: waiting...\n");
    
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
                
                lseek(fd, (off_t)node.ptrs[0], SEEK_SET);
                do {
                    read(fd, &dir, sizeof(MFS_DirEnt_t));
                    printf("dirname: %s\n", dir.name);
                    if(strcmp(dir.name, msg.name)==0){
                        msg.ret = dir.inum;
                        break;
                    }
                }
                while(dir.name[0]!=0);
            }
                
                
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));
        }
         else if(msg.MFS_type == STAT){
            msg.ret = 0;
            if(msg.inum>num_inodes || msg.inum<0){
                msg.ret = -1;
            }else{
                Inode node;
                lseek(fd, (off_t)map[msg.inum], SEEK_SET);
                read(fd, &node,sizeof(Inode));
                //printf("Stat Node addr: %p", map[msg.inum]);

                MFS_Stat_t stat;
                stat.type = node.type;
                int sum = 0;
                //find largest index block being used, multiply by 4096
                if(node.type == 1){
                    for(int i=13;i>=0;i--){
                        if(node.ptrs[i]!=0){
                            sum+=(i+1)*4096;
                            break;
                        }
                    }
                }
                stat.size = sum;
                msg.m = stat;

            }     
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd,&addr,buffer,sizeof(Message));
        }
        else if(msg.MFS_type == WRITE){
            printf("size: %d, type: %d, inum: %d block:%d \n",rc, msg.MFS_type, msg.pinum , msg.block);
            msg.ret = -1;
            if(msg.inum<num_inodes&&msg.inum>=0&&msg.block>=0&&msg.block<14){
                Inode node;
                lseek(fd, (off_t)map[msg.inum], SEEK_SET);
                read(fd, &node, sizeof(Inode));
                if(node.type==MFS_REGULAR_FILE){
                    printf("Write checkpoint 1 --> Regular File\n");

                    Inode new_node;
                    new_node.size = node.size+ MFS_BLOCK_SIZE;
                    new_node.type = MFS_REGULAR_FILE;
                    memcpy(new_node.ptrs, node.ptrs, sizeof(node.ptrs));
                    new_node.ptrs[msg.block] = checkpoint.end_ptr;
                    
                    void* imap[16];
                    lseek(fd,(off_t)checkpoint.end_ptr-sizeof(imap), SEEK_SET);

                    if((num_inodes)%16 == 0){
                        imap[0] = checkpoint.end_ptr+MFS_BLOCK_SIZE;
                    }else{
                        read(fd, &imap, sizeof(imap));
                        imap[num_inodes%16] = checkpoint.end_ptr + MFS_BLOCK_SIZE;
                    } 

                    lseek(fd,(off_t)checkpoint.end_ptr, SEEK_SET);
                    printf("msg.buffer: %s, msg location: %p\n", msg.buffer, new_node.ptrs[msg.block]);
                    write(fd, &msg.buffer, MFS_BLOCK_SIZE);
                    checkpoint.end_ptr += MFS_BLOCK_SIZE;

                    write(fd, &new_node, sizeof(Inode));
                    write(fd, &imap, sizeof(imap));

                    map[msg.inum] = checkpoint.end_ptr;
                    
                    printf("inodes: %d",num_inodes);

                    checkpoint.end_ptr+= sizeof(Inode);
                    checkpoint.imap_arr[num_inodes/16] = checkpoint.end_ptr;
                    checkpoint.end_ptr+= sizeof(imap);

                    msg.ret = 0;

                }
            }

            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));      
        }
        else if(msg.MFS_type == READ){
            printf("size: %d, type: %d, inum: %d block:%d \n",rc, msg.MFS_type, msg.inum , msg.block);
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
                printf("read loc: %p", node.ptrs[msg.block]);
                msg.ret = 0;
            }
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));

        }
        else if(msg.MFS_type == CREAT){
            //printf("size: %d, type: %d, pinum: %d name:%s \n",rc, msg.MFS_type, msg.pinum , msg.name);

            Inode node;
            lseek(fd, (off_t)map[msg.pinum], SEEK_SET);
            read(fd, &node, sizeof(Inode));
            msg.ret = -1;
            
            if(node.type == MFS_DIRECTORY){
                
                MFS_DirEnt_t dir;
                
                lseek(fd, (off_t)node.ptrs[0], SEEK_SET);
                read(fd, &dir, sizeof(MFS_DirEnt_t));
                void* write_offset;
                
                while(dir.name[0]!=0) {
                    read(fd, &dir, sizeof(MFS_DirEnt_t));
                    
                    write_offset += sizeof(MFS_DirEnt_t);  
                }
                lseek(fd, (off_t)node.ptrs[0]+(off_t)write_offset, SEEK_SET);
                MFS_DirEnt_t new_dir;
                new_dir.inum=num_inodes;
                strcpy(new_dir.name,msg.name);
                printf("name: %s", new_dir.name);
                write(fd,&new_dir,sizeof(MFS_DirEnt_t));
                void* imap[16];
                lseek(fd,(off_t)checkpoint.end_ptr-sizeof(imap), SEEK_SET);

                if((num_inodes)%16 == 0){
                    imap[0] = checkpoint.end_ptr+MFS_BLOCK_SIZE;
                }else{
                    read(fd, &imap, sizeof(imap));
                    imap[num_inodes%16] = checkpoint.end_ptr + MFS_BLOCK_SIZE;
                } 
                            
                
                lseek(fd,(off_t)checkpoint.end_ptr, SEEK_SET);
                char pad[MFS_BLOCK_SIZE];
                write(fd,&pad,MFS_BLOCK_SIZE);

                Inode new_node;
                new_node.size = MFS_BLOCK_SIZE;
                new_node.type = msg.type;
                        
                checkpoint.end_ptr+=MFS_BLOCK_SIZE;
                write(fd,&new_node, sizeof(Inode));
                write(fd,imap, sizeof(imap));

                map[num_inodes] = checkpoint.end_ptr;
                num_inodes+=1;

                checkpoint.end_ptr+= sizeof(Inode);
                checkpoint.imap_arr[num_inodes/16] = checkpoint.end_ptr;
                checkpoint.end_ptr+= sizeof(imap);

                msg.ret = 0;               
                
            }
            
            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));   
        }
        else if(msg.MFS_type == UNLINK){
            Inode node;
            msg.ret = -1;
            lseek(fd, (off_t)map[msg.pinum], SEEK_SET);
            read(fd, &node, sizeof(Inode));
            if (node.type==MFS_DIRECTORY){
                
                MFS_DirEnt_t highest_dir;
                void* highest_offset = 0;
                
                lseek(fd, (off_t)node.ptrs[0], SEEK_SET);
                do {
                    read(fd, &highest_dir, sizeof(MFS_DirEnt_t));
                    highest_offset+= sizeof(MFS_DirEnt_t);
                }
                while(highest_dir.name[0]!=0);
                highest_offset-=sizeof(MFS_DirEnt_t);
                lseek(fd, (off_t)((off_t)node.ptrs[0]+highest_offset), SEEK_SET);
                char empty_space[sizeof(MFS_DirEnt_t)];
                write(fd, &empty_space, sizeof(MFS_DirEnt_t));

                MFS_DirEnt_t dir;
                lseek(fd, (off_t)node.ptrs[0], SEEK_SET);
                do {
                    read(fd, &dir, sizeof(MFS_DirEnt_t));
                    if(strcmp(dir.name,msg.name)==0){
                        write(fd, &highest_dir, sizeof(MFS_DirEnt_t));
                    }
                }while(dir.name[0]!=0);
                msg.ret = 0;

            }

            memcpy((Message*)buffer, &msg, sizeof(Message));
            rc = UDP_Write(sd, &addr, buffer, sizeof(Message));   

            
            
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

