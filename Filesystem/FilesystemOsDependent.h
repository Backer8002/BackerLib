#ifndef BL_FILESYSTEM_FILESYSTEM_OS_DEPENDENT_H
#define BL_FILESYSTEM_FILESYSTEM_OS_DEPENDENT_H

#include<BackerLibTypes.h>
#include<stddef.h>

#ifdef __unix__
typedef int BL_Filesystem_File;
#elif defined(_Win32)
#include<windows.h>
typedef HANDLE BL_Filesystem_File;
#endif

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif


typedef enum BL_Filesystem_AccessError {
    BL_Filesystem_Access_Vaild,
    BL_Filesystem_Access_Invalid,
    BL_Filesystem_Access_PermissionDenied,
    BL_Filesystem_Access_ExistsNot,
    BL_Filesystem_Access_Exists
} BL_Filesystem_AccessError;

typedef enum BL_Filesystem_EntryType {
    BL_Filesystem_EntryFile,
    BL_Filesystem_EntryDirectory,
    BL_Filesystem_EntrySymlink,
    BL_Filesystem_EntryDevice
} BL_Filesystem_EntryType;

typedef enum BL_Filesystem_FileFlags {
    BL_Filesystem_FileRead,
    BL_Filesystem_FileWrite,
    BL_Filesystem_FileExec,
    BL_Filesystem_FileCreate,
    BL_Filesystem_FileMustCreate,
    BL_Filesystem_FileTemp,
} BL_Filesystem_FileFlags;

BL_String bl_filesystem_get_current_path(void) noexcept;

BL_Filesystem_AccessError bl_filesystem_file_open(BL_Filesystem_File* file,BL_StringView path,unsigned flags) noexcept;

BL_DynamicContainer bl_filesystem_file_read(BL_Filesystem_File* file,size_t amount,size_t elementSize) noexcept;

BL_Filesystem_AccessError bl_filesystem_file_write(BL_Filesystem_File* file, const BL_Container* content);

BL_Filesystem_AccessError bl_filesystem_file_remove(BL_StringView path) noexcept;


#ifdef __cplusplus
}
#else
#undef noexcept
#endif
#endif
