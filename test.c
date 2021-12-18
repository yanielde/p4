#include<stdio.h>
#include "mfs.h"
int main(){
	int rc = MFS_Init("lol",5000);
	printf("%d",rc);
	return rc;
}