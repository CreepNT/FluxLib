#include "FluxFS.hpp"
#include "../logging.h"

static struct FluxFS_Global_Table* g_pTable = NULL; //Pointer to the FluxFS table
static char g_FluxFS_root_path[MAX_ROOT_PATH_LENGTH] = "app0:\0"; //TODO : check the actual size (64 should be reasonable large, tho)
static SceUInt32 g_FluxFS_root_path_length = 5; //== strlen("app0:")
static FluxFile* g_pFluxFilesHead = NULL;       //Head of linked list
static FluxFile* g_pFluxFilesTail = NULL;       //Tail of linked list
static SceUInt32 g_FluxFiles_created = 0;
static bool g_FluxFSInit_ran = false;            //Was FluxFileSystemInit already called ?
static bool g_FluxFS_Init_complete = false;      //Did FluxFileSystemInit finish initializing the library ?
static SceKernelLwMutexWork* g_pFlxFILE_mutex;   //Global mutex
static SceFiosParams g_FluxFIOSParams;          //Parameters for usage of SceFIOS

//TODO : initialize this correctly
static SceInt64 g_FIOS_opStorage_buffer[FIOS_OPSTORAGE_SIZE];
static SceInt64 g_FIOS_fhStorage_buffer[FIOS_FHSTORAGE_SIZE];
static SceInt64 g_FIOS_dhStorage_buffer[FIOS_DHSTORAGE_SIZE];
static SceInt64 g_FIOS_chunkStorage_buffer[FIOS_CHUNKSTORAGE_SIZE];

/**
 * @brief [INTERNAL] Creates a LwMutex
 * 
 * @param pWork Pointer to a SceKernelLwMutexWork
 * @param pOptName Optional : pointer to a string used as name for the mutex.
 *                 Default name is "NULL"
 * @return SceKernelLwMutexWork* same as pWork
 */
static SceKernelLwMutexWork* create_LwMutex(SceKernelLwMutexWork* pWork, char* pOptName){
    if (pOptName == NULL){
        pOptName = "NULL";
    }
    sceKernelCreateLwMutex(pWork, pOptName, SCE_KERNEL_LW_MUTEX_ATTR_RECURSIVE | SCE_KERNEL_LW_MUTEX_ATTR_TH_FIFO, 0x0, NULL);
    return pWork;
}

/**
 * @brief [INTERNAL] Phase 2 of FluxFSInit
 * 
 * @return int -1 on error, 0 on success
 */
static int FluxFSInitializeDevice(char* pFluxRootPath){ //TODO: finish this
    /*
        if (PTR_s_app0:_81fd6ad4 != NULL) {
        iVar5 = 0x0;
        ppuVar6 = &PTR_s_app0:_81fd6ad4;
        pcVar3 = PTR_s_app0:_81fd6ad4;
        do {
            iVar1 = strlen(pcVar3);
            iVar1 = strncmp(pFluxRootPath,pcVar3,iVar1);
            if (iVar1 == 0x0) {
                iVar5 = *(int *)((int)&DAT_81fd6ad8 + iVar5);
                if (-0x1 < iVar5) {
                    if ((&FluxFile_ptr_to_device???)[iVar5] != NULL) {
                        return 0x0;
                    }
                    piVar2 = (int *)operator_new(0x4);
                    piVar4 = NULL;
                    if (piVar2 != NULL) {
                        *piVar2 = iVar5;
                        piVar4 = piVar2;
                    }
                    (&FluxFile_ptr_to_device???)[iVar5] = piVar4;
                    memset(SceKernelLwMutexWork_ARRAY_82152c04,0x0,0x320);
                    DAT_82152f24 = 0x0;
                    return 0x0;
                }
                break;
            }
            pcVar3 = ppuVar6[0x2];
            ppuVar6 = ppuVar6 + 0x2;
            iVar5 = iVar5 + 0x8;
        } while (pcVar3 != NULL);
    }*/

    if (/* something */ NULL != NULL) {
        /* ... */
    }
    LOG_ERROR("Failed to locate specified device (%s)\n", pFluxRootPath);
    BREAK();
    return -1;
}

/**
 * @brief Set the new root path for all FluxFS lib functions
 * 
 * @param pFluxRootPath Pointer to a string that will be used as the new root path
 */
inline void FluxFS_set_new_root_path(char* pFluxRootPath){
    g_FluxFS_root_path_length = strlen(pFluxRootPath);
    strcpy(g_FluxFS_root_path, pFluxRootPath);
    LOG_INFO("Flux root path = \"%s\"\n",g_FluxFS_root_path);
    return;
}

void FluxFileSystemInit(char* pFluxRootPath){
    if (!g_FluxFSInit_ran){
        g_FluxFSInit_ran = true;
        struct FluxFS_Global_Table* pTable = malloc(sizeof(struct FluxFS_Global_Table)); //Should be operator.new[](0x808), but I don't like the way it worked so guess it'll be what it is now.
        if (pTable != NULL){
            pTable->maxFilesAmount = MAX_FLUXFILES_COUNT;
            pTable->unk = 0x20; //MAX_FLUXFILES_COUNT / 2 ??
            pTable->FluxFiles = new FluxFile[MAX_FLUXFILES_COUNT];             
        }
        g_pTable = pTable;
        SceKernelLwMutexWork* pMutex = malloc(sizeof(SceKernelLwMutexWork)); //Should be operator.new
        if (pMutex != NULL){
            g_pFlxFILE_mutex = create_LwMutex(pMutex, "FlxFILE");
        }
        if (pFluxRootPath != NULL){
            FluxFS_set_new_root_path(pFluxRootPath);
        }

        g_FluxFIOSParams.pathMax = MAX_ROOT_PATH_LENGTH;
        g_FluxFIOSParams.opStorage.pPtr = g_FIOS_opStorage_buffer;
        g_FluxFIOSParams.opStorage.length = sizeof(g_FIOS_opStorage_buffer);
        g_FluxFIOSParams.fhStorage.pPtr = g_FIOS_fhStorage_buffer;
        g_FluxFIOSParams.fhStorage.length = sizeof(g_FIOS_fhStorage_buffer);
        g_FluxFIOSParams.dhStorage.pPtr = g_FIOS_dhStorage_buffer;
        g_FluxFIOSParams.dhStorage.length = sizeof(g_FIOS_dhStorage_buffer);
        g_FluxFIOSParams.chunkStorage.pPtr = g_FIOS_chunkStorage_buffer;
        g_FluxFIOSParams.chunkStorage.length = sizeof(g_FIOS_chunkStorage_buffer);
        g_FluxFIOSParams.pMemcpy = memcpy;

        int ret;
        ret = sceFiosInitialize(&g_FluxFIOSParams);
        if (ret == 0){
            FluxFSInitializeDevice(g_FluxFS_root_path);
            LOG_INFO("%s: init complete\n",__func__);
            g_FluxFS_Init_complete = true;
            return;
        }
    }
    else {
        LOG_ERROR("%s : init has already been called\n", __func__);
        BREAK();
    }
}

int FluxFOpen(char* pFilePath, char* pMode, bool async){
    if (async){
        LOG_ERROR("%s called with Async option. Need to implement !\n");
        BREAK();
        goto exit;
    }
    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    int return_code;
    if (!g_FluxFS_Init_complete){
        sceKernelUnlockLwMutex(g_pFlxFILE_mutex, 1); //Ghidra says sceKernelUnlockLwMutex2
        return_code = -1;
    }
    else{
        //TODO : implement this
exit:
    sceKernelUnlockLwMutex(g_pFlxFILE_mutex, 1); //Ghidra says sceKernelUnlockLwMutex2
    }
    return return_code;

}

int FluxFSize(int fileHandle, uint* pOutSize){
    return 0;
}

int FluxFBytesLeft(int fileHandle, uint* pOutRemaining){
    return 0;
}

int FluxFStat(int fileHandle, FluxStats* pOutStats){
    return 0;
}

int FluxFRead(void* pBuf, unsigned int readSize, int fileHandle, bool infiniteLoopOnFailure, bool async){

}

int FluxFWrite(void* pBuf, unsigned int writeSize, int fileHandle, bool async){
    if (async){
        LOG_ERROR("%s called with async option. Need to implement !\n",__func__);
        BREAK();
        return -1;
    }
    return 0;
}

int FluxFAddRedirect(char* unk){
    return 0;
}

int FluxFRemoveRedirect(char* unk){
    return 0;
}


/* Class functions */
FluxFile::FluxFile()
{
    this->fileDescriptor = 0;
    this->unk1 = 0;
    this->unk2 = 0;
    this->unk3 = 0;
    this->unk4 = 0;
    this->FIOSFileHandle = 0;
    this->currentOperation = 0;
    this->pNext = NULL;
    if (g_pFluxFilesHead == NULL){
        g_FluxFiles_created = 0;
    }
    //*(char*)&this->unk3 = 0 //Useless decompiler artifact ??
    this->fileDescriptor = g_FluxFiles_created;
    g_FluxFiles_created++;
    if (g_FluxFiles_created > MAX_FLUXFILES_COUNT){
        LOG_ERROR("%s: Created more FluxFiles than allowed (max: %d)",__func__,MAX_FLUXFILES_COUNT);
        BREAK();
    }

    if(g_pFluxFilesTail != NULL){
        g_pFluxFilesTail->pNext = this;
    }
    else {
        g_pFluxFilesHead = this;
    }
    g_pFluxFilesTail = this;
}

FluxFile::~FluxFile(){
    FluxFile* pHead;
    FluxFile* pTail;
    FluxFile* pPrev;
    FluxFile* pCurrent;
    if (this->FIOSFileHandle != 0){
        this->FIOSFileHandle = 0;
    }
    if (g_pFluxFilesHead != NULL){ //Should never fail
        if(g_pFluxFilesHead == this){//We are the Head
            g_pFluxFilesHead = NULL;
            g_pFluxFilesTail = NULL;
        }

        pPrev = NULL;
        pCurrent = g_pFluxFilesHead;
        while (pCurrent != NULL){
            pPrev = pCurrent;
            pCurrent = pCurrent->pNext;
            if (pCurrent == this){ //Found ourselves
                if (pCurrent == g_pFluxFilesTail){ //and we are the Tail !
                    g_pFluxFilesTail = pPrev;
                    pPrev->pNext = NULL;
                }
                else{//and we're not the Tail.
                    pPrev->pNext = this->pNext;
                }
                break;
            }
        }
        
    }
    LOG_INFO("%s: this: %p  new Head:%p  Tail:%p\n",__func__,this,g_pFluxFilesHead,g_pFluxFilesTail);
}
