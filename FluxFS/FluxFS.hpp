#ifndef __FLUX_FILESYSTEM_H
#define __FLUX_FILESYSTEM_H

#include <psp2/fios2.h>
#include <psp2common/types.h>
#include <psp2/kernel/threadmgr.h>
#include "../common/common.h"

//Maximum amount of FluxFiles
static const unsigned int MAX_FLUXFILES_COUNT = 64;
//Maximum length of the root path
static const unsigned int MAX_ROOT_PATH_LENGTH = 128;

//TODO : find the correct sizes etc

static const unsigned int FIOS_OPSTORAGE_SIZE = 0x8010;     //111 max ops ?
static const unsigned int FIOS_FHSTORAGE_SIZE = 0x1008;     //20 max FH ?
static const unsigned int FIOS_DHSTORAGE_SIZE = 0x408;      //5 max DH ?
static const unsigned int FIOS_CHUNKSTORAGE_SIZE = 0x20008;  //2045 max chunks ?

//This macro unlocks the FlxFILE mutex then returns the expression passes as a parameter.
#define UNLOCK_MUTEX_AND_RETURN(rcode) sceKernelUnlockLwMutex(g_pFlxFILE_mutex, 1); return (rcode)

typedef SceInt32 FluxFH;

class FluxFile {
    public:
    FluxFile();
    ~FluxFile();
    FluxFH      fileDescriptor;     //FluxFS file descriptor
    SceUInt32   unk1;               //Unknown
    SceUInt32   unk2;               //Unknown
    SceUInt32   unk3;               //Unknown
    SceUInt32   isArchive;          //Is this FluxFile an archive ?
    //note : this ^ is originally a byte-sized operand, the rest was padding so :shrug:
    SceFiosFH   FIOSFileHandle;     //FIOS filehandle
    SceFiosOp   currentOperation;   //Current FIOS operation done on this FluxFile
    FluxFile*   pNext;               //Pointer to next element
};

typedef struct FluxStats{
    SceFiosOffset fileSize;             //Size of file
    SceFiosDate   modificationDate;     //Last modification date
} FluxStats;

struct FluxFS_Global_Table {
    SceUInt32 maxFilesAmount;   //The maximum ammount of FluxFiles that can be created
    SceUInt32 sizeOfFluxFile;   //Size of a FluxFile ?
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