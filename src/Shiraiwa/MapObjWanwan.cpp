#include "Shiraiwa/Objects/MapObjWanwan.h"
#include "Kaneshige/Course/CrsGround.h"
#include "Kaneshige/RaceMgr.h"
#include "Sato/GeographyObj.h"
#include "Sato/StateObserver.h"
#include "types.h"

StateObserver::StateFuncSet<TMapObjWanwan> TMapObjWanwan::sTable[7] = {
    {0, &TMapObjWanwan::initFunc_Wait, &TMapObjWanwan::doFunc_Wait},
    {1, &TMapObjWanwan::initFunc_Attack, &TMapObjWanwan::doFunc_Attack},
    {2, &TMapObjWanwan::initFunc_Walk, &TMapObjWanwan::doFunc_Walk},
    {3, &TMapObjWanwan::initFunc_Back, &TMapObjWanwan::doFunc_Back},
    {4, &TMapObjWanwan::initFunc_Jump, &TMapObjWanwan::doFunc_Jump},
    {5, &TMapObjWanwan::initFunc_Attacked, &TMapObjWanwan::doFunc_Attacked},
    {6, &TMapObjWanwan::initFunc_Jumped, &TMapObjWanwan::doFunc_Jumped},
};

TMapObjWanwan::TMapObjWanwan(const CrsData::SObject &obj) : TMapObjHioNode(obj), mpPile(nullptr), mpEmitter(nullptr) {
    GeographyObj::NewAnmCtrl();
    GeographyObj::createSoundMgr();
    mpGround = new CrsGround(RCMGetCourse());
    getParameters();
    makeChain(_216);
    mFreeMove.init(&mPos, &mVel, 100.0f);
    mpStringNodeMgr = new ExStringNodeManager(_216, 30.0f, true, false, 0);
    mpStringNodeMgr->set_3c(0x40000);
}

TMapObjWanwan::~TMapObjWanwan() {
    delete mpStringNodeMgr;
    delete mpGround;
}

void TMapObjWanwan::makeChain(int) {}

void TMapObjWanwan::reset() {}

void TMapObjWanwan::getParameters() {
    if (mObjData->mParam1 < 2) {
        _216 = 21;
    }
    else {
        _216 = mObjData->mParam1 + 1;
    }

    u8 clamped = _216; 
    if (clamped > 30) {
        clamped = 30;
    }
    else if (clamped < 2) {
        clamped = 2;
    } 

    _216 = clamped;

    if (mObjData->mParam2 == 0) {
        mWalkDuration = 120;
    }
    else {
        mWalkDuration = mObjData->mParam2;
    }

    if (mObjData->mParam3 == 0) {
        _224 = 20.0f;
    }
    else {
        _224 = mObjData->mParam3;
    }
}

void TMapObjWanwan::resetPosition(int) {}

void TMapObjWanwan::loadAnimation() {}

const char *TMapObjWanwan::getShadowBmdFileName() {
    static const char *cShadowBmdName = "/Objects/Wanwan1Shadow.bmd";
    return cShadowBmdName;
}

void TMapObjWanwan::createColModel(J3DModelData *) {}

void TMapObjWanwan::createModel(JKRSolidHeap *heap, u32) {

    mHeightOffset = sOffsetHeight * mScale.y;
}

const char *TMapObjWanwan::getBmdFileName() {
    static const char *cBmdName = "/Objects/Wanwan1.bmd";
    return cBmdName;
}

void TMapObjWanwan::doKartColCallBack(int) {}

void TMapObjWanwan::setCurrentViewNo(u32) {}

void TMapObjWanwan::initFunc_Wait() {}

void TMapObjWanwan::doFunc_Wait() {}

void TMapObjWanwan::initFunc_Jump() {}

void TMapObjWanwan::doFunc_Jump() {}

void TMapObjWanwan::initFunc_Attack() {}

void TMapObjWanwan::initFunc_Attacked() {}

void TMapObjWanwan::doFunc_Attacked() {}

void TMapObjWanwan::initFunc_Jumped() {}

void TMapObjWanwan::doFunc_Jumped() {}

void TMapObjWanwan::setRotate(f32) {}

void TMapObjWanwan::doFunc_Attack() {}

void TMapObjWanwan::turnDown(bool) {}

void TMapObjWanwan::initFunc_Walk() {}

void TMapObjWanwan::doFunc_Walk() {}

void TMapObjWanwan::initFunc_Back() {}

void TMapObjWanwan::doFunc_Back() {}

void TMapObjWanwan::fallDown(s32, bool) {}

void TMapObjWanwan::turnTo() {}

void TMapObjWanwan::jumpStart(JGeometry::TVec3f &, f32) {}

void TMapObjWanwan::isTouchGround() {}

void TMapObjWanwan::getCourseHeight(JGeometry::TVec3f &) {}

void TMapObjWanwan::getAngleToRand() {}

void TMapObjWanwan::getRandRadius(u8, u8) {}

void TMapObjWanwan::getWanwanBackPos(JGeometry::TVec3f *) {}

void TMapObjWanwan::getAngleToAttack(JGeometry::TVec3f &) {}

void TMapObjWanwan::InitExec() {
    Observer_FindAndInit(TMapObjWanwan, 7);
}

void TMapObjWanwan::searchTargetKart() {}

void TMapObjWanwan::calc() {}

void TMapObjWanwan::update() {
    mAnmObjCluster.update();

    if (mAnmObjCluster.getFrameCtrl()->getFrame() == 0.0f && mAnmObjCluster.getFrameCtrl()->getRate() != 0.0f) {
        mSoundMgr->setSe(0x40021);
    }
    mSoundMgr->frameWork();
    setModelMatrixAndScale();
    mModel.update(0);
}

void TMapObjWanwan::chainCorrect() {}

void TMapObjWanwan::checkWanwanHitting() {}

void TMapObjWanwan::fixChain() {}

void TMapObjWanwan::setChainPosition(TMapObjWanwanChain *, JGeometry::TVec3f &, JGeometry::TVec3f &, float) {}

void TMapObjWanwan::MoveExec() {
    Observer_FindAndExec(TMapObjWanwan, 7);
}

void TMapObjWanwan::createEmitterOnGround(JPABaseEmitter **, const char *) {}

#include "JSystem/JAudio/JASFakeMatch2.h"
