/* Scroll
 *
 * Copyright (c) 2008- Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#ifndef _SCROLLED_H_
#define _SCROLLED_H_


//! Includes
#include "ScrollBase.h"


//! ScrollEd class
class ScrollEd
{
public:

  static        ScrollEd &      GetInstance();

                void            Execute(int argc, char **argv);

                orxSTATUS       SetMapName(const orxSTRING _zMapName);
          const orxSTRING       GetMapName() const;


protected:


private:

                                ScrollEd();
                               ~ScrollEd();

                orxSTATUS       Init();
                orxSTATUS       Run();
                void            Exit();
                void            Update(const orxCLOCK_INFO &_rstInfo);
                orxBOOL         MapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption);
                orxSTATUS       SaveMap() const;
                orxSTATUS       SaveMapBackup() const;

  static  orxSTATUS orxFASTCALL StaticProcessParams(orxU32 _u32ParamCount, const orxSTRING _azParams[]);
  static  orxSTATUS orxFASTCALL StaticInit();
  static  orxSTATUS orxFASTCALL StaticRun();
  static  void      orxFASTCALL StaticExit();
  static  void      orxFASTCALL StaticUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext);
  static  orxSTATUS orxFASTCALL StaticEventHandler(const orxEVENT *_pstEvent);
  static  orxBOOL   orxFASTCALL StaticMapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption);

                orxSTATUS       InitSets();
                void            ExitSets();

                void            ShowPreviousSet();
                void            ShowNextSet();
                void            HideSet();
          const ScrollObject *  GetSetModel(const orxVECTOR &_rvPosition) const;

                orxSTATUS       InitUI();
                void            ExitUI();
                void            UpdateUI(const orxCLOCK_INFO &_rstInfo);

                void            UpdateButtons(const orxCLOCK_INFO &_rstInfo);

                void            UpdateLayerDisplay();

                ScrollObject   *CreateModel(const orxSTRING _zModelName, const orxSTRING _zInstanceName = orxNULL, ScrollObject::Flag _xFlags = ScrollObject::FlagNone);

                void            CreateCursorObject(const ScrollObject *_poModel = orxNULL);
                void            DeleteCursorObject();
                void            UpdateCursor();

                void            ShowBasicCursor(orxBOOL _bShow = orxTRUE);

                void            SetPropertyText(const ScrollObject *_poObject = orxNULL, const orxSTRING _zText = orxNULL, const orxCOLOR *_pstColor = orxNULL);
                void            DisplayObjectName(const ScrollObject *_poObject = orxNULL, orxBOOL _bDisplayInstanceName = orxFALSE);
                void            DisplayObjectColor(const ScrollObject *_poObject = orxNULL);
                void            DisplayObjectAlpha(const ScrollObject *_poObject = orxNULL);
                void            DisplayObjectScale(const ScrollObject *_poObject = orxNULL);
                void            DisplayObjectRotation(const ScrollObject *_poObject = orxNULL);
                void            DisplayObjectPosition(const ScrollObject *_poObject = orxNULL);

                orxSTATUS       AddActionDisplay(const orxSTRING _zAction) const;

                void            SetSelection(ScrollObject *_poObject = orxNULL);
                orxOBJECT *     PickObject(const orxVECTOR &_rvPos, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;
                ScrollObject *  AddObject(const ScrollObject *_poModel = orxNULL);
                void            RemoveObject(ScrollObject *_poObject);

                orxFLOAT        GetDepthCoef(orxFLOAT _fDepth) const;
                orxVECTOR &     GetDepthCorrectedPosition(orxFLOAT _fDepth, orxVECTOR &_rvPos) const;
                orxVECTOR &     GetDepthCorrectedScale(orxFLOAT _fDepth, orxVECTOR &_rvScale) const;


  enum Flag
  {
    FlagNone            = 0x00000000,

    FlagInGame          = 0x00000001,
    FlagSnap            = 0x00000002,
    FlagUsingCopy       = 0x00000004,
    FlagShowBasicCursor = 0x00000008,

    FlagToggleMove      = 0x00000010,
    FlagToggleScale     = 0x00000020,
    FlagToggleRotate    = 0x00000040,

    MaskToggle          = 0x00000070,

    FlagSetShown    = 0x00000100,

    MaskAll             = 0xFFFFFFFF
  };

                void            UpdateToggle(const orxSTRING _zToggleName, Flag _xToggleFlag);
                void            SetFlags(Flag _xAddFlags, Flag _xRemoveFlags = FlagNone)  {mxFlags = (Flag)(mxFlags & ~_xRemoveFlags); mxFlags = (Flag)(mxFlags | _xAddFlags);}
                void            SwapFlags(Flag _xSwapFlags)                               {mxFlags = (Flag)(mxFlags ^ _xSwapFlags);}
                orxBOOL         TestFlags(Flag _xTestFlags) const                         {return (mxFlags & _xTestFlags) ? orxTRUE : orxFALSE;}
                orxBOOL         TestAllFlags(Flag _xTestFlags) const                      {return ((mxFlags & _xTestFlags) == _xTestFlags) ? orxTRUE : orxFALSE;}
                Flag            GetFlags(Flag _xMask = MaskAll) const                     {return (Flag)(mxFlags & _xMask);}


  struct ButtonData
  {
    orxLINKLIST_NODE  mstNode;
    orxOBJECT        *mpstObject;
    const orxSTRING   mzName;
    const orxSTRING   mzActive;
    const orxSTRING   mzBase;
    const orxSTRING   mzHighlight;
    const orxSTRING   mzAction;
  };

                ButtonData *    GetActiveButton(const orxSTRING _zAction) const;


  static  const orxSTRING       szMapShortParam;
  static  const orxSTRING       szMapLongParam;
  static  const orxSTRING       szMapShortDesc;
  static  const orxSTRING       szMapLongDesc;

  static  const orxSTRING       szInputResetZoom;
  static  const orxSTRING       szInputAction;
  static  const orxSTRING       szInputClearSelection;
  static  const orxSTRING       szInputSoftClearSelection;
  static  const orxSTRING       szInputCopy;
  static  const orxSTRING       szInputZoom;
  static  const orxSTRING       szInputPan;
  static  const orxSTRING       szInputDelete;
  static  const orxSTRING       szInputSave;
  static  const orxSTRING       szInputReload;
  static  const orxSTRING       szInputNextSet;
  static  const orxSTRING       szInputPreviousSet;
  static  const orxSTRING       szInputHideSet;
  static  const orxSTRING       szInputPreviousLayer;
  static  const orxSTRING       szInputNextLayer;
  static  const orxSTRING       szInputAntialiasing;
  static  const orxSTRING       szInputTiling;
  static  const orxSTRING       szInputDifferential;
  static  const orxSTRING       szInputStartGame;
  static  const orxSTRING       szInputStopGame;
  static  const orxSTRING       szInputScreenshot;
  static  const orxSTRING       szInputToggleMove;
  static  const orxSTRING       szInputToggleScale;
  static  const orxSTRING       szInputToggleRotate;
  static  const orxSTRING       szInputToggleGrid;
  static  const orxSTRING       szInputToggleSnap;
  static  const orxSTRING       szInputEditAlpha;
  static  const orxSTRING       szInputEditRed;
  static  const orxSTRING       szInputEditGreen;
  static  const orxSTRING       szInputEditBlue;
  static  const orxSTRING       szInputToggleFullScreen;
  static  const orxSTRING       szInputReloadHistory;
  static  const orxSTRING       szInputSetEditor;

  static  const orxSTRING       szConfigFile;
  static  const orxSTRING       szConfigSectionEditor;
  static  const orxSTRING       szConfigScrollEdSet;
  static  const orxSTRING       szConfigSetContainer;
  static  const orxSTRING       szConfigSetContainerLine;
  static  const orxSTRING       szConfigSetContainerSize;
  static  const orxSTRING       szConfigSetContainerShow;
  static  const orxSTRING       szConfigSetContainerHide;
  static  const orxSTRING       szConfigEncryptSave;
  static  const orxSTRING       szConfigZoom;
  static  const orxSTRING       szConfigZoomLerp;
  static  const orxSTRING       szConfigZoomStep;
  static  const orxSTRING       szConfigZoomMin;
  static  const orxSTRING       szConfigZoomMax;
  static  const orxSTRING       szConfigSelectionShader;
  static  const orxSTRING       szConfigGainFocusFX;
  static  const orxSTRING       szConfigLoseFocusFX;
  static  const orxSTRING       szConfigRotationSnap;
  static  const orxSTRING       szConfigGridSize;
  static  const orxSTRING       szConfigNameText;
  static  const orxSTRING       szConfigLayerText;
  static  const orxSTRING       szConfigPropertyText;
  static  const orxSTRING       szConfigActionText;
  static  const orxSTRING       szConfigLayerTextFormat;
  static  const orxSTRING       szConfigColorTextFormat;
  static  const orxSTRING       szConfigAlphaTextFormat;
  static  const orxSTRING       szConfigScaleTextFormat;
  static  const orxSTRING       szConfigRotationTextFormat;
  static  const orxSTRING       szConfigPositionTextFormat;
  static  const orxSTRING       szConfigLayerNumber;
  static  const orxSTRING       szConfigLayerIndex;
  static  const orxSTRING       szConfigDefaultCursor;
  static  const orxSTRING       szConfigMoveCursor;
  static  const orxSTRING       szConfigScaleCursor;
  static  const orxSTRING       szConfigRotateCursor;
  static  const orxSTRING       szConfigGrid;
  static  const orxSTRING       szAutoSaveInterval;
  static  const orxSTRING       szLocaleBackup;
  static  const orxSTRING       szConfigCursor;
  static  const orxSTRING       szConfigButtonList;
  static  const orxSTRING       szConfigButtonBase;
  static  const orxSTRING       szConfigButtonHighlight;
  static  const orxSTRING       szConfigButtonActive;
  static  const orxSTRING       szConfigButtonAction;
  static  const orxSTRING       szConfigButtonCursor;
  static  const orxSTRING       szConfigShaderCameraPos;
  static  const orxSTRING       szConfigShaderCameraZoom;
  static  const orxSTRING       szConfigShaderColor;
  static  const orxSTRING       szConfigShaderTime;


//! Variables
private:

                orxVECTOR       mvWorkBase;
                orxVECTOR       mvWorkPosition;
                orxVECTOR       mvWorkScale;
                orxVECTOR       mvWorkSize;
                orxLINKLIST     mstButtonList;
                orxBANK *       mpstButtonBank;
                orxBANK *       mpstSetBank;
                orxU32          mu32SelectedSet;
                orxOBJECT *     mpstGrid;
                orxOBJECT *     mpstLayerText;
                orxOBJECT *     mpstNameText;
                orxOBJECT *     mpstPropertyText;
                orxOBJECT *     mpstBasicCursor;
          const ScrollObject *  mpoSelectedModel;
                ScrollObject *  mpoCursorObject;
                ScrollObject *  mpoSelection;
                ButtonData *    mpstButtonHighlight;
                orxSTRING       mzMapName;
                orxFLOAT        mfLocalTime;
                orxFLOAT        mfAutoSaveInterval;
                orxFLOAT        mfAutoSaveTimeStamp;
                orxFLOAT        mfZoom;
                orxFLOAT        mfWorkRotation;
                orxU32          mu32LayerIndex;
                orxU32          mu32CursorObjectIndex;
                Flag            mxFlags;

  static        ScrollEd *      spoInstance;
};

#ifdef __SCROLL_IMPL__

//! Inline include
#include "ScrollEd.inl"

#endif // __SCROLL_IMPL__

#endif // _SCROLLED_H_
