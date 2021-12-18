#include <stdio.h>
#include "mfs.h"
#include "udp.h"
#include <string.h>

int fd;
struct sockaddr_in addr;
int MFS_Init(char *hostname, int port){
	fd = UDP_Open(port+1);
	if(fd>0){
		UDP_FillSockAddr(&addr,hostname,port);
	}
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

	return 0;
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
	printf("%s %d\n","size ",m->size);
	printf("%s %d\n","Type ", m->type);
	return msg.ret;
}
int MFS_Write(int inum, char *buffer, int block){
	struct Message msg;
	strcpy(msg.buffer,buffer);
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
	strcpy(msg.buffer,buffer);
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
	return 0;
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
	return 0;
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

