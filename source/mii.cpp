#include "mii.hpp"

const unsigned int TARGET_ID = 0xf000000b;
char TARGET_PATH[] = "/CFL_DB.dat";
char DAT_MAGIC[] = "CFOG";

int readMiis(mii*dest,string*msg){
	int miiNum = 0,miiSlot = 0;
	Handle fh;
	u32 actSize,fileSize;
	char headBuf[5] = {};
	Archive arc(TARGET_ID);
	if(!arc.valid){
		*msg = "Error:Archive::Archive";
		return -1;
	}
    if(arc.Open(&fh,TARGET_PATH,FS_OPEN_READ)){
		*msg = "Error:Archive::Open";
		return -1;
	}
	fileSize = arc.GetFileSize(fh);
	if(fileSize < DAT_MIN_SIZE){
		*msg = "Error:CFL_DB.dat is too small";
		arc.Close(fh);
		return -1;
	}
	unsigned char *DatFileBuf; //CFL_DB.datを格納するバッファ
	DatFileBuf = (unsigned char*)calloc(fileSize,sizeof(unsigned char));
	if(!DatFileBuf){
		*msg = "Error:calloc";
		arc.Close(fh);
		return -1;
	}
	arc.Read(fh,&actSize,0,DatFileBuf,fileSize);
	arc.Close(fh);
	memcpy(headBuf,DatFileBuf,4);
    if(strcmp(headBuf,DAT_MAGIC)){
        *msg = "Error:magic doesn't match";
		free(DatFileBuf);
		return -1;
	}
	while(miiSlot < MAX_MII_NUM){
		memcpy((dest + miiNum)->rawData,DatFileBuf + 8 + miiSlot * MII_FILE_SIZE,MII_FILE_SIZE);
        miiSlot++;
		if(miiRawDataCheck((dest + miiNum)->rawData))continue;
		miiNum++;
	}
	if(miiNum == 0)*msg = "Error:There is NO Miis";
	allGetMiiInfo(dest,miiNum);
	return miiNum;
}

int miiRawDataCheck(unsigned char*src){
    int i;
	for(i = 0;i < MII_FILE_SIZE;i++){
		if(src[i] != 0)return 0;
	}
	return -1;
}

string installMii(const char *fn,bool installAsPersonal){
	bool MiiShownSlot[MAX_MII_NUM];
	unsigned short crc;
	unsigned char MiiFileBuf[MII_FILE_SIZE];
	unsigned char tmpMiiID[4];
	int miiSlot = 0,curMiiShownSlot,miiInstallSlot = -1,fd;
	Handle fh;
	FILE *f;
	u32 actSize,fileSize;
	char headBuf[5] = {};
	Archive arc(TARGET_ID);
    fd = open(fn,O_RDONLY);
	if(fd < 0)return "Error:open";
	if((int)getFileSize(fd) != MII_FILE_SIZE){
		close(fd);
		return "Error:invalid file";
	}
    f = fdopen(fd,"rb");
	if(!f){
		close(fd);
		return "Error:fdopen";
	}
	fread(MiiFileBuf,sizeof(unsigned char),MII_FILE_SIZE,f);
	fclose(f);
	if(MiiFileBuf[0] != 3 || MiiFileBuf[0x16] != 0 || MiiFileBuf[0x17] != 0){
		return "Error:invalid file";
	}
	if(!arc.valid)return "Error:Archive::Archive";
    if(arc.Open(&fh,TARGET_PATH,FS_OPEN_WRITE))return "Error:Archive::Open";
	fileSize = arc.GetFileSize(fh);
	if(fileSize < DAT_MIN_SIZE){
		arc.Close(fh);
		return "Error:CFL_DB.dat is too small";
	}
	unsigned char *DatFileBuf; //CFL_DB.datを格納するバッファ
	DatFileBuf = (unsigned char*)calloc(fileSize,sizeof(unsigned char));
	if(!DatFileBuf){
		arc.Close(fh);
		return "Error:calloc";
	}
	arc.Read(fh,&actSize,0,DatFileBuf,fileSize);
	memcpy(headBuf,DatFileBuf,4);
    if(strcmp(headBuf,DAT_MAGIC)){
		arc.Close(fh);
		free(DatFileBuf);
		return "Error:magic doesn't match";
	}
	while(miiSlot < MAX_MII_NUM){
        if(miiRawDataCheck(DatFileBuf + 8 + miiSlot * MII_FILE_SIZE)){
			if(miiInstallSlot == -1){
				miiInstallSlot = miiSlot;
			}
		}else{
            curMiiShownSlot = (DatFileBuf[8 + miiSlot * MII_FILE_SIZE + 2] & 0xf) * 10 + ((DatFileBuf[8 + miiSlot * MII_FILE_SIZE + 2] >> 4) & 0xf);
			MiiShownSlot[curMiiShownSlot] = true;
		}
		miiSlot++;
	}
	if(miiInstallSlot == -1){
		free(DatFileBuf);
		arc.Close(fh);
		return "Error:Mii is full";
	}
	curMiiShownSlot = 0;
	while(curMiiShownSlot < MAX_MII_NUM){
		if(!MiiShownSlot[curMiiShownSlot])break;
        curMiiShownSlot++;
	}
    MiiFileBuf[2] = (curMiiShownSlot / 10) + ((curMiiShownSlot % 10) << 4);
	if(installAsPersonal){
		memcpy(MiiFileBuf + 4,DatFileBuf + 8 + 4,8);
        memcpy(DatFileBuf + 8 + miiInstallSlot * MII_FILE_SIZE,DatFileBuf + 8,MII_FILE_SIZE);
		memcpy(DatFileBuf + 8,MiiFileBuf,MII_FILE_SIZE);
	}else{
		memcpy(DatFileBuf + 8 + miiInstallSlot * MII_FILE_SIZE,MiiFileBuf,MII_FILE_SIZE);
	}
    crc = getCrc(DatFileBuf,DAT_MIN_SIZE - 2);
    DatFileBuf[DAT_MIN_SIZE - 2] = (unsigned char)(crc >> 8);
	DatFileBuf[DAT_MIN_SIZE - 1] = (unsigned char)(crc & 0xff);
	arc.Write(fh,&actSize,0LL,DatFileBuf,fileSize);
	free(DatFileBuf);
	arc.Close(fh);
    return "Mii was successfully installed";
}

int miiFileWrite(mii *Miis,int index,string dir,string*msg){
	char path[16] = {};
	path[15] = 0;
	FILE *f;
	sprintf(path,"%08d.3dsmii",index + 1);
	string fullPath = dir + string(path);
    f = fopen(fullPath.c_str(),"wb");
	if(!f){
		*msg = "Error:fopen";
		return -1;
	}
	fwrite((Miis[index]).rawData,sizeof(unsigned char),MII_FILE_SIZE,f);
	fclose(f);
	*msg = string(Miis[index].name) + " was dumped to\n" + fullPath;
	return 0;
}

void getMiiInfo(mii *pmii){
	int i,curIdx = 0;
	unsigned short curCh;
	memset(pmii->name,0,MII_NAME_LENGTH * 3 + 1);
    for(i = 0;i < MII_NAME_LENGTH;i++){
		curCh = (pmii->rawData[0x1B + 2 * i] << 8) + pmii->rawData[0x1A + 2 * i];
		if(curCh == 0)break;
		if(curCh < 128){
			pmii->name[curIdx] = (char)curCh;
            curIdx++;
		}else if(curCh < 2048){
			pmii->name[curIdx] = (char)((curCh >> 6) + 0xc0);
            curIdx++;
			pmii->name[curIdx] = (char)((curCh & 0x3f) + 0x80);
			curIdx++;
		}else{
			pmii->name[curIdx] = (char)((curCh >> 12) + 0xe0);
			curIdx++;
			pmii->name[curIdx] = (char)(((curCh >> 6) & 0x3f) + 0x80);
			curIdx++;
			pmii->name[curIdx] = (char)((curCh & 0x3f) + 0x80);
			curIdx++;
		}
	}
	pmii->shownSlot = (pmii->rawData[2] & 0xf) * 10 + ((pmii->rawData[2] >> 4) & 0xf);
	return;
}

void allGetMiiInfo(mii*Miis,int num){
	int i;
    for(i = 0;i < num;i++){
		getMiiInfo(Miis + i);
	}
}

int selectMii(mii* Miis,int miiNum){
	MiiSelectorConf msConf;
	MiiSelectorReturn msRet;
	int i,shownSlot;
	miiSelectorInit(&msConf);
	miiSelectorSetTitle(&msConf, "Select Mii you want to dump");
	miiSelectorSetOptions(&msConf, MIISELECTOR_CANCEL);
	miiSelectorLaunch(&msConf, &msRet);
	if (msRet.no_mii_selected || !miiSelectorChecksumIsValid(&msRet))return -1;
	shownSlot = msRet.mii.mii_pos.page_index * 10 +  msRet.mii.mii_pos.slot_index;
    for(i = 0;i < miiNum;i++){
        if(shownSlot == Miis[i].shownSlot)return i;
	}
	return -1;
}

long getFileSize(int fd){
    struct stat stbuf;
	if(fstat(fd,&stbuf) == -1){
		printf("Error:fstat\n");
		close(fd);
        return -1L;
	}
	return stbuf.st_size;
}

int dumpMii(string dir,string *msg){
    mii Miis[MAX_MII_NUM];
	int miiSlot;
	int miiNum =  readMiis(Miis,msg);
	if(miiNum < 1){
		return -1;
	}
    miiSlot = selectMii(Miis,miiNum);
	if(miiSlot == -1){
        return 1;
	}
	if(dir[dir.size() - 1] != '/')dir += "/";
	return miiFileWrite(Miis,miiSlot,dir,msg);
}