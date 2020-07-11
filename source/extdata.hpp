#ifndef _EXTDATA_HPP_
#define _EXTDATA_HPP_
#include <3ds.h>

class Archive{
        FS_Archive extdata_archive;
    public:
        bool valid;
        Archive(unsigned int);
        Result Open(Handle*,char*,u32);
        unsigned long long GetFileSize(Handle filehandle);
        Result Read(Handle,u32*,u64,void*,u32);
        Result Write(Handle,u32*,u64,void*,u32);
        Result Close(Handle);
        ~Archive();
};

#endif //_EXTDATA_HPP_