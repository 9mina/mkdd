#include "Sato/StringObj.h"
#include "Inagaki/GameSoundMgr.h"
#include "JSystem/JGeometry/Matrix.h"
#include "JSystem/JGeometry/Vec.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "JSystem/JMath/JMath.h"
#include "JSystem/JSupport/JSUList.h"
#include "JSystem/JUtility/JUTAssert.h"
#include "Kaneshige/Course/Course.h"
#include "Kaneshige/Course/CrsArea.h"
#include "Kaneshige/Course/CrsGround.h"
#include "Kaneshige/DarkAnmMgr.h"
#include "Kaneshige/ExModel.h"
#include "Kaneshige/RaceMgr.h"
#include "Kaneshige/SimpleDrawer.h"
#include "Kaneshige/TexLODControl.h"
#include "Sato/ObjUtility.h"
#include "Sato/stMath.h"
#include "dolphin/mtx.h"
#include "JSystem/JGeometry/Util.h"
#include "types.h"

#include "JSystem/JAudio/JASFakeMatch2.h"

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

void StringNodeManager::calcBetweenNodeVelAll(const f32 f1) {

}

void StringNodeManager::calcBetweenNodePosAll(const f32 f1) {
    JSUListIterator<StringNode> it = mStrNodeList.getLast()->getPrev();
    
    for (JSUListIterator<StringNode> it2(mStrNodeList.getLast()); it != mStrNodeList.getFirst(); --it, --it2) {
        calcBetweenNodePos(it2.getObject(), it.getObject(), f1, _10);
    }
    
    it = mStrNodeList.getFirst()->getNext();
    for (JSUListIterator<StringNode> it2(mStrNodeList.getFirst()); it != mStrNodeList.getEnd(); ++it, ++it2) {
        calcBetweenNodePos(it2.getObject(), it.getObject(), f1, _10);
    }
}

void StringNodeManager::calcBetweenNodeVel(StringNode *strNodeOne, StringNode *strNodeTwo, f32 f1) {

}

void StringNodeManager::calcBetweenNodePos(StringNode *strNodeOne, StringNode *strNodeTwo, const f32 f1, const f32 f2) {
    JGeometry::TVec3f vecInput(strNodeTwo->mPos); // TODO: Declaration is needed, but why is this unused...?
    JGeometry::TVec3f vecDiff;
    JGeometry::TVec3f vecNormalised;
    JGeometry::TVec3f vecScaled;
    JGeometry::TVec3f vecFinal;

    PSVECSubtract(&strNodeOne->mPos, &strNodeTwo->mPos, &vecDiff);
    f32 magnitude = vecDiff.length();

    if (magnitude > f2) {
        PSVECNormalize(&vecDiff, &vecNormalised);
        PSVECScale(&vecNormalised, &vecScaled, -f2);
        PSVECAdd(&strNodeOne->mPos, &vecScaled, &strNodeTwo->mPos);

        vecFinal.normalize(vecDiff);

        f32 fVar2 = (f1 * (magnitude - f2));
        f32 fVar3 = fVar2 > _44 ? _44 : fVar2;

        vecFinal.scale(fVar3);
        strNodeTwo->mVel += vecFinal;
    }
}

void StringNodeManager::resetNodeAll(JGeometry::TVec3f *pPos) {
    for (JSUListIterator<StringNode> it(mStrNodeList.getFirst()); it.isAvailable(); ++it) {
        if (!pPos) {
            it->mPos.zero();
            it->_18.zero();
        }
        else {
            it->mPos = *pPos;
            it->_18 = it->mPos;
        }
        it->mVel.zero();
        it->_24.zero();
        it->_31 = 0;
        it->_34 = 0.0f;
    }
}

void StringNodeManager::moveNodeAll() {
    for (JSULink<StringNode> *link = mStrNodeList.getFirst()->getNext(); link != nullptr; link = link->getNext()) {
        StringNode *stringNode = link->getObject();
        PSVECAdd(&stringNode->mPos, &stringNode->mVel, &stringNode->mPos);
    }
}

void StringNodeManager::doAirFricG(const f32 friction, const f32 globalScale) {
    for (JSULink<StringNode> *link = mStrNodeList.getFirst()->getNext(); link != nullptr; link = link->getNext()) {
        link->getObject()->mVel.scale(friction);
        JMAVECScaleAdd(&_28, &link->getObject()->mVel, &link->getObject()->mVel, globalScale); // why does this not use scale mVel.scaleAdd?
    }
}

void StringNodeManager::doHeightCol() {
    JSULink<StringNode> *node;
    CrsArea crsArea;
    u8 someCount = 0;
    
    for (node = mStrNodeList.getFirst()->getNext(); node != nullptr; node = node->getNext()) {
        if (node->getObject()->_30 != 0) {
            bool someCheck = false;
            crsArea.search(2, node->getObject()->mPos);

            if (crsArea.getArea() != nullptr) {
                JGeometry::TVec3f roofPos;
                crsArea.getRoofPosition(&roofPos);

                if (node->getObject()->mPos.y + 100.0f > roofPos.y) {
                    someCheck = true;
                    node->getObject()->mVel.y = 0.0f;
                    node->getObject()->mVel.x *= _18;
                    node->getObject()->mVel.z *= _18;
                    node->getObject()->mPos.y = roofPos.y - 100.0f;
                }
            }
            
            CrsGround crsGroundInit(RaceMgr::sRaceManager->getCourse());
            CrsGround *crsGround = &crsGroundInit;

            if ((mCrsGround != nullptr) && (someCount == _4c)) {
                crsGround = mCrsGround;
            }

            crsGround->search(node->getObject()->mPos, node->getObject()->_18);

            if (crsGround->getAttribute() != 10) {
                if (crsGround->getHeight() > (node->getObject()->mPos.y - _14)) {
                    JGeometry::TVec3f crsGroundNormal;
                    crsGround->getNormal(&crsGroundNormal);
                    
                    f32 crsHeight = crsGround->getHeight();
                    f32 newScale = crsHeight - (node->getObject()->mPos.y - _14);
                    crsGroundNormal.scale(newScale * _18);
                    node->getObject()->mVel.add(crsGroundNormal);

                    StringNode *stringNode = node->getObject();
                    crsHeight = crsGround->getHeight();
                    someCheck = true;
                    stringNode->mPos.y = _14 + crsHeight;
                }
            }

            if (someCheck) {
                node->getObject()->_31 = true;
                if ((mObjSoundMgr != nullptr) && (someCount == _40)) {
                    mObjSoundMgr->setSe(_3c);
                }
                
            } else {
                node->getObject()->_31 = false;
            }

            crsGround->getNormal(&node->getObject()->_24);
            node->getObject()->_18 = node->getObject()->mPos;
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

void ExStringNodeManager::calcBetweenNodePosAll(const f32 p1) {
    JSUListIterator<StringNode> it, it2;
    for (it = mStrNodeList.getLast()->getPrev(), it2 = mStrNodeList.getLast(); it != mStrNodeList.getFirst(); --it, --it2) {
        f32 a = it->_34; // whatever
        calcBetweenNodePos(it2.getObject(), it.getObject(), p1, a);
    }
    
    for (it2 = mStrNodeList.getFirst()->getNext(), it = mStrNodeList.getFirst(); it2.isAvailable(); ++it2, ++it) {
        calcBetweenNodePos(it.getObject(), it2.getObject(), p1, it->_34);
    }
}

void ExStringNodeManager::setNodeLengthAll(f32 newLength) {
    for (JSULink<StringNode> *link = mStrNodeList.getFirst(); link != nullptr; link = link->getNext()) {
        link->getObject()->_34 = newLength;
    }
}

StringObj::StringObj(u8 nodeCount, bool someFlag) {
    mStringNodeMgr = new ExStringNodeManager(nodeCount, someFlag, true, false, 0);
    mExModel = new ExModel[nodeCount];
    _c = 0;
    _18 = 1;
    mItemDarkAnmPlayer = new ItemDarkAnmPlayer*[nodeCount];

    for (u8 i = 0; i < nodeCount; i++) {
        mItemDarkAnmPlayer[i] = new ItemDarkAnmPlayer(7);
    }

    mStringNodeMgr->set_3c(0x40000);
    reset();
}

void StringObj::createModel(JKRSolidHeap *, u32) {
    return;
}

void StringObj::loadmodel(J3DModelData *modelData) {
    ExModel *exModel = mExModel;
    u32 i = 0; 
    while (i < mStringNodeMgr->mStrNodeList.getNumLinks()) {
        exModel->setModelData(modelData);
        RCMGetCourse()->setFogInfo(exModel);
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
    mStringNodeMgr->calc();

    JGeometry::TVec3f modelScale;
    modelScale.setAll(mScale);

    ExModel *exModel = mExModel;
    JSUListIterator<StringNode> it(mStringNodeMgr->mStrNodeList.getFirst());

    JGeometry::TRot3f m;
    PSMTXCopy(_14, m);

    f32 scale = 0.0001f;
    if (mScale > 0.0f) {
        scale = 1.0f / mScale;
    }
    PSMTXScaleApply(m, m, scale, scale, scale);

    Mtx r_m;
    stMTXRotDeg(r_m, 'Z', 90.0f);

    for (u32 i = 0; i < mStringNodeMgr->mStrNodeList.getNumLinks() - 1; i++) {
        if (i < mStringNodeMgr->mStrNodeList.getNumLinks() - 1) {
            JGeometry::TVec3f cp, ang, vel, pos, pos2;

            m.getYDir(ang);

            mStringNodeMgr->getNodePos(i, &pos);
            mStringNodeMgr->getNodePos(i + 1, &pos2);
            vel.sub(pos, pos2);
            
            stVecNormalize(vel);
            PSVECCrossProduct(&ang, &vel, &cp);
            stVecNormalize(cp);
            PSVECCrossProduct(&vel,&cp,&ang);
            stVecNormalize(ang);
            stMakeRMtx(m, cp, ang, vel);
            PSMTXConcat(m, r_m, m);

        }
        exModel->setBaseScale(modelScale);

        const JGeometry::TVec3f &modelPos(it->mPos);
        m[0][3] = modelPos.x;
        m[1][3] = modelPos.y;
        m[2][3] = modelPos.z;
        exModel->setBaseTRMtx(m);
        exModel++;
        ++it;
    }

    CrsArea area;
    for (u32 i = 0; i < mStringNodeMgr->mStrNodeList.getNumLinks() - 1; i++) {
        JGeometry::TVec3f pos;
        mStringNodeMgr->getNodePos(i, &pos);
        area.search(0, pos);
        mItemDarkAnmPlayer[i]->calc(area, 0.0f);
    }
}

void StringObj::update() {
    ExModel *exModel = mExModel;

    for (u32 i = 0; i < mStringNodeMgr->mStrNodeList.getNumLinks() - 1; i++, exModel++) {
        exModel->update(0);
    }
}

void StringObj::setCurrentViewNo(const u32 viewNo) {
    
    ExModel *exModel = mExModel;
    for (u32 i = 0; i < mStringNodeMgr->mStrNodeList.getNumLinks() - 1; i++, exModel++) {
        exModel->setCurrentViewNo(viewNo);

        JGeometry::TVec3f pos;
        mStringNodeMgr->getNodePos(i, &pos);

        Mtx lightMtx;
        ObjUtility::getCamDependLightMtx(viewNo, pos, lightMtx);
        mExModel[i].setEffectMtx(lightMtx, 0);
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

            mExModel[num].clipBySphere(p1, j3duClipper, mtx1, mScale);
            mItemDarkAnmPlayer[num]->setTevColor(&mExModel[num]);
            mExModel[num].simpleDraw(p1, mtx2, 1);
        }
    }
}
