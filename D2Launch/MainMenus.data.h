/**
 * Data defined in D2Launch/MainMenus.cpp that other translation units name.
 * Binary: Game.exe
 */

#pragma once

#include "d2_platform.h"
#include "D2Win/D2WinAnimImage.h"

// Forward declarations
struct D2WinTextBox;
struct DC6;
struct FORMS_AnimFrameStrc;

extern char gpCutscenesBlackBarTexture[8];
extern int32_t gaMenuFormFontParamsFont1[2];
extern int32_t gaMenuFormFontParamsFont0Ex4[2];
extern int32_t gaMenuFormFontParamsFont4Ex1[2];
extern int32_t gaMenuFormFontParamsFont5[2];
extern int32_t gaMenuFormFontParamsFont4[2];
extern int32_t gaMenuFormFontParamsFont8[2];
extern int32_t gaMenuTextBoxScrollbarGeom0[3];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesAssassin[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesDruid[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesAmazon[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesNecromancer[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesBarbarian[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesSorceress[5];
extern FORMS_AnimFrameStrc gaCharSelectAnimFramesPaladin[5];
extern FORMS_AnimFrameStrc gaMainMenuLogoLeftAnimFrames[5];
extern FORMS_AnimFrameStrc gaMainMenuLogoRightAnimFrames[5];
extern int32_t gaMenuTextBoxScrollbarGeom1[3];
extern int32_t gaMenuTextBoxScrollbarGeom2[3];
extern int32_t gaMenuTextBoxScrollbarGeom3[3];
extern int32_t gaMenuTextBoxScrollbarGeom4[3];
extern int32_t gaMenuTextBoxScrollbarGeom5[3];
extern char gszPasswordChangeNewPasswordMB[24];
extern uint32_t gnBNetBusyFlag;
extern DC6 * DC6_FrontEnd_blizno;
extern DC6 * DC6_FrontEnd_trademark;
extern DC6 * DC6_FrontEnd_Diablo2;
extern DC6 * DC6_FrontEnd_D2logoBlackLeft;
extern DC6 * DC6_FrontEnd_D2logoFireLeft;
extern DC6 * DC6_FrontEnd_D2logoBlackRight;
extern DC6 * DC6_FrontEnd_D2logoFireRight;
extern DC6 * DC6_FrontEnd_TitleScreen_or_gameselectscreenEXP;
extern DC6 * DC6_CharSelect_creditsbckg;
extern DC6 * DC6_FrontEnd_CharacterCreate_or_charactercreationscreenEXP;
extern DC6 * DC6_FrontEnd_fire;
extern DC6 * DC6_CharSelect_TallButtonBlank;
extern DC6 * DC6_CharSelect__characterselectscreenEXP;
extern DC6 * DC6_FrontEnd_WideButtonBlank;
extern DC6 * DC6_FrontEnd_MediumButtonBlank;
extern DC6 * DC6_CharSelect_ShortButtonBlank;
extern DC6 * DC6_FrontEnd_MediumSelButtonBlank;
extern DC6 * DC6_FrontEnd_textbox2;
extern DC6 * DC6_FrontEnd_3WideButtonBlank;
extern DC6 * DC6_FrontEnd_EditTextBoxWide;
extern DC6 * DC6_FrontEnd_WideButtonBlank02;
extern DC6 * DC6_FrontEnd_NarrowButtonBlank;
extern DC6 * DC6_FrontEnd_GWListBox;
extern DC6 * DC6_FrontEnd_TermsofUseBckg;
extern DC6 * DC6_CharSelect_charselectbckg;
extern DC6 * DC6_CharSelect_DifficultyLevels;
extern DC6 * DC6_FrontEnd_CinematicsSelection;
extern DC6 * DC6_FrontEnd_CinematicsSelectionEXP;
extern DC6 * DC6_CharSelect_charselectbox;
extern DC6 * DC6_CharSelect_charselectboxgrey;
extern DC6 * DC6_FrontEnd_PopUpOKCancel;
extern DC6 * DC6_FrontEnd_PopUpOKCancel2;
extern DC6 * DC6_CharSelect_selectchararrows;
extern DC6 * DC6_FrontEnd_CurRealm_or_Realmselect;
extern DC6 * DC6_FrontEnd_NarrowButtonBlank02_or_RealmSelectbuttonthin;
extern DC6 * DC6_CharSelect_CreateNewCharImage;
extern DC6 * DC6_FrontEnd_PopUpOK;
extern DC6 * DC6_FrontEnd_PopUp_340x224;
extern DC6 * DC6_FrontEnd_PopUpWide;
extern DC6 * DC6_FrontEnd_PopUpLargest;
extern DC6 * DC6_FrontEnd_TileableDialog;
extern DC6 * DC6_FrontEnd_PopUpOK2;
extern DC6 * DC6_FrontEnd_OkCancelButtonBlank;
extern DC6 * DC6_FrontEnd_CancelButtonBlank;
extern DC6 * DC6_FrontEnd_textbox;
extern DC6 * DC6_FrontEnd_clickbox;
extern DC6 * DC6_FrontEnd_joingameclickboxgrey;
extern DC6 * DC6_FrontEnd_joingamescrollbars;
extern DC6 * DC6_FrontEnd_tcpipbckg_or_tcpipscreen;
extern DC6 * DC6_FrontEnd_IPAddressBox;
extern DC6 * DC6_FrontEnd_downloadpatchbckg;
extern DC6 * DC6_FrontEnd_downloadpatchball2;
// Character-create class preview DC6s. Seven contiguous 5-element runs follow, one per class, in
// element order [0]=nu1 [1]=nu2 [2]=fw [3]=nu3 [4]=bw (see the load sequence in IMAGE_LoadPredefinedImagesEx).
// Each run is freed by a strided loop bounded at head+20 in IMAGE_FreePredefinedImages, so each run is
// one array, not five independent globals. Three of the old element labels named the wrong class
// (gpImageSorceressWeapon was barbarian\banu2, gpImagePaladinWeapon was sorceress\sonu2,
// gpImageNecromancerWeapon was paladin\panu2).
extern DC6 * gaDC6CharCreateAssassin[5];
extern DC6 * gaDC6CharCreateDruid[5];
extern DC6 * gaDC6CharCreateAmazon[5];
extern DC6 * gaDC6CharCreateNecromancer[5];
extern DC6 * gaDC6CharCreateBarbarian[5];
extern DC6 * gaDC6CharCreateSorceress[5];
extern DC6 * gaDC6CharCreatePaladin[5];
extern DC6 * gaDC6CharCreateNecromancerSel[3];
extern DC6 * gaDC6CharCreateSorceressSel[3];
extern DC6 * DC6_FrontEnd_barbarian_bafws;
extern DC6 * DC6_FrontEnd_paladin_pafws;
extern uint32_t gbCinematicsMenuShown;
extern uint32_t fpInsertPlayCd;
extern uint32_t gbCdCheckPassed[25];
extern ::D2WinTextBox * gpCtlBnetProgressBar;
extern ::D2WinTextBox * gbBnetConnectionTimerActive;
