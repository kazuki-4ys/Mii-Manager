#ifndef _MII_H_
#define _MII_H_
#define DAT_MIN_SIZE 0xC820
#define MII_FILE_SIZE 0x5C
#define MAX_MII_NUM 100
#define MII_NAME_LENGTH 10
#define SHOW_MII_NUM 10

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include "extdata.hpp"

using namespace std;

typedef struct {
	unsigned char rawData[MII_FILE_SIZE];
    char name[3 * MII_NAME_LENGTH + 1];
	int shownSlot;
} mii;

int readMiis(mii*,char*);
int miiRawDataCheck(unsigned char*);
int installMii(const char*,bool,char*);
int miiFileWrite(mii*,int,const char*,char*);
void getMiiInfo(mii*);
void allGetMiiInfo(mii*,int);
int selectMii(mii*,int);
long getFileSize(int);
int dumpMii(string,char*);
unsigned short getCrc(unsigned char*,int);

#endif //_MII_H_