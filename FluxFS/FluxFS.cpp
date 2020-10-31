#include "FluxFS.hpp"
#include "../common/logging.h"

static struct FluxFS_Global_Table* g_pTable = NULL; //Pointer to the FluxFS table
static char g_FluxFS_root_path[MAX_ROOT_PATH_LENGTH] = "app0:\0";
static SceUInt32 g_FluxFS_root_path_length = 5; //== strlen("app0:")
static FluxFile* g_pFluxFilesHead = NULL;       //Head of linked list
static FluxFile* g_pFluxFilesTail = NULL;       //Tail of linked list
static SceUInt32 g_FluxFiles_created = 0;
static bool g_FluxFSInit_ran = false;           //Was FluxFileSystemInit already called ?
static bool g_FluxFS_Init_complete = false;     //Did FluxFileSystemInit finish initializing the library ?
static SceKernelLwMutexWork* g_pFlxFILE_mutex;  //Global mutex
static SceFiosParams g_FluxFIOSParams;          //Parameters for usage of SceFIOS

static SceFiosFH g_FluxFS_Archive_FH;           //Filehandle for the FluxFS archive
static SceFiosBuffer g_FluxFS_Archive_buffer;   //SceFiosBuffer for the FluxFS archive
static int g_FluxFS_DearchiverIOFilter_index;   //Index of the SceFiosDearchiverIOFilter

//TODO : initialize this correctly
static SceInt64 g_FIOS_opStorage_buffer[FIOS_OPSTORAGE_SIZE];
static SceInt64 g_FIOS_fhStorage_buffer[FIOS_FHSTORAGE_SIZE];
static SceInt64 g_FIOS_dhStorage_buffer[FIOS_DHSTORAGE_SIZE];
static SceInt64 g_FIOS_chunkStorage_buffer[FIOS_CHUNKSTORAGE_SIZE];

/**
 * @brief [INTERNAL] Creates a LwMutex with ATTR_RECURSIVE and ATTR_TH_FIFO
 * 
 * @param pWork Pointer to a SceKernelLwMutexWork
 * @param pOptName Optional : pointer to a string used as name for the mutex.
 *                 Default name is "NULL"
 * @return Same as pWork
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
 * @return -1 on error, 0 on success
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
 * @brief Sets the new root path for all FluxFS lib functions
 * 
 * @param pFluxRootPath Pointer to a string that will be used as the new root path
 */
inline void FluxFS_set_new_root_path(char* pFluxRootPath){
    g_FluxFS_root_path_length = strlen(pFluxRootPath);
    strcpy(g_FluxFS_root_path, pFluxRootPath);
    LOG_INFO("Flux root path = \"%s\"\n",g_FluxFS_root_path);
    return;
}

/**
 * @brief Initiliazes the FluxFS library
 * 
 * @param pFluxRootPath Path to use as root path for the FluxFS library
 * 
 * @note Only call this function once, before calling any other FluxFS function.
 */
void FluxFileSystemInit(char* pFluxRootPath){
    if (!g_FluxFSInit_ran){
        g_FluxFSInit_ran = true;
        struct FluxFS_Global_Table* pTable = malloc(sizeof(struct FluxFS_Global_Table)); //Should be operator.new[](0x808), but I don't like the way it worked so guess it'll be what it is now.
        if (pTable != NULL){
            pTable->maxFilesAmount = MAX_FLUXFILES_COUNT;
            pTable->sizeOfFluxFile = sizeof(FluxFile);
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

//STUB
FluxFH FluxFOpen(char* pFilePath, char* pMode, bool async){
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

//STUB
int FluxFClose(FluxFH fileHandle, bool async){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(NULL);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(NULL);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[offset];

    if (pFlux != NULL){
        if (pFlux->isArchive){
            sceFiosArchiveUnmountSync(NULL, g_FluxFS_Archive_FH);
            g_FluxFS_Archive_FH = 0;
            free(g_FluxFS_Archive_buffer.pPtr);
            g_FluxFS_Archive_buffer.pPtr = NULL;
            sceFiosIOFilterRemove(g_FluxFS_DearchiverIOFilter_index);
            g_FluxFS_DearchiverIOFilter_index = 0;
            //SOME_GLOBAL_THING = -1;
        }
        if (!async){
            int ret;
            ret = sceFiosFHCloseSync(NULL, pFlux->FIOSFileHandle);
            if (ret != 0){
                UNLOCK_MUTEX_AND_RETURN(-1);
            }
        }
        else {  //Async
            sceFiosOpDelete(pFlux->currentOperation);
            SceFiosOp op;
            op = sceFiosFHClose(NULL, pFlux->FIOSFileHandle);
            sceFiosOpDelete(op);
        }
        pFlux->FIOSFileHandle = 0;
        //SOME_FUNCTION(fileHandle);
    }
    UNLOCK_MUTEX_AND_RETURN(0);
}

/**
 * @brief Gets the size of a file.
 * 
 * @param fileHandle FluxFH of the file
 * @param pOutSize Pointer to size variable
 * @return 1 on success, < 1 on error
 */
int FluxFSize(FluxFH fileHandle, SceFiosSize* pOutSize){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(NULL);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(NULL);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle){
        pFlux = &g_pTable->FluxFiles[offset];
        if (pFlux->FIOSFileHandle != 0){
            SceFiosSize siz;
            siz = sceFiosFHGetSize(pFlux->FIOSFileHandle);
            *pOutSize = siz;
            UNLOCK_MUTEX_AND_RETURN(1);
        }
        else {
            LOG_ERROR("%s: Invalid handle:%08x  Closed?\n",__func__,fileHandle);
            UNLOCK_MUTEX_AND_RETURN(0);
        }
    }
    else{
        LOG_ERROR("%s: Invalid handle:%08x  Closed?\n",__func__,fileHandle);
        UNLOCK_MUTEX_AND_RETURN(-1);
    }
}

/**
 * @brief Gets the remaining bytes before EOF of a file.
 * 
 * @param fileHandle FluxFH of the file
 * @param pOutRemaining Pointer to a variable that will contain the remaining bytes
 * @return 1 on success, < 1 on error
 */
int FluxFBytesLeft(FluxFH fileHandle, uint* pOutRemaining){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[offset];
    
    if (pFlux != NULL){
        if (pFlux->FIOSFileHandle == 0){
            UNLOCK_MUTEX_AND_RETURN(0);
        }
        else{
            SceFiosSize siz, tell;
            siz = sceFiosFHGetSize(pFlux->FIOSFileHandle);
            tell = sceFiosFHTell(pFlux->FIOSFileHandle);
            *pOutRemaining = (siz - tell);
            UNLOCK_MUTEX_AND_RETURN(1);
        }
    }
    else{
        LOG_ERROR("%s: Invalid handle:%08x  Closed?\n",__func__,fileHandle);
        UNLOCK_MUTEX_AND_RETURN(-1);
    }
}

/**
 * @brief Gets stats about a file.
 * 
 * @param fileHandle FluxFH of the file
 * @param pOutStats Pointer to stats about the file (zero'ed out on error)
 * @return 1 on success, < 1 on error
 */
int FluxFStat(FluxFH fileHandle, FluxStats* pOutStats){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[offset];
    else {
        LOG_ERROR("%s: Invalid handle:%08x  Closed?\n",__func__,fileHandle);
        UNLOCK_MUTEX_AND_RETURN(-1);
    }
    
    if (pFlux->fileDescriptor == 0){
        UNLOCK_MUTEX_AND_RETURN(0);
    }
    else {
        SceFiosStat stats;
        memset(&stats, 0, sizeof(stats));
        int ret = sceFiosFHStatSync(NULL, pFlux->FIOSFileHandle, &stats);
        if (ret == 0){
            pOutStats->fileSize = stats.fileSize;
            pOutStats->modificationDate = stats.modificationDate;
            UNLOCK_MUTEX_AND_RETURN(1);
        }
        else {
            LOG_ERROR("%s: sceFiosFHStatSync returned error  filehandle:%08x  error:%d\n",__func__,fileHandle,ret);
            pOutStats->fileSize = 0;
            pOutStats->modificationDate = 0;
            UNLOCK_MUTEX_AND_RETURN(0);
        }
    }
}

//STUB
int FluxFRead(void* pBuf, unsigned int readSize, FluxFH fileHandle, bool infiniteLoopOnFailure, bool async){
    return 0;
}

/**
 * @brief Writes data to a file.
 * 
 * @param pBuf Pointer to buffer containing the data
 * @param writeSize Size of the ammount of data to write
 * @param fileHandle FluxFH of the file
 * @param async Run in asynchronous mode ?
 * @return 0 on success or if the FIOSFileHandle was 0, -1 otherwise
 */
int FluxFWrite(void* pBuf, unsigned int writeSize, FluxFH fileHandle, bool async){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[offset];
    
    if ((pFlux != NULL) && (pFlux->FIOSFileHandle != 0)){
        if (async){
            LOG_ERROR("%s called with async option. Need to implement !\n",__func__);
            BREAK();
            UNLOCK_MUTEX_AND_RETURN(-1);
        }
        SceFiosSize siz = sceFiosFHWriteSync(NULL, pFlux->FIOSFileHandle, pBuf, (SceFiosSize)writeSize); //Ghidra says sceFiosFHWrite, but it makes no sense (this is the Sync case)
        //The check in Ghidra also makes no sense (error out if siz != 0). I'll go ahead and assume FluxFWrite is never used, hence they didn't catch the bug, or something.
        if (siz < 0){
            LOG_ERROR("%s: FIOS failed to write to filehandle(%d)\n",__func__,fileHandle);
            UNLOCK_MUTEX_AND_RETURN(-1);
        }
    }
    UNLOCK_MUTEX_AND_RETURN(0);

}

//STUB
int FluxFAddRedirect(char* unk){
    return 0;
}

//STUB
int FluxFRemoveRedirect(char* unk){
    return 0;
}

/**
 * @brief Sets a file's current position to a specified offset.
 * 
 * @param fileHandle FluxFH of the file
 * @param offset Offset to seek to (absolute, from the beggining of the file)
 * @return New position in the file on success, -1 on error.
 */
SceFiosOffset FluxFSeek(FluxFH fileHandle, SceFiosOffset offset){
    int off;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(-1LL);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(-1LL);
    }

    off = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[off].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[off];

    if ((pFlux != NULL) && (pFlux->FIOSFileHandle != 0)){
        SceFiosOffset rcode;
        rcode = sceFiosFHSeek(pFlux->FIOSFileHandle, offset, SCE_FIOS_SEEK_SET);
        UNLOCK_MUTEX_AND_RETURN(rcode);
    }
    else {
        UNLOCK_MUTEX_AND_RETURN(-1LL);
    }
}

/**
 * @brief Waits until the end of the operation on a file.
 * 
 * @param fileHandle FluxFH of the file
 * @return 0 on success, -1 if an error happened
 */
int FluxFWaitForOpEnd(FluxFH fileHandle){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(-1);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[offset];
    
    if (pFlux != NULL){//There is no such test according to Ghidra, but I felt like it was important to add it.
        int ret;
        ret = sceFiosOpWait(pFlux->currentOperation);
        if (ret < 0){
            UNLOCK_MUTEX_AND_RETURN(-1);
        }
        else {
            UNLOCK_MUTEX_AND_RETURN(0);
        }
    }
    else{
        LOG_ERROR("%s: Invalid handle:%08x  Closed?\n",__func__,fileHandle);
        UNLOCK_MUTEX_AND_RETURN(-1);
    }
}

/**
 * @brief Checks if the operation on a file is finished.
 * 
 * @param fileHandle FluxFH of the file
 * @return 0 on error/if the operation isn't finished, 1 if the operation is finished.
 */
bool FluxFIsOpFinished(FluxFH fileHandle){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(0);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(0);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[offset];
    
    if (pFlux != NULL){//There is no such test according to Ghidra, but I felt like it was important to add it.
        bool ret;
        ret = sceFiosOpIsDone(pFlux->currentOperation);
        UNLOCK_MUTEX_AND_RETURN(ret);
    }
    else{
        LOG_ERROR("%s: Invalid handle:%08x  Closed?\n",__func__,fileHandle);
        UNLOCK_MUTEX_AND_RETURN(0);
    }
}

/**
 * @brief Gets the FluxFile object associated to a FluxFH.
 * 
 * @param fileHandle FluxFH of the file
 * @return Pointer to the FluxFile object on success, NULL on error.
 */
FluxFile* FluxFGetFluxFile(FluxFH fileHandle){
    int offset;
    FluxFile* pFlux = NULL;

    sceKernelLockLwMutex(g_pFlxFILE_mutex, 1, NULL);
    if (!g_FluxFS_Init_complete) {
        UNLOCK_MUTEX_AND_RETURN(NULL);
    }

    if((fileHandle < 1) || ((fileHandle & 0x3FFU) > MAX_FLUXFILES_COUNT)){
        LOG_ERROR("Filehandle %08x is garbage.\n",fileHandle);
        BREAK();
        UNLOCK_MUTEX_AND_RETURN(NULL);
    }

    offset = (fileHandle & 0x3FFU);

    if (g_pTable->FluxFiles[offset].fileDescriptor == fileHandle)
        pFlux = &g_pTable->FluxFiles[offset];
    
    UNLOCK_MUTEX_AND_RETURN(pFlux);
}

FluxFile::FluxFile()
{
    this->fileDescriptor = 0;
    this->unk1 = 0;
    this->unk2 = 0;
    this->unk3 = 0;
    this->isArchive = 0;
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
