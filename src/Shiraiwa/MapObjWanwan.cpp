#include "Shiraiwa/Objects/MapObjWanwan.h"
#include "JSystem/J3D/J3DFrameCtrl.h"
#include "JSystem/J3D/J3DModel.h"
#include "JSystem/JGeometry/Matrix.h"
#include "JSystem/JGeometry/Util.h"
#include "JSystem/JGeometry/Vec.h"
#include "Kaneshige/Course/CrsGround.h"
#include "Kaneshige/RaceMgr.h"
#include "Sato/GeographyObj.h"
#include "Sato/GeographyObjMgr.h"
#include "Sato/J3DAnmObject.h"
#include "Sato/JPEffectMgr.h"
#include "Sato/ObjUtility.h"
#include "Sato/StateObserver.h"
#include "Shiraiwa/SiUtil.h"
#include "Yamamoto/kartCtrl.h"
#include "dolphin/mtx.h"
#include "kartEnums.h"
#include "types.h"
#include "mathHelper.h"
#include "std/math.h"
#include <math.h>
#include <stdlib.h>

J3DAnmTransform *TMapObjWanwan::sWanwanBckAnmTrans;
J3DMtxCalc *TMapObjWanwan::sWanwanBckMtxCalc;
J3DDeformData *TMapObjWanwan::sWanwanBlsAnmData;
J3DAnmCluster *TMapObjWanwan::sWanwanBlkAnmData;
int TMapObjWanwan::sChainJointNo;

f32 TMapObjWanwan::sOffsetHeight = 300.0f;
f32 TMapObjWanwan::sJumpHeight = 8.0f;
f32 TMapObjWanwan::sChainBackInside = 30.0f;
s16 TMapObjWanwan::sDefaultRingNodeNo = 9;
f32 TMapObjWanwan::sAirFriction = 0.9999f;
f32 TMapObjWanwan::sGravity = 15.0f;
f32 TMapObjWanwan::sItemHitVelScale = 1.0f;
f32 TMapObjWanwan::sKartHitVelScale = 1.0f;
f32 TMapObjWanwan::sJumpVelY = 350.0f;
s16 TMapObjWanwan::sJumpTopFrame = 15;

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
    makeChain(mNumChains);
    mFreeMove.init(&mPos, &mVel, 100.0f);
    mpStringNodeMgr = new ExStringNodeManager(mNumChains, 30.0f, true, false, 0);
    mpStringNodeMgr->set_3c(0x40000);
}

TMapObjWanwan::~TMapObjWanwan() {
    delete mpStringNodeMgr;
    delete mpGround;
}

void TMapObjWanwan::makeChain(int count) {
    mpPile = (TMapObjWanwanPile *)GetGeoObjMgr()->createSubObj(0xe7a);

    count = MIN(count, 30);

    for (u8 i = 0; i < count; i++) {
        mpaChain[i] = (TMapObjWanwanChain *)GetGeoObjMgr()->createSubObj(0xe79);
        mpaChain[i]->setPile(mpPile);
    }
    mChainLength = (count - 1) * TMapObjWanwanChain::sChainOneLength;
    mpaChain[0]->setObjFlagHidding();
}

void TMapObjWanwan::reset() {
    GeographyObj::resetObject();
    GeographyObj::setObjFlagCheckItemHitting();
    _58 = 0xb;
    mReaction.setFlg(7, 0);
    StateObserver::ResetState();
    mAnmCtrl->Reset();
    mWalkTimer = 0;
    mTimesAttacked = 0;
    _20c = -10.0f;
    _218 = -100.0f;
    _22c = 0.05f;
    mRotate = 0.0f;
    mKartNo = -1;
    mRingNodeNo = sDefaultRingNodeNo;
    mAttackAngle = 0.0f;
    _230 = 0.0f;
    _1dc = true;
    mpEmitter = nullptr;
    mAnmObjCluster.resetFrame();
    mFreeMove.reset();
    setRotate(mAttackAngle);
    resetPosition(mNumChains);
    mpStringNodeMgr->resetNodeAll(nullptr);
    mpStringNodeMgr->setNodeLengthAll(TMapObjWanwanChain::sChainOneLength);
}

void TMapObjWanwan::getParameters() {
    if (mObjData->mParam1 < 2) {
        mNumChains = 21;
    }
    else {
        mNumChains = mObjData->mParam1 + 1;
    }

    u8 clamped = mNumChains; 
    if (clamped > 30) {
        clamped = 30;
    }
    else if (clamped < 2) {
        clamped = 2;
    } 

    mNumChains = clamped;

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

void TMapObjWanwan::resetPosition(int count) {
    for (u8 i = 0; i < count; i++) {
        if (mpaChain[i]) {
            mpaChain[i]->setPosition(mPos);
        }
    }
    
    if (mpPile) {
        mpPile->setPosition(mPos);
    }
}

void TMapObjWanwan::loadAnimation() {
    J3DModelData *mdlData = mModel.getModelData();
    void *bca = ObjUtility::getPtrCourseArc("/Objects/Wanwan1.bca");
    J3DAnmObjTrans::setupTransAnmData(&sWanwanBckAnmTrans, &sWanwanBckMtxCalc, mdlData, bca);

    void *bls = ObjUtility::getPtrCourseArc("/Objects/Wanwan1.bls");
    J3DAnmObjCluster::loadClusterData(&sWanwanBlsAnmData, bls);

    void *bla = ObjUtility::getPtrCourseArc("/Objects/Wanwan1.bla");
    J3DAnmObjCluster::loadClusterAnmData(&sWanwanBlkAnmData, bla);
}

const char *TMapObjWanwan::getShadowBmdFileName() {
    static const char *cShadowBmdName = "/Objects/Wanwan1Shadow.bmd";
    return cShadowBmdName;
}

void TMapObjWanwan::createColModel(J3DModelData *mdlData) {
    createBoundsSphere(mdlData);
    mBounds[0]->scaleRadius(0.8f);
}

void TMapObjWanwan::createModel(JKRSolidHeap *heap, u32 viewNo) {
    mModel.createDifferedModel(heap, viewNo, 0x200, true);
    GeographyObj::getAnmCtrl()->InitRegistration(1, &mModel);
    GeographyObj::getAnmCtrl()->RegisterTrans(0, sWanwanBckAnmTrans, sWanwanBckMtxCalc);
    GeographyObj::getAnmCtrl()->getFrameCtrl(0)->setAttribute(2);

    J3DAnmObjCluster::setDeformData(&mModel, sWanwanBlsAnmData, false);
    mAnmObjCluster.setExModel(&mModel, sWanwanBlsAnmData);
    mAnmObjCluster.attach(sWanwanBlkAnmData);
    mAnmObjCluster.getFrameCtrl()->setAttribute(2);
    sChainJointNo = mModel.getModelData()->getJointName()->getIndex("pos_chn");
    mHeightOffset = sOffsetHeight * mScale.y;
}

const char *TMapObjWanwan::getBmdFileName() {
    static const char *cBmdName = "/Objects/Wanwan1.bmd";
    return cBmdName;
}

void TMapObjWanwan::doKartColCallBack(int kartNo) {
    if (!(GetKartCtrl()->GetKartStatus(kartNo) & 0x40000)) return;

    mKartNo = kartNo;
    StateObserver::setState(4);
}

void TMapObjWanwan::setCurrentViewNo(u32 viewNo) {
    mModel.setCurrentViewNo(viewNo);

    Mtx m;
    ObjUtility::getCamDependLightMtx(viewNo, mPos, m);
    mModel.setEffectMtx(m, 0);
}

void TMapObjWanwan::initFunc_Wait() {}

void TMapObjWanwan::doFunc_Wait() {
    if (StateObserver::getStateCount() > 10) {
        StateObserver::setState(2);
    }
    else if (mWalkTimer > 0) {
        mWalkTimer--;
    }
createEmitterOnGround(&mpEmitter, "mk_wanSmoke_a");
}

void TMapObjWanwan::initFunc_Jump() {
    if (GeographyObj::tstItemHitting()) {
        mVel.scale(sItemHitVelScale, GeographyObj::getColItemObj()->getVel());
    }
    else {
        ObjUtility::getKartVel(mKartNo, &mVel);
        mVel.scale(sKartHitVelScale);
    }
    mVel.y += sJumpVelY;
    mAnmObjCluster.stop();
}

void TMapObjWanwan::doFunc_Jump() {
    mVel.scale(sAirFriction);
    mVel.y -= sGravity;
    mPos.add(mVel);

    JGeometry::TVec3f posdiff, backPos;
    getWanwanBackPos(&backPos);

    posdiff.sub(backPos, mObjData->position);

    if (posdiff.length() > mChainLength) {
        mVel.zero();
        StateObserver::setState(6);
    }
}

void TMapObjWanwan::initFunc_Attack() {
    JGeometry::TVec3f pos;
    ObjUtility::getKartPos(mKartNo, &pos);
    pos.sub(mPos);
    pos.scale(2.0f);
    pos.add(mPos);

    JGeometry::TVec3f kartVel;
    ObjUtility::getKartVel(mKartNo, &kartVel);
    kartVel.y = 0.0f;
    kartVel.normalize();

    f32 velScale = GetKartCtrl()->GetCarSpeed(mKartNo);
    velScale = velScale * velScale * (mScale.y * 0.2f + 0.7f);
    kartVel.scale(velScale);
    pos.add(kartVel);
    
    pos.y = mHeightOffset + getCourseHeight(pos);
    pos.y = mPos.y;
    mFreeMove.setTargetPos(pos, 6.0f, 70.0f);
    mVel.zero();
    pos.sub(mPos);
    mAttackAngle = getAngleToAttack(pos);
}

void TMapObjWanwan::initFunc_Attacked() {}

void TMapObjWanwan::doFunc_Attacked() {
    if (isTouchGround() && StateObserver::getStateCount() > 50) {
        StateObserver::setState(3);
    }
    else {
        if (mAnmObjCluster.getFrame() == 0.0f) {
            mAnmObjCluster.setFrame(0.0f);
        }
        fallDown(-1, true);
    }

    mTimesAttacked++;
    createEmitterOnGround(&mpEmitter, "mk_wanSmoke_a");
}

void TMapObjWanwan::initFunc_Jumped() {}

void TMapObjWanwan::doFunc_Jumped() {
    if (StateObserver::getStateCount() <sJumpTopFrame) return;

    if (isTouchGround()) {
        mAnmObjCluster.getFrameCtrl()->setRate(1.0f);
        StateObserver::setState(2);
        return;
    }

    mVel.scale(sAirFriction);
    mVel.y -= sGravity;
    mPos.add(mVel);    
}

void TMapObjWanwan::setRotate(f32 rotate)
{
    mRotate = rotate;
    const f32 x = 0;
    const f32 y = mRotate;
    const f32 z = _230;
    
    f32 dVar2 = cos(x); // f26
    f32 dVar6 = cos(y); // f31
    f32 dVar5 = cos(z); // f30
    f32 dVar4 = sin(x);   // f29
    f32 dVar3 = sin(y); // f28
    f32 dVar1 = sin(z); // f7

    f32 cxy = dVar6 * dVar2;

    mRotMtx[0][0] = (dVar1 * dVar3) * dVar4 + cxy;
    mRotMtx[1][0] = (dVar4 * dVar5);
    mRotMtx[2][0] = -(dVar3 * dVar2) + ((dVar1 * dVar6) * dVar4);

    mRotMtx[0][1] = ((-dVar4 * dVar6) + dVar2 * (dVar1 * dVar3));
    mRotMtx[1][1] = (dVar5 * dVar2);
    mRotMtx[2][1] = (dVar3 * dVar4 + dVar1 * (cxy));

    mRotMtx[0][2] = (dVar5 * dVar3);
    mRotMtx[1][2] = (-dVar1);
    mRotMtx[2][2] = (dVar5 * dVar6);
}

void TMapObjWanwan::doFunc_Attack() {
    if (mAnmObjCluster.getRate() != 0.0f && mAnmObjCluster.getFrame() == 5.0f) {
        mAnmObjCluster.stop();
    }

    if (!mFreeMove.hasTarget()) {
        mWalkTimer = mWalkDuration;
        mTimesAttacked = 0;
        mVel.zero();
        mAnmObjCluster.getFrameCtrl()->setRate(1.0f);
        StateObserver::setState(5);
        return;
    }

    turnDown(true);
    turnTo();
}

void TMapObjWanwan::turnDown(bool p1) {
    if (p1) {
        if (_230 < 0.75f) {
            _230 += 0.03f;
        }
    }
    else {
        if (_230 > 0.0f) {
            _230 -= 0.05f;
        }
    }
}

void TMapObjWanwan::initFunc_Walk() {
    if (!isTouchGround()) return;

    if (mRotate == mAttackAngle) {
        mAttackAngle = getAngleToRand();
    }

    JGeometry::TVec3f t;
    mRotMtx.getZDir(t);
    jumpStart(t, 12.0f);
}

void TMapObjWanwan::doFunc_Walk() {
    fallDown(0, false);
    if (RCMGetManager()->getRaceMode() == TIME_ATTACK) return;

    if (mWalkTimer <= 0) {
        int targetKart = -1;
        if (!(StateObserver::getStateCount() & 0x1f)) {
            targetKart = searchTargetKart();
        }
        if (targetKart != -1) {
            mKartNo = targetKart;
            StateObserver::setState(1);
        }
        return;
    }
    mWalkTimer--;
}

void TMapObjWanwan::initFunc_Back() {
    mAnmObjCluster.getFrameCtrl()->setRate(1.0f);
    if (!isTouchGround()) return;

    JGeometry::TVec3f t;
    mRotMtx.getZDir(t); 
    jumpStart(t, 12.0f);
}

void TMapObjWanwan::doFunc_Back() {
    fallDown(3, (mWalkDuration - mWalkTimer < 30));
    if (mWalkTimer < 0) {
        mAnmObjCluster.getFrameCtrl()->setRate(1.0f);
        StateObserver::setState(2);
        return;
    }

    if (mWalkDuration - mWalkTimer == 30) {
        mAnmObjCluster.getFrameCtrl()->setRate(1.0f);
    }
    mWalkTimer--;
}

void TMapObjWanwan::fallDown(s32 nextState, bool p2) {
    mPos.add(mVel);

    if (isTouchGround()) {
        if (nextState != -1) {
            StateObserver::setState(nextState);
        }
    }
    else {
        turnDown(p2);
        turnTo();
        mVel.y += _20c;
        if (mVel.y < _218) {
            mVel.y = _218;
        }
    }

    createEmitterOnGround(&mpEmitter, "mk_wanSmoke_a");
}

// Nonmatching
// https://decomp.me/scratch/N6GWY
void TMapObjWanwan::turnTo() {
    f32 targetAngle = mAttackAngle;
    f32 rotate = mRotate;

    f32 minAngle = targetAngle - 3.141593f;
    f32 maxAngle = targetAngle + 3.141593f;

    if (minAngle <= maxAngle)
    {
        f32 range = maxAngle - minAngle; // 2pi

        while (rotate < minAngle)
        {
            rotate += range;
        }

        while (rotate >= maxAngle)
        {
            rotate -= range;
        }
    }

    rotate = _22c;
    if (StateObserver::getState() == 1) {
        rotate *= 2.0f;
    }

    if (-rotate < targetAngle - minAngle) {
        mRotate -= rotate;
    }
    else if (rotate > targetAngle - minAngle) {
        mRotate += rotate;
    }
    else {
        mRotate = targetAngle;
    }

    setRotate(mRotate);
}

void TMapObjWanwan::jumpStart(JGeometry::TVec3f &vel, f32 s) {
    vel.y = 0.0f;

    f32 scale = MAX(mScale.x, mScale.z);
    vel.normalize(s * scale);

    vel.y = sJumpHeight * mScale.y;
    mVel.set(vel);
}

bool TMapObjWanwan::isTouchGround() {
    return (mPos.y - mHeightOffset) <= getCourseHeight(mPos) ? true : false;
}

f32 TMapObjWanwan::getCourseHeight(JGeometry::TVec3f &pos) {
    mpGround->search(pos);
    return mpGround->getHeight();
}

f32 TMapObjWanwan::getAngleToRand() {
    JGeometry::TVec3f posDiff, backPos;
    posDiff.sub(mObjData->position, mPos);
    f32 angle = mAttackAngle;

    getWanwanBackPos(&backPos);
    backPos.sub(mObjData->position);

    const f32 l = posDiff.x * mRotMtx[0][2] + posDiff.z * mRotMtx[2][2]; 

    if ((l < 0.0f)
        && (backPos.length() > mChainLength * 0.8f)) 
    {
            f32 a = std::atan2f(posDiff.x, posDiff.z);
            a += getRandRadius(90, 20);
            angle = a;
            for (; angle < -F_PI; angle += F_TAU);
            for (; angle >= F_PI; angle -= F_TAU);
             
    }
    return angle;
}

f32 TMapObjWanwan::getRandRadius(u8 max, u8 min) {
    u32 range = max - min;
    u32 rand = GeographyObj::getGeoRnd()->get();
    int deg = (rand % range) * 2 - range;
    if (deg < 0) {
        deg -= min;
    }
    else {
        deg += min;
    }
    return MTXDegToRad(deg);
    
}

void TMapObjWanwan::getWanwanBackPos(JGeometry::TVec3f *out) {
    if (_1dc) {
        f32 x = mPos.x;
        f32 y = mPos.y + mHeightOffset;
        f32 z = mPos.z;
        out->set(x, y, z);
        return;
    }

    // TODO: all matrices from J3D might already be TPos3f of itself
    JGeometry::TPos3f *bumpMtx = (JGeometry::TPos3f *)mModel.getModel()->getAnmMtx(sChainJointNo);
    bumpMtx->getTrans(*out);
}

f32 TMapObjWanwan::getAngleToAttack(JGeometry::TVec3f &v) {
    f32 a = std::atan2f(v.x, v.z);
    for (; a < -F_PI; a += F_TAU);
    for (; a >= F_PI; a -= F_TAU);

    return a;
}

void TMapObjWanwan::InitExec() {
    Observer_FindAndInit(TMapObjWanwan, 7);
}

int TMapObjWanwan::searchTargetKart() {
    return SiUtil::searchNearKartBall(mObjData->position, _224);
}

void TMapObjWanwan::calc() {
    JGeometry::TVec3f pos(mPos);
    StateObserver::ExecuteState();
    if (StateObserver::getState() == 1) {
        mFreeMove.update();
    }
    chainCorrect();
    mpGround->search(mPos);
    if (mPos.y - mHeightOffset <= mpGround->getHeight() ) {
        mpGround->search(mPos);
        mPos.y = mHeightOffset + mpGround->getHeight() - 0.1f;
        mVel.y = 0.0f;
    }
    
    bool isWall = false;
    mpGround->search(mPos, pos);
    
    
    if (mpGround->getAttribute() == CrsGround::Attr_2) {
        JGeometry::TVec3f wall;
        
        f32 wallNorm = mpGround->getWallNormal(&wall, nullptr) + 20.0f;
        isWall = true;

        wall.scale(wallNorm);
        mPos.add(wall);
        mVel.set(wall);        
    }

    if (isWall) {
        switch (StateObserver::getState()) {
            case 2: {
                mAttackAngle = std::atan2f(mVel.x, mVel.z);
                mAttackAngle += getRandRadius(90, 20);

                f32 a = mAttackAngle;
                for (; a < -F_PI; a += F_TAU);
                for (; a >= F_PI; a -= F_TAU);
                mAttackAngle = a;
            }

        }
    }

    if (checkWanwanHitting()) {
        StateObserver::setState(4);
    }
    GeographyObj::moveShadowModel();
    if (_1dc) {
        _1dc = false;
    }
}

void TMapObjWanwan::update() {
    mAnmObjCluster.update();

    if (mAnmObjCluster.getFrameCtrl()->getFrame() == 0.0f && mAnmObjCluster.getFrameCtrl()->getRate() != 0.0f) {
        mSoundMgr->setSe(0x40021);
    }
    mSoundMgr->frameWork();
    setModelMatrixAndScale();
    mModel.update(0);
}

void TMapObjWanwan::chainCorrect() {
    JGeometry::TVec3f chainPos, objPos, chainOffset, zDir, vel, posDiff;
    if (_1dc) {
        objPos.set(mPos.x, mPos.y + mHeightOffset, mPos.z);
    }
    else {
        const Mtx &anm_mtx = mModel.getModel()->getAnmMtx(sChainJointNo);
        objPos.set(anm_mtx[0][3], anm_mtx[1][3], anm_mtx[2][3]);
    }

    chainPos.sub(objPos, mObjData->position);
    if (chainPos.length() > mChainLength) {
        if (mFreeMove.hasTarget() && chainPos.dot(mVel) > 0.0f) {
            mFreeMove.releaseTarget();
        
        }
        chainOffset.sub(mPos, objPos); 
        chainPos.normalize(mChainLength);
        chainPos.add(chainOffset);
        mPos.add(mObjData->position, chainPos);
    }

    mpStringNodeMgr->calc();
    
    mRotMtx.getZDir(zDir);
    zDir.scale(sChainBackInside * mScale.z);
    objPos.add(zDir);
    mpStringNodeMgr->setNodePos(0, objPos);

    posDiff.sub(mPos, mObjData->position);
    posDiff.normalize();
    posDiff.scaleAdd(50.0f, posDiff, mObjData->position);
    posDiff.y += 50.0f;
    mpStringNodeMgr->setNodePos(mNumChains - 1, posDiff);
    
    
    vel.zero();
    mpStringNodeMgr->setNodeVel(mNumChains - 1, vel);


    fixChain();
}

bool TMapObjWanwan::checkWanwanHitting() {
    bool ret = false;

    if (StateObserver::getState() != 4 && GeographyObj::tstItemHitting() ) {
        u32 itemKind = mColItemObj->getKind();
        if (itemKind == 7 || itemKind == 8 || itemKind == 1) {
            ret = true;
        }
    }
    return ret;
}

void TMapObjWanwan::fixChain() {
    static f32 addf = F_HALF_PI;

    f32 f = (mNumChains & 1) ? 0.0f : addf;

    JGeometry::TVec3f pos, nextPos, posDiff;
    for (u32 i = 0; i < mNumChains - 1; i++) {
        mpStringNodeMgr->getNodePos(i, &pos);
        mpStringNodeMgr->getNodePos(i + 1, &nextPos);

        posDiff.sub(pos, nextPos);
        posDiff.normalize();

        setChainPosition(mpaChain[i], pos, posDiff, f);
        f += addf;
    }
    
    mpStringNodeMgr->getNodePos(mNumChains - 1, &pos);
    posDiff.sub(mPos, mObjData->position);
    posDiff.y = 0.0f;
    posDiff.normalize();
    setChainPosition(mpaChain[mNumChains - 1], pos, posDiff, f);
}

void TMapObjWanwan::setChainPosition(TMapObjWanwanChain *pChain, JGeometry::TVec3f &rPos, JGeometry::TVec3f &rPosDiff, f32 f) {

    JGeometry::TVec3f cp, yDir;
    mRotMtx.getYDir(yDir);
    pChain->setPosition(rPos);

    cp.cross(rPosDiff, yDir);
    cp.normalize();
    yDir.cross(cp, rPosDiff);
    yDir.normalize();

    JGeometry::TPos3f r_m, b;
    r_m.setXYZDir(cp, yDir, rPosDiff);
    PSMTXIdentity(b);

    f32 s = sin(f);
    f32 c = cos(f);

    b[0][0] = c;
    b[0][1] = -s;
    b[1][0] = s;
    b[1][1] = c;
    b[2][2] = 1.0;
    b[2][1] = 0.0;
    b[1][2] = 0.0;
    b[2][0] = 0.0;
    b[0][2] = 0.0;
    
    PSMTXConcat(r_m, b, r_m);
    pChain->mRotMtx.set( r_m );
}

void TMapObjWanwan::MoveExec() {
    Observer_FindAndExec(TMapObjWanwan, 7);
}

void TMapObjWanwan::createEmitterOnGround(JPABaseEmitter **paEmitter, const char *pName) {
    J3DFrameCtrl *ctrl = mAnmCtrl->getFrameCtrl(0);
    if (ctrl->getFrame() != 4.0f) return;

    CrsGround ground(RCMGetCourse());
    ground.search(mPos);
    if (ground.getMaterial() != CrsGround::Mat_1) return;

    *paEmitter = GetJPAMgr()->createEmt(pName, mPos);
    if (*paEmitter) {
        JGeometry::TVec3f scale(mScale);
        (*paEmitter)->setGlobalScale(scale);
    }
}

#include "JSystem/JAudio/JASFakeMatch2.h"
