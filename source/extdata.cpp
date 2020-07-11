#include "extdata.hpp"

Archive::Archive(unsigned int id){
    FS_ArchiveID dataType;
    uint32_t path[3];
    path[1] = id;
    if(id < 0x80000000){
        path[0] = MEDIATYPE_SD;
        path[2] = 0;
        dataType = ARCHIVE_EXTDATA;
    }else{
        path[0] = MEDIATYPE_NAND;
        path[2] = 0x00048000;
        dataType = ARCHIVE_SHARED_EXTDATA;
    }
    FS_Path binPath  = {PATH_BINARY,0xC,path};
	Result ret = FSUSER_OpenArchive(&extdata_archive,dataType, binPath);
	if(ret){
        valid = false;
    }else{
        valid = true;
    }
}

Result Archive::Open(Handle*filehandle,char*path,u32 openFlag){
    return FSUSER_OpenFile(filehandle, extdata_archive, fsMakePath(PATH_ASCII, path),openFlag, 0);
}

unsigned long long Archive::GetFileSize(Handle filehandle){
    unsigned long long size = 0;
    if(FSFILE_GetSize(filehandle,&size)){
        return 0;
    }
    return size;
}

Result Archive::Read(Handle filehandle,u32*readsize,u64 offset,void*buf,u32 size){
    return FSFILE_Read(filehandle,readsize,offset,buf,size);
}

Result Archive::Write(Handle filehandle,u32*writesize,u64 offset,void*buf,u32 size){
    return FSFILE_Write(filehandle,writesize,offset,buf,size,FS_WRITE_FLUSH);
}

Result Archive::Close(Handle filehandle){
    return FSFILE_Close(filehandle);
}

Archive::~Archive(){
    if(valid){
        FSUSER_CloseArchive(extdata_archive);
    }
}