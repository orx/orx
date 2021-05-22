/* Scroll
 *
 * Copyright (c) 2008-2021 Orx-Project
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

//! Constants

// Param
const orxSTRING ScrollEd::szMapShortParam           = "m";
const orxSTRING ScrollEd::szMapLongParam            = "map";
const orxSTRING ScrollEd::szMapShortDesc            = "Works on the specified map.";
const orxSTRING ScrollEd::szMapLongDesc             = "Loads the specified map and allows to work on it, add/remove tiles and save the map.";

// Input
const orxSTRING ScrollEd::szInputResetZoom          = "ResetZoom";
const orxSTRING ScrollEd::szInputAction             = "Action";
const orxSTRING ScrollEd::szInputClearSelection     = "ClearSelection";
const orxSTRING ScrollEd::szInputSoftClearSelection = "SoftClearSelection";
const orxSTRING ScrollEd::szInputCopy               = "Copy";
const orxSTRING ScrollEd::szInputZoom               = "Zoom";
const orxSTRING ScrollEd::szInputPan                = "Pan";
const orxSTRING ScrollEd::szInputDelete             = "Delete";
const orxSTRING ScrollEd::szInputSave               = "Save";
const orxSTRING ScrollEd::szInputReload             = "Reload";
const orxSTRING ScrollEd::szInputNextSet            = "NextSet";
const orxSTRING ScrollEd::szInputPreviousSet        = "PreviousSet";
const orxSTRING ScrollEd::szInputHideSet            = "HideSet";
const orxSTRING ScrollEd::szInputPreviousLayer      = "PreviousLayer";
const orxSTRING ScrollEd::szInputNextLayer          = "NextLayer";
const orxSTRING ScrollEd::szInputAntialiasing       = "Antialiasing";
const orxSTRING ScrollEd::szInputTiling             = "Tiling";
const orxSTRING ScrollEd::szInputStartGame          = "StartGame";
const orxSTRING ScrollEd::szInputStopGame           = "StopGame";
const orxSTRING ScrollEd::szInputScreenshot         = "Screenshot";
const orxSTRING ScrollEd::szInputToggleMove         = "ToggleMove";
const orxSTRING ScrollEd::szInputToggleScale        = "ToggleScale";
const orxSTRING ScrollEd::szInputToggleRotate       = "ToggleRotate";
const orxSTRING ScrollEd::szInputToggleGrid         = "ToggleGrid";
const orxSTRING ScrollEd::szInputDifferential       = "ToggleDifferential";
const orxSTRING ScrollEd::szInputToggleSnap         = "ToggleSnap";
const orxSTRING ScrollEd::szInputEditAlpha          = "EditAlpha";
const orxSTRING ScrollEd::szInputEditRed            = "EditRed";
const orxSTRING ScrollEd::szInputEditGreen          = "EditGreen";
const orxSTRING ScrollEd::szInputEditBlue           = "EditBlue";
const orxSTRING ScrollEd::szInputToggleFullScreen   = "ToggleFullScreen";
const orxSTRING ScrollEd::szInputReloadHistory      = "ReloadHistory";

const orxSTRING ScrollEd::szInputSetEditor          = "ScrollEdInput";

// Config
const orxSTRING ScrollEd::szConfigFile              = "scrolled/scrolled.ini";
const orxSTRING ScrollEd::szConfigSectionEditor     = "ScrollEd";
const orxSTRING ScrollEd::szConfigScrollEdSet       = "ScrollEdSet";
const orxSTRING ScrollEd::szConfigSetContainer      = "SetContainer";
const orxSTRING ScrollEd::szConfigSetContainerLine  = "SetContainerLine";
const orxSTRING ScrollEd::szConfigSetContainerSize  = "SetContainerSize";
const orxSTRING ScrollEd::szConfigSetContainerShow  = "SetContainerShow";
const orxSTRING ScrollEd::szConfigSetContainerHide  = "SetContainerHide";
const orxSTRING ScrollEd::szConfigEncryptSave       = "EncryptSave";
const orxSTRING ScrollEd::szConfigZoom              = "Zoom";
const orxSTRING ScrollEd::szConfigZoomLerp          = "ZoomLerp";
const orxSTRING ScrollEd::szConfigZoomStep          = "ZoomStep";
const orxSTRING ScrollEd::szConfigZoomMin           = "ZoomMin";
const orxSTRING ScrollEd::szConfigZoomMax           = "ZoomMax";
const orxSTRING ScrollEd::szConfigSelectionShader   = "SelectionShader";
const orxSTRING ScrollEd::szConfigGainFocusFX       = "GainFocusFX";
const orxSTRING ScrollEd::szConfigLoseFocusFX       = "LoseFocusFX";
const orxSTRING ScrollEd::szConfigRotationSnap      = "RotationSnap";
const orxSTRING ScrollEd::szConfigGridSize          = "GridSize";
const orxSTRING ScrollEd::szConfigNameText          = "NameText";
const orxSTRING ScrollEd::szConfigLayerText         = "LayerText";
const orxSTRING ScrollEd::szConfigPropertyText      = "PropertyText";
const orxSTRING ScrollEd::szConfigActionText        = "ActionText";
const orxSTRING ScrollEd::szConfigLayerTextFormat   = "LayerTextFormat";
const orxSTRING ScrollEd::szConfigColorTextFormat   = "ColorTextFormat";
const orxSTRING ScrollEd::szConfigAlphaTextFormat   = "AlphaTextFormat";
const orxSTRING ScrollEd::szConfigScaleTextFormat   = "ScaleTextFormat";
const orxSTRING ScrollEd::szConfigRotationTextFormat= "RotationTextFormat";
const orxSTRING ScrollEd::szConfigPositionTextFormat= "PositionTextFormat";
const orxSTRING ScrollEd::szConfigLayerNumber       = "LayerNumber";
const orxSTRING ScrollEd::szConfigLayerIndex        = "LayerIndex";
const orxSTRING ScrollEd::szConfigDefaultCursor     = "DefaultCursor";
const orxSTRING ScrollEd::szConfigMoveCursor        = "MoveCursor";
const orxSTRING ScrollEd::szConfigScaleCursor       = "ScaleCursor";
const orxSTRING ScrollEd::szConfigRotateCursor      = "RotateCursor";

const orxSTRING ScrollEd::szConfigCursor            = "Cursor";

const orxSTRING ScrollEd::szConfigGrid              = "Grid";

const orxSTRING ScrollEd::szAutoSaveInterval        = "AutoSaveInterval";

const orxSTRING ScrollEd::szLocaleBackup            = "Backup";

const orxSTRING ScrollEd::szConfigButtonList        = "ButtonList";
const orxSTRING ScrollEd::szConfigButtonBase        = "Base";
const orxSTRING ScrollEd::szConfigButtonHighlight   = "Highlight";
const orxSTRING ScrollEd::szConfigButtonActive      = "Active";
const orxSTRING ScrollEd::szConfigButtonAction      = "Action";
const orxSTRING ScrollEd::szConfigButtonCursor      = "Cursor";

const orxSTRING ScrollEd::szConfigShaderCameraPos   = "camerapos";
const orxSTRING ScrollEd::szConfigShaderCameraZoom  = "camerazoom";
const orxSTRING ScrollEd::szConfigShaderColor       = "color";
const orxSTRING ScrollEd::szConfigShaderTime        = "time";


//! Static variables
ScrollEd *ScrollEd::spoInstance = orxNULL;


//! Code
class ScrollEdSet
{
public:

  orxBANK          *mpstObjectBank;
  ScrollObject     *mpoContainer;
  const orxSTRING   mzName;
};

ScrollEd &ScrollEd::GetInstance()
{
  if(!spoInstance)
  {
    spoInstance = new ScrollEd();
  }

  return *spoInstance;
}

ScrollEd::ScrollEd() : mpstButtonBank(orxNULL), mpstSetBank(orxNULL), mu32SelectedSet(0), mpstGrid(orxNULL), mpstLayerText(orxNULL), mpstNameText(orxNULL),
                       mpstPropertyText(orxNULL), mpstBasicCursor(orxNULL), mpoSelectedModel(orxNULL), mpoCursorObject(orxNULL), mpoSelection(NULL),
                       mpstButtonHighlight(NULL), mzMapName(orxNULL), mfLocalTime(orxFLOAT_0), mfAutoSaveInterval(orxFLOAT_0),
                       mfAutoSaveTimeStamp(orxFLOAT_0), mfZoom(orxFLOAT_0), mfWorkRotation(orxFLOAT_0), mu32LayerIndex(0), mu32CursorObjectIndex(0),
                       mxFlags(FlagNone)
{
  // Clears other members
  orxVector_Copy(&mvWorkBase, &orxVECTOR_0);
  orxVector_Copy(&mvWorkPosition, &orxVECTOR_0);
  orxVector_Copy(&mvWorkScale, &orxVECTOR_0);
  orxVector_Copy(&mvWorkSize, &orxVECTOR_0);
  orxMemory_Zero(&mstButtonList, sizeof(orxLINKLIST));
}

ScrollEd::~ScrollEd()
{
}

orxSTATUS ScrollEd::Init()
{
  orxSTATUS eResult;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Inits base game
  roGame.SetEditorMode();
  eResult = roGame.BaseInit();

  // Valid?
  if(eResult != orxSTATUS_FAILURE)
  {
    orxPARAM  stParams;
    orxCOLOR  stColor;

    // Loads config file
    orxConfig_Load(szConfigFile);

    // Enforces video mode
    orxDisplay_SetVideoMode(orxNULL);

    // Reloads inputs
    orxInput_Load(orxSTRING_EMPTY);

    // Has any locale language?
    if(orxLocale_GetLanguageCount())
    {
      // Selects first available
      orxLocale_SelectLanguage(orxLocale_GetLanguage(0));
    }

    // Pushes viewport section
    orxConfig_PushSection(roGame.szConfigMainViewport);

    // Has color?
    if(orxConfig_HasValue("BackgroundColor"))
    {
      // Gets it
      orxVector_Mulf(&stColor.vRGB, orxConfig_GetVector("BackgroundColor", &stColor.vRGB), orxCOLOR_NORMALIZER);

      // Updates viewport color
      orxViewport_SetBackgroundColor(roGame.GetMainViewport(), &stColor);
    }

    // Pops config section
    orxConfig_PopSection();

    // Asks for command line map parameter
    orxMemory_Zero(&stParams, sizeof(orxPARAM));
    stParams.pfnParser  = StaticProcessParams;
    stParams.u32Flags   = orxPARAM_KU32_FLAG_NONE;
    stParams.zShortName = szMapShortParam;
    stParams.zLongName  = szMapLongParam;
    stParams.zShortDesc = szMapShortDesc;
    stParams.zLongDesc  = szMapLongDesc;
    orxParam_Register(&stParams);

    // Disables physics simulation while editing
    orxPhysics_EnableSimulation(orxFALSE);

    // Pushes editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Has default layer count?
    if(orxConfig_HasValue(szConfigLayerNumber))
    {
      // Sets it
      roGame.SetLayerNumber(orxConfig_GetU32(szConfigLayerNumber));
    }

    // Gets autosave interval
    mfAutoSaveInterval = orxConfig_GetFloat(szAutoSaveInterval);

    // Pops section
    orxConfig_PopSection();

    // Can load map?
    if((roGame.SetMapName(mzMapName) != orxSTATUS_FAILURE) && (roGame.LoadMap() != orxSTATUS_FAILURE))
    {
      // Pushes editor section
      orxConfig_PushSection(szConfigSectionEditor);

      // Gets current layer
      mu32LayerIndex = orxConfig_GetU32(szConfigLayerIndex);

      // Clamps it
      if(mu32LayerIndex >= roGame.GetLayerNumber())
      {
        mu32LayerIndex = roGame.GetLayerNumber() - 1;
      }

      // Pops previous section
      orxConfig_PopSection();

      // Updates layer
      UpdateLayerDisplay();

      // Inits sets
      InitSets();
    }
    else
    {
      // Updates result
      eResult = orxSTATUS_FAILURE;
    }

    // Successful?
    if(eResult != orxSTATUS_FAILURE)
    {
      // Inits UI
      if((eResult = InitUI()) != orxSTATUS_FAILURE)
      {
        // Is map defined?
        if(GetMapName())
        {
          // Adds group to camera
          orxCamera_AddGroupID(roGame.GetMainCamera(), orxString_GetID(orxOBJECT_KZ_DEFAULT_GROUP), orxFALSE);

          // Registers update
          eResult = orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), StaticUpdate, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

          // Registers event handler
          eResult = ((eResult != orxSTATUS_FAILURE) && (orxEvent_AddHandler(orxEVENT_TYPE_SHADER, StaticEventHandler) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
          eResult = ((eResult != orxSTATUS_FAILURE) && (orxEvent_AddHandler(orxEVENT_TYPE_INPUT, StaticEventHandler) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
        }
      }
    }
  }

  // Done!
  return eResult;
}

orxSTATUS ScrollEd::Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Quiting?
  if(orxInput_IsActive("Quit"))
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }
  else
  {
    // Runs base game
    ScrollBase::GetInstance().Run();
  }

  // Done!
  return eResult;
}

void ScrollEd::Exit()
{
  // Removes event handlers
  orxEvent_RemoveHandler(orxEVENT_TYPE_INPUT, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_SHADER, StaticEventHandler);

  // Exits UI
  ExitUI();

  // Exits sets
  ExitSets();

  // Exits from base game
  ScrollBase::GetInstance().Exit();
}

void ScrollEd::Update(const orxCLOCK_INFO &_rstInfo)
{
  orxFLOAT fZoom;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Not in game?
  if(!TestFlags(FlagInGame))
  {
    // Hides object properties
    SetPropertyText();

    // Updates buttons
    UpdateButtons(_rstInfo);

    // Start game?
    if(orxInput_IsActive(szInputStartGame) && orxInput_HasNewStatus(szInputStartGame))
    {
      // Removes selection
      SetSelection();

      // Deletes cursor
      DeleteCursorObject();

      // Hides basic cursor
      ShowBasicCursor(orxFALSE);

      // Sets game input set
      orxInput_SelectSet(roGame.szInputSetGame);

      // Updates status
      SetFlags(FlagInGame);

      // Updates layer display
      UpdateLayerDisplay();

      // Re-enables physics simulation
      orxPhysics_EnableSimulation(orxTRUE);

      // Selects camera section
      orxConfig_PushSection(orxCamera_GetName(roGame.GetMainCamera()));

      // Resets it
      mfZoom = orxConfig_GetFloat(szConfigZoom);

      // Pops config section
      orxConfig_PopSection();

      // Removes editor mode
      roGame.SetEditorMode(orxFALSE);

      // Saves map
      SaveMap();

      // Reloads it
      roGame.LoadMap();

      // Starts game
      roGame.StartGame();
    }
    else
    {
      orxVECTOR vMousePos, vMouseMove;
      orxBOOL   bIsMouseInScope;

      // Updates local time
      mfLocalTime += _rstInfo.fDT;

      // Uses autosave?
      if(mfAutoSaveInterval > orxFLOAT_0)
      {
        // Is it time?
        if(mfLocalTime >= mfAutoSaveTimeStamp + mfAutoSaveInterval)
        {
          // Saves backup
          SaveMapBackup();

          // Updates time stamp
          mfAutoSaveTimeStamp = mfLocalTime;
        }
      }

      // Gets mouse position
      bIsMouseInScope = orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos) != orxNULL;
      vMousePos.fZ    = roGame.GetLayerDepth(mu32LayerIndex);

      // In differential mode?
      if(roGame.IsDifferentialMode())
      {
        // Gets depth corrected position
        GetDepthCorrectedPosition(vMousePos.fZ, vMousePos);
      }

      // Gets mouse move
      orxMouse_GetMoveDelta(&vMouseMove);

      // Is set shown?
      if(TestFlags(FlagSetShown))
      {
        // Action?
        if(orxInput_IsActive(szInputAction))
        {
          const ScrollObject *poModel;

          // Selects corresponding model
          if((poModel = GetSetModel(vMousePos)))
          {
            // Selects it
            mpoSelectedModel = poModel;

            // Creates cursor from it
            CreateCursorObject();

            // Hides set
            HideSet();
          }
        }
        // Should hide it?
        else if(orxInput_IsActive(szInputHideSet))
        {
          // Creates former cursor
          CreateCursorObject();

          // Hides selected set
          HideSet();
        }
        // Next set?
        else if(orxInput_IsActive(szInputNextSet))
        {
          // Deletes current cursor
          DeleteCursorObject();

          // Removes selection
          SetSelection();

          // Displays next set
          ShowNextSet();
        }
        // Previous set?
        else if(orxInput_IsActive(szInputPreviousSet))
        {
          // Deletes current cursor
          DeleteCursorObject();

          // Removes selection
          SetSelection();

          // Displays previous set
          ShowPreviousSet();
        }
      }
      else
      {
        // Has mouse in scope?
        if(bIsMouseInScope)
        {
          // Zoom?
          if(orxInput_IsActive(szInputZoom))
          {
            // Updates desired zoom
            orxConfig_PushSection(szConfigSectionEditor);
            mfZoom *= orxFLOAT_1 + (orxConfig_GetFloat(szConfigZoomStep) * -vMouseMove.fY) + (orxConfig_GetFloat(szConfigZoomStep) * vMouseMove.fX);
            mfZoom = orxCLAMP(mfZoom, orxConfig_GetFloat(szConfigZoomMin), orxConfig_GetFloat(szConfigZoomMax));
            orxConfig_PopSection();
          }
          // Pan?
          else if(orxInput_IsActive(szInputPan))
          {
            orxVECTOR   vPos, vMouseRelMove;
            orxCAMERA  *pstCamera;

            // Gets camera
            pstCamera = roGame.GetMainCamera();

            // Updates mouse move with current zoom
            orxVector_Divf(&vMouseRelMove, &vMouseMove, orxCamera_GetZoom(pstCamera));

            // In differential mode?
            if(roGame.IsDifferentialMode())
            {
              // Updates mouse relative move
              orxVector_Divf(&vMouseRelMove, &vMouseRelMove, GetDepthCoef(roGame.GetLayerDepth(mu32LayerIndex)));
            }

            // Updates camera position
            orxCamera_SetPosition(pstCamera, orxVector_Sub(&vPos, orxCamera_GetPosition(pstCamera, &vPos), &vMouseRelMove));
          }
          // Edit alpha?
          else if(orxInput_IsActive(szInputEditAlpha) && mpoSelection)
          {
            // Action?
            if(orxInput_IsActive(szInputAction))
            {
              orxCOLOR stColor;

              // Gets selection object color
              if(orxObject_HasColor(mpoSelection->GetOrxObject()))
              {
                orxObject_GetColor(mpoSelection->GetOrxObject(), &stColor);
              }
              else
              {
                roGame.GetObjectConfigColor(mpoSelection, stColor);
              }

              // Updates its alpha
              stColor.fAlpha += orx2F(0.0025f) * vMouseMove.fX;
              stColor.fAlpha  = orxCLAMP(stColor.fAlpha, orxFLOAT_0, orxFLOAT_1);

              // Sets selection object color
              roGame.SetObjectConfigColor(mpoSelection, stColor);
            }

            // Shows selection's alpha property
            DisplayObjectAlpha(mpoSelection);
          }
          // Edit color?
          else if((orxInput_IsActive(szInputEditRed) || orxInput_IsActive(szInputEditGreen) || orxInput_IsActive(szInputEditBlue)) && mpoSelection)
          {
            // Action?
            if(orxInput_IsActive(szInputAction))
            {
              orxCOLOR stColor;

              // Gets selection object color
              if(orxObject_HasColor(mpoSelection->GetOrxObject()))
              {
                orxObject_GetColor(mpoSelection->GetOrxObject(), &stColor);
              }
              else
              {
                roGame.GetObjectConfigColor(mpoSelection, stColor);
              }

              // Edit red?
              if(orxInput_IsActive(szInputEditRed))
              {
                stColor.vRGB.fR  += orx2F(0.0025f) * vMouseMove.fX;
                stColor.vRGB.fR   = orxCLAMP(stColor.vRGB.fR, orxFLOAT_0, orxFLOAT_1);
              }
              // Edit green?
              if(orxInput_IsActive(szInputEditGreen))
              {
                stColor.vRGB.fG  += orx2F(0.0025f) * vMouseMove.fX;
                stColor.vRGB.fG   = orxCLAMP(stColor.vRGB.fG, orxFLOAT_0, orxFLOAT_1);
              }
              // Edit blue?
              if(orxInput_IsActive(szInputEditBlue))
              {
                stColor.vRGB.fB  += orx2F(0.0025f) * vMouseMove.fX;
                stColor.vRGB.fB   = orxCLAMP(stColor.vRGB.fB, orxFLOAT_0, orxFLOAT_1);
              }

              // Sets selection object color
              roGame.SetObjectConfigColor(mpoSelection, stColor);
            }

            // Shows selection's color property
            DisplayObjectColor(mpoSelection);
          }
          // Copy?
          else if(orxInput_IsActive(szInputCopy) && orxInput_HasNewStatus(szInputCopy))
          {
            orxOBJECT    *pstPick;
            ScrollObject *poObject;

            // Picks object
            pstPick = PickObject(vMousePos);

            // Clears selected model
            mpoSelectedModel = orxNULL;

            // Found?
            if(pstPick && (poObject = (ScrollObject *)orxObject_GetUserData(pstPick)))
            {
              // Creates cursor from it
              CreateCursorObject(poObject);
            }
            else
            {
              // Deletes cursor
              DeleteCursorObject();

              // Removes selection
              SetSelection();
            }
          }
          // Action?
          else if(orxInput_IsActive(szInputAction))
          {
            // Has cursor?
            if(mpoCursorObject)
            {
              // First time?
              if(orxInput_HasNewStatus(szInputAction))
              {
                // Adds object
                AddObject(mpoCursorObject);
              }
            }
            else
            {
              // First time?
              if(orxInput_HasNewStatus(szInputAction))
              {
                orxOBJECT *pstPick;

                // Gets object in current layer
                pstPick = PickObject(vMousePos);

                // Selects it
                SetSelection(pstPick ? (ScrollObject *)orxObject_GetUserData(pstPick) : orxNULL);

                // Deletes cursor object
                DeleteCursorObject();
              }

              // Has selection?
              if(mpoSelection)
              {
                // Depending on mode
                switch(GetFlags(MaskToggle))
                {
                  // Move
                  case FlagToggleMove:
                  {
                    // First time?
                    if(orxInput_HasNewStatus(szInputAction))
                    {
                      // Stores current mouse & object positions
                      orxVector_Copy(&mvWorkBase, &vMousePos);
                      orxObject_GetPosition(mpoSelection->GetOrxObject(), &mvWorkPosition);
                    }
                    else
                    {
                      orxVECTOR vPos;
                      orxFLOAT  fSnap;

                      // Gets object position
                      orxObject_GetPosition(mpoSelection->GetOrxObject(), &vPos);

                      // Pushes editor section
                      orxConfig_PushSection(szConfigSectionEditor);

                      // Snap?
                      if(TestFlags(FlagSnap) && ((fSnap = orxConfig_GetFloat(szConfigGridSize)) > orxFLOAT_0))
                      {
                        // Updates position
                        orxVector_Set(&vPos, orxMath_Round((mvWorkPosition.fX + vMousePos.fX - mvWorkBase.fX) / fSnap) * fSnap, orxMath_Round((mvWorkPosition.fY + vMousePos.fY - mvWorkBase.fY) / fSnap) * fSnap, vPos.fZ);
                      }
                      else
                      {
                        // Updates position
                        orxVector_Set(&vPos, orxMath_Round(mvWorkPosition.fX + vMousePos.fX - mvWorkBase.fX), orxMath_Round(mvWorkPosition.fY + vMousePos.fY - mvWorkBase.fY), vPos.fZ);
                      }

                      // Pops to previous section
                      orxConfig_PopSection();

                      // Updates its position
                      orxObject_SetPosition(mpoSelection->GetOrxObject(), &vPos);
                    }

                    break;
                  }

                  // Scale
                  case FlagToggleScale:
                  {
                    // First time?
                    if(orxInput_HasNewStatus(szInputAction))
                    {
                      orxOBJECT *pstPick;

                      // Gets object in current layer
                      pstPick = PickObject(vMousePos);

                      // Selects it
                      SetSelection(pstPick ? (ScrollObject *)orxObject_GetUserData(pstPick) : orxNULL);

                      // Deletes cursor object
                      DeleteCursorObject();
                    }

                    // Has selection?
                    if(mpoSelection)
                    {
                      orxOBJECT *pstOrxObject;

                      // Gets its internal object
                      pstOrxObject = mpoSelection->GetOrxObject();

                      // First activation?
                      if(orxInput_HasNewStatus(szInputAction))
                      {
                        orxVECTOR vTemp;

                        // Stores current work info
                        orxVector_FromCartesianToSpherical(&mvWorkBase, orxVector_Sub(&vTemp, &vMousePos, orxObject_GetPosition(pstOrxObject, &vTemp)));
                        orxObject_GetScale(pstOrxObject, &mvWorkScale);
                        orxObject_GetSize(pstOrxObject, &mvWorkSize);
                        mfWorkRotation = orxObject_GetRotation(pstOrxObject);
                      }
                      else
                      {
                        orxVECTOR vNewBase, vScale;

                        // Pushes editor section
                        orxConfig_PushSection(szConfigSectionEditor);

                        // Gets spherical diff vector
                        orxVector_FromCartesianToSpherical(&vNewBase, orxVector_Sub(&vNewBase, &vMousePos, orxObject_GetPosition(pstOrxObject, &vNewBase)));

                        // Gets values
                        vScale.fX = (orxMath_Cos(vNewBase.fTheta - mfWorkRotation) * vNewBase.fRho) / (orxMath_Cos(mvWorkBase.fTheta - mfWorkRotation) * mvWorkBase.fRho);
                        vScale.fY = (orxMath_Sin(vNewBase.fTheta - mfWorkRotation) * vNewBase.fRho) / (orxMath_Sin(mvWorkBase.fTheta - mfWorkRotation) * mvWorkBase.fRho);

                        // Applies old scale
                        orxVector_Mul(&vScale, &vScale, &mvWorkScale);

                        // Snap?
                        if(TestFlags(FlagSnap))
                        {
                          // Updates scale
                          orxVector_Floor(&vScale, &vScale);

                          // Needs correction
                          if(vScale.fX == orxFLOAT_0)
                          {
                            vScale.fX = orxFLOAT_1;
                          }
                          if(vScale.fY == orxFLOAT_0)
                          {
                            vScale.fY = orxFLOAT_1;
                          }
                        }
                        else
                        {
                          // Gets final size
                          vScale.fX *= mvWorkSize.fX;
                          vScale.fY *= mvWorkSize.fY;

                          // Rounds it
                          orxVector_Floor(&vScale, &vScale);

                          // Gets final scale
                          vScale.fX /= mvWorkSize.fX;
                          vScale.fY /= mvWorkSize.fY;

                          // Corrects them if too small
                          if(orxMath_Abs(vScale.fX) <= orx2F(0.01f))
                          {
                            vScale.fX = orx2F(0.01f);
                          }
                          if(orxMath_Abs(vScale.fY) <= orx2F(0.01f))
                          {
                            vScale.fY = orx2F(0.01f);
                          }
                          vScale.fZ = orxFLOAT_1;
                        }

                        // Scales object
                        orxObject_SetScale(pstOrxObject, &vScale);

                        // Uses tiling?
                        if(mpoSelection->TestFlags(ScrollObject::FlagTiled))
                        {
                          // Sets its repeat factor
                          orxObject_SetRepeat(pstOrxObject, orxMath_Abs(vScale.fX), orxMath_Abs(vScale.fY));
                        }
                        else
                        {
                          // Resets its repeat factor
                          orxObject_SetRepeat(pstOrxObject, orxFLOAT_1, orxFLOAT_1);
                        }

                        // Restores map section
                        orxConfig_PopSection();
                      }

                      // Displays its name
                      DisplayObjectName(mpoSelection, orxTRUE);
                    }

                    break;
                  }

                  // Rotate
                  case FlagToggleRotate:
                  {
                    // First time?
                    if(orxInput_HasNewStatus(szInputAction))
                    {
                      orxOBJECT *pstPick;

                      // Gets object in current layer
                      pstPick = PickObject(vMousePos);

                      // Selects it
                      SetSelection(pstPick ? (ScrollObject *)orxObject_GetUserData(pstPick) : orxNULL);

                      // Deletes cursor object
                      DeleteCursorObject();
                    }

                    // Has selection?
                    if(mpoSelection)
                    {
                      orxOBJECT *pstOrxObject;

                      // Gets its internal object
                      pstOrxObject = mpoSelection->GetOrxObject();

                      // First activation?
                      if(orxInput_HasNewStatus(szInputAction))
                      {
                        orxVECTOR vTemp;

                        // Stores current work info
                        orxVector_FromCartesianToSpherical(&mvWorkBase, orxVector_Sub(&vTemp, &vMousePos, orxObject_GetPosition(pstOrxObject, &vTemp)));
                        mfWorkRotation = orxObject_GetRotation(pstOrxObject);
                      }
                      else
                      {
                        orxVECTOR vNewBase;
                        orxFLOAT  fRotation, fSnap;

                        // Pushes editor section
                        orxConfig_PushSection(szConfigSectionEditor);

                        // Gets spherical diff vector
                        orxVector_FromCartesianToSpherical(&vNewBase, orxVector_Sub(&vNewBase, &vMousePos, orxObject_GetPosition(pstOrxObject, &vNewBase)));

                        // Updates rotation
                        fRotation = mfWorkRotation + vNewBase.fTheta - mvWorkBase.fTheta;

                        // Snap?
                        if(TestFlags(FlagSnap) && ((fSnap = orxConfig_GetFloat(szConfigRotationSnap)) > orxFLOAT_0))
                        {
                          // Updates snapping value
                          fSnap *= orxMATH_KF_DEG_TO_RAD;

                          // Updates rotation
                          fRotation /= fSnap;
                          fRotation  = orxMath_Round(fRotation);
                          fRotation *= fSnap;
                        }

                        // Rotates object
                        orxObject_SetRotation(pstOrxObject, fRotation);

                        // Restores map section
                        orxConfig_PopSection();
                      }
                    }

                    break;
                  }

                  default:
                  {
                    orxASSERT(0 && "Invalid control mode (%#X).", GetFlags(MaskToggle));
                    break;
                  }
                }
              }
            }
          }
        }

        // Update control toggles
        UpdateToggle(szInputToggleMove, FlagToggleMove);
        UpdateToggle(szInputToggleScale, FlagToggleScale);
        UpdateToggle(szInputToggleRotate, FlagToggleRotate);

        // Screenshot?
        if(orxInput_IsActive(szInputScreenshot) && orxInput_HasNewStatus(szInputScreenshot))
        {
          // Captures a screenshot
          orxScreenshot_Capture();
        }
        // Toggle snap?
        if(orxInput_IsActive(szInputToggleSnap) && orxInput_HasNewStatus(szInputToggleSnap))
        {
          // Swap its flag
          SwapFlags(FlagSnap);
        }
        // Toggle grid?
        if(orxInput_IsActive(szInputToggleGrid) && orxInput_HasNewStatus(szInputToggleGrid))
        {
          // Has grid?
          if(mpstGrid)
          {
            // Updates shader status
            orxObject_EnableShader(mpstGrid, !orxObject_IsShaderEnabled(mpstGrid));
          }
        }
        // Previous layer?
        if(orxInput_IsActive(szInputPreviousLayer))
        {
          // Updates current layer
          mu32LayerIndex = (mu32LayerIndex == 0) ? roGame.GetLayerNumber() - 1 : mu32LayerIndex - 1;

          // Updates layer display
          UpdateLayerDisplay();
        }
        // Next layer?
        else if(orxInput_IsActive(szInputNextLayer))
        {
          // Updates current layer
          mu32LayerIndex = (mu32LayerIndex >= roGame.GetLayerNumber() - 1) ? 0 : mu32LayerIndex + 1;

          // Updates layer display
          UpdateLayerDisplay();
        }
        // Next set?
        else if(orxInput_IsActive(szInputNextSet))
        {
          // Deletes current cursor
          DeleteCursorObject();

          // Removes selection
          SetSelection();

          // Displays next set
          ShowNextSet();
        }
        // Previous set?
        else if(orxInput_IsActive(szInputPreviousSet))
        {
          // Deletes current cursor
          DeleteCursorObject();

          // Removes selection
          SetSelection();

          // Displays previous set
          ShowPreviousSet();
        }
        // Clear selection?
        else if(orxInput_IsActive(szInputClearSelection) && orxInput_HasNewStatus(szInputClearSelection))
        {
          // Clears selected model
          mpoSelectedModel = orxNULL;

          // Removes selection
          SetSelection();

          // Deletes cursor
          DeleteCursorObject();
        }
        // Soft clear selection?
        else if(orxInput_IsActive(szInputSoftClearSelection) && orxInput_HasNewStatus(szInputSoftClearSelection))
        {
          // Clears selected model
          mpoSelectedModel = orxNULL;

          // Removes selection
          SetSelection();

          // Not a copy?
          if(!TestFlags(FlagUsingCopy))
          {
            // Deletes cursor
            DeleteCursorObject();
          }
        }
        // Delete?
        if(orxInput_IsActive(szInputDelete))
        {
          // Has selection?
          if(mpoSelection)
          {
            // Deletes it
            RemoveObject(mpoSelection);
          }
        }
        // Antialiasing?
        if(orxInput_IsActive(szInputAntialiasing) && orxInput_HasNewStatus(szInputAntialiasing))
        {
          // Has selection?
          if(mpoSelection)
          {
            // Smoothed?
            if(mpoSelection->TestFlags(ScrollObject::FlagSmoothed))
            {
              // Removes its smoothing
              orxObject_SetSmoothing(mpoSelection->GetOrxObject(), orxDISPLAY_SMOOTHING_OFF);

              // Updates its flags
              mpoSelection->SetFlags(ScrollObject::FlagNone, ScrollObject::FlagSmoothed);
            }
            else
            {
              // Activates its smoothing
              orxObject_SetSmoothing(mpoSelection->GetOrxObject(), orxDISPLAY_SMOOTHING_ON);

              // Updates its flags
              mpoSelection->SetFlags(ScrollObject::FlagSmoothed);
            }
          }
        }
        // Tiling?
        if(orxInput_IsActive(szInputTiling) && orxInput_HasNewStatus(szInputTiling))
        {
          // Has selection?
          if(mpoSelection)
          {
            // Was using tiling?
            if(mpoSelection->TestFlags(ScrollObject::FlagTiled))
            {
              // Resets its repeat factor
              orxObject_SetRepeat(mpoSelection->GetOrxObject(), orxFLOAT_1, orxFLOAT_1);

              // Updates its flags
              mpoSelection->SetFlags(ScrollObject::FlagNone, ScrollObject::FlagTiled);
            }
            else
            {
              orxVECTOR vScale;

              // Gets object scale
              orxObject_GetScale(mpoSelection->GetOrxObject(), &vScale);

              // Sets its repeat factor
              orxObject_SetRepeat(mpoSelection->GetOrxObject(), orxMath_Abs(vScale.fX), orxMath_Abs(vScale.fY));

              // Updates its flags
              mpoSelection->SetFlags(ScrollObject::FlagTiled);
            }
          }
        }
        // Differential scrolling?
        if(orxInput_IsActive(szInputDifferential) && orxInput_HasNewStatus(szInputDifferential))
        {
          // Swaps differential mode
          roGame.SetDifferentialMode(!roGame.IsDifferentialMode());

          // Updates layer display
          UpdateLayerDisplay();

          // Is in differential scrolling?
          if(roGame.IsDifferentialMode())
          {
            // Has name text?
            if(mpstNameText)
            {
              // Enforces differential flags
              orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstNameText, FRAME), orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y, orxFRAME_KU32_FLAG_NONE);
            }

            // Has property text?
            if(mpstPropertyText)
            {
              // Enforces differential flags
              orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstPropertyText, FRAME), orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y, orxFRAME_KU32_FLAG_NONE);
            }
          }
          else
          {
            // Has name text?
            if(mpstNameText)
            {
              // Removes differential flags
              orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstNameText, FRAME), orxFRAME_KU32_FLAG_NONE, orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y);
            }

            // Has property text?
            if(mpstPropertyText)
            {
              // Enforces differential flags
              orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstPropertyText, FRAME), orxFRAME_KU32_FLAG_NONE, orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y);
            }
          }
        }
        // Toggle full screen?
        if(orxInput_IsActive(szInputToggleFullScreen) && orxInput_HasNewStatus(szInputToggleFullScreen))
        {
          // Toggles full screen
          orxDisplay_SetFullScreen(!orxDisplay_IsFullScreen());
        }
        // Reload history?
        if(orxInput_IsActive(szInputReloadHistory) && orxInput_HasNewStatus(szInputReloadHistory))
        {
          // Reloads config history
          orxConfig_ReloadHistory();
        }
        // Save?
        if(orxInput_IsActive(szInputSave) && orxInput_HasNewStatus(szInputSave))
        {
          // Save maps
          SaveMap();
        }
        // Reload?
        if(orxInput_IsActive(szInputReload) && orxInput_HasNewStatus(szInputReload))
        {
          // Removes selection
          SetSelection();

          // Deletes cursor
          DeleteCursorObject();

          // Reloads map
          roGame.ResetMap();

          // Updates layer
          UpdateLayerDisplay();

          // For all buttons
          for(ButtonData *pstButtonData = (ButtonData *)orxLinkList_GetFirst(&mstButtonList);
              pstButtonData;
              pstButtonData = (ButtonData *)orxLinkList_GetNext(&pstButtonData->mstNode))
          {
            // Is active button?
            if(!orxString_Compare(pstButtonData->mzName, pstButtonData->mzActive))
            {
              // Matches?
              if(!orxString_Compare(szInputDifferential, pstButtonData->mzAction))
              {
                // Updates its status
                orxObject_Enable(pstButtonData->mpstObject, roGame.IsDifferentialMode());

                break;
              }
            }
          }
        }
      }
    }
  }
  else
  {
    // Stop game?
    if(orxInput_IsActive(szInputStopGame) && orxInput_HasNewStatus(szInputStopGame))
    {
      // Stops game
      roGame.StopGame();

      // Restores editor mode
      roGame.SetEditorMode();

      // Disables physics simulation
      orxPhysics_EnableSimulation(orxFALSE);

      // Restores map section
      orxConfig_SelectSection(ScrollBase::szConfigSectionMap);

      // Updates status
      SetFlags(FlagNone, FlagInGame);

      // Removes selection
      SetSelection();

      // Deletes cursor
      DeleteCursorObject();

      // Reloads map
      roGame.LoadMap();

      // Updates layer
      UpdateLayerDisplay();

      // Restores input set
      orxInput_SelectSet(szInputSetEditor);
    }
  }

  // No zoom yet?
  if(mfZoom == orxFLOAT_0)
  {
    // Updates it
    mfZoom = orxCamera_GetZoom(roGame.GetMainCamera());
  }

  // Reset zoom?
  if(orxInput_IsActive(szInputResetZoom))
  {
    // Selects camera section
    orxConfig_PushSection(orxCamera_GetName(roGame.GetMainCamera()));

    // Resets it
    mfZoom = orxConfig_GetFloat(szConfigZoom);
    if(mfZoom <= orxFLOAT_0)
    {
      mfZoom = orxFLOAT_1;
    }

    // Pops config section
    orxConfig_PopSection();
  }

  // Smoothes camera zoom
  orxConfig_PushSection(szConfigSectionEditor);
  fZoom = orxLERP(orxCamera_GetZoom(roGame.GetMainCamera()), mfZoom, orxConfig_GetFloat(szConfigZoomLerp) * _rstInfo.fDT);
  orxConfig_PopSection();

  // Updates it
  orxCamera_SetZoom(roGame.GetMainCamera(), fZoom);

  // Update UI
  UpdateUI(_rstInfo);
}

orxBOOL ScrollEd::MapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult;

  // Is editor section layer count or encrypt save?
  if(!orxString_Compare(_zSectionName, szConfigSectionEditor)
  && (!_zKeyName
   || !orxString_Compare(_zKeyName, szConfigLayerIndex)
   || !orxString_Compare(_zKeyName, szConfigEncryptSave)))
  {
    bResult = orxTRUE;
  }
  else
  {
    bResult = orxFALSE;
  }

  // Done!
  return bResult;
}

orxSTATUS ScrollEd::SaveMap() const
{
  orxBOOL bEncrypt;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Pushes editor section
  orxConfig_PushSection(szConfigSectionEditor);

  // Gets encryption
  bEncrypt = orxConfig_GetBool(szConfigEncryptSave);

  // Sets layer index
  orxConfig_SetU32(szConfigLayerIndex, mu32LayerIndex);

  // Pops previous section
  orxConfig_PopSection();

  // Saves map
  return roGame.SaveMap(bEncrypt, StaticMapSaveFilter);
}

orxSTATUS ScrollEd::SaveMapBackup() const
{
  orxCHAR   acBuffer[256];
  orxBOOL   bEncrypt;
  orxSTRING zMapName;
  orxSTATUS eResult;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Pushes editor section
  orxConfig_PushSection(szConfigSectionEditor);

  // Gets encryption
  bEncrypt = orxConfig_GetBool(szConfigEncryptSave);

  // Sets layer index
  orxConfig_SetU32(szConfigLayerIndex, mu32LayerIndex);

  // Pops previous section
  orxConfig_PopSection();

  // Gets current map name
  zMapName = orxString_Duplicate(roGame.GetMapName());

  // Sets backup name
  orxString_NPrint(acBuffer, 256, "%s-%d.bak", zMapName, orxSystem_GetRealTime());
  acBuffer[255] = orxCHAR_NULL;
  roGame.SetMapName(acBuffer);

  // Saves backup
  eResult = SaveMap();

  // Successful?
  if(eResult != orxSTATUS_FAILURE)
  {
    // Adds action display
    AddActionDisplay(orxLocale_GetString(szLocaleBackup));
  }

  // Restores map name
  roGame.SetMapName(zMapName);

  // Deletes local copy
  orxString_Delete(zMapName);

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL ScrollEd::StaticProcessParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult;

  // Has a valid map parameter?
  if(_u32ParamCount >= 2)
  {
    // Stores map's name
    ScrollEd::GetInstance().SetMapName(_azParams[1]);

    // Updates result
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

orxBOOL orxFASTCALL ScrollEd::StaticMapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  ScrollEd &roEditor = GetInstance();

  // Calls map save filter
  return roEditor.MapSaveFilter(_zSectionName, _zKeyName, _zFileName, _bUseEncryption);
}

orxSTATUS ScrollEd::StaticInit()
{
  orxSTATUS   eResult;
  ScrollEd   &roEditor = GetInstance();

  // Calls init method
  eResult = roEditor.Init();

  // Done!
  return eResult;
}

orxSTATUS ScrollEd::StaticRun()
{
  orxSTATUS   eResult;
  ScrollEd   &roEditor = GetInstance();

  // Calls run method
  eResult = roEditor.Run();

  // Done!
  return eResult;
}

void ScrollEd::StaticExit()
{
  ScrollEd &roEditor = GetInstance();

  // Calls exit method
  roEditor.Exit();
}

void orxFASTCALL ScrollEd::StaticUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext)
{
  ScrollEd &roEditor = GetInstance();

  // Calls base method
  roEditor.Update(*_pstInfo);
}

// Event handler
orxSTATUS orxFASTCALL ScrollEd::StaticEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Depending on event type
  switch(_pstEvent->eType)
  {
    // Shader
    case orxEVENT_TYPE_SHADER:
    {
      orxSHADER_EVENT_PAYLOAD *pstPayload;

      // Get game instance
      ScrollBase &roGame = ScrollBase::GetInstance();

      // Checks
      orxASSERT(_pstEvent->eID == orxSHADER_EVENT_SET_PARAM);

      // Gets its payload
      pstPayload = (orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      // Camera zoom?
      if(!orxString_Compare(pstPayload->zParamName, szConfigShaderCameraZoom))
      {
        // Updates its value
        pstPayload->fValue = orxCamera_GetZoom(roGame.GetMainCamera());
      }
      // Camera position?
      else if(!orxString_Compare(pstPayload->zParamName, szConfigShaderCameraPos))
      {
        // Updates its value
        orxCamera_GetPosition(roGame.GetMainCamera(), &pstPayload->vValue);
      }
      // Color?
      else if(!orxString_Compare(pstPayload->zParamName, szConfigShaderColor))
      {
        // Does object has color?
        if(orxObject_HasColor(orxOBJECT(_pstEvent->hSender)))
        {
          orxCOLOR stColor;

          // Gets it
          orxObject_GetColor(orxOBJECT(_pstEvent->hSender), &stColor);

          // Updates shader's value
          orxVector_Copy(&pstPayload->vValue, &stColor.vRGB);
        }
        else
        {
          // Updates shader's value
          orxVector_SetAll(&pstPayload->vValue, orxFLOAT_1);
        }
      }
      // Time?
      else if(!orxString_Compare(pstPayload->zParamName, szConfigShaderTime))
      {
        // Updates its value
        pstPayload->fValue = ScrollEd::GetInstance().mfLocalTime;
      }

      break;
    }

    // Input
    case orxEVENT_TYPE_INPUT:
    {
      // Is on?
      if(_pstEvent->eID == orxINPUT_EVENT_ON)
      {
        // Not game running?
        if(!ScrollBase::GetInstance().IsGameRunning())
        {
          ButtonData             *pstButtonData;
          orxINPUT_EVENT_PAYLOAD *pstPayload;

          // Gets instance
          ScrollEd &roEditor = GetInstance();

          // Gets its payload
          pstPayload = (orxINPUT_EVENT_PAYLOAD *)_pstEvent->pstPayload;

          // No set shown?
          if(!roEditor.TestFlags(ScrollEd::FlagSetShown))
          {
            // Has localized description?
            if(orxLocale_HasString(pstPayload->zInputName))
            {
              // Display it
              roEditor.AddActionDisplay(orxLocale_GetString(pstPayload->zInputName));
            }
          }

          // For all buttons
          for(pstButtonData = (ButtonData *)orxLinkList_GetFirst(&roEditor.mstButtonList);
              pstButtonData;
              pstButtonData = (ButtonData *)orxLinkList_GetNext(&pstButtonData->mstNode))
          {
            // Is active button?
            if(!orxString_Compare(pstButtonData->mzName, pstButtonData->mzActive))
            {
              // Matches?
              if(!orxString_Compare(pstPayload->zInputName, pstButtonData->mzAction))
              {
                // Updates its status
                orxObject_Enable(pstButtonData->mpstObject, !orxObject_IsEnabled(pstButtonData->mpstObject));

                break;
              }
            }
          }
        }
      }
      break;
    }

    default:
    {
      break;
    }
  }

  // Done!
  return eResult;
}

void ScrollEd::Execute(int argc, char **argv)
{
  // Makes sure the game instance has been created
  ScrollBase::GetInstance();

  // Executes orx
  orx_Execute(argc, argv, StaticInit, StaticRun, StaticExit);
}

orxSTATUS ScrollEd::SetMapName(const orxSTRING _zMapName)
{
  if(mzMapName)
  {
    orxString_Delete(mzMapName);
    mzMapName = orxNULL;
  }

  if(_zMapName)
  {
    mzMapName = orxString_Duplicate(_zMapName);
  }

  return orxSTATUS_SUCCESS;
}

const orxSTRING ScrollEd::GetMapName() const
{
  return mzMapName;
}

orxSTATUS ScrollEd::InitSets()
{
  orxVECTOR vFXOffest;
  orxS32    s32ContainerSize;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Pushes editor section
  orxConfig_PushSection(szConfigSectionEditor);

  // Gets container size
  s32ContainerSize = orxConfig_GetS32(szConfigSetContainerSize);

  // Pops config section
  orxConfig_PopSection();

  // Gets FX offset
  orxDisplay_GetScreenSize(&vFXOffest.fX, &vFXOffest.fY);
  vFXOffest.fY = vFXOffest.fZ = orxFLOAT_0;

  // Pushes set container show section
  orxConfig_PushSection(szConfigSetContainerShow);

  // Updates its end value
  orxConfig_SetVector("EndValue", &vFXOffest);

  // Pops config section
  orxConfig_PopSection();

  // Pushes set container hide section
  orxConfig_PushSection(szConfigSetContainerHide);

  // Updates its end value
  orxConfig_SetVector("EndValue", orxVector_Neg(&vFXOffest, &vFXOffest));

  // Pops config section
  orxConfig_PopSection();

  // Creates set bank
  mpstSetBank = orxBank_Create(32, sizeof(ScrollEdSet), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  // For all config sections
  for(orxS32 i = 0, s32SectionCount = orxConfig_GetSectionCount();
      i < s32SectionCount;
      i++)
  {
    const orxSTRING zSection;

    // Gets its name
    zSection = orxConfig_GetSection(i);

    // Pushes it
    orxConfig_PushSection(zSection);

    // Has ScrollEd type and a graphic?
    if(orxConfig_HasValue(szConfigScrollEdSet) && orxConfig_HasValue("Graphic"))
    {
      orxCHAR         zInstanceName[32];
      const orxSTRING zSet;
      ScrollEdSet    *poSelectedSet = orxNULL;
      ScrollObject   *poModel;

      // Gets its type
      zSet = orxConfig_GetString(szConfigScrollEdSet);

      // For all existing sets
      for(ScrollEdSet *poSet = (ScrollEdSet *)orxBank_GetNext(mpstSetBank, orxNULL);
          poSet;
          poSet = (ScrollEdSet *)orxBank_GetNext(mpstSetBank, poSet))
      {
        // Matches?
        if(!orxString_Compare(zSet, poSet->mzName))
        {
          // Selects it
          poSelectedSet = poSet;
          break;
        }
      }

      // No set selected?
      if(!poSelectedSet)
      {
        orxCHAR   zInstanceName[32];
        orxVECTOR vSize;

        // Creates a new set
        poSelectedSet = (ScrollEdSet *)orxBank_Allocate(mpstSetBank);

        // Inits it
        poSelectedSet->mpstObjectBank = orxBank_Create(64, sizeof(ScrollObject *), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
        poSelectedSet->mpoContainer   = roGame.CreateObject(szConfigSetContainer, ScrollObject::FlagNone, roGame.GetNewObjectName(zInstanceName, orxTRUE));
        poSelectedSet->mzName         = orxString_Duplicate(zSet);
        poSelectedSet->mpoContainer->PushConfigSection(orxTRUE);
        orxConfig_SetBool(ScrollBase::szConfigNoSave, orxTRUE);
        poSelectedSet->mpoContainer->PopConfigSection();

        // Gets its size
        orxObject_GetScale(poSelectedSet->mpoContainer->GetOrxObject(), &vSize);

        // For all cells
        for(orxS32 j = 1; j < s32ContainerSize; j++)
        {
          orxVECTOR vPosition, vScale;
          orxOBJECT *poLine;

          // Creates horizontal line
          poLine = orxObject_CreateFromConfig(szConfigSetContainerLine);

          // Updates its parent
          orxObject_SetParent(poLine, poSelectedSet->mpoContainer->GetOrxObject());
          orxObject_SetOwner(poLine, poSelectedSet->mpoContainer->GetOrxObject());

          // Updates its position
          orxVector_Set(&vPosition, orxFLOAT_0, orxS2F(j) / orxS2F(s32ContainerSize), orx2F(-0.0002f));
          orxObject_SetPosition(poLine, &vPosition);

          // Updates its scale
          orxVector_Set(&vScale, orxFLOAT_1, orxFLOAT_1 / vSize.fY, orxFLOAT_1);
          orxObject_SetScale(poLine, &vScale);

          // Creates vertical line
          poLine = orxObject_CreateFromConfig(szConfigSetContainerLine);

          // Updates its parent
          orxObject_SetParent(poLine, poSelectedSet->mpoContainer->GetOrxObject());
          orxObject_SetOwner(poLine, poSelectedSet->mpoContainer->GetOrxObject());

          // Updates its position
          orxVector_Set(&vPosition, orxS2F(j) / orxS2F(s32ContainerSize), orxFLOAT_0, orx2F(-0.0002f));
          orxObject_SetPosition(poLine, &vPosition);

          // Updates its scale
          orxVector_Set(&vScale, orxFLOAT_1 / vSize.fX, orxFLOAT_1, orxFLOAT_1);
          orxObject_SetScale(poLine, &vScale);
        }
      }

      // Creates model
      poModel = CreateModel(zSection, roGame.GetNewObjectName(zInstanceName, orxTRUE));

      // Valid?
      if(poModel)
      {
        orxVECTOR vSize;

        // Gets model size
        if(orxObject_GetSize(poModel->GetOrxObject(), &vSize))
        {
          orxVECTOR vPosition, vContainerScale, vFitScale, vScale, vPivot, vOffset;
          orxS32    s32ModelIndex;

          // Fixes Z dimension
          vSize.fZ = orxFLOAT_1;

          // Gets model's index
          s32ModelIndex = orxBank_GetCount(poSelectedSet->mpstObjectBank);

          // Stores it
          *((ScrollObject **)orxBank_Allocate(poSelectedSet->mpstObjectBank)) = poModel;

          // Sets container as parent
          orxObject_SetParent(poModel->GetOrxObject(), poSelectedSet->mpoContainer->GetOrxObject());

          // Gets container scale
          orxVector_Rec(&vContainerScale, orxObject_GetScale(poSelectedSet->mpoContainer->GetOrxObject(), &vContainerScale));

          // Gets anisotropic scale
          orxVector_Div(&vFitScale, orxVector_Divf(&vFitScale, orxVector_Set(&vFitScale, orxFLOAT_1, orxFLOAT_1, orxS2F(s32ContainerSize)), orxS2F(s32ContainerSize)), &vSize);
          orxVector_Set(&vScale, orxMIN(vContainerScale.fX, orxMIN(vFitScale.fX, vFitScale.fY)) - orx2F(0.005f) * vContainerScale.fX, orxMIN(vContainerScale.fY, orxMIN(vFitScale.fX, vFitScale.fY)) - orx2F(0.005f) * vContainerScale.fY, orxFLOAT_1);

          // Applies scale
          orxObject_SetScale(poModel->GetOrxObject(), &vScale);

          // Gets offset
          orxVector_Sub(&vFitScale, &vFitScale, &vScale);
          orxVector_Set(&vOffset, orx2F(0.5f) * orxMAX(orxFLOAT_0, vFitScale.fX) * vSize.fX, orx2F(0.5f) * orxMAX(orxFLOAT_0, vFitScale.fY) * vSize.fY, orx2F(-0.0001f));

          // Gets object's scaled pivot
          orxObject_GetPivot(poModel->GetOrxObject(), &vPivot);
          orxVector_Add(&vOffset, &vOffset, orxVector_Mul(&vPivot, &vPivot, &vScale));

          // Gets grid position
          orxVector_Set(&vPosition, orxS2F(s32ModelIndex % s32ContainerSize), orxS2F(s32ModelIndex / s32ContainerSize), orxFLOAT_0);

          // Gets position in container space
          orxVector_Add(&vPosition, orxVector_Divf(&vPosition, &vPosition, orxS2F(s32ContainerSize)), &vOffset);

          // Applies it
          orxObject_SetPosition(poModel->GetOrxObject(), &vPosition);
        }
        else
        {
          // Deletes model
          roGame.DeleteObject(poModel);
        }
      }
      else
      {
        // Logs message
        orxLOG("Couldn't create a model object <%s> to store in set <%s>.", zSection, zSet);
      }
    }

    // Pops config section
    orxConfig_PopSection();
  }

  // Done!
  return orxSTATUS_SUCCESS;
}

void ScrollEd::ExitSets()
{
  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // For all sets
  for(ScrollEdSet *poSet= (ScrollEdSet *)orxBank_GetNext(mpstSetBank, orxNULL);
      poSet;
      poSet = (ScrollEdSet *)orxBank_GetNext(mpstSetBank, poSet))
  {
    ScrollObject **ppoModel;

    // For all its objects
    for(ppoModel = (ScrollObject **)orxBank_GetNext(poSet->mpstObjectBank, orxNULL);
        ppoModel;
        ppoModel = (ScrollObject **)orxBank_GetNext(poSet->mpstObjectBank, ppoModel))
    {
      // Deletes it
      roGame.DeleteObject(*ppoModel);
    }

    // Deletes its bank
    orxBank_Delete(poSet->mpstObjectBank);

    // Deletes its container
    roGame.DeleteObject(poSet->mpoContainer);

    // Deletes its name
    orxString_Delete((orxSTRING)poSet->mzName);
  }

  // Deletes set bank
  orxBank_Delete(mpstSetBank);
  mpstSetBank = orxNULL;
}

void ScrollEd::ShowPreviousSet()
{
  // Has sets?
  if(orxBank_GetCount(mpstSetBank) > 0)
  {
    ScrollEdSet *poSet;

    // Is selected set already shown?
    if(TestFlags(FlagSetShown))
    {
      // Gets it
      poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

      // Hides it
      poSet->mpoContainer->AddFX(szConfigSetContainerHide, orxFALSE);

      // Gets previous one
      mu32SelectedSet = (mu32SelectedSet == 0) ? orxBank_GetCount(mpstSetBank) - 1 : mu32SelectedSet - 1;
      poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

      // Pushes hide FX section
      orxConfig_PushSection(szConfigSetContainerHide);

      // Shows it
      poSet->mpoContainer->AddFX(szConfigSetContainerShow, orxConfig_GetFloat("EndTime"), orxFALSE);

      // Pops config section
      orxConfig_PopSection();
    }
    else
    {
      // Gets selected set
      poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

      // Shows it
      poSet->mpoContainer->AddFX(szConfigSetContainerShow, orxFALSE);
    }

    // Adds action display
    AddActionDisplay(poSet->mzName);

    // Updates status
    SetFlags(FlagSetShown);
  }
}

void ScrollEd::ShowNextSet()
{
  // Has sets?
  if(orxBank_GetCount(mpstSetBank) > 0)
  {
    ScrollEdSet *poSet;

    // Is selected set already shown?
    if(TestFlags(FlagSetShown))
    {
      // Gets it
      poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

      // Hides it
      poSet->mpoContainer->AddFX(szConfigSetContainerHide, orxFALSE);

      // Gets next one
      mu32SelectedSet = (mu32SelectedSet + 1) % orxBank_GetCount(mpstSetBank);
      poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

      // Pushes hide FX section
      orxConfig_PushSection(szConfigSetContainerHide);

      // Shows it
      poSet->mpoContainer->AddFX(szConfigSetContainerShow, orxConfig_GetFloat("EndTime"), orxFALSE);

      // Pops config section
      orxConfig_PopSection();
    }
    else
    {
      // Gets selected set
      poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

      // Shows it
      poSet->mpoContainer->AddFX(szConfigSetContainerShow, orxFALSE);
    }

    // Adds action display
    AddActionDisplay(poSet->mzName);

    // Updates status
    SetFlags(FlagSetShown);
  }
}

void ScrollEd::HideSet()
{
  // Has sets?
  if(orxBank_GetCount(mpstSetBank) > 0)
  {
    ScrollEdSet *poSet;

    // Is selected set shown?
    if(TestFlags(FlagSetShown))
    {
      // Gets it
      poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

      // Hides it
      poSet->mpoContainer->AddFX(szConfigSetContainerHide, orxFALSE);
    }

    // Updates status
    SetFlags(FlagNone, FlagSetShown);
  }
}

const ScrollObject *ScrollEd::GetSetModel(const orxVECTOR &_rvPosition) const
{
  const ScrollObject *poResult = orxNULL;

  // Is set shown?
  if(TestFlags(FlagSetShown))
  {
    orxVECTOR     vRelativePos, vSize;
    orxFLOAT      fContainerSize;
    orxU32        u32ModelIndex;
    ScrollEdSet  *poSet;

    // Gets selected set
    poSet = (ScrollEdSet *)orxBank_GetAtIndex(mpstSetBank, mu32SelectedSet);

    // Gets container size
    orxObject_GetWorldScale(poSet->mpoContainer->GetOrxObject(), &vSize);

    // Gets relative position
    orxVector_Sub(&vRelativePos, &_rvPosition, orxObject_GetWorldPosition(poSet->mpoContainer->GetOrxObject(), &vRelativePos));

    // Pushs editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Gets container size
    fContainerSize = orxConfig_GetFloat(szConfigSetContainerSize);

    // Pops config section
    orxConfig_PopSection();

    // Gets container grid position
    orxVector_Floor(&vRelativePos, orxVector_Mulf(&vRelativePos, orxVector_Div(&vRelativePos, &vRelativePos, &vSize), fContainerSize));

    // Gets model index
    u32ModelIndex = orxF2U(vRelativePos.fX + vRelativePos.fY * fContainerSize);

    // Valid?
    if(u32ModelIndex < orxBank_GetCount(poSet->mpstObjectBank))
    {
      // Updates result
      poResult = *(ScrollObject **)orxBank_GetAtIndex(poSet->mpstObjectBank, u32ModelIndex);
    }
  }

  // Done!
  return poResult;
}

orxSTATUS ScrollEd::InitUI()
{
  orxS32    i, s32Number;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Pushes editor section
  orxConfig_PushSection(szConfigSectionEditor);

  // Creates grid
  mpstGrid = orxObject_CreateFromConfig(orxConfig_GetString(szConfigGrid));

  // Valid?
  if(mpstGrid)
  {
    // Disables its shader
    orxObject_EnableShader(mpstGrid, orxFALSE);
  }

  // Creates button bank
  mpstButtonBank = orxBank_Create((orxU32)orxConfig_GetListCount(szConfigButtonList), sizeof(ButtonData), orxBANK_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_MAIN);

  // For all buttons
  for(i = 0, s32Number = orxConfig_GetListCount(szConfigButtonList); i < s32Number; i++)
  {
    const orxSTRING zName;
    orxOBJECT      *pstButton;

    // Gets its name
    zName = orxConfig_GetListString(szConfigButtonList, i);

    // Creates it
    pstButton = orxObject_CreateFromConfig(zName);

    // Valid?
    if(pstButton)
    {
      ButtonData *pstData;

      // Pushes its section
      orxConfig_PushSection(zName);

      // Allocates a data cell
      pstData = (ButtonData *)orxBank_Allocate(mpstButtonBank);
      orxMemory_Zero(pstData, sizeof(ButtonData));

      // Inits it
      pstData->mpstObject   = pstButton;
      pstData->mzName       = orxObject_GetName(pstButton);
      pstData->mzBase       = orxConfig_GetString(szConfigButtonBase);
      pstData->mzHighlight  = orxConfig_GetString(szConfigButtonHighlight);
      pstData->mzActive     = orxConfig_GetString(szConfigButtonActive);
      pstData->mzAction     = orxConfig_GetString(szConfigButtonAction);

      // Adds it to list
      orxLinkList_AddEnd(&mstButtonList, &pstData->mstNode);

      // Not its base?
      if(orxString_Compare(pstData->mzName, pstData->mzBase))
      {
        // Disables it
        orxObject_Enable(pstButton, orxFALSE);
      }

      // Pops config section
      orxConfig_PopSection();
    }
  }

  // Pops config section
  orxConfig_PopSection();

  // Creates default cursor
  CreateCursorObject();

  // For all buttons
  for(ButtonData *pstButtonData = (ButtonData *)orxLinkList_GetFirst(&mstButtonList);
      pstButtonData;
      pstButtonData = (ButtonData *)orxLinkList_GetNext(&pstButtonData->mstNode))
  {
    // Is active button?
    if(!orxString_Compare(pstButtonData->mzName, pstButtonData->mzActive))
    {
      // Matches?
      if(!orxString_Compare(szInputDifferential, pstButtonData->mzAction))
      {
        // Updates its status
        orxObject_Enable(pstButtonData->mpstObject, ScrollBase::GetInstance().IsDifferentialMode());

        break;
      }
    }
  }

  // Done!
  return eResult;
}

void ScrollEd::ExitUI()
{
  // Deletes button bank
  orxBank_Delete(mpstButtonBank);
  mpstButtonBank = orxNULL;
}

void ScrollEd::UpdateUI(const orxCLOCK_INFO &_rstInfo)
{
  // Updates cursor
  UpdateCursor();

  // No control mode
  if(!TestFlags(MaskToggle))
  {
    // Still active?
    if(orxInput_IsActive(szInputToggleMove) && !orxInput_HasNewStatus(szInputToggleMove))
    {
      // Removes its input
      orxInput_ResetValue(szInputToggleMove);
    }
    else
    {
      // Defaults to move mode
      orxInput_SetValue(szInputToggleMove, orxFLOAT_1);
    }
  }

  // Had an highlighted button?
  if(mpstButtonHighlight)
  {
    // Disables it
    orxObject_Enable(mpstButtonHighlight->mpstObject, orxFALSE);

    // Removes its reference
    mpstButtonHighlight = orxNULL;
  }

  // Not in game?
  if(!TestFlags(FlagInGame))
  {
    // Has a selection and no property text?
    if(mpoSelection && !mpstPropertyText)
    {
      // Depending on current toggle
      switch(GetFlags(MaskToggle))
      {
        case FlagToggleMove:
        {
          // Displays its position
          DisplayObjectPosition(mpoSelection);
          break;
        }
        case FlagToggleScale:
        {
          // Displays its scale
          DisplayObjectScale(mpoSelection);
          break;
        }
        case FlagToggleRotate:
        {
          // Displays its rotation
          DisplayObjectRotation(mpoSelection);
          break;
        }
        default:
        {
          break;
        }
      }
    }
    // Has cursor?
    else if(mpoCursorObject)
    {
      // Displays its position
      DisplayObjectPosition(mpoCursorObject);
    }
  }
}

void ScrollEd::UpdateButtons(const orxCLOCK_INFO &_rstInfo)
{
  orxVECTOR vMousePos;

  // Gets mouse world position
  if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos))
  {
    orxOBJECT  *pstPickedObject;
    orxAABOX    stCameraFrustum;
    orxVECTOR   vCameraPos;

    // Gets game instance
    ScrollBase &roGame = ScrollBase::GetInstance();

    // Updates mouse position
    orxCamera_GetFrustum(roGame.GetMainCamera(), &stCameraFrustum);
    vMousePos.fZ = stCameraFrustum.vTL.fZ + orxCamera_GetPosition(roGame.GetMainCamera(), &vCameraPos)->fZ + orx2F(0.0002f);

    // Picks object
    pstPickedObject = orxObject_Pick(&vMousePos, orxU32_UNDEFINED);

    // Found?
    if(pstPickedObject)
    {
      // Pushes its section
      orxConfig_PushSection(orxObject_GetName(pstPickedObject));

      // Is a button?
      if(orxConfig_HasValue(szConfigButtonBase))
      {
        ButtonData     *pstButtonData;
        const orxSTRING zHighlightName;

        // Action input?
        if(orxInput_IsActive(szInputAction) && orxInput_HasNewStatus(szInputAction))
        {
          // Deletes cursor object
          DeleteCursorObject();

          // Has action?
          if(orxConfig_HasValue(szConfigButtonAction))
          {
            const orxSTRING zActionName;

            // Gets action
            zActionName = orxConfig_GetString(szConfigButtonAction);

            // Activates it
            orxInput_SetValue(zActionName, orxFLOAT_1);
          }
        }

        // Gets its highlight name
        zHighlightName = orxConfig_GetString(szConfigButtonHighlight);

        // For all buttons
        for(pstButtonData = (ButtonData *)orxLinkList_GetFirst(&mstButtonList);
            pstButtonData;
            pstButtonData = (ButtonData *)orxLinkList_GetNext(&pstButtonData->mstNode))
        {
          // Is the highlight?
          if(!orxString_Compare(zHighlightName, pstButtonData->mzName))
          {
            // Stores it
            mpstButtonHighlight = pstButtonData;

            // Enables it
            orxObject_Enable(pstButtonData->mpstObject, orxTRUE);

            // Shows basic cursor
            ShowBasicCursor();

            break;
          }
        }
      }

      // Pops config section
      orxConfig_PopSection();
    }
  }
}

void ScrollEd::UpdateLayerDisplay()
{
  ScrollObject *poObject;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Removes selection
  SetSelection();

  // In game?
  if(TestFlags(FlagInGame))
  {
    // Has layer text?
    if(mpstLayerText)
    {
      // Deletes it
      orxObject_Delete(mpstLayerText);
      mpstLayerText = orxNULL;
    }
  }
  else
  {
    orxCHAR acBuffer[64];

    // Hides mouse cursor
    orxMouse_ShowCursor(orxFALSE);

    // No layer text?
    if(!mpstLayerText)
    {
      // Creates it
      mpstLayerText = orxObject_CreateFromConfig(szConfigLayerText);
    }

    // Pushes editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Updates layer text
    orxString_NPrint(acBuffer, 64, orxConfig_GetString(szConfigLayerTextFormat), mu32LayerIndex - (roGame.GetLayerNumber() >> 1));

    // Updates its content
    orxObject_SetTextString(mpstLayerText, acBuffer);

    // Restores map section
    orxConfig_PopSection();

    // For all map objects
    for(poObject = roGame.GetNextObject();
        poObject;
        poObject = roGame.GetNextObject(poObject))
    {
      // Has no owner? (otherwise it'll get updated through it)
      if(!orxObject_GetOwner(poObject->GetOrxObject()))
      {
        static const ScrollObject::Flag seFocusFlag = (ScrollObject::Flag)0x80000000;
        orxVECTOR                       vPos;

        // Gets its position
        orxObject_GetPosition(poObject->GetOrxObject(), &vPos);

        // Is in current layer or cursor?
        if((roGame.GetLayer(vPos) == mu32LayerIndex)
        || (poObject == mpoCursorObject))
        {
          // Was out of layer?
          if(poObject->TestFlags(seFocusFlag))
          {
            // Adds gain focus FX
            poObject->AddFX(szConfigGainFocusFX);

            // Updates status
            poObject->SetFlags(ScrollObject::FlagNone, seFocusFlag);
          }
        }
        else
        {
          // Wasn't out of focus?
          if(!poObject->TestFlags(seFocusFlag))
          {
            // Adds lose focus FX
            poObject->AddFX(szConfigLoseFocusFX);

            // Updates status
            poObject->SetFlags(seFocusFlag);
          }
        }

        // Updates its differential mode
        poObject->SetDifferentialMode(roGame.IsDifferentialMode());
      }
    }
  }
}

ScrollObject *ScrollEd::CreateModel(const orxSTRING _zModelName, const orxSTRING _zInstanceName, ScrollObject::Flag _xFlags)
{
  ScrollObject *poResult;

  // Checks
  orxASSERT(_zModelName);

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Creates object
  poResult = roGame.CreateObject(_zModelName, _xFlags & (~ScrollObject::FlagSave), _zInstanceName);

  // Valid?
  if(poResult)
  {
    orxANIMPOINTER *pstAnimPointer;
    orxOBJECT      *pstOrxObject;

    // Gets its internal object
    pstOrxObject = poResult->GetOrxObject();

    // Updates its status
    poResult->SetFlags(ScrollObject::FlagNone, ScrollObject::FlagSave);
    poResult->PushConfigSection(orxTRUE);
    orxConfig_SetBool(ScrollBase::szConfigNoSave, orxTRUE);
    poResult->PopConfigSection();

    // Gets animpointer
    pstAnimPointer = orxOBJECT_GET_STRUCTURE(pstOrxObject, ANIMPOINTER);

    // Valid?
    if(pstAnimPointer)
    {
      // Updates its animation time
      orxAnimPointer_SetTime(pstAnimPointer, mfLocalTime);
    }
  }

  // Done!
  return poResult;
}

void ScrollEd::CreateCursorObject(const ScrollObject *_poModel)
{
  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Deletes previous cursor
  DeleteCursorObject();

  // Removes selection
  SetSelection();

  // Has a model?
  if(_poModel)
  {
    // Creates it
    mpoCursorObject = CreateModel(_poModel->GetModelName(), szConfigCursor, _poModel->GetFlags());

    // Updates status
    SetFlags(FlagUsingCopy);

    // Valid?
    if(mpoCursorObject)
    {
      orxVECTOR   vTemp;
      orxCOLOR    stColor;
      orxOBJECT  *pstModelOrxObject;
      orxOBJECT  *pstOrxObject;

      // Gets both internal objects
      pstOrxObject = mpoCursorObject->GetOrxObject();
      pstModelOrxObject = _poModel->GetOrxObject();

      // Gets model color
      if(roGame.GetObjectConfigColor(_poModel, stColor))
      {
        // Applies it
        roGame.SetObjectConfigColor(mpoCursorObject, stColor);
      }

      // Uses differential scrolling?
      if(roGame.IsDifferentialMode())
      {
        orxVECTOR vPos;

        // Gets model position
        orxObject_GetWorldPosition(pstModelOrxObject, &vPos);

        // Gets corrected scale
        orxVector_Mulf(&vTemp, orxObject_GetScale(pstModelOrxObject, &vTemp), GetDepthCoef(vPos.fZ));
      }
      else
      {
        // Gets model scale
        orxObject_GetScale(pstModelOrxObject, &vTemp);
      }

      // Updates it
      orxObject_SetScale(pstOrxObject, &vTemp);
      orxObject_SetRotation(pstOrxObject, orxObject_GetRotation(pstModelOrxObject));
      orxObject_SetSmoothing(pstOrxObject, orxObject_GetSmoothing(pstModelOrxObject));

      // Has tiling?
      if(_poModel->TestFlags(ScrollObject::FlagTiled))
      {
        orxFLOAT fRepeatX, fRepeatY;

        // Gets its tiling
        if(orxObject_GetRepeat(pstModelOrxObject, &fRepeatX, &fRepeatY) != orxSTATUS_FAILURE)
        {
          // Updates cursor tiling
          orxObject_SetRepeat(pstOrxObject, fRepeatX, fRepeatY);
        }
      }
    }
  }
  else
  {
    // Updates status
    SetFlags(FlagNone, FlagUsingCopy);

    // Has selected model?
    if(mpoSelectedModel)
    {
      // Creates it
      mpoCursorObject = CreateModel(mpoSelectedModel->GetModelName(), szConfigCursor);
    }
  }

  // Valid?
  if(mpoCursorObject)
  {
    // Updates cursor
    UpdateCursor();
  }
}

void ScrollEd::DeleteCursorObject()
{
  // Updates status
  SetFlags(FlagNone, FlagUsingCopy);

  // Has a cursor?
  if(mpoCursorObject)
  {
    // Gets game instance
    ScrollBase &roGame = ScrollBase::GetInstance();

    // Deletes it
    roGame.DeleteObject(mpoCursorObject);
    mpoCursorObject = orxNULL;

    // Hides its name
    DisplayObjectName();
  }
}

void ScrollEd::UpdateCursor()
{
  // Not in game?
  if(!TestFlags(FlagInGame))
  {
    // Has cursor object?
    if(mpoCursorObject)
    {
      orxVECTOR vMousePos;

      // Gets mouse position
      if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos))
      {
        orxVECTOR vPos;
        orxFLOAT  fSnap;

        // Gets game instance
        ScrollBase &roGame = ScrollBase::GetInstance();

        // Pushes editor section
        orxConfig_PushSection(szConfigSectionEditor);

        // Snap?
        if(TestFlags(FlagSnap) && ((fSnap = orxConfig_GetFloat(szConfigGridSize)) > orxFLOAT_0))
        {
          // Updates position
          orxVector_Set(&vPos, orxMath_Round(vMousePos.fX / fSnap) * fSnap, orxMath_Round(vMousePos.fY / fSnap) * fSnap, roGame.GetLayerDepth(mu32LayerIndex) - orx2F(0.01f));
        }
        else
        {
          // Updates cursor position
          orxVector_Set(&vPos, orxMath_Round(vMousePos.fX), orxMath_Round(vMousePos.fY), roGame.GetLayerDepth(mu32LayerIndex) - orx2F(0.01f));
        }

        // Pops to previous section
        orxConfig_PopSection();

        // Updates cursor
        orxObject_SetPosition(mpoCursorObject->GetOrxObject(), &vPos);
      }

      // Displays cursor name
      DisplayObjectName(mpoCursorObject, orxTRUE);

      // Hides basic cursor
      ShowBasicCursor(orxFALSE);
    }
    else
    {
      // Shows basic cursor
      ShowBasicCursor();

      // Is set shown?
      if(TestFlags(FlagSetShown))
      {
        orxVECTOR vMousePos;

        // Gets mouse position
        if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos))
        {
          const ScrollObject *poModel;

          // Gets model under mouse position
          poModel = GetSetModel(vMousePos);

          // Valid?
          if(poModel)
          {
            // Displays its name
            DisplayObjectName(poModel, orxFALSE);
          }
          else
          {
            // Hides name
            DisplayObjectName();
          }
        }
      }
      else
      {
        // Has selection?
        if(mpoSelection)
        {
          // Displays its name
          DisplayObjectName(mpoSelection);
        }
      }
    }
  }
  else
  {
    // Hides basic cursor
    ShowBasicCursor(orxFALSE);
  }

  // Shoud display basic cursor?
  if(TestFlags(FlagShowBasicCursor))
  {
    orxVECTOR vMousePos;

    // Gets mouse position
    if(orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos))
    {
      orxVECTOR vPos, vScale;
      orxFLOAT  fScale;
      const orxSTRING zCursorName = szConfigDefaultCursor;

      // Gets game instance
      ScrollBase &roGame = ScrollBase::GetInstance();

      // Has selection?
      if(mpoSelection)
      {
        // Depending on current toggle
        switch(GetFlags(MaskToggle))
        {
          case FlagToggleMove:
          {
            zCursorName = szConfigMoveCursor;
            break;
          }
          case FlagToggleScale:
          {
            zCursorName = szConfigScaleCursor;
            break;
          }
          case FlagToggleRotate:
          {
            zCursorName = szConfigRotateCursor;
            break;
          }
          default:
          {
            break;
          }
        }
      }

      // Not already created?
      if(!mpstBasicCursor)
      {
        // Creates it
        mpstBasicCursor = orxObject_CreateFromConfig(zCursorName);
      }
      else
      {
        // Not the current cursor?
        if(orxString_Compare(orxObject_GetName(mpstBasicCursor), zCursorName))
        {
          // Deletes current cursor
          orxObject_Delete(mpstBasicCursor);

          // Creates new one
          mpstBasicCursor = orxObject_CreateFromConfig(zCursorName);
        }
      }

      // Calculates cursor position
      orxVector_Set(&vPos, vMousePos.fX, vMousePos.fY, vMousePos.fZ + orx2F(0.0001f));

      // Calculates its scale
      fScale = orxFLOAT_1 / orxCamera_GetZoom(roGame.GetMainCamera());
      orxVector_Set(&vScale, fScale, fScale, orxFLOAT_1);

      // Updates cursor
      orxObject_SetScale(mpstBasicCursor, &vScale);
      orxObject_SetPosition(mpstBasicCursor, &vPos);
    }
  }
  else
  {
    // Not already deleted?
    if(mpstBasicCursor)
    {
      // Deletes it
      orxObject_Delete(mpstBasicCursor);
      mpstBasicCursor = orxNULL;
    }
  }
}

void ScrollEd::ShowBasicCursor(orxBOOL _bShow)
{
  // Show?
  if(_bShow)
  {
    // Updates status
    SetFlags(FlagShowBasicCursor);
  }
  else
  {
    // Updates status
    SetFlags(FlagNone, FlagShowBasicCursor);
  }
}

void ScrollEd::SetPropertyText(const ScrollObject *_poObject, const orxSTRING _zText, const orxCOLOR *_pstColor)
{
  // Has property text?
  if(mpstPropertyText)
  {
    // Deletes it
    orxObject_Delete(mpstPropertyText);
    mpstPropertyText = orxNULL;
  }

  // Has object?
  if(_poObject)
  {
    // Creates property text
    mpstPropertyText = orxObject_CreateFromConfig(szConfigPropertyText);

    // Valid?
    if(mpstPropertyText)
    {
      orxVECTOR   vPos, vSize, vPivot, vScale, vTemp;
      orxOBJECT  *pstOrxObject;

      // Gets game instance
      ScrollBase &roGame = ScrollBase::GetInstance();

      // Gets its internal object
      pstOrxObject = _poObject->GetOrxObject();

      // Updates differential flags
      orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstPropertyText, FRAME), orxStructure_GetFlags(orxOBJECT_GET_STRUCTURE(pstOrxObject, FRAME), orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y), orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y);

      // Updates its parent
      orxObject_SetParent(mpstPropertyText, pstOrxObject);

      // Gets object size, scale & pivot
      orxObject_GetSize(pstOrxObject, &vSize);
      orxObject_GetScale(pstOrxObject, &vScale);
      orxObject_GetPivot(pstOrxObject, &vPivot);

      // Updates its position
      orxVector_Mul(&vPos, &vSize, orxObject_GetPosition(mpstPropertyText, &vTemp));
      orxVector_Set(&vPos, vPos.fX - vPivot.fX, vPos.fY - vPivot.fY, orxFLOAT_0);
      orxObject_SetPosition(mpstPropertyText, &vPos);

      // Updates its scale for conservation
      orxVector_Div(&vScale, orxObject_GetScale(mpstPropertyText, &vTemp), &vScale);
      orxObject_SetScale(mpstPropertyText, orxVector_Divf(&vScale, &vScale, orxCamera_GetZoom(roGame.GetMainCamera())));

      // Updates its text
      orxObject_SetTextString(mpstPropertyText, _zText);

      // Has color?
      if(_pstColor)
      {
        // Updates its color
        orxObject_SetColor(mpstPropertyText, _pstColor);
      }
    }
  }
}

void ScrollEd::DisplayObjectName(const ScrollObject *_poObject, orxBOOL _bDisplayInstanceName)
{
  // Has name text?
  if(mpstNameText)
  {
    // Deletes it
    orxObject_Delete(mpstNameText);
    mpstNameText = orxNULL;
  }

  // Has object?
  if(_poObject)
  {
    // Creates name text
    mpstNameText = orxObject_CreateFromConfig(szConfigNameText);

    // Valid?
    if(mpstNameText)
    {
      orxVECTOR   vPos, vSize, vPivot, vScale, vTemp;
      orxCHAR     acBuffer[256];
      orxOBJECT  *pstOrxObject;

      // Gets its internal object
      pstOrxObject = _poObject->GetOrxObject();

      // Gets game instance
      ScrollBase &roGame = ScrollBase::GetInstance();

      // Updates differential flags
      orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstNameText, FRAME), orxStructure_GetFlags(orxOBJECT_GET_STRUCTURE(pstOrxObject, FRAME), orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y), orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y);

      // Updates its parent
      orxObject_SetParent(mpstNameText, pstOrxObject);

      // Gets object size, scale & pivot
      orxObject_GetSize(pstOrxObject, &vSize);
      orxObject_GetWorldScale(pstOrxObject, &vScale);
      orxObject_GetPivot(pstOrxObject, &vPivot);

      // Updates its position
      orxVector_Mul(&vPos, &vSize, orxObject_GetPosition(mpstNameText, &vTemp));
      orxVector_Set(&vPos, vPos.fX - vPivot.fX, vPos.fY - vPivot.fY, orxFLOAT_0);
      orxObject_SetPosition(mpstNameText, &vPos);

      // Updates its scale for conservation
      orxVector_Div(&vScale, orxObject_GetScale(mpstNameText, &vTemp), &vScale);
      orxObject_SetScale(mpstNameText, orxVector_Divf(&vScale, &vScale, orxCamera_GetZoom(roGame.GetMainCamera())));

      // Updates its content
      if(_bDisplayInstanceName)
      {
        orxString_NPrint(acBuffer, 256, "%s(%s)", _poObject->GetName(), _poObject->GetModelName());
      }
      else
      {
        orxString_NPrint(acBuffer, 256, "%s", _poObject->GetModelName());
      }
      acBuffer[255] = orxCHAR_NULL;
      orxObject_SetTextString(mpstNameText, acBuffer);
    }
  }
}

void ScrollEd::DisplayObjectColor(const ScrollObject *_poObject)
{
  // Has object?
  if(_poObject)
  {
    static const orxS32 ss32BufferSize = 256;
    orxCOLOR            stColor;
    orxCHAR             acBuffer[ss32BufferSize];

    // Gets game instance
    ScrollBase &roGame = ScrollBase::GetInstance();

    // Pushes editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Gets object's color
    if(orxObject_HasColor(_poObject->GetOrxObject()))
    {
      orxObject_GetColor(_poObject->GetOrxObject(), &stColor);
    }
    else
    {
      roGame.GetObjectConfigColor(_poObject, stColor);
    }

    // Updates it
    stColor.fAlpha = orxFLOAT_1;

    // Creates color string
    orxString_NPrint(acBuffer, ss32BufferSize, orxConfig_GetString(szConfigColorTextFormat), orxF2S(stColor.vRGB.fR * orxCOLOR_DENORMALIZER), orxF2S(stColor.vRGB.fG * orxCOLOR_DENORMALIZER), orxF2S(stColor.vRGB.fB * orxCOLOR_DENORMALIZER));

    // Sets property text
    SetPropertyText(_poObject, acBuffer, &stColor);

    // Pops previous section
    orxConfig_PopSection();
  }
  else
  {
    // Clears property text
    SetPropertyText();
  }
}

void ScrollEd::DisplayObjectAlpha(const ScrollObject *_poObject)
{
  // Has object?
  if(_poObject)
  {
    static const orxS32 ss32BufferSize = 256;
    orxCOLOR            stColor;
    orxCHAR             acBuffer[ss32BufferSize];

    // Gets game instance
    ScrollBase &roGame = ScrollBase::GetInstance();

    // Pushes editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Gets object's color
    if(orxObject_HasColor(_poObject->GetOrxObject()))
    {
      orxObject_GetColor(_poObject->GetOrxObject(), &stColor);
    }
    else
    {
      roGame.GetObjectConfigColor(_poObject, stColor);
    }

    // Updates it
    orxVector_Copy(&stColor.vRGB, &orxVECTOR_WHITE);

    // Creates alpha string
    orxString_NPrint(acBuffer, ss32BufferSize, orxConfig_GetString(szConfigAlphaTextFormat), orxF2S(orx2F(100.0f) * stColor.fAlpha));

    // Sets property text
    SetPropertyText(_poObject, acBuffer, &stColor);

    // Pops previous section
    orxConfig_PopSection();
  }
  else
  {
    // Clears property text
    SetPropertyText();
  }
}

void ScrollEd::DisplayObjectScale(const ScrollObject *_poObject)
{
  // Has object?
  if(_poObject)
  {
    static const orxS32 ss32BufferSize = 256;
    orxVECTOR           vScale, vSize;
    orxCHAR             acBuffer[ss32BufferSize];

    // Pushes editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Gets object's scale & size
    orxObject_GetScale(_poObject->GetOrxObject(), &vScale);
    orxObject_GetSize(_poObject->GetOrxObject(), &vSize);
    orxVector_Mul(&vSize, &vSize, &vScale);
    vSize.fX = orxMath_Abs(vSize.fX);
    vSize.fY = orxMath_Abs(vSize.fY);

    // Creates text
    orxString_NPrint(acBuffer, ss32BufferSize, orxConfig_GetString(szConfigScaleTextFormat), vScale.fX, vScale.fY, vSize.fX, vSize.fY);

    // Sets property text
    SetPropertyText(_poObject, acBuffer);

    // Pops previous section
    orxConfig_PopSection();
  }
  else
  {
    // Clears property text
    SetPropertyText();
  }
}

void ScrollEd::DisplayObjectRotation(const ScrollObject *_poObject)
{
  // Has object?
  if(_poObject)
  {
    static const orxS32 ss32BufferSize = 256;
    orxFLOAT            fRotation;
    orxCHAR             acBuffer[ss32BufferSize];

    // Pushes editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Gets object's rotation
    fRotation = orxObject_GetRotation(_poObject->GetOrxObject());

    // Clamps it in ]-Pi, Pi]
    fRotation = (fRotation + orxMATH_KF_PI) * (orxFLOAT_1 / orxMATH_KF_2_PI);
    fRotation = fRotation - orxMath_Floor(fRotation);
    fRotation = (fRotation * orxMATH_KF_2_PI) - orxMATH_KF_PI;

    // Creates text
    orxString_NPrint(acBuffer, ss32BufferSize, orxConfig_GetString(szConfigRotationTextFormat), orxMATH_KF_RAD_TO_DEG * fRotation);

    // Sets property text
    SetPropertyText(_poObject, acBuffer);

    // Pops previous section
    orxConfig_PopSection();
  }
  else
  {
    // Clears property text
    SetPropertyText();
  }
}

void ScrollEd::DisplayObjectPosition(const ScrollObject *_poObject)
{
  // Has object?
  if(_poObject)
  {
    static const orxS32 ss32BufferSize = 256;
    orxVECTOR           vPosition;
    orxCHAR             acBuffer[ss32BufferSize];

    // Pushes editor section
    orxConfig_PushSection(szConfigSectionEditor);

    // Gets object's position
    orxObject_GetPosition(_poObject->GetOrxObject(), &vPosition);

    // Creates text
    orxString_NPrint(acBuffer, ss32BufferSize, orxConfig_GetString(szConfigPositionTextFormat), vPosition.fX, vPosition.fY, vPosition.fZ);

    // Sets property text
    SetPropertyText(_poObject, acBuffer);

    // Pops previous section
    orxConfig_PopSection();
  }
  else
  {
    // Clears property text
    SetPropertyText();
  }
}

orxSTATUS ScrollEd::AddActionDisplay(const orxSTRING _zAction) const
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Has valid action
  if(_zAction)
  {
    orxOBJECT *pstActionText;

    // Creates action test
    pstActionText = orxObject_CreateFromConfig(szConfigActionText);

    // Valid?
    if(pstActionText)
    {
      // Updates its content
      orxObject_SetTextString(pstActionText, _zAction);

      // Updates result
      eResult = orxSTATUS_SUCCESS;
    }
  }

  // Done!
  return eResult;
}

void ScrollEd::SetSelection(ScrollObject *_poObject)
{
  // New selection?
  if(_poObject != mpoSelection)
  {
    // Deletes cursor
    DeleteCursorObject();

    // Removes selected model
    mpoSelectedModel = orxNULL;

    // Has a previous selection?
    if(mpoSelection)
    {
      // Removes its shader
      mpoSelection->RemoveShader("SelectionShader");
    }

    // Stores new selection
    mpoSelection = _poObject;

    // Valid?
    if(mpoSelection)
    {
      // Adds selection FX
      mpoSelection->AddShader("SelectionShader");

      // Displays its name
      DisplayObjectName(mpoSelection, orxTRUE);
    }
    else
    {
      // Hides name
      DisplayObjectName();
    }
  }
}

orxOBJECT *ScrollEd::PickObject(const orxVECTOR &_rvPos, orxU32 _u32GroupID) const
{
  orxOBJECT  *pstResult;
  orxVECTOR   vPick;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Sets picking vector
  orxVector_Copy(&vPick, &_rvPos);
  vPick.fZ = roGame.GetLayerDepth(mu32LayerIndex);

  // Updates picking position
  vPick.fZ -= orx2F(0.001f);

  // Updates result
  pstResult = orxObject_Pick(&vPick, _u32GroupID);

  // Has result?
  if(pstResult)
  {
    // Not in the current layer?
    if((!orxObject_GetWorldPosition(pstResult, &vPick) || (vPick.fZ > (roGame.GetLayerDepth(mu32LayerIndex + 1) - orx2F(0.01f)))))
    {
      // Cleans result
      pstResult = orxNULL;
    }
    else
    {
      // Gets its root
      for(orxOBJECT *pstOwner = orxOBJECT(orxObject_GetOwner(pstResult));
          pstOwner;
          pstResult = pstOwner, pstOwner = orxOBJECT(orxObject_GetOwner(pstOwner)));
    }
  }

  // Done!
  return pstResult;
}

ScrollObject *ScrollEd::AddObject(const ScrollObject *_poModel)
{
  ScrollObject *poResult = orxNULL;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Creates object
  poResult = roGame.CreateObject(_poModel->GetModelName(), ScrollObject::FlagSave);

  // Valid?
  if(poResult)
  {
    orxVECTOR   vPos, vScale;
    orxFLOAT    fRepeatX, fRepeatY;
    orxCOLOR    stColor;
    orxOBJECT  *pstOrxObject, *pstModelOrxObject;

    // Gets internal objects
    pstOrxObject      = poResult->GetOrxObject();
    pstModelOrxObject = _poModel->GetOrxObject();

    // Gets model color
    if(roGame.GetObjectConfigColor(_poModel, stColor))
    {
      // Applies it
      roGame.SetObjectConfigColor(poResult, stColor);
    }

    // Updates it
    orxObject_GetPosition(pstModelOrxObject, &vPos);
    vPos.fZ = roGame.GetLayerDepth(mu32LayerIndex);
    orxObject_GetScale(pstModelOrxObject, &vScale);
    orxObject_SetRotation(pstOrxObject, orxObject_GetRotation(pstModelOrxObject));
    orxObject_SetSmoothing(pstOrxObject, orxObject_GetSmoothing(pstModelOrxObject));

    // Updates its differential mode
    poResult->SetDifferentialMode(roGame.IsDifferentialMode());

    // Uses differential scrolling?
    if(roGame.IsDifferentialMode())
    {
      // Gets depth-corrected scale & position
      GetDepthCorrectedScale(vPos.fZ, vScale);
      GetDepthCorrectedPosition(vPos.fZ, vPos);
    }

    // Updates object's position & scale
    orxObject_SetPosition(pstOrxObject, &vPos);
    orxObject_SetScale(pstOrxObject, &vScale);

    // Updates its flags
    poResult->SetFlags(_poModel->GetFlags() | ScrollObject::FlagSave);

    // Use tiling?
    if(poResult->TestFlags(ScrollObject::FlagTiled))
    {
      // Gets tiling values
      orxObject_GetRepeat(pstModelOrxObject, &fRepeatX, &fRepeatY);

      // Applies it to object
      orxObject_SetRepeat(pstOrxObject, fRepeatX, fRepeatY);
    }
  }

  // Done!
  return poResult;
}

void ScrollEd::RemoveObject(ScrollObject *_poObject)
{
  // Was selected?
  if(_poObject == mpoSelection)
  {
    // Clears selection
    SetSelection();
  }

  // Deletes it
  ScrollBase::GetInstance().DeleteObject(_poObject);
}

orxFLOAT ScrollEd::GetDepthCoef(orxFLOAT _fDepth) const
{
  orxVECTOR vCameraPosition;
  orxAABOX  stFrustum;
  orxFLOAT  fRelativeDepth, fCameraDepth, fResult;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Gets camera position
  orxFrame_GetPosition(orxCamera_GetFrame(roGame.GetMainCamera()), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

  // Gets camera frustum
  orxCamera_GetFrustum(roGame.GetMainCamera(), &stFrustum);

  // Gets camera depth
  fCameraDepth = stFrustum.vBR.fZ - stFrustum.vTL.fZ;

  // Gets relative depth
  fRelativeDepth = _fDepth - vCameraPosition.fZ;

  // Near space?
  if(fRelativeDepth < (orx2F(0.5f) * fCameraDepth))
  {
    // Updates result
    fResult = (orx2F(0.5f) * fCameraDepth) / fRelativeDepth;
  }
  // Far space
  else
  {
    // Updates result
    fResult = (fCameraDepth - fRelativeDepth) / (orx2F(0.5f) * fCameraDepth);
  }

  // Done!
  return fResult;
}

orxVECTOR &ScrollEd::GetDepthCorrectedPosition(orxFLOAT _fDepth, orxVECTOR &_rvPos) const
{
  orxVECTOR vCameraCenter, vCameraPos;
  orxAABOX  stFrustum;
  orxFLOAT  fRecDepthCoef;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Gets reciprocal depth coef
  fRecDepthCoef = orxFLOAT_1 / GetDepthCoef(_fDepth);

  // Gets camera frustum and position
  orxCamera_GetFrustum(roGame.GetMainCamera(), &stFrustum);
  orxCamera_GetPosition(roGame.GetMainCamera(), &vCameraPos);

  // Gets camera center
  orxVector_Add(&vCameraCenter, &(stFrustum.vTL), &(stFrustum.vBR));
  orxVector_Mulf(&vCameraCenter, &vCameraCenter, orx2F(0.5f));
  orxVector_Add(&vCameraCenter, &vCameraCenter, &vCameraPos);

  // Gets position in camera space
  orxVector_Sub(&_rvPos, &_rvPos, &vCameraCenter);

  // Corrects it
  _rvPos.fX *= fRecDepthCoef;
  _rvPos.fY *= fRecDepthCoef;

  // Gets position in world space
  orxVector_Add(&_rvPos, &_rvPos, &vCameraCenter);

  // Done!
  return _rvPos;
}

orxVECTOR &ScrollEd::GetDepthCorrectedScale(orxFLOAT _fDepth, orxVECTOR &_rvScale) const
{
  orxFLOAT fRecDepthCoef;

  // Gets reciprocal depth coef
  fRecDepthCoef = orxFLOAT_1 / GetDepthCoef(_fDepth);

  // Gets corrected scale
  _rvScale.fX = fRecDepthCoef * _rvScale.fX;
  _rvScale.fY = fRecDepthCoef * _rvScale.fY;

  // Done!
  return _rvScale;
}

void ScrollEd::UpdateToggle(const orxSTRING _zToggleName, Flag _xToggleFlag)
{
  Flag        xCurrentFlag;
  ButtonData *pstButtonData;

  // Checks
  orxASSERT(orxFLAG_TEST(_xToggleFlag, MaskToggle));

  // Gets current toggle flag
  xCurrentFlag = GetFlags(MaskToggle);

  // Has new active status?
  if(orxInput_IsActive(_zToggleName) && orxInput_HasNewStatus(_zToggleName))
  {
    // Updates current mode
    SetFlags(_xToggleFlag, MaskToggle);
  }

  // Gets corresponding button
  pstButtonData = GetActiveButton(_zToggleName);

  // Found?
  if(pstButtonData)
  {
    // Updates its status
    orxObject_Enable(pstButtonData->mpstObject, TestFlags(_xToggleFlag));
  }
}

ScrollEd::ButtonData *ScrollEd::GetActiveButton(const orxSTRING _zAction) const
{
  ButtonData *pstButtonData, *pstResult = orxNULL;

  // For all buttons
  for(pstButtonData = (ButtonData *)orxLinkList_GetFirst(&mstButtonList);
      pstButtonData;
      pstButtonData = (ButtonData *)orxLinkList_GetNext(&pstButtonData->mstNode))
  {
    // Is active button?
    if(!orxString_Compare(pstButtonData->mzName, pstButtonData->mzActive))
    {
      // Matches?
      if(!orxString_Compare(_zAction, pstButtonData->mzAction))
      {
        // Updates result
        pstResult = pstButtonData;
        break;
      }
    }
  }

  // Done!
  return pstResult;
}
