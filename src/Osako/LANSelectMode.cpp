#include "Osako/LANSelectMode.h"
#include "Inagaki/GameAudioMain.h"
#include "JSystem/J2D/J2DAnimation.h"
#include "JSystem/J2D/J2DAnmLoader.h"
#include "JSystem/J2D/J2DPane.h"
#include "JSystem/J2D/J2DPicture.h"
#include "JSystem/J2D/J2DScreen.h"
#include "JSystem/JKernel/JKRFileLoader.h"
#include "JSystem/JUtility/JUTAssert.h"
#include "JSystem/ResTIMG.h"
#include "Osako/Kart2DCommon.h"
#include "Osako/LANPlayInfo.h"
#include "Osako/MenuTitleLine.h"
#include "Osako/NetGameMgr.h"
#include "Osako/kartPad.h"
#include "Osako/system.h"
#include "mathHelper.h"
#include "types.h"
#include <utility>

static J2DPicture *spaColorPicture[3];

void LANSelectMode::Background::setup(JKRArchive *archive) {
    mScreen.setPriority("Back2.blo", 0x20000, archive);
    void *bck = JKRGetNameResource("Back2.bck", archive);
    void *btk = JKRGetNameResource("Back2.btk", archive);
    _118 = (J2DAnmTransform*)J2DAnmLoaderDataBase::load(bck);
    _11c = (J2DAnmTextureSRTKey*)J2DAnmLoaderDataBase::load(btk);
    mScreen.setAnimation(_118);
    mScreen.setAnimation(_11c);
}

void LANSelectMode::Background::calc() {
    _118->setFrame(_120);
    _11c->setFrame(_122);

    if (++_120 >= 178) {
        _120 = 0;
    }

    if (++_122 >= 1400) {
        _122 = 0;
    }
    mScreen.animation();
}

LANSelectMode::Item::Item(u32 tagPrefix, J2DScreen *screen, void *bck) {
    // Size mismatch, should be 0x148 (currently 0x144)
    // however appears to match
    _4 = 0;
    _10 = (J2DAnmTransform*)J2DAnmLoaderDataBase::load(bck);
    
    const u64 tag = (u64)tagPrefix << 16ull;
    _8 = screen->search(0x5F43 + tag);
    J2DPane *pPane = screen->search(0x5F4D + tag);
#line 96
    JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);
    _c = (J2DPicture *)pPane;
    _8->setAnimation(_10);
    _c->setAnimation(_10);
    _14 = 0x14;
}

void LANSelectMode::Item::changeColor(LANSelectMode::Color color) {
    _c->setWhite(spaColorPicture[color]->getWhite());
    _c->setBlack(spaColorPicture[color]->getBlack());
}

// perhaps in and out should be swapped, who knows
void LANSelectMode::Item::selectIn() {
    _4 = 3;
    changeColor(cLanColor_1);
}

void LANSelectMode::Item::selectOut() {
    _4 = 1;
    changeColor(cLanColor_2);
}

void LANSelectMode::Item::unselect() {
    _14 = 20;
    _4 = 0;
    changeColor(cLanColor_1);
}

void LANSelectMode::Item::calc() {
    _10->setFrame(_14);
    switch (_4) {
    case 1: {
        if (++_14 >= 29) {
            _14 = 29;
            _4 = 2;
        }
        break;
    }
    case 3: {
        if (--_14 < 20) {
            _14 = 20;
            _4 = 0;
        }
        break;
    }
    }
}

LANSelectMode::SelectableItem::SelectableItem(u32 p1, J2DScreen *pScreen, void *p3, void *p4, void *p5, int p6, int p7, u32 p8) : Item(p1, pScreen, p3) {
    _18 = p7;
    _1c = p6;
    _20 = p8;
    _24 = 0;
    _28 = (J2DAnmTransform*)J2DAnmLoaderDataBase::load(p3);
    _2c = (J2DAnmTransform*)J2DAnmLoaderDataBase::load(p3);
    
    _30 = (J2DAnmTextureSRTKey*)J2DAnmLoaderDataBase::load(p4);
    _30->searchUpdateMaterialID(pScreen);
    
    _34 = (J2DAnmTevRegKey*)J2DAnmLoaderDataBase::load(p5);
    _34->searchUpdateMaterialID(pScreen);

    _38 = (J2DAnmTevRegKey*)J2DAnmLoaderDataBase::load(p5);
    _38->searchUpdateMaterialID(pScreen);
    _44 = 90;
    _46 = 40;
    _48 = 0;
    _4a = 0;
    _4c = 0;

    J2DPane *pPane;
    
    pPane = pScreen->search(p1 | 0x4e5f000000ull);
    pPane->setAnimation(_10);

    
    const u64 tag = (u64)p1 << 24;
    pPane = pScreen->search(0x744c30 + tag);
    pPane->setAnimation(_2c);

    pPane = pScreen->search(0x745230 + tag);
    pPane->setAnimation(_2c);

    _3c = pScreen->search(0x744c31 + tag);
    _3c->setAnimation(_34);

    _40 = pScreen->search(0x745231 + tag);
    _40->setAnimation(_34);

    pPane = pScreen->search(0x744c32 + tag);
    pPane->setAnimation(_30);

    pPane = pScreen->search(0x745232 + tag);
    pPane->setAnimation(_30);
}

bool LANSelectMode::SelectableItem::changeValueUp() {
    int origValue = _18;
    do {
        if (++_18 >= _1c) {
            _18 = 0;
        }
    } while (_20 & (1 << _18));

    if (_18 != origValue) {
        _24 = 1;
        _3c->setAnimation((J2DAnmTransform*)nullptr);
        _3c->setAnimation(_34);
        _40->setAnimation(_28);
        _40->setAnimation(_38);
        _44 = 90;
        _4c = 2;
        return true;
    }
    return false;
}

bool LANSelectMode::SelectableItem::changeValueDown() {
    int origValue = _18;
    do {
        if (--_18 < 0) {
            _18 = _1c - 1;
        }
    } while (_20 & (1 << _18));

    if (_18 != origValue) {
        _24 = 1;
        _3c->setAnimation(_28);
        _3c->setAnimation(_38);
        _40->setAnimation((J2DAnmTransform*)nullptr);
        _40->setAnimation(_34);
        _44 = 90;
        _4c = 2;
        return true;
    }
    return false;
}

void LANSelectMode::SelectableItem::setMask(u32 mask) {
    _20 = mask;
    if (_20 & 1 << _18) {
        changeValueDown();
    }
}

void LANSelectMode::SelectableItem::setMax(int max) {
    _1c = max;
    if (_18 >= _1c) {
        changeValueDown();
    }
}

void LANSelectMode::SelectableItem::update() {
    _24 = 1;
    _44 = 93;
    calc();
    _44 = 90;
    _24 = 0;
}

void LANSelectMode::SelectableItem::calc() {
    Item::calc();
    _28->setFrame(_44);
    
    switch (_24) {
    case 1: {
        if (++_44 >= 99) {
            _44 = 98;
        }
        if (_4c > 1 && ++_4c >= 0x14) {
            _4c = 1;
            _24 = 0;
        } 
        break;
    }
    
    }
    _2c->setFrame(_46);
    _30->setFrame(_48);

    switch (_4) {
    case 1: {
        _4a = 1;
        _4c = 1;
        break;
    }
    case 2: {
        if (++_46 >= 80) {
            _46 = 40;
        }
        if (++_48 >= 120) {
            _48 = 0;
        }
        _4a = 1;
        break;
    }
    case 0: {
        _46 = 40;
        _4a = 0;
        _48 = 0;
        _4c = 0;
        break;
    }
    }

    _34->setFrame(_4a);
    _38->setFrame(_4c);
}

void LANSelectMode::SelectableItem1::changeColor(LANSelectMode::Color color) {
    Item::changeColor(color);
    _50->setWhite(spaColorPicture[color]->getWhite());
    _50->setBlack(spaColorPicture[color]->getBlack());
}

LANSelectMode::SelectableItem1::SelectableItem1(u32 tagPrefix, J2DScreen *screen, void *bck, void *p4, void *p5, ResTIMG **paImg, int p7) : SelectableItem(tagPrefix, screen, bck, p4, p5, p7, 0, 0) {
    _54 = paImg;
    J2DPane *pPane = screen->search(0x7377ull + ((u64)tagPrefix << 16));
#line 406
    JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);
    _50 = (J2DPicture*)pPane;
    _50->setAnimation(_28);
    _50->changeTexture(_54[_18], 0);
}

void LANSelectMode::SelectableItem1::calc() {
    SelectableItem::calc();
    if (_24 == 1 && _44 == 94) {
        _50->changeTexture(_54[_18], 0);
    }
}

LANSelectMode::SelectableItem2::SelectableItem2(u32 tagPrefix, J2DScreen *screen, void *bck, void *btk, void *brk) : SelectableItem(tagPrefix, screen, bck, btk, brk, 10, 0, 0) {
    J2DPane *pPane;
    pPane = screen->search(0x737730ull + ((u64)tagPrefix << 24));

#line 457
    JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);
    _54 = (J2DPicture*)pPane;
    _54->setAnimation(_28);
    
    pPane = screen->search(0x737731ull + ((u64)tagPrefix << 24));
    JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);
    _50 = (J2DPicture*)pPane;
    _50->setAnimation(_28);
    _50->hide();
}

void LANSelectMode::SelectableItem2::changeColor(LANSelectMode::Color color) {
    Item::changeColor(color);
    _54->setWhite(spaColorPicture[color]->getWhite());
    _54->setBlack(spaColorPicture[color]->getBlack());
    _50->setWhite(spaColorPicture[color]->getWhite());
    _50->setBlack(spaColorPicture[color]->getBlack());
}

void LANSelectMode::SelectableItem2::calc() {
    SelectableItem::calc();

    if (_24 == 1 && _44 == 94) {
        if (_18 != 0) {
            _50->changeTexture(Kart2DCommon::ptr()->getNumberTexture((Kart2DCommon::NumberTextureID)_18), 0);
            _50->show();
            _54->hide();
        }
        else {
            _54->show();
            _50->hide();
        }
    }
}

LANSelectMode::SelectableItemSw::SelectableItemSw(u32 tagPrefix, J2DScreen *screen, void *bck, void *btk, void *brk, ResTIMG **p6, ResTIMG **p7, int p8, ResTIMG **p9, int p10) : SelectableItem1(tagPrefix, screen, bck, btk, brk, p9, p10) {
    // perhaps swap p6 and p7 with p9 and p10
    _58 = cValueMode_0;
    _5c = 0;
    _60 = p8;
    _64 = 0;
    _68 = p6;
    _6c = p7;
}

void LANSelectMode::SelectableItemSw::changeValueMode(LANSelectMode::SelectableItemSw::ValueMode mode) {
    if (_58 == mode) return;

    std::swap(_18, _5c);
    std::swap(_1c, _60);
    std::swap(_20, _64);
    std::swap(_54, _6c);

    _50->changeTexture(_54[_18], 0);
    _c->changeTexture(_68[mode], 0);

    _58 = mode;
}

void LANSelectMode::SelectableItemSw::setMaskSw(u32 mask) {
    _64 = mask;
    while (_64 & 1 << _5c) {
        if (--_5c < 0) {
            _5c = _60 - 1;
        }
    }
}

LANSelectMode::SelectableItemDisp::SelectableItemDisp(u32 tagPrefix, J2DScreen *screen, void *bck, void *btk, void *brk) : SelectableItem(tagPrefix, screen, bck, btk, brk, 3, 0, 0) {
    J2DPane *pPane;
    pPane = screen->search(0x737730ull + ((u64)tagPrefix << 24));
    pPane->setAnimation(_28);

#line 571
    pPane = screen->search(0x737731ull + ((u64)tagPrefix << 24));
    JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);

    _58 = (J2DPicture*)pPane;
    _58->setAnimation(_30);

    _5c = (J2DAnmTevRegKey*)J2DAnmLoaderDataBase::load(brk);
    _5c->searchUpdateMaterialID(screen);

    _60 = 0;

#line 585
    pPane = screen->search(0x6c31ull + ((u64)tagPrefix << 16));
    JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);

    _50 = (J2DPicture*)pPane;
    _50->setAnimation(_5c);
    _50->hide();

#line 592
    pPane = screen->search(0x6c32ull + ((u64)tagPrefix << 16));
    JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);

    _54 = (J2DPicture*)pPane;
    _54->setAnimation(_5c);
    _54->hide();
    
    pPane = screen->search(0x737730ull + ((u64)tagPrefix << 24));
    pPane->setAnimation(_30);
}

void LANSelectMode::SelectableItemDisp::changeColor(LANSelectMode::Color color) {
    Item::changeColor(color);

    _58->setWhite(spaColorPicture[color]->getWhite());
    _58->setBlack(spaColorPicture[color]->getBlack());

    _50->setWhite(spaColorPicture[color]->getWhite());
    _50->setBlack(spaColorPicture[color]->getBlack());

    _54->setWhite(spaColorPicture[color]->getWhite());
    _54->setBlack(spaColorPicture[color]->getBlack());

    if (color == cLanColor_2) {
        _50->setAnimation(_5c);
        _54->setAnimation(_5c);
    }
    else {
        _50->setAnimation((J2DAnmTevRegKey*)nullptr);
        _54->setAnimation((J2DAnmTevRegKey*)nullptr);
    }
}

void LANSelectMode::SelectableItemDisp::calc() {
    SelectableItem::calc();

    if (_24 == 1 && _44 == 94) {
        switch (_18) {
        case 0: {
            _50->hide();
            _54->hide();
            break;
        }
        case 1: {
            _50->show();
            _54->hide();
            break;
        }
        case 2: {
            _50->show();
            _54->show();
            break;
        }
        }
    }
    
    if (_4 == 2) {
        _5c->setFrame(_60);
        if (++_60 >= 120) {
            _60 = 0;
        }
    }
}

const char *cpaGameModeTex[] = {
    "Entry_Versus",
    "Mozi_Battle1",
    "Mozi_Battle2"
};

const char *cpaCrsOrderTex[] = {
    "Mozi_Random01",
    "Mozi_OneCourse01",
    "Mozi_AllCourse01",
    "CupName_MUSHROOM_CUP",
    "CupName_FLOWER_CUP",
    "CupName_STAR_CUP",
    "CupName_SPECIAL_CUP"
};

const char *cpaStageOrderTex[] = {
    "Mozi_OneStage01",
    "Mozi_AllStage01"
};

const char *cpaLabelNameTex[] = {
    "Course",
    "Stage"
};

const char *cpaTagTex[] = {
    "Off", "On"
};

const char *cpaCrsNameTex[] = {
    "LUIGI_CIRCUIT",
    "PEACH_BEACH",
    "BABY_PARK",
    "KARA_KARA_DESERT",
    "KINOKO_BRIDGE",
    "MARIO_CIRCUIT",
    "DAISY_SHIP",
    "WALUIGI_STADIUM",
    "SHERBET_LAND",
    "KINOKO_CITY",
    "YOSHI_CIRCUIT",
    "DK_MOUNTAIN",
    "WARIO_COLOSSEUM",
    "DINO_DINO_JUNGLE",
    "BOWSER_CASTLE",
    "RAINBOW_ROAD",

};

LANSelectMode::LANSelectMode(JKRArchive *pArchive) : mBackground(pArchive) {
    mState = 1;
    _250 = 0;
    _254 = 0;
    mScreen.setPriority("LANSelectMode.blo", 0x20000, pArchive);

    void *bck = JKRGetNameResource("LANSelectMode.bck", pArchive);
    void *btk = JKRGetNameResource("LANSelectMode.btk", pArchive);
    void *brk = JKRGetNameResource("LANSelectMode.brk", pArchive);

    _248 = J2DAnmLoaderDataBase::load(bck);
    _24c = 0;

    char buf[0x40];
    for (int  i = 0; i < 3; i++) {
        snprintf(buf, sizeof(buf), "timg/%s.bti", cpaGameModeTex[i]);
        _298[i] = (ResTIMG *)pArchive->getResource(buf);
    }

    for (int  i = 0; i < 4; i++) {
        snprintf(buf, sizeof(buf), "timg/Power_Mozi.%d.bti", i + 1);
        _2a4[i] = (ResTIMG*)pArchive->getResource(buf);
    }

    for (int  i = 0; i < 2; i++) {
        snprintf(buf, sizeof(buf), "timg/Mozi_Order%s01.bti", cpaLabelNameTex[i]);
        _2b4[i] = (ResTIMG*)pArchive->getResource(buf);
    }

    for (int  i = 0; i < 7; i++) {
        snprintf(buf, sizeof(buf), "timg/%s.bti", cpaCrsOrderTex[i]);
        _2bc[i] = (ResTIMG*)pArchive->getResource(buf);
    }

    _2d8 = _2bc[0];

    snprintf(buf, sizeof(buf), "timg/%s.bti", cpaStageOrderTex[0]);
    _2dc = (ResTIMG*)pArchive->getResource(buf);

    snprintf(buf, sizeof(buf), "timg/%s.bti", cpaStageOrderTex[1]);
    _2e0 = (ResTIMG*)pArchive->getResource(buf);

    for (int  i = 0; i < 2; i++) {
        snprintf(buf, sizeof(buf), "timg/Mozi_%sSelect01.bti", cpaLabelNameTex[i]);
        _2e4[i] = (ResTIMG*)pArchive->getResource(buf);
    }

    for (int i = 0; i < 0x10; i++) {
        snprintf(buf, sizeof(buf), "timg/CoName_%s.bti", cpaCrsNameTex[i]);
        _2ec[i] = (ResTIMG*)pArchive->getResource(buf);
    }

    for (int i = 0; i < 6; i++) {
        snprintf(buf, sizeof(buf), "timg/Mozi_Map%d.bti", i + 1);
        _32c[i] = (ResTIMG*)pArchive->getResource(buf);
    }

    std::swap(_32c[5], _32c[4]);

    for (int i = 0; i < 2; i++) {
        snprintf(buf, sizeof(buf), "timg/Mozi_%s.bti", cpaTagTex[i]);
        _344[i] = (ResTIMG*)pArchive->getResource(buf);
    }

    for (int i = 0; i < 3; i++) {
#line 820
        J2DPane *pPane = mScreen.search(0x436f6c30ull + i);
        JUT_ASSERT(pPane->getTypeID() == J2DPaneType_Picture);
        spaColorPicture[i] = (J2DPicture*)pPane; 
    }

    _258 = new SelectableItem1('GMM', &mScreen, bck, btk, brk, _298, 3);
    _25c = new SelectableItem1('POW', &mScreen, bck, btk, brk, _2a4, 4);
    _260 = new SelectableItem2('LAP', &mScreen, bck, btk, brk);
    _264 = new SelectableItem1('TOU', &mScreen, bck, btk, brk, _2bc, 7);
    _268 = new SelectableItemSw('COU', &mScreen, bck, btk, brk, _2e4, _32c, 6, _2ec, 16);
    _26c = new SelectableItem1('TAG', &mScreen, bck, btk, brk, _344, 2);
    _270 = new SelectableItemDisp('DIS', &mScreen, bck, btk, brk);

    mItem = new Item('OKK', &mScreen, bck);
    _278[0] = _258;
    _278[1] = _25c;
    _278[2] = _260;
    _278[3] = _264;
    _278[4] = _268;
    _278[5] = _26c;
    _278[6] = _270;
    _278[7] = mItem;

    _278[0]->changeColor(cLanColor_2);
    _278[1]->changeColor(cLanColor_1);
    _278[2]->changeColor(cLanColor_1);
    _278[3]->changeColor(cLanColor_1);
    _278[4]->changeColor(cLanColor_0);
    _278[5]->changeColor(cLanColor_1);
    _278[6]->changeColor(cLanColor_1);
    _278[7]->changeColor(cLanColor_1);
    gLANPlayInfo.resumeInfo(this);

    NetGameMgr *netMgr = NetGameMgr::ptr();
    if (!(netMgr->get12ec() & 4)) {
        _25c->setMask(8);
    }
    if (!(netMgr->get12ec() & 8)) {
        _264->setMask(0x40);
    }

    u32 mask1 = 0;
    if (!(netMgr->get12ec() & 0x8)) {
        mask1 |= 0xf000;
    }

    u32 mask2 = 0;
    if (!(netMgr->get12ec() & 0x80)) {
        mask2 |= 0x10;
    }
    if (!(netMgr->get12ec() & 0x100)) {
        mask2 |= 0x20;
    }

    if (_268->getValueMode() == SelectableItemSw::cValueMode_0) {
        _268->setMask(mask1);
        _268->setMaskSw(mask2);
    }
    else {
        _268->setMask(mask2);
        _268->setMaskSw(mask1);
    }

    for (int i = 0; i < 7; i++) {
        _278[i]->update();
        directView(i);
    }

    _254 |= 0x10;
    
    _258->selectOut();
}

LANSelectMode::Background::~Background() {}

void LANSelectMode::directView(int i) {
    switch (i) {
    case 0: {
        switch (_258->get_18()) {
        case 0:
        {
            _25c->changeColor(cLanColor_1);
            _254 &= ~2;
            _260->changeColor(cLanColor_1);
            _254 &= ~4;
            u32 mask = 0;
            if (!(NetGameMgr::ptr()->get12ec() & 8)) {
                mask |= 0x40;
            }
            _264->changeTex(_2b4[0]);
            _264->changeImg(_2bc);



            _264->setMask(mask);
            _268->changeValueMode(SelectableItemSw::cValueMode_0);
            break;
        }
        case 1:
        case 2: {
            _25c->changeColor(cLanColor_0);
            _254 |= 2;
            _260->changeColor(cLanColor_0);
            _254 |= 4;

            _264->changeTex(_2b4[1]);
            _264->changeImg(&_2d8);

            _264->setMask(0x78);
            _268->changeValueMode(SelectableItemSw::cValueMode_1);
            break;
        }
        }
        break;
    }
    case 3: {
        switch (_264->get_18()) {
        
        case 1: {
            _268->changeColor(cLanColor_1);
            _254 &= ~0x10;
            break;
        }
        case 0:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6: {
            _268->changeColor(cLanColor_0);
            _254 |= 0x10;
            break;
        }
        default: {
            break;
        }
        }
        break;
    }
    case 5: {
        setDisplayMax();
        break;
    }
    }
}

int LANSelectMode::calc() {
    KartGamePad *pad = gpaKartPad[0];
    int ret = 0;

    switch (mState) {
    case 2: {
        if (pad->testRepeat(KartGamePad::MAINSTICK_UP)) {
            GetGameAudioMain()->startSystemSe(0x20000);
            _278[_250]->selectIn();
            do {
                if (--_250 < 0) {
                    _250 = 7;
                }
            } while(_254 & 1 << _250);

            _278[_250]->selectOut();
        }
        else if (pad->testRepeat(KartGamePad::MAINSTICK_DOWN)) {
            GetGameAudioMain()->startSystemSe(0x20000);
            _278[_250]->selectIn();
            do {
                if (++_250 >= 8) {
                    _250 = 0;
                }
            } while(_254 & 1 << _250);

            _278[_250]->selectOut();
        }
        else if (pad->testRepeat(KartGamePad::MAINSTICK_LEFT)) {
            if (_278[_250]->changeValueDown()) {
                GetGameAudioMain()->startSystemSe(0x2000c);
                directView(_250);
            }
        }
        else if (pad->testRepeat(KartGamePad::MAINSTICK_RIGHT)) {
            if (_278[_250]->changeValueUp()) {
                directView(_250);
                GetGameAudioMain()->startSystemSe(0x2000c);
            }
        }

        if (pad->testTrigger(KartGamePad::A)) {
            if (_250 == 7) {
                GetGameAudioMain()->startSystemSe(0x20003);
                mState = 3;
                _24c = 120;
                _5 = true;
                MenuTitleLine::ptr()->lift();
                SYSTEM_StartFadeOut(15);
                GetGameAudioMain()->fadeOutAll(15);
            }
        }
        else if (pad->testTrigger(KartGamePad::B)) {
            if (_4) {
                GetGameAudioMain()->startSystemSe(0x20004);
                mState = 3;
                _24c = 120;
                _5 = false;
                MenuTitleLine::ptr()->lift();
                SYSTEM_StartFadeOut(15);
                GetGameAudioMain()->fadeOutAll(15);
            }
        }
        break;
    }
    case 4: {
        if (SYSTEM_IsFadingOut()) {
            if (_5) {
                ret = 1;
                gLANPlayInfo.saveInfo(this);
            }
            else {
                ret = 2;
            }
        }
        break;
    }
    }

    calcAnm();
    MenuTitleLine::ptr()->calc();
    return ret;
}

void LANSelectMode::calcAnm() {
    mBackground.calc();

    for (int i = 0; i < 8; i++) {
        _278[i]->calc();
    }

    _248->setFrame(_24c);
    switch(mState) {
    case 1: {
        if (++_24c >= 24) {
            mState = 2;
        }
        break;
    }
    case 3: {
        if (++_24c >= 131) {
            mState = 4;
        }
        break;
    }
    }

    mScreen.animation();
}

void LANSelectMode::draw(const J2DGrafContext *graf) {
    mBackground.draw(graf);
    mScreen.draw(0.0f, 0.0f, graf);
    MenuTitleLine::ptr()->draw(graf);
}

void LANSelectMode::start(bool p1) {
    _4 = p1;

    if (_4) {
        GetGameAudioMain()->startSequenceBgm(0x1000002);
    }

    if (_250 != 0) {
        _278[_250]->unselect();
        _250 = 0;
    }
    _278[_250]->selectOut();
    mState = 1;
    _24c = 0;
    gLANPlayInfo.resumeInfo(this);
    for (int i = 0; i < 7; i++) {
        _278[i]->update();
        directView(i);
    }

    MenuTitleLine::ptr()->drop(MenuTitleLine::mcSelectMode);
    SYSTEM_StartFadeIn(15);
}

void LANSelectMode::setDisplayMax() {
    if (NetGameMgr::ptr()->getNetworkCubes() > 4) {
        _270->setMax(1);
    }
    else if (NetGameMgr::ptr()->getNetworkCubes() > 2 || _26c->get_18() == 1) {
        _270->setMax(2);
    }
    else {
        _270->setMax(3);
    }
}

#include "JSystem/JAudio/JASFakeMatch2.h"
