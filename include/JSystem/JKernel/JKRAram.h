#ifndef JKRARAM_H
#define JKRARAM_H

//#include "dolphin/ar.h"
#include <dolphin/os.h>
#include "JSystem/JKernel/JKRDecomp.h"
#include "JSystem/JKernel/JKRThread.h"
#include "JSystem/JKernel/JKRDvdRipper.h"
#include "JSystem/JSupport/JSUList.h"
#include "types.h"

class JKRAramHeap;

class JKRAramBlock
{
public:
    JKRAramBlock(u32, u32, u32, u8, bool);

    virtual ~JKRAramBlock(); // _08

    JKRAramBlock *allocHead(u32, u8, JKRAramHeap *);
    JKRAramBlock *allocTail(u32, u8, JKRAramHeap *);

    void newGroupID(u8 newID) { mGroupID = newID; }
    u32 getAddress() const { return mAddress; }

    // _00 = VTBL
    JSULink<JKRAramBlock> mLink; // _04
    u32 mAddress;                // _14
    u32 mSize;                   // _18
    u32 mFreeSize;               // _1C
    u8 mGroupID;                 // _20
    bool mIsTempMemory;          // _21
};

class JKRAramHeap : public JKRDisposer
{
public:
    enum EAllocMode
    {
        AM_Head = 0,
        AM_Tail
    };

    JKRAramHeap(u32, u32);

    virtual ~JKRAramHeap(); // _08

    JKRAramBlock *alloc(u32, EAllocMode);
    JKRAramBlock *allocFromHead(u32);
    JKRAramBlock *allocFromTail(u32);
    u32 getFreeSize();
    u32 getTotalFreeSize();
    void dump();

    void free(JKRAramBlock *block) {
        delete block;
    }

    u8 getCurrentGroupID() const { return mGroupID; }
    JKRHeap *getMgrHeap() const { return mHeap; }

    void lock() { OSLockMutex(&mMutex); }

    void unlock() { OSUnlockMutex(&mMutex); }

    static JSUList<JKRAramBlock> sAramList;

    // _00     = VTBL
    // _00-_18 = JKRDisposer
    OSMutex mMutex; // _18
    JKRHeap *mHeap;       // _30
    u32 mHeadAddress;     // _34
    u32 mTailAddress;     // _38
    u32 mSize;            // _3C
    u8 mGroupID;          // _40
};

class JKRAram : public JKRThread
{
public:
    JKRAram(u32, u32, long);

    virtual ~JKRAram();  // _08
    virtual void *run(); // _0C

    static void checkOkAddress(u8* addr, u32 size, JKRAramBlock* block, u32 param_4);
    static void changeGroupIdIfNeed(u8* data, int groupId);

    static JKRAram *create(u32, u32, long, long, long);
    static JKRAramBlock *mainRamToAram(u8 *, u32, u32, JKRExpandSwitch, u32, JKRHeap *, int, u32 *);
    static u8 *aramToMainRam(u32, u8 *, u32, JKRExpandSwitch, u32, JKRHeap *, int, u32 *);
    static u8 *aramToMainRam(JKRAramBlock *, u8 *, u32, u32, JKRExpandSwitch, u32, JKRHeap *, int, u32 *);

    void aramSync(JKRAMCommand *, int);

    static u32 getSZSBufferSize() {
        return sSZSBufferSize;
    }

    static JKRAramHeap *getAramHeap() {
        return sAramObject->mAramHeap;
    }

    static u8 decideAramGroupId(int id)
    {
        if(id < 0)
            return getAramHeap()->getCurrentGroupID();
        else return id;
    }

    static u32 sSZSBufferSize;

    static JKRAram *sAramObject;
    static OSMessage sMessageBuffer[4];
    static OSMessageQueue sMessageQueue;
    static JSUList<JKRAMCommand> sAramCommandList;

    u32 mAudioMemoryPtr;    // _7C
    u32 mAudioMemorySize;   // _80
    u32 mGraphMemoryPtr;    // _84
    u32 mGraphMemorySize;   // _88
    u32 mUserMemoryPtr;     // _8C
    u32 mUserMemorySize;    // _90
    JKRAramHeap *mAramHeap; // _94
    u32 mStackArray[3];     // _98
};

class JSUFileInputStream;

class JKRAramStreamCommand
{
public:
    enum ECommandType
    {
        ECT_UNK = 0,
        ECT_READ = 1,
        ECT_WRITE = 2,
    };

    JKRAramStreamCommand();

    ECommandType type;             // _00
    u32 mAddress;                  // _04
    u32 mSize;                     // _08
    u32 _0C;                       // _0C
    JSUFileInputStream *mStream;   // _10
    u32 mOffset;                   // _14
    u32 *mReturnSize;              // _18
    u8 *mTransferBuffer;           // _1C
    u32 mTransferBufferSize;       // _20
    JKRHeap *mHeap;                // _24
    bool mAllocatedTransferBuffer; // _28
    u32 _2C;                       // _2C
    OSMessageQueue mMessageQueue;  // _30
    void *mMessage;                // _50
    u32 _54;                       // _54
    u32 _58;                       // _58
};

class JKRAramStream : public JKRThread
{
public:
    JKRAramStream(long);

    virtual ~JKRAramStream(); // _08
    virtual void *run();      // _0C

    static JKRAramStream *create(s32);

    static u32 readFromAram();
    static s32 writeToAram(JKRAramStreamCommand *);
    static JKRAramStreamCommand *write_StreamToAram_Async(JSUFileInputStream *, JKRAramBlock *, u32, u32, u32 *);
    static JKRAramStreamCommand *write_StreamToAram_Async(JSUFileInputStream *, u32, u32, u32, u32 *);
    static JKRAramStreamCommand *sync(JKRAramStreamCommand *, BOOL);
    static void setTransBuffer(u8 *, u32, JKRHeap *);

    static JKRAramStream *sAramStreamObject;
    static OSMessage sMessageBuffer[4];
    static OSMessageQueue sMessageQueue;

    static u8 *transBuffer;
    static JKRHeap *transHeap;
    static u32 transSize;

    // _00     = VTBL
    // _00-_7C = JKRThread
};

inline JKRAramBlock *JKRAllocFromAram(u32 size, JKRAramHeap::EAllocMode allocMode) {
    return JKRAram::getAramHeap()->alloc(size, allocMode);
}

inline void JKRFreeToAram(JKRAramBlock * block) {
    JKRAram::getAramHeap()->free(block);
} 

inline JKRAramStream * JKRCreateAramStreamManager(s32 priority) {
    return JKRAramStream::create(priority);
}

#endif
