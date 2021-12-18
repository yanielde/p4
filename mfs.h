#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)

#define INIT 2
#define LOOKUP 3
#define STAT 4
#define WRITE 5
#define READ 6
#define CREAT 7
#define UNLINK 8
#define SHUTDOWN 9

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_DirEnt_t {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

typedef struct {
    int size;
    int type;
    void* ptrs[14];
} Inode;

typedef struct Message {
    char name[28];
    int port;
    int inum;
    char buffer[4096];
    int block;
    int pinum;
    int type;
    MFS_Stat_t m;
    int MFS_type;
    int ret;
} Message;

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

#endif // __MFS_h__