#include<stdio.h>
#include "mfs.h"


int MFS_Init(char *hostname, int port){
	printf("%s %d", hostname, port);
	return 0;
}
int MFS_Lookup(int pinum, char *name){
	printf("%s","lookup");
	return 0;
}
int MFS_Stat(int inum, MFS_Stat_t *m){
	printf("%s","stat");
	return 9;
}
int MFS_Write(int inum, char *buffer, int block){
	printf("%s","write");
	return 0;
}
int MFS_Read(int inum, char *buffer, int block){
	printf("%s","read");
	return 0;
}
int MFS_Creat(int pinum, int type, char *name){
	printf("%s","creat");
	return 0;
}
int MFS_Unlink(int pinum, char *name){
	printf("%s","unlink");
	return 0;
}
int MFS_Shutdown(){
	printf("%s","shutdown");
	return 0;
}

