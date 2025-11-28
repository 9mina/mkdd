#include "Sato/StringObj.h"
#include "Inagaki/GameSoundMgr.h"
#include "JSystem/JGeometry/Vec.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "JSystem/JMath/JMath.h"
#include "JSystem/JSupport/JSUList.h"
#include "JSystem/JUtility/JUTAssert.h"
#include "Kaneshige/Course/Course.h"
#include "Kaneshige/Course/CrsGround.h"
#include "Kaneshige/DarkAnmMgr.h"
#include "Kaneshige/ExModel.h"
#include "Kaneshige/RaceMgr.h"
#include "Kaneshige/SimpleDrawer.h"
#include "Kaneshige/TexLODControl.h"
#include "Sato/ObjUtility.h"
#include "dolphin/mtx.h"
#include "JSystem/JGeometry/Util.h"
#include "types.h"


StringNodeManager::StringNodeManager(u8 count, f32 speed, bool makeSoundMgr, bool makeCrsGround, u8 someFlag) {
    mCrsGround = nullptr;
    _10 = speed;

    _34 = new StringNode[count];
    StringNode* node = _34;

    for (u8 i = 0; i < count; i++, node++) {
        mStrNodeList.append(&node->_38);
    }

    _24 = 0.8f;
    _20 = 0.97f;
    _1c = -1.0f;
    _14 = 30.0f;
    _18 = 0.0f;
    _28.x = 0.0f;
    _28.y = 1.0f;
    _28.z = 0.0f;
    _44 = 100.0f;
    _3c = 0;
    _40 = 0;

    mObjSoundMgr = makeSoundMgr
        ? new GameAudio::ObjectSoundMgr(&mStrNodeList.getNth(_40)->getObject()->mPos, nullptr)
        : nullptr;

    if (makeCrsGround) {
        mCrsGround = new CrsGround(RaceMgr::sRaceManager->getCourse());
    }

    _4c = someFlag;
}

StringNode::~StringNode() {}

StringNode::StringNode() : _38(this) {
    mVel.zero();
    mPos.zero();
    _18.zero();
    _24.zero();
    _30 = true;
    _31 = false;
    _34 = 0.0f;
}

void StringNodeManager::setAllNodePos(const JGeometry::TVec3f &param_1) {
    for (u8 i = 0; i < mStrNodeList.getNumLinks(); i++) {
        setNodePos(i, param_1);
    }
}

void StringNodeManager::calc() {
    doAirFricG(_20, _1c);
    moveNodeAll();
    calcBetweenNodePosAll(_24);
    doHeightCol();
    if (mObjSoundMgr != nullptr) {
        mObjSoundMgr->frameWork();
    }
}

void StringNodeManager::calcBetweenNodePosAll(f32 param_1) {
}

void StringNodeManager::calcBetweenNodePos(StringNode *strNodeOne, StringNode *strNodeTwo, f32 f1, f32 f2) {
    JGeometry::TVec3f vecInput = strNodeTwo->mPos; // TODO: Declaration is needed, but why is this unused...?
    JGeometry::TVec3f vecDiff;
    JGeometry::TVec3f vecNormalised;
    JGeometry::TVec3f vecScaled;
    JGeometry::TVec3f vecFinal;

    PSVECSubtract(&strNodeOne->mPos, &strNodeTwo->mPos, &vecDiff);
    f32 magnitude = PSVECMag(&vecDiff);

    if (magnitude > f2) {
        PSVECNormalize(&vecDiff, &vecNormalised);
        PSVECScale(&vecNormalised, &vecScaled, -f2);
        PSVECAdd(&strNodeOne->mPos, &vecScaled, &strNodeTwo->mPos);

        f32 fVar1 = vecDiff.squared();
        if (fVar1 <= JGeometry::TUtilf::epsilon()) {
            vecFinal.zero();
        } else {
            fVar1 = JGeometry::TUtilf::inv_sqrt(fVar1);
            vecFinal.scale(fVar1, vecDiff);
        }

        f32 fVar2 = (f1 * (magnitude - f2));
        f32 fVar3 = fVar2 > _44 ? _44 : fVar2;

        vecFinal.scale(fVar3);
        strNodeTwo->mVel += vecFinal;
    }
}

void StringNodeManager::resetNodeAll(JGeometry::TVec3f *param_1) {
}

void StringNodeManager::moveNodeAll() {
    for (JSULink<StringNode> *link = mStrNodeList.getFirst()->getNext(); link != nullptr; link = link->getNext()) {
        StringNode *stringNode = link->getObject();
        PSVECAdd(&stringNode->mPos, &stringNode->mVel, &stringNode->mPos);
    }
}

void StringNodeManager::doAirFricG(f32 friction, f32 globalScale) {
    for (JSULink<StringNode> *link = mStrNodeList.getFirst()->getNext(); link != nullptr; link = link->getNext()) {
        link->getObject()->mVel.scale((f32)friction);
        JMAVECScaleAdd(&_28, &link->getObject()->mVel, &link->getObject()->mVel, globalScale);
    }
}

void StringNodeManager::doHeightCol() {
    CrsArea crsArea;
    bool hittingRoof;
    u8 someCount = 0;

    for (JSULink<StringNode> *link = mStrNodeList.getFirst()->getNext(); link != nullptr; link = link->getNext()) {
        if (link->getObject()->_30 != 0) {
            hittingRoof = false;
            crsArea.search(2, link->getObject()->mPos);

            if (crsArea.getArea() != nullptr) {
                JGeometry::TVec3f roofPos;
                crsArea.getRoofPosition(&roofPos);
                if ((link->getObject()->mPos.y + 100.0f) > roofPos.y) {
                    hittingRoof = true;
                    link->getObject()->mVel.y = 0.0f;
                    link->getObject()->mVel.x *= _18;
                    link->getObject()->mVel.z *= _18;
                    link->getObject()->mPos.y = roofPos.y - 100.0f;
                }
            }

            CrsGround crsGround2(RaceMgr::sRaceManager->getCourse());
            CrsGround *crsGround = &crsGround2;
            if ((mCrsGround != nullptr) && (someCount == _4c)) {
                crsGround = mCrsGround;
            }

            crsGround->search(link->getObject()->mPos, link->getObject()->_18);
            StringNode *stringNode;
            if ((crsGround->getAttribute() != CrsGround::Attr_10) &&
                (crsGround->getHeight() > link->getObject()->mPos.y - _14)) {

                JGeometry::TVec3f crsGroundNormal;
                crsGround->getNormal(&crsGroundNormal);
                
                f32 crsHeight = crsGround->getHeight();

                f32 yPos = link->getObject()->mPos.y - crsHeight - _14;
                crsGroundNormal *= yPos * _18;
                link->getObject()->mVel.add(crsGroundNormal);
                
                stringNode = link->getObject();
                crsHeight = crsGround->getHeight();
                hittingRoof = true;
                stringNode->mPos.y = _14 + crsHeight;
            }
            
            if (hittingRoof) {
                link->getObject()->_31 = true;
                if ((mObjSoundMgr != nullptr) && (someCount == _40)) {
                    mObjSoundMgr->setSe(_3c);
                }
            } else {
                link->getObject()->_31 = false;
            }
            crsGround->getNormal(&link->getObject()->_24);
            stringNode = link->getObject();
            stringNode->_18 = stringNode->mPos;
        }
        someCount++;
    }
}

void StringNodeManager::getNodeVel(u32 num, JGeometry::TVec3f *vel) {
#line 421
    JUT_MINMAX_ASSERT(0, num, mStrNodeList.getNumLinks());
    *vel = mStrNodeList.getNth(num)->getObject()->mVel;
}

void StringNodeManager::getNodePos(u32 num, JGeometry::TVec3f *pos) {
#line 434
    JUT_MINMAX_ASSERT(0, num, mStrNodeList.getNumLinks());
    *pos = mStrNodeList.getNth(num)->getObject()->mPos;
}

void StringNodeManager::addNodeVel(u32 num, JGeometry::TVec3f newVel) {
#line 448
    JUT_MINMAX_ASSERT(0, num, mStrNodeList.getNumLinks());
    mStrNodeList.getNth(num)->getObject()->mVel += newVel;
}

void StringNodeManager::setNodeVel(u32 num, JGeometry::TVec3f newVel) {
#line 461
    JUT_MINMAX_ASSERT(0, num, mStrNodeList.getNumLinks());
    mStrNodeList.getNth(num)->getObject()->mVel = newVel;
}

void StringNodeManager::addNodePos(u32 num, JGeometry::TVec3f newPos) {
#line 474
    JUT_MINMAX_ASSERT(0, num, mStrNodeList.getNumLinks());
    mStrNodeList.getNth(num)->getObject()->mPos += newPos;
}

void StringNodeManager::setNodePos(u32 num, JGeometry::TVec3f newPos) {
#line 487
    JUT_MINMAX_ASSERT(0, num, mStrNodeList.getNumLinks());
    mStrNodeList.getNth(num)->getObject()->mPos = newPos;
}

void ExStringNodeManager::calcBetweenNodePosAll(f32 param_1) {
}

void ExStringNodeManager::setNodeLengthAll(f32 newLength) {
    for (JSULink<StringNode> *link = mStrNodeList.getFirst(); link != nullptr; link = link->getNext()) {
        link->getObject()->_34 = newLength;
    }
}

StringObj::StringObj(u8 nodeCount, bool someFlag) {
}

StringNodeManager::~StringNodeManager() {
    delete[] _34;
}

void StringObj::createModel(JKRSolidHeap *, u32) {
    return;
}

void StringObj::loadmodel(J3DModelData *modelData) {
    ExModel *exModel = mExModel;
    u32 i = 0; 
    while (i < mStringNodeMgr->mStrNodeList.getNumLinks()) {
        exModel->setModelData(modelData);
        RaceMgr::sRaceManager->getCourse()->setFogInfo(exModel);
        exModel->setLODBias(
            TexLODControl::getManager()->getLODBias(TexLODControl::cLODBiasID_3)
        );
        i++;
        exModel++;
    }
}

void StringObj::reset() { 
    for (u32 i = 0; i < mStringNodeMgr->mStrNodeList.getNumLinks(); i++) {
        mItemDarkAnmPlayer[i]->reset();
    }
}

void StringObj::calc() {
    
}

void StringObj::update() {
    ExModel *exModel = mExModel;

    for (u32 i = 0; i < mStringNodeMgr->mStrNodeList.getNumLinks() - 1; i++, exModel++) {
        exModel->update(0);
    }
}

void StringObj::setCurrentViewNo(u32 viewNo) {
    ExModel *exModel = mExModel;
    for (u32 num = 0; num < mStringNodeMgr->mStrNodeList.getNumLinks() - 1; num++, exModel++) {
        exModel->setCurrentViewNo(viewNo);

        JGeometry::TVec3f pos;
        mStringNodeMgr->getNodePos(viewNo, &pos);

        Mtx lightMtx;
        ObjUtility::getCamDependLightMtx(viewNo, pos, lightMtx);
        mExModel[num].setEffectMtx(lightMtx, 0);
    }
}

void StringObj::drawSimpleModel(u32 p1, Mtx mtx1, J3DUClipper *j3duClipper, Mtx mtx2) {
    SimpleDrawer simpleDrawer;
    
    if (!mExModel->getModelData()) return;
    
    simpleDrawer.drawInit(mExModel);
    
    while (simpleDrawer.loadPreDrawSetting()) {
        for (u32 num = 0; num < mStringNodeMgr->mStrNodeList.getNumLinks() - 1; num++) {
            JGeometry::TVec3f pos;
            mStringNodeMgr->getNodePos(num, &pos);
            f32 radius = pos.x; // FIX: should be `pos.z`, but this reorders instructions weirdly...?

            mExModel[num].clipBySphere(p1, j3duClipper, mtx1, radius);
            mItemDarkAnmPlayer[num]->setTevColor(&mExModel[num]);
            mExModel[num].simpleDraw(p1, mtx2, 1);
        }
    }
}

ExStringNodeManager::~ExStringNodeManager() {}
