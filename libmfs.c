//export LD_LIBRARY_PATH=.
//~cs537-1/tests/p4/runtests -c

#include <stdio.h>
#include "mfs.h"
#include "udp.h"
#include <string.h>

int fd;
struct sockaddr_in addr;
int MFS_Init(char *hostname, int port){
	fd = UDP_Open(port+1);
	UDP_FillSockAddr(&addr,hostname,port);
	
	return 0;
}
int MFS_Lookup(int pinum, char *name){
	if(sizeof(name)>29){
		return -1;
	}
	struct Message msg;
	strcpy(msg.name,name);
	msg.MFS_type = LOOKUP;
	msg.pinum = pinum;
	char message_buffer[sizeof(Message)];
	memcpy((Message *) message_buffer, &msg,sizeof(Message));
	int rc = UDP_Write(fd,&addr,message_buffer,sizeof(Message));
	if(rc<0){
		printf("%s","failure");
		exit(1);
	}
	rc = UDP_Read(fd,&addr,message_buffer,sizeof(Message));
	memcpy(&msg, (Message *) message_buffer,sizeof(Message));
	// rc = UDP_write(fd,&addr,message_buffer,sizeof(Message));

	return msg.ret;
}
int MFS_Stat(int inum, MFS_Stat_t *m){
	struct Message msg;
	msg.MFS_type = STAT;
	msg.inum = inum;
	int rc = UDP_Write(fd,&addr,(char*) &msg,sizeof(msg));
	if(rc<0){
		printf("%s","failure");
		exit(1);
	}
	char message_buffer[sizeof(Message)];
	rc = UDP_Read(fd,&addr,message_buffer,sizeof(Message));
	memcpy(&msg, (Message *) message_buffer,sizeof(Message));
	memcpy(m,&msg.m,sizeof(MFS_Stat_t));
	m = &msg.m;
	// printf("%s %d\n","size ",m->size);
	//printf("%s %d\n","Type ", m->type);
	return msg.ret;
	
}
int MFS_Write(int inum, char *buffer, int block){
	struct Message msg;
	memcpy(msg.buffer,buffer, MFS_BLOCK_SIZE);
	msg.inum = inum;
	msg.block = block;
	msg.MFS_type = WRITE;
	char message_buffer[sizeof(Message)];
	memcpy((Message *) message_buffer, &msg,sizeof(Message));
	int rc = UDP_Write(fd,&addr,(char*) &msg,sizeof(msg));
	if(rc<0){
		printf("%s","failure");
		exit(1);
	}
	rc = UDP_Read(fd,&addr,message_buffer,sizeof(Message));
	memcpy(&msg, (Message *) message_buffer,sizeof(Message));
	return 0;
}
int MFS_Read(int inum, char *buffer, int block){
	struct Message msg;
	//strcpy(msg.buffer,buffer);
	msg.MFS_type = READ;
	msg.block = block;
	msg.inum = inum;
	char message_buffer[sizeof(Message)];
	memcpy((Message *) message_buffer, &msg,sizeof(Message));
	int rc = UDP_Write(fd,&addr,(char*) &msg,sizeof(msg));
	if(rc<0){
		printf("%s","failure");
		exit(1);
	}
	rc = UDP_Read(fd,&addr,message_buffer,sizeof(Message));
	memcpy(&msg, (Message *) message_buffer,sizeof(Message));
	memcpy(buffer,msg.buffer, MFS_BLOCK_SIZE);
	return msg.ret;
}
int MFS_Creat(int pinum, int type, char *name){
	struct Message msg;
	strcpy(msg.name,name);
	msg.MFS_type = CREAT;
	msg.type = type;
	msg.pinum = pinum;
	char message_buffer[sizeof(Message)];
	memcpy((Message *) message_buffer, &msg,sizeof(Message));
	int rc = UDP_Write(fd,&addr,(char*) &msg,sizeof(msg));
	if(rc<0){
		printf("%s","failure");
		exit(1);
	}
	rc = UDP_Read(fd,&addr,message_buffer,sizeof(Message));
	memcpy(&msg, (Message *) message_buffer,sizeof(Message));
	return msg.ret;
}
int MFS_Unlink(int pinum, char *name){
	struct Message msg;
	strcpy(msg.name,name);
	msg.MFS_type = UNLINK;
	msg.pinum = pinum;
	char message_buffer[sizeof(Message)];
	memcpy((Message *) message_buffer, &msg,sizeof(Message));
	int rc = UDP_Write(fd,&addr,(char*) &msg,sizeof(msg));
	if(rc<0){
		printf("%s","failure");
		exit(1);
	}
	rc = UDP_Read(fd,&addr,message_buffer,sizeof(Message));
	memcpy(&msg, (Message *) message_buffer,sizeof(Message));

	return 0;
}
int MFS_Shutdown(){
	struct Message msg;
	msg.MFS_type = SHUTDOWN;
	char message_buffer[sizeof(Message)];
	memcpy((Message *) message_buffer, &msg,sizeof(Message));
	int rc = UDP_Write(fd,&addr,(char*) &msg,sizeof(msg));
	if(rc<0){
		printf("%s","failure");
		exit(1);
	}
	rc = UDP_Read(fd,&addr,message_buffer,sizeof(Message));
	memcpy(&msg, (Message *) message_buffer,sizeof(Message));
	return 0;
}

