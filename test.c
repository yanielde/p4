#include<stdio.h>
#include "mfs.h"
int main(){
	int rc = MFS_Init("lol",5000);
	int rc2 = MFS_Lookup(5,"Hi");
	return rc;
}