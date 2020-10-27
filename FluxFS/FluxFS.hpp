#ifndef __FLUX_FILESYSTEM_H
#define __FLUX_FILESYSTEM_H

//Maximum amount of FluxFiles
static const unsigned int MAX_FLUXFILES_COUNT = 64;
//Maximum length of the root path
static const unsigned int MAX_ROOT_PATH_LENGTH = 128;

//TODO : find the correct sizes etc

static const unsigned int FIOS_OPSTORAGE_SIZE = 0x8010;     //111 max ops ?
static const unsigned int FIOS_FHSTORAGE_SIZE = 0x1008;     //20 max FH ?
static const unsigned int FIOS_DHSTORAGE_SIZE = 0x408;      //5 max DH ?
static const unsigned int FIOS_CHUNKSTORAGE_SIZE = 0x20008;  //2045 max chunks ?

#include <psp2common/types.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/fios2.h>
#include "../common/common.h"

class FluxFile{
    public:
    FluxFile();
    ~FluxFile();
    SceInt32    fileDescriptor;     //FluxFS file descriptor
    SceUInt32   unk1;               //Unknown
    SceUInt32   unk2;               //Unknown
    SceUInt32   unk3;               //Unknown
    SceUInt32   unk4;               //Unknown
    SceFiosFH   FIOSFileHandle;     //FIOS filehandle
    SceFiosOp   currentOperation;   //Current FIOS operation done on this FluxFile
    FluxFile*   pNext;               //Pointer to next element
};

typedef struct FluxStats{
    SceFiosOffset fileSize;             //Size of file
    SceFiosDate   modificationDate;     //Modification date
} FluxStats;

struct FluxFS_Global_Table {
    SceUInt32 maxFilesAmount;   //The maximum ammount of FluxFiles that can be created
    SceUInt32 unk;              //Unknown
    FluxFile* FluxFiles;        //Array of FluxFiles
};

/**
 * @brief Initializes the FluxFileSystem library. 
 *        Call this before using any other FluxFS function.
 * 
 * @param pFluxRootPath Path to the Flux root.
 */
void FluxFileSystemInit(char* pFluxRootPath);

#endif