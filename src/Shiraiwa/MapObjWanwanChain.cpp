#include "Sato/GeographyObj.h"
#include "Sato/ObjUtility.h"
#include "Shiraiwa/Objects/MapObjWanwan.h"

#include "JSystem/JAudio/JASFakeMatch2.h"

f32 TMapObjWanwanChain::sChainOneLength = 120.0f;

TMapObjWanwanChain::TMapObjWanwanChain() : GeographyObj(0xe79) {
    GeographyObj::setObjFlagSimpleDraw();
    GeographyObj::NewAnmCtrl();
}

TMapObjWanwanChain::~TMapObjWanwanChain() {}

void TMapObjWanwanChain::reset() {
    GeographyObj::resetObject();
    GeographyObj::clrObjFlagCheckGeoHitting();
    GeographyObj::clrObjFlagCheckItemHitting();
    GeographyObj::clrAllCheckKartHitFlag();
    _58 = 0;
}

void TMapObjWanwanChain::createColModel(J3DModelData *) {}

const char *TMapObjWanwanChain::getBmdFileName() {
    static const char *cBmdName = "/Item/wanwan_chn.bmd";
    return cBmdName;
}

void TMapObjWanwanChain::setCurrentViewNo(u32 viewNo) {
    mModel.setCurrentViewNo(viewNo);
    Mtx m;
    ObjUtility::getCamDependLightMtx(viewNo, mPos, m);
    mModel.setEffectMtx(m, 0);
}

TMapObjWanwanPile::TMapObjWanwanPile() : GeographyObj(0xe7a) {
    GeographyObj::setObjFlagSimpleDraw();
    GeographyObj::NewAnmCtrl();
}

TMapObjWanwanPile::~TMapObjWanwanPile() {}

void TMapObjWanwanPile::doKartColCallBack(int) {}

void TMapObjWanwanPile::reset() {
    GeographyObj::resetObject();
    GeographyObj::clrObjFlagCheckGeoHitting();
    GeographyObj::clrObjFlagCheckItemHitting();
    GeographyObj::clrAllCheckKartHitFlag();
    _58 = 0;
}

void TMapObjWanwanPile::createColModel(J3DModelData *) {}

const char *TMapObjWanwanPile::getBmdFileName() {
    static const char *cBmdName = "/Objects/Wanwan1Pile.bmd";
    return cBmdName;
}
