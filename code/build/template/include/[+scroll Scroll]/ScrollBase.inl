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

#include <stddef.h>


// Deactivates invalid offsetof warnings for GCC & Clang for the rest of this file
#if defined(__orxGCC__)
  #pragma GCC system_header
#elif defined(__orxLLVM__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif


//! Constants
const orxSTRING ScrollBase::szInputSetGame                    = "ScrollInput";
const orxSTRING ScrollBase::szConfigClockFrequency            = "ClockFrequency";
const orxSTRING ScrollBase::szConfigSectionMap                = "Map";
const orxSTRING ScrollBase::szConfigMainViewport              = "MainViewport";
const orxSTRING ScrollBase::szConfigCameraPosition            = "CameraPosition";
const orxSTRING ScrollBase::szConfigCameraZoom                = "CameraZoom";
const orxSTRING ScrollBase::szConfigDifferential              = "Differential";
const orxSTRING ScrollBase::szConfigLayerNumber               = "LayerNumber";
const orxSTRING ScrollBase::szConfigNoSave                    = "NoSave";
const orxSTRING ScrollBase::szConfigSectionScrollObject       = "ScrollObject";
const orxSTRING ScrollBase::szConfigScrollObjectNumber        = "ObjectNumber";
const orxSTRING ScrollBase::szConfigScrollObjectPrefix        = "SO:";
const orxSTRING ScrollBase::szConfigScrollObjectFormat        = "SO:%08u";
const orxSTRING ScrollBase::szConfigScrollObjectRuntimeFormat = "RT:%08u";
const orxSTRING ScrollBase::szConfigScrollObjectName          = "Name";
const orxSTRING ScrollBase::szConfigScrollObjectPosition      = "Position";
const orxSTRING ScrollBase::szConfigScrollObjectRotation      = "Rotation";
const orxSTRING ScrollBase::szConfigScrollObjectScale         = "Scale";
const orxSTRING ScrollBase::szConfigScrollObjectColor         = "Color";
const orxSTRING ScrollBase::szConfigScrollObjectAlpha         = "Alpha";
const orxSTRING ScrollBase::szConfigScrollObjectSmoothing     = "Smoothing";
const orxSTRING ScrollBase::szConfigScrollObjectTiling        = "Tiling";
const orxSTRING ScrollBase::szConfigScrollObjectPausable      = "Pausable";


//! Static variables
ScrollBase *ScrollBase::spoInstance                           = orxNULL;

template<class O>
ScrollObjectBinder<O> *ScrollObjectBinder<O>::spoInstance     = orxNULL;


//! Code
ScrollBase &ScrollBase::GetInstance()
{
  // Checks
  orxASSERT(spoInstance);

  return *spoInstance;
}

ScrollBase::ScrollBase() : mzMapName(orxNULL), mzCurrentCreateObject(orxNULL), mzCurrentDeleteObject(orxNULL),
                           mpstMainViewport(orxNULL), mpstMainCamera(orxNULL), mpfnCustomMapSaveFilter(orxNULL),
                           mu32NextObjectID(0), mu32RuntimeObjectID(0), mu32LayerNumber(1), mu32FrameCount(0),
                           mbEditorMode(orxFALSE), mbDifferentialMode(orxFALSE), mbObjectListLocked(orxFALSE), mbIsRunning(orxFALSE), mbIsPaused(orxFALSE)
{
}

ScrollBase::~ScrollBase()
{
}

void ScrollBase::Execute(int argc, char **argv)
{
  // Sets bootstrap
  orxConfig_SetBootstrap(&ScrollBase::StaticBootstrap);

  // Inits encrypt key
  orxConfig_SetEncryptionKey(GetEncryptionKey());

  // Executes orx
  orx_Execute(argc, argv, StaticInit, StaticRun, StaticExit);
}

ScrollObject *ScrollBase::CreateObject(const orxSTRING _zModelName, ScrollObject::Flag _xFlags, const orxSTRING _zInstanceName)
{
  ScrollObject *poResult = orxNULL;

  // Checks
  orxASSERT((_xFlags & ScrollObject::MaskAll) == _xFlags);

  // Valid?
  if(_zModelName && (_zModelName != orxSTRING_EMPTY))
  {
    orxCHAR                 zInstanceName[32];
    const orxSTRING         zPreviousObject;
    ScrollObjectBinderBase *poBinder;

    // Gets binder
    poBinder = ScrollObjectBinderBase::GetBinder(orxFLAG_TEST(_xFlags, ScrollObject::FlagSave | ScrollObject::FlagRunTime) ? _zModelName : orxSTRING_EMPTY);

    // Stores current object
    zPreviousObject = mzCurrentCreateObject;

    // Flags current object creation
    mzCurrentCreateObject = _zModelName;

    // Uses it
    poResult = poBinder->CreateObject(_zModelName, _zInstanceName ? _zInstanceName : GetNewObjectName(zInstanceName, (_xFlags & ScrollObject::FlagRunTime) ? orxTRUE : orxFALSE), _xFlags);

    // Restores previous object
    mzCurrentCreateObject = zPreviousObject;

    // Valid?
    if(poResult)
    {
      // Savable or runtime object?
      if(poResult->TestFlags(ScrollObject::FlagSave | ScrollObject::FlagRunTime))
      {
        // Calls create callback
        OnObjectCreate(poResult);
      }

      // Editor mode?
      if(IsEditorMode())
      {
        // For all linkable structures
        for(orxSTRUCTURE_ID eID = (orxSTRUCTURE_ID)0; eID < orxSTRUCTURE_ID_LINKABLE_NUMBER; eID = (orxSTRUCTURE_ID)(eID + 1))
        {
          // Not graphic, anim pointer nor frame?
          if((eID != orxSTRUCTURE_ID_GRAPHIC) && (eID != orxSTRUCTURE_ID_ANIMPOINTER) && (eID != orxSTRUCTURE_ID_FRAME))
          {
            // Unlinks it
            orxObject_UnlinkStructure(poResult->GetOrxObject(), eID);
          }
        }
      }
    }
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollBase::CreateObject(orxOBJECT *_pstOrxObject, ScrollObject::Flag _xFlags, const orxSTRING _zInstanceName)
{
  ScrollObject *poResult = orxNULL;

  // Checks
  orxASSERT((_xFlags & ScrollObject::MaskAll) == _xFlags);

  // Valid?
  if(_pstOrxObject)
  {
    orxCHAR                 zInstanceName[32];
    ScrollObjectBinderBase *poBinder;
    const orxSTRING         zObjectName;

    // Checks
    orxSTRUCTURE_ASSERT(_pstOrxObject);

    // Gets its name
    zObjectName = orxObject_GetName(_pstOrxObject);

    // Gets binder
    poBinder = ScrollObjectBinderBase::GetBinder(zObjectName);

    // Uses it
    poResult = poBinder->CreateObject(_pstOrxObject, _zInstanceName ? _zInstanceName : GetNewObjectName(zInstanceName, (_xFlags & ScrollObject::FlagRunTime) ? orxTRUE : orxFALSE), _xFlags);

    // Valid?
    if(poResult)
    {
      // Updates it
      poResult->SetFlags(_xFlags, ScrollObject::MaskAll);

      // Savable or runtime object?
      if(poResult->TestFlags(ScrollObject::FlagSave | ScrollObject::FlagRunTime))
      {
        // Calls create callback
        OnObjectCreate(poResult);
      }

      // Editor mode?
      if(IsEditorMode())
      {
        // For all linkable structures
        for(orxSTRUCTURE_ID eID = (orxSTRUCTURE_ID)0; eID < orxSTRUCTURE_ID_LINKABLE_NUMBER; eID = (orxSTRUCTURE_ID)(eID + 1))
        {
          // Not graphic, anim pointer nor frame?
          if((eID != orxSTRUCTURE_ID_GRAPHIC) && (eID != orxSTRUCTURE_ID_ANIMPOINTER) && (eID != orxSTRUCTURE_ID_FRAME))
          {
            // Unlinks it
            orxObject_UnlinkStructure(poResult->GetOrxObject(), eID);
          }
        }
      }
    }
  }

  // Done!
  return poResult;
}

void ScrollBase::DeleteObject(ScrollObject *_poObject)
{
  // Valid?
  if(_poObject)
  {
    // Object list not locked?
    if(!mbObjectListLocked)
    {
      ScrollObjectBinderBase *poBinder;
      const orxSTRING         zPreviousObject;

      // Gets binder
      poBinder = ScrollObjectBinderBase::GetBinder(_poObject->TestFlags(ScrollObject::FlagSave | ScrollObject::FlagRunTime) ? _poObject->GetModelName() : orxSTRING_EMPTY);

      // Stores current object
      zPreviousObject = mzCurrentDeleteObject;

      // Flags current object deletion
      mzCurrentDeleteObject = _poObject->GetModelName();

      // Deletes it
      poBinder->DeleteObject(_poObject);

      // Restores previous object
      mzCurrentDeleteObject = zPreviousObject;
    }
    else
    {
      // Resets its life time
      orxObject_SetLifeTime(_poObject->GetOrxObject(), orxFLOAT_0);
    }
  }
}

ScrollObject *ScrollBase::PickObject(const orxVECTOR &_rvPosition, orxSTRINGID _stGroupID) const
{
  orxOBJECT    *pstObject;
  ScrollObject *poResult = orxNULL;

  // Picks object
  pstObject = orxObject_Pick(&_rvPosition, _stGroupID);

  // Found?
  if(pstObject)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstObject);
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollBase::PickObject(const orxVECTOR &_rvPosition, const orxVECTOR *_avOffsetList, orxU32 _u32ListSize, orxSTRINGID _stGroupID) const
{
  ScrollObject *poResult = orxNULL;

  // Picks initial position
  poResult = PickObject(_rvPosition, _stGroupID);

  // For all offsets, till found
  for(orxU32 i = 0; (poResult == orxNULL) && (i < _u32ListSize); i++)
  {
    orxVECTOR vPickPosition;

    // Inits pick position
    orxVector_Add(&vPickPosition, &_rvPosition, &_avOffsetList[i]);

    // Updates result
    poResult = PickObject(vPickPosition, _stGroupID);
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollBase::PickObject(const orxVECTOR &_rvPosition, const orxVECTOR &_rvExtent, orxSTRINGID _stGroupID) const
{
  orxOBOX       stBox;
  ScrollObject *poResult;

  // Inits Box
  orxVector_Copy(&stBox.vPosition, &_rvPosition);
  orxVector_Copy(&stBox.vPivot, &_rvExtent);
  orxVector_Set(&stBox.vX, orx2F(2.0f) * _rvExtent.fX, orxFLOAT_0, orxFLOAT_0);
  orxVector_Set(&stBox.vY, orxFLOAT_0, orx2F(2.0f) * _rvExtent.fY, orxFLOAT_0);
  orxVector_Set(&stBox.vZ, orxFLOAT_0, orxFLOAT_0, orx2F(2.0f) * _rvExtent.fZ);

  // Updates result
  poResult = PickObject(stBox, _stGroupID);

  // Done!
  return poResult;
}

ScrollObject *ScrollBase::PickObject(const orxOBOX &_rstBox, orxSTRINGID _stGroupID) const
{
  orxOBJECT    *pstObject;
  ScrollObject *poResult = orxNULL;

  // Picks object
  pstObject = orxObject_BoxPick(&_rstBox, _stGroupID);

  // Found?
  if(pstObject)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstObject);
  }

  // Done!
  return poResult;
}

orxCOLOR *ScrollBase::GetObjectConfigColor(const ScrollObject *_poObject, orxCOLOR &_rstColor)
{
  orxCOLOR *pstResult = orxNULL;

  // Can select object's config section
  if(_poObject && (orxConfig_PushSection(_poObject->GetName()) != orxSTATUS_FAILURE))
  {
    // Has alpha?
    if(orxConfig_HasValue(szConfigScrollObjectAlpha))
    {
      // Stores it
      _rstColor.fAlpha = orxConfig_GetFloat(szConfigScrollObjectAlpha);

      // Updates result
      pstResult = &_rstColor;
    }
    else
    {
      // Uses default alpha
      _rstColor.fAlpha = orxFLOAT_1;
    }
    if(orxConfig_GetVector(szConfigScrollObjectColor, &_rstColor.vRGB))
    {
      // Updates result
      pstResult = &_rstColor;
    }
    else
    {
      // Uses white color
      orxVector_Copy(&_rstColor.vRGB, &orxVECTOR_WHITE);
    }

    // Pops config section
    orxConfig_PopSection();
  }

  // Done!
  return pstResult;
}

orxSTATUS ScrollBase::SetObjectConfigColor(ScrollObject *_poObject, const orxCOLOR &_rstColor)
{
  orxSTATUS eResult;

  // Can select object's config section
  if(_poObject && (orxConfig_PushSection(_poObject->GetName()) != orxSTATUS_FAILURE))
  {
    // Sets alpha
    orxConfig_SetFloat(szConfigScrollObjectAlpha, _rstColor.fAlpha);

    // Sets color
    orxConfig_SetVector(szConfigScrollObjectColor, &_rstColor.vRGB);

    // Sets real time color
    _poObject->SetColor(_rstColor);

    // Updates result
    eResult = orxSTATUS_SUCCESS;

    // Pops config section
    orxConfig_PopSection();
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

orxSTATUS ScrollBase::SetMapName(const orxSTRING _zMapName)
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

const orxSTRING ScrollBase::GetMapName() const
{
  return mzMapName;
}

const orxSTRING ScrollBase::GetMapShortName() const
{
  orxS32    s32LastSeparator, i;
  orxSTRING zResult;

  // Gets last directory separator
  for(s32LastSeparator = -1, i = orxString_SearchCharIndex(mzMapName, orxCHAR_DIRECTORY_SEPARATOR_LINUX, 0);
      i >= 0;
      s32LastSeparator = i, i = orxString_SearchCharIndex(mzMapName, orxCHAR_DIRECTORY_SEPARATOR_LINUX, i + 1));

  // Not found?
  if(s32LastSeparator < 0)
  {
    // Gets last directory separator
    for(s32LastSeparator = -1, i = orxString_SearchCharIndex(mzMapName, orxCHAR_DIRECTORY_SEPARATOR_WINDOWS, 0);
        i >= 0;
        s32LastSeparator = i, i = orxString_SearchCharIndex(mzMapName, orxCHAR_DIRECTORY_SEPARATOR_WINDOWS, i + 1));
  }

  // Has directory?
  if(s32LastSeparator >= 0)
  {
    // Updates text's content
    zResult = mzMapName + s32LastSeparator + 1;
  }
  else
  {
    // Updates result
    zResult = mzMapName;
  }

  // Done!
  return zResult;
}

orxSTATUS ScrollBase::LoadMap()
{
  orxCHAR   acBuffer[32];
  orxU32    i;
  orxSTATUS eResult;

  // Object list not locked and has a valid map name?
  if(!mbObjectListLocked && mzMapName && (mzMapName != orxSTRING_EMPTY))
  {
    // Updates result
    eResult = orxSTATUS_SUCCESS;

    // For all current objects
    for(ScrollObject *poObject = GetNextObject(), *poNextObject = GetNextObject(poObject);
        poObject;
        poObject = poNextObject, poNextObject = GetNextObject(poNextObject))
    {
      // Is it a map object?
      if(poObject && poObject->TestFlags(ScrollObject::FlagSave))
      {
        // Deletes it
        DeleteObject(poObject);
      }
    }

    // Loads map
    if(orxConfig_Load(mzMapName))
    {
      orxS32 s32ScrollObjectCount, s32ScrollObjectNumber;

      // Selects map section
      orxConfig_SelectSection(szConfigSectionMap);

      // Updates differential mode
      mbDifferentialMode = orxConfig_GetBool(szConfigDifferential);

      // Gets number of objects to load
      orxConfig_SelectSection(szConfigSectionMap);
      s32ScrollObjectNumber = orxConfig_GetS32(szConfigScrollObjectNumber);

      // For all objects to load
      for(s32ScrollObjectCount = 0, i = 0, orxString_NPrint(acBuffer, sizeof(acBuffer), szConfigScrollObjectFormat, i);
          s32ScrollObjectCount < s32ScrollObjectNumber;
          i++, orxString_NPrint(acBuffer, sizeof(acBuffer), szConfigScrollObjectFormat, i))
      {
        // Has section?
        if(orxConfig_HasSection(acBuffer))
        {
          ScrollObject *poObject;

          // Selects its section
          orxConfig_SelectSection(acBuffer);

          // Creates it
          poObject = CreateObject(orxConfig_GetString(szConfigScrollObjectName), ScrollObject::FlagSave, acBuffer);

          // Valid?
          if(poObject)
          {
            orxVECTOR   vPos, vScale;
            orxCOLOR    stColor;
            orxOBJECT  *pstOrxObject;
            orxFRAME   *pstFrame;
            orxBOOL     bApplyColor = orxFALSE;

            // Gets internal object
            pstOrxObject = poObject->GetOrxObject();

            // Updates it
            orxObject_SetPosition(pstOrxObject, orxConfig_GetVector(szConfigScrollObjectPosition, &vPos));
            orxObject_SetScale(pstOrxObject, orxConfig_GetVector(szConfigScrollObjectScale, &vScale));
            orxObject_SetRotation(pstOrxObject, orxConfig_GetFloat(szConfigScrollObjectRotation));

            // Gets its frame
            pstFrame = orxOBJECT_GET_STRUCTURE(pstOrxObject, FRAME);

            // Updates its differential mode
            poObject->SetDifferentialMode(mbDifferentialMode);

            // Has alpha?
            if(orxConfig_HasValue(szConfigScrollObjectAlpha))
            {
              stColor.fAlpha  = orxConfig_GetFloat(szConfigScrollObjectAlpha);
              bApplyColor     = orxTRUE;
            }
            else
            {
              stColor.fAlpha = orxFLOAT_1;
            }
            if(orxConfig_GetVector(szConfigScrollObjectColor, &stColor.vRGB))
            {
              bApplyColor     = orxTRUE;
            }
            else
            {
              // Uses white color
              orxVector_Copy(&stColor.vRGB, &orxVECTOR_WHITE);
            }

            // Should apply color?
            if(bApplyColor)
            {
              // Applies it
              poObject->SetColor(stColor);
            }

            // Gets its smoothing
            if(orxConfig_GetBool(szConfigScrollObjectSmoothing))
            {
              // Applies it to object
              orxObject_SetSmoothing(pstOrxObject, orxDISPLAY_SMOOTHING_ON);
              poObject->SetFlags(ScrollObject::FlagSmoothed);
            }
            else
            {
              // Resets its smoothing
              orxObject_SetSmoothing(pstOrxObject, orxDISPLAY_SMOOTHING_OFF);
            }

            // Gets its tiling
            if(orxConfig_GetBool(szConfigScrollObjectTiling))
            {
              // Applies it to object
              orxObject_SetRepeat(pstOrxObject, orxMath_Abs(vScale.fX), orxMath_Abs(vScale.fY));
              poObject->SetFlags(ScrollObject::FlagTiled);
            }
            else
            {
              // Resets its tiling
              orxObject_SetRepeat(pstOrxObject, orxFLOAT_1, orxFLOAT_1);
            }
          }
          else
          {
            // Logs message
            orxLOG("Couldn't load object [%s]: invalid model <%s>.", acBuffer, orxConfig_GetString(szConfigScrollObjectName));

            // Clears its section
            orxConfig_ClearSection(acBuffer);
          }

          // Updates count
          s32ScrollObjectCount++;
        }
      }

      // Updates next object ID
      mu32NextObjectID = orxMAX(mu32NextObjectID, i);

      // Selects map section
      orxConfig_SelectSection(szConfigSectionMap);

      // Has layer number?
      if(orxConfig_HasValue(szConfigLayerNumber))
      {
        // Stores it
        SetLayerNumber(orxConfig_GetU32(szConfigLayerNumber));
      }

      // Has camera position?
      if(orxConfig_HasValue(szConfigCameraPosition))
      {
        orxVECTOR vPos;

        // Updates camera position
        orxConfig_GetVector(szConfigCameraPosition, &vPos);
        orxCamera_SetPosition(mpstMainCamera, &vPos);
      }

      // Has camera zoom?
      if(orxConfig_HasValue(szConfigCameraZoom))
      {
        // Updates camera zoom
        orxCamera_SetZoom(mpstMainCamera, orxConfig_GetFloat(szConfigCameraZoom));
      }
    }

    // Calls map load callback
    OnMapLoad();
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

orxSTATUS ScrollBase::SaveMap(orxBOOL _bEncrypt, const orxCONFIG_SAVE_FUNCTION _pfnMapSaveFilter)
{
  ScrollObject *poObject;
  orxU32        u32Count;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  // Valid?
  if(mzMapName && (mzMapName != orxSTRING_EMPTY))
  {
    orxVECTOR vPos;

    // Stores camera position
    orxConfig_SetVector(szConfigCameraPosition, orxCamera_GetPosition(mpstMainCamera, &vPos));

    // Stores camera zoom
    orxConfig_SetFloat(szConfigCameraZoom, orxCamera_GetZoom(mpstMainCamera));

    // Stores differential mode
    orxConfig_SetBool(szConfigDifferential, mbDifferentialMode);

    // Stores layer number
    orxConfig_SetU32(szConfigLayerNumber, mu32LayerNumber);

    // Locks object list
    mbObjectListLocked = orxTRUE;

    // For all objects
    for(poObject = GetNextObject(), u32Count = 0;
        poObject;
        poObject = GetNextObject(poObject))
    {
      // Selects its section
      orxConfig_SelectSection(poObject->GetName());

      // Should save it?
      if(poObject->TestFlags(ScrollObject::FlagSave))
      {
        orxCOLOR    stColor;
        orxVECTOR   vTemp;
        orxOBJECT  *pstOrxObject;

        // Gets internal object
        pstOrxObject = poObject->GetOrxObject();

        // Adds all its properties
        orxConfig_SetString(szConfigScrollObjectName, poObject->GetModelName());
        if(orxObject_GetPosition(pstOrxObject, &vTemp))
        {
          orxConfig_SetVector(szConfigScrollObjectPosition, &vTemp);
        }
        if(orxObject_GetScale(pstOrxObject, &vTemp))
        {
          orxConfig_SetVector(szConfigScrollObjectScale, &vTemp);
        }
        orxConfig_SetFloat(szConfigScrollObjectRotation, orxObject_GetRotation(pstOrxObject));
        if(GetObjectConfigColor(poObject, stColor))
        {
          orxConfig_SetVector(szConfigScrollObjectColor, &stColor.vRGB);
          orxConfig_SetFloat(szConfigScrollObjectAlpha, stColor.fAlpha);
        }
        orxConfig_SetBool(szConfigScrollObjectSmoothing, poObject->TestFlags(ScrollObject::FlagSmoothed));
        orxConfig_SetBool(szConfigScrollObjectTiling, poObject->TestFlags(ScrollObject::FlagTiled));

        // Updates count
        u32Count++;
      }
      else
      {
        // Don't save
        orxConfig_SetBool(szConfigNoSave, orxTRUE);
      }
    }

    // Unlocks object list
    mbObjectListLocked = orxFALSE;

    // Restores map section
    orxConfig_SelectSection(szConfigSectionMap);

    // Stores game object count
    orxConfig_SetS32(szConfigScrollObjectNumber, u32Count);

    // Calls save callback
    OnMapSave(_bEncrypt);

    // Stores custom save filter
    mpfnCustomMapSaveFilter = _pfnMapSaveFilter;

    // Saves to file
    eResult = orxConfig_Save(mzMapName, _bEncrypt, StaticMapSaveFilter);

    // Clears custom save filter
    mpfnCustomMapSaveFilter = orxNULL;
  }

  // Done!
  return eResult;
}

orxSTATUS ScrollBase::ResetMap()
{
  orxVECTOR vPos;
  orxFLOAT  fZoom;
  orxSTATUS eResult;

  // Backups camera position & zoom
  orxCamera_GetPosition(mpstMainCamera, &vPos);
  fZoom = orxCamera_GetZoom(mpstMainCamera);

  // Reloads it
  eResult = LoadMap();

  // Restores main camera position & zoom
  orxCamera_SetPosition(mpstMainCamera, &vPos);
  orxCamera_SetZoom(mpstMainCamera, fZoom);

  // Done!
  return eResult;
}

void ScrollBase::SetLayerNumber(orxU32 _u32LayerNumber)
{
  orxAABOX    stFrustum;
  orxVECTOR   vPos;
  orxFLOAT    fDepth, fWidth, fHeight;
  orxCAMERA  *pstCamera;

  // Is even?
  if(!(_u32LayerNumber & 1))
  {
    // Adds an extra layer
    mu32LayerNumber = _u32LayerNumber + 1;

    // Logs message
    orxLOG("Can't use an even number of layers, adding an extra one => %d layers will be used.", mu32LayerNumber);
  }
  else
  {
    // Stores it
    mu32LayerNumber = _u32LayerNumber;
  }

  // Gets main camera
  pstCamera = GetMainCamera();

  // Gets its frustum & position
  orxCamera_GetFrustum(pstCamera, &stFrustum);
  orxCamera_GetPosition(pstCamera, &vPos);

  // Gets its size & depth
  fWidth  = stFrustum.vBR.fX - stFrustum.vTL.fX;
  fHeight = stFrustum.vBR.fY - stFrustum.vTL.fY;
  fDepth  = stFrustum.vBR.fZ - stFrustum.vTL.fZ;

  // Calculates correct position for the camera
  vPos.fZ = -orx2F(0.5f) * fDepth;

  // Updates camera position
  orxCamera_SetPosition(pstCamera, &vPos);

  // Updates camera frustum
  orxCamera_SetFrustum(pstCamera, fWidth, fHeight, orxFLOAT_0, fDepth);
}

orxU32 ScrollBase::GetLayerNumber() const
{
  // Done!
  return mu32LayerNumber;
}

orxU32 ScrollBase::GetLayer(const orxVECTOR &_rvPosition) const
{
  orxAABOX  stFrustum;
  orxVECTOR vCameraPos;
  orxU32    u32Result;

  // Gets asbolute frustum
  orxCamera_GetFrustum(GetMainCamera(), &stFrustum);
  orxCamera_GetPosition(GetMainCamera(), &vCameraPos);
  orxVector_Add(&(stFrustum.vTL), &(stFrustum.vTL), &vCameraPos);
  orxVector_Add(&(stFrustum.vBR), &(stFrustum.vBR), &vCameraPos);

  // Updates result
  u32Result = orxF2U((_rvPosition.fZ - stFrustum.vTL.fZ) / (stFrustum.vBR.fZ - stFrustum.vTL.fZ) * orxU2F(mu32LayerNumber));

  // Done!
  return u32Result;
}

orxFLOAT ScrollBase::GetLayerDepth(orxU32 _u32Layer) const
{
  orxAABOX stFrustum;
  orxFLOAT fResult;

  // Corrects range
  _u32Layer = orxMIN(_u32Layer, mu32LayerNumber);

  // Gets camera depth
  orxCamera_GetFrustum(GetMainCamera(), &stFrustum);

  // Game layer?
  if(((mu32LayerNumber - 1) >> 1) == _u32Layer)
  {
    fResult = orxFLOAT_0;
  }
  else
  {
    orxVECTOR vCameraPos;

    // Gets camera position
    orxCamera_GetPosition(GetMainCamera(), &vCameraPos);

    // Updates result
    fResult = orxLERP(stFrustum.vTL.fZ, stFrustum.vBR.fZ, (orxU2F(_u32Layer) + orx2F(0.5f)) / orxU2F(mu32LayerNumber)) + vCameraPos.fZ;
  }

  // Done!
  return fResult;
}

orxSTATUS ScrollBase::StartGame()
{
  orxSTATUS eResult;

  // Not already running?
  if(!mbIsRunning)
  {
    // Updates result
    eResult = orxSTATUS_SUCCESS;

    // Locks object list
    mbObjectListLocked = orxTRUE;

    // For all objects
    for(ScrollObject *poObject = GetNextObject();
        poObject;
        poObject = GetNextObject(poObject))
    {
      // Calls its callback
      poObject->OnStartGame();
    }

    // Unlocks object list
    mbObjectListLocked = orxFALSE;

    // Updates running status
    mbIsRunning = orxTRUE;

    // Calls start game callback
    OnStartGame();
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

orxSTATUS ScrollBase::StopGame()
{
  orxSTATUS eResult;

  // Is running?
  if(mbIsRunning)
  {
    // Updates result
    eResult = orxSTATUS_SUCCESS;

    // Unpauses game
    PauseGame(orxFALSE);

    // Locks object list
    mbObjectListLocked = orxTRUE;

    // For all objects
    for(ScrollObject *poObject = GetNextObject();
        poObject;
        poObject = GetNextObject(poObject))
    {
      // Calls its callback
      poObject->OnStopGame();
    }

    // Unlocks object list
    mbObjectListLocked = orxFALSE;

    // Updates running status
    mbIsRunning = orxFALSE;

    // Calls stop game callback
    OnStopGame();
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

orxBOOL ScrollBase::IsGameRunning() const
{
  // Done!
  return mbIsRunning;
}

orxSTATUS ScrollBase::PauseGame(orxBOOL _bPause)
{
  orxSTATUS eResult;

  // Change status?
  if(_bPause != mbIsPaused)
  {
    // Disables physics simulation
    orxPhysics_EnableSimulation(!_bPause);

    // Locks object list
    mbObjectListLocked = orxTRUE;

    // For all objects
    for(ScrollObject *poObject = GetNextObject();
        poObject;
        poObject = GetNextObject(poObject))
    {
      // Is pausable?
      if(poObject->TestFlags(ScrollObject::FlagPausable))
      {
        // Calls its callback
        if(poObject->OnPauseGame(_bPause))
        {
          // Updates it
          orxObject_Pause(poObject->GetOrxObject(), _bPause);
        }
      }
    }

    // Unlocks object list
    mbObjectListLocked = orxFALSE;

    // Updates status
    mbIsPaused = _bPause;

    // Updates result
    eResult = orxSTATUS_SUCCESS;

    // Calls pause game callback
    OnPauseGame(_bPause);
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

orxBOOL ScrollBase::IsGamePaused() const
{
  // Done!
  return mbIsPaused;
}

orxU32 ScrollBase::GetFrameCount() const
{
  // Done!
  return mu32FrameCount;
}

orxVIEWPORT *ScrollBase::GetMainViewport() const
{
  // Done!
  return mpstMainViewport;
}

orxCAMERA *ScrollBase::GetMainCamera() const
{
  // Done!
  return mpstMainCamera;
}

ScrollObject *ScrollBase::GetObject(orxU64 _u64GUID) const
{
  orxOBJECT    *pstObject;
  ScrollObject *poResult = orxNULL;

  // Gets object
  pstObject = orxOBJECT(orxStructure_Get(_u64GUID));

  // Valid?
  if(pstObject)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstObject);
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollBase::GetNextObject(const ScrollObject *_poObject, orxBOOL _bChronological) const
{
  orxLINKLIST_NODE *pstNode;
  ScrollObject     *poResult;

  // Chronological order?
  if(_bChronological)
  {
    // First one?
    if(!_poObject)
    {
      // Gets its node
      pstNode = orxLinkList_GetFirst(&mstObjectChronoList);
    }
    else
    {
      // Gets next node
      pstNode = orxLinkList_GetNext(&_poObject->mstChronoNode);
    }

    // Updates result
    poResult = pstNode ? (ScrollObject *)((orxU8 *)pstNode - offsetof(ScrollObject, mstChronoNode)) : (ScrollObject *)orxNULL;
  }
  else
  {
    // First one?
    if(!_poObject)
    {
      // Gets its node
      pstNode = orxLinkList_GetFirst(&mstObjectList);
    }
    else
    {
      // Gets next node
      pstNode = orxLinkList_GetNext(&_poObject->mstNode);
    }

    // Updates result
    poResult = pstNode ? (ScrollObject *)((orxU8 *)pstNode - offsetof(ScrollObject, mstNode)) : (ScrollObject *)orxNULL;
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollBase::GetPreviousObject(const ScrollObject *_poObject, orxBOOL _bChronological) const
{
  orxLINKLIST_NODE *pstNode;
  ScrollObject     *poResult;

  // Chronological order?
  if(_bChronological)
  {
    // Last one?
    if(!_poObject)
    {
      // Gets its node
      pstNode = orxLinkList_GetLast(&mstObjectChronoList);
    }
    else
    {
      // Gets previous node
      pstNode = orxLinkList_GetPrevious(&_poObject->mstChronoNode);
    }

    // Updates result
    poResult = pstNode ? (ScrollObject *)((orxU8 *)pstNode - offsetof(ScrollObject, mstChronoNode)) : (ScrollObject *)orxNULL;
  }
  else
  {
    // Last one?
    if(!_poObject)
    {
      // Gets its node
      pstNode = orxLinkList_GetLast(&mstObjectList);
    }
    else
    {
      // Gets previous node
      pstNode = orxLinkList_GetPrevious(&_poObject->mstNode);
    }

    // Updates result
    poResult = pstNode ? (ScrollObject *)((orxU8 *)pstNode - offsetof(ScrollObject, mstNode)) : (ScrollObject *)orxNULL;
  }

  // Done!
  return poResult;
}

void ScrollBase::SetEditorMode(orxBOOL _bEnable)
{
  mbEditorMode = _bEnable;
}

orxBOOL ScrollBase::IsEditorMode() const
{
  return mbEditorMode;
}

void ScrollBase::SetDifferentialMode(orxBOOL _bEnable)
{
  mbDifferentialMode = _bEnable;
}

orxBOOL ScrollBase::IsDifferentialMode() const
{
  return mbDifferentialMode;
}

orxSTATUS ScrollBase::BaseInit()
{
  orxCLOCK *pstCoreClock;
  orxSTATUS eResult;

  // Binds default ScrollObject section
  orxConfig_PushSection(szConfigSectionScrollObject);
  orxConfig_PopSection();
  ScrollBindObject<ScrollObject>(szConfigSectionScrollObject);

  // Binds objects
  BindObjects();

  // Selects map section
  orxConfig_SelectSection(szConfigSectionMap);

  // Gets core clock
  pstCoreClock = orxClock_Get(orxCLOCK_KZ_CORE);

  // Registers update function
  eResult = ((orxClock_Register(pstCoreClock, StaticUpdate, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL) != orxSTATUS_FAILURE)
          && (orxClock_Register(pstCoreClock, StaticCameraUpdate, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_LOWER) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  // Successful?
  if(eResult != orxSTATUS_FAILURE)
  {
    // Adds event handler
    eResult = ((orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_ANIM, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_RENDER, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_SHADER, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_FX, StaticEventHandler) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    // Successful?
    if(eResult != orxSTATUS_FAILURE)
    {
      // Filters events
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_GAME_LOOP_START), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_OBJECT, orxNULL, orxEVENT_GET_FLAG(orxOBJECT_EVENT_CREATE) | orxEVENT_GET_FLAG(orxOBJECT_EVENT_DELETE), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_ANIM, orxNULL, orxEVENT_GET_FLAG(orxANIM_EVENT_STOP) | orxEVENT_GET_FLAG(orxANIM_EVENT_CUT) | orxEVENT_GET_FLAG(orxANIM_EVENT_LOOP) | orxEVENT_GET_FLAG(orxANIM_EVENT_UPDATE) | orxEVENT_GET_FLAG(orxANIM_EVENT_CUSTOM_EVENT), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_OBJECT_START), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_SHADER, orxNULL, orxEVENT_GET_FLAG(orxSHADER_EVENT_SET_PARAM), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_PHYSICS, orxNULL, orxEVENT_GET_FLAG(orxPHYSICS_EVENT_CONTACT_ADD) | orxEVENT_GET_FLAG(orxPHYSICS_EVENT_CONTACT_REMOVE), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_FX, orxNULL, orxEVENT_GET_FLAG(orxFX_EVENT_START) | orxEVENT_GET_FLAG(orxFX_EVENT_STOP) | orxEVENT_GET_FLAG(orxFX_EVENT_LOOP), orxEVENT_KU32_MASK_ID_ALL);

      // Clears object lists
      orxMemory_Zero(&mstObjectList, sizeof(orxLINKLIST));
      orxMemory_Zero(&mstObjectChronoList, sizeof(orxLINKLIST));

      // Creates main viewport
      mpstMainViewport = orxViewport_CreateFromConfig(szConfigMainViewport);

      // Gets main camera
      mpstMainCamera = orxViewport_GetCamera(mpstMainViewport);

      // Sets default layer
      SetLayerNumber(1);

      // Calls child init
      eResult = Init();

      // Successful?
      if(eResult != orxSTATUS_FAILURE)
      {
        // Updates running status
        mbIsRunning = orxFALSE;

        // Not in editor mode?
        if(!IsEditorMode())
        {
          // Starts game
          StartGame();
        }
      }
    }
    else
    {
      // Updates result
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

orxSTATUS ScrollBase::BaseRun()
{
  orxSTATUS eResult;

  // Calls child run
  eResult = Run();

  // Done!
  return eResult;
}

void ScrollBase::BaseExit()
{
  // Calls child exit
  Exit();

  // Clears map
  SetMapName(orxNULL);

  // Disables object create handler
  orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_OBJECT, orxNULL, orxEVENT_KU32_FLAG_ID_NONE, orxEVENT_GET_FLAG(orxOBJECT_EVENT_CREATE));

  // For all objects
  for(ScrollObject *poObject = GetNextObject(orxNULL, orxTRUE);
      poObject;
      poObject = GetNextObject(orxNULL, orxTRUE))
  {
    // Deletes it
    DeleteObject(poObject);
  }

  // Removes event handler
  orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_ANIM, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_SHADER, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_PHYSICS, StaticEventHandler);

  // Deletes binder's table
  ScrollObjectBinderBase::DeleteTable();
}

void ScrollBase::BaseUpdate(const orxCLOCK_INFO &_rstInfo)
{
  // Not paused?
  if(!mbIsPaused)
  {
    orxOBJECT *pstObject;

    // Locks object list
    mbObjectListLocked = orxTRUE;

    // For all enabled objects
    for(pstObject = orxObject_GetNextEnabled(orxNULL, orxSTRINGID_UNDEFINED);
        pstObject;
        pstObject = orxObject_GetNextEnabled(pstObject, orxSTRINGID_UNDEFINED))
    {
      // Not paused and not pending deletion?
      if(!orxObject_IsPaused(pstObject)
      && (orxObject_GetLifeTime(pstObject) != orxFLOAT_0))
      {
        ScrollObject *poObject;

        // Gets its associated scroll object
        poObject = (ScrollObject *)orxObject_GetUserData(pstObject);

        // Valid?
        if(poObject)
        {
          orxCLOCK *pstClock;

          // Gets its clock
          pstClock = orxObject_GetClock(pstObject);

          // Valid?
          if(pstClock)
          {
            // Not paused?
            if(!orxClock_IsPaused(pstClock))
            {
              // Updates object
              poObject->Update(*orxClock_GetInfo(pstClock));
            }
          }
          else
          {
            // Updates object
            poObject->Update(_rstInfo);
          }
        }
      }
    }

    // Unlocks object list
    mbObjectListLocked = orxFALSE;
  }

  // Calls child update
  Update(_rstInfo);
}

void ScrollBase::BaseCameraUpdate(const orxCLOCK_INFO &_rstInfo)
{
  // Not in editor mode?
  if(!IsEditorMode())
  {
    // Calls child update
    CameraUpdate(_rstInfo);
  }
}

orxBOOL ScrollBase::BaseMapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult;

  // Map section or game object?
  if((!orxString_Compare(_zSectionName, szConfigSectionMap)) || (orxString_SearchString(_zSectionName, szConfigScrollObjectPrefix) == _zSectionName))
  {
    // Full section?
    if(!_zKeyName)
    {
      // Pushes section
      orxConfig_PushSection(_zSectionName);

      // Updates result
      bResult = !orxConfig_GetBool(szConfigNoSave);

      // Pops section
      orxConfig_PopSection();
    }
    else
    {
      // Updates result
      bResult = orxTRUE;
    }
  }
  else
  {
    // Calls child method
    bResult = MapSaveFilter(_zSectionName, _zKeyName, _zFileName, _bUseEncryption);

    // Not saving yet?
    if(!bResult)
    {
      // Has custom filter
      if(mpfnCustomMapSaveFilter)
      {
        // Calls it
        bResult = mpfnCustomMapSaveFilter(_zSectionName, _zKeyName, _zFileName, _bUseEncryption);
      }
    }
  }

  // Done!
  return bResult;
}

orxSTRING ScrollBase::GetNewObjectName(orxCHAR _zInstanceName[32], orxBOOL _bRunTime)
{
  orxSTRING zResult = _zInstanceName;

  // Checks
  orxASSERT(_zInstanceName);

  // Runtime?
  if(_bRunTime)
  {
    // Creates name
    orxString_NPrint(zResult, 32, szConfigScrollObjectRuntimeFormat, mu32RuntimeObjectID++);
  }
  else
  {
    // Creates name
    orxString_NPrint(zResult, 32, szConfigScrollObjectFormat, mu32NextObjectID++);
  }

  // Done
  return zResult;
}

void ScrollBase::DeleteInstance()
{
  // Has instance?
  if(spoInstance)
  {
    // Deletes it
    delete spoInstance;
    spoInstance = orxNULL;
  }
}

orxSTATUS orxFASTCALL ScrollBase::StaticInit()
{
  orxSTATUS   eResult;
  ScrollBase &roGame = GetInstance();

  // Calls base init
  eResult = roGame.BaseInit();

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL ScrollBase::StaticRun()
{
  orxSTATUS   eResult;
  ScrollBase &roGame = GetInstance();

  // Calls base run
  eResult = roGame.BaseRun();

  // Done!
  return eResult;
}

void orxFASTCALL ScrollBase::StaticExit()
{
  ScrollBase &roGame = GetInstance();

  // Calls base exit
  roGame.BaseExit();

  // Deletes game instance
  DeleteInstance();
}

void orxFASTCALL ScrollBase::StaticUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext)
{
  ScrollBase &roGame = GetInstance();

  // Running?
  if(roGame.mbIsRunning)
  {
    // Calls base update
    roGame.BaseUpdate(*_pstInfo);
  }
}

void orxFASTCALL ScrollBase::StaticCameraUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext)
{
  ScrollBase &roGame = GetInstance();

  // Calls base camera update
  roGame.BaseCameraUpdate(*_pstInfo);
}

orxBOOL orxFASTCALL ScrollBase::StaticMapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  ScrollBase &roGame = GetInstance();

  // Calls base map save filter
  return roGame.BaseMapSaveFilter(_zSectionName, _zKeyName, _zFileName, _bUseEncryption);
}

orxSTATUS orxFASTCALL ScrollBase::StaticEventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Depending on event type
  switch(_pstEvent->eType)
  {
    // System event
    case orxEVENT_TYPE_SYSTEM:
    {
      orxSYSTEM_EVENT_PAYLOAD *pstPayload;

      // Gets game instance
      ScrollBase &roGame = ScrollBase::GetInstance();

      // Gets payload
      pstPayload = (orxSYSTEM_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      // Stores frame count
      roGame.mu32FrameCount = pstPayload->u32FrameCount;
      break;
    }

    // Object event
    case orxEVENT_TYPE_OBJECT:
    {
      orxOBJECT *pstObject;

      // Gets game instance
      ScrollBase &roGame = ScrollBase::GetInstance();

      // Gets object
      pstObject = orxOBJECT(_pstEvent->hSender);

      // Create?
      if(_pstEvent->eID == orxOBJECT_EVENT_CREATE)
      {
        // Not an internal creation?
        if(!roGame.mzCurrentCreateObject || orxString_Compare(roGame.mzCurrentCreateObject, orxObject_GetName(pstObject)))
        {
          ScrollObjectBinderBase *poBinder;

          // Gets binder
          poBinder = ScrollObjectBinderBase::GetBinder(orxObject_GetName(pstObject), roGame.mzCurrentCreateObject ? orxTRUE : orxFALSE);

          // Found?
          if(poBinder)
          {
            orxCHAR       zInstanceName[32];
            ScrollObject *poObject;

            // Uses it
            poObject = poBinder->CreateObject(pstObject, roGame.GetNewObjectName(zInstanceName, orxTRUE), ScrollObject::FlagRunTime);

            // Valid?
            if(poObject)
            {
              // Updates its flags
              poObject->SetFlags(ScrollObject::FlagRunTime);
            }
          }
        }
        else
        {
          // Clears internal object
          roGame.mzCurrentCreateObject = orxNULL;
        }
      }
      // Delete?
      else if(_pstEvent->eID == orxOBJECT_EVENT_DELETE)
      {
        // Not an internal deletion?
        if(!roGame.mzCurrentDeleteObject || orxString_Compare(roGame.mzCurrentDeleteObject, orxObject_GetName(pstObject)))
        {
          ScrollObject *poObject;

          // Gets scroll object
          poObject = (ScrollObject *)orxObject_GetUserData(pstObject);

          // Checks
          orxASSERT((!poObject) || (!poObject->mpstObject) || (poObject->mpstObject == pstObject));

          // Valid object (first deletion)?
          if(poObject && poObject->mpstObject)
          {
            ScrollObjectBinderBase *poBinder;

            // Gets binder
            poBinder = ScrollObjectBinderBase::GetBinder(orxObject_GetName(pstObject));
            orxASSERT(poBinder);

            // Uses it to delete object
            poBinder->DeleteObject(poObject, orxObject_GetName(pstObject));
          }
        }
        else
        {
          // Clears internal object
          roGame.mzCurrentDeleteObject = orxNULL;
        }
      }
      break;
    }

    // Physics event
    case orxEVENT_TYPE_PHYSICS:
    {
      orxPHYSICS_EVENT_PAYLOAD *pstPayload;
      ScrollObject             *poSender, *poRecipient;

      // Gets payload
      pstPayload = (orxPHYSICS_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      // Gets both objects
      poSender    = (ScrollObject *)orxObject_GetUserData(orxOBJECT(_pstEvent->hSender));
      poRecipient = (ScrollObject *)orxObject_GetUserData(orxOBJECT(_pstEvent->hRecipient));

      // Is sender valid?
      if(poSender)
      {
        orxVECTOR vNormal;

        // Gets reverse normal
        orxVector_Neg(&vNormal, &pstPayload->vNormal);

        // New collision?
        if(_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
        {
          // Calls its callback
          poSender->OnCollide(poRecipient, pstPayload->pstSenderPart, pstPayload->pstRecipientPart, pstPayload->vPosition, vNormal);
        }
        else
        {
          // Calls its callback
          poSender->OnSeparate(poRecipient, pstPayload->pstSenderPart, pstPayload->pstRecipientPart);
        }
      }

      // Is recipient valid?
      if(poRecipient)
      {
        // New collision?
        if(_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
        {
          // Calls its callback
          poRecipient->OnCollide(poSender, pstPayload->pstRecipientPart, pstPayload->pstSenderPart, pstPayload->vPosition, pstPayload->vNormal);
        }
        else
        {
          // Calls its callback
          poRecipient->OnSeparate(poSender, pstPayload->pstRecipientPart, pstPayload->pstSenderPart);
        }
      }
      break;
    }

    // Anim event
    case orxEVENT_TYPE_ANIM:
    {
      ScrollObject *poSender;

      // Gets sender object
      poSender = (ScrollObject *)orxObject_GetUserData(orxOBJECT(_pstEvent->hSender));

      // Valid?
      if(poSender)
      {
        orxANIM_EVENT_PAYLOAD *pstPayload;

        // Gets payload
        pstPayload = (orxANIM_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        // Depending on event ID
        switch(_pstEvent->eID)
        {
          // Update
          case orxANIM_EVENT_UPDATE:
          {
            // Calls object callback
            poSender->OnAnimUpdate(pstPayload->zAnimName);
            break;
          }

          // Custom event
          case orxANIM_EVENT_CUSTOM_EVENT:
          {
            // Calls object callback
            poSender->OnAnimEvent(pstPayload->zAnimName, pstPayload->stCustom.zName, pstPayload->stCustom.fTime, pstPayload->stCustom.fValue);
            break;
          }

          // Stop, cut or loop
          case orxANIM_EVENT_STOP:
          case orxANIM_EVENT_CUT:
          case orxANIM_EVENT_LOOP:
          {
            orxANIMPOINTER *pstAnimPointer;
            const orxSTRING zOldAnim;
            const orxSTRING zNewAnim = orxSTRING_EMPTY;
            orxU32          u32AnimID;

            // Gets old anim
            zOldAnim = pstPayload->zAnimName;

            // Gets its anim pointer
            pstAnimPointer = orxOBJECT_GET_STRUCTURE(poSender->GetOrxObject(), ANIMPOINTER);

            // Gets current anim
            u32AnimID = orxAnimPointer_GetCurrentAnim(pstAnimPointer);

            // Valid?
            if(u32AnimID != orxU32_UNDEFINED)
            {
              orxANIM *pstAnim;

              // Gets new anim
              pstAnim = orxAnimSet_GetAnim(orxAnimPointer_GetAnimSet(pstAnimPointer), u32AnimID);

              // Valid?
              if(pstAnim)
              {
                // Gets its name
                zNewAnim = orxAnim_GetName(pstAnim);
              }
            }

            // Calls object callback
            poSender->OnNewAnim(zOldAnim, zNewAnim, (_pstEvent->eID == orxANIM_EVENT_CUT) ? orxTRUE : orxFALSE);
            break;
          }

          default:
          {
            // Should not happen
            orxASSERT(orxFALSE);
            break;
          }
        }
      }
      break;
    }

    // FX event
    case orxEVENT_TYPE_FX:
    {
      ScrollObject *poSender;

      // Gets sender object
      poSender = (ScrollObject *)orxObject_GetUserData(orxOBJECT(_pstEvent->hSender));

      // Valid?
      if(poSender)
      {
        orxFX_EVENT_PAYLOAD *pstPayload;

        // Gets payload
        pstPayload = (orxFX_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        // Depending on event ID
        switch(_pstEvent->eID)
        {
          // Start
          case orxFX_EVENT_START:
          {
            // Calls object callback
            poSender->OnFXStart(pstPayload->zFXName, pstPayload->pstFX);
            break;
          }

          // Stop
          case orxFX_EVENT_STOP:
          {
            // Calls object callback
            poSender->OnFXStop(pstPayload->zFXName, pstPayload->pstFX);
            break;
          }

          // Loop
          case orxFX_EVENT_LOOP:
          {
            // Calls object callback
            poSender->OnFXLoop(pstPayload->zFXName, pstPayload->pstFX);
            break;
          }

          default:
          {
            // Should not happen
            orxASSERT(orxFALSE);
            break;
          }
        }
      }

      break;
    }

    // Render event
    case orxEVENT_TYPE_RENDER:
    {
      ScrollObject *poSender;

      // Gets sender object
      poSender = (ScrollObject *)orxObject_GetUserData(orxOBJECT(_pstEvent->hSender));

      // Valid?
      if(poSender)
      {
        // Calls object callback
        eResult = poSender->OnRender(*(orxRENDER_EVENT_PAYLOAD *)_pstEvent->pstPayload) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      }
      break;
    }

    // Shader event
    case orxEVENT_TYPE_SHADER:
    {
      ScrollObject *poSender;

      // Gets sender object
      poSender = (ScrollObject *)orxObject_GetUserData(orxOBJECT(_pstEvent->hSender));

      // Valid?
      if(poSender)
      {
        // Calls object callback
        eResult = poSender->OnShader(*(orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      }
      break;
    }

    default:
    {
      // Should not happen
      orxASSERT(orxFALSE);
      break;
    }
  }

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL ScrollBase::StaticBootstrap()
{
  const ScrollBase &roGame = GetInstance();

  // Calls game bootstrap
  return roGame.Bootstrap();
}


//! Binder classes

//! Constants

// Misc
const orxU32            ScrollObjectBinderBase::su32TableSize     = 64;


//! Static variables
orxHASHTABLE *          ScrollObjectBinderBase::spstTable         = orxNULL;


//! Code
orxHASHTABLE *ScrollObjectBinderBase::GetTable()
{
  if(!spstTable)
  {
    spstTable = orxHashTable_Create(su32TableSize, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  }

  return spstTable;
}

void ScrollObjectBinderBase::DeleteTable()
{
  orxHASHTABLE           *pstDeleteTable;
  ScrollObjectBinderBase *poBinder;

  // Creates table for deletion
  pstDeleteTable = orxHashTable_Create(su32TableSize, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  // For all registered binders
  for(orxHANDLE h = orxHashTable_GetNext(spstTable, orxNULL, orxNULL, (void **)&poBinder);
      h != orxHANDLE_UNDEFINED;
      h = orxHashTable_GetNext(spstTable, h, orxNULL, (void **)&poBinder))
  {
    // Not already deleted?
    if(!orxHashTable_Get(pstDeleteTable, (orxU64)poBinder))
    {
      // Adds it to delete table
      orxHashTable_Add(pstDeleteTable, (orxU64)poBinder, (void *)poBinder);

      // Deletes it
      delete poBinder;
    }
  }

  // Deletes local deletion table
  orxHashTable_Delete(pstDeleteTable);

  // Deletes binder table
  orxHashTable_Delete(spstTable);
  spstTable = orxNULL;
}

ScrollObjectBinderBase *ScrollObjectBinderBase::GetDefaultBinder()
{
  // Done!
  return ScrollObjectBinder<ScrollObject>::GetInstance(512);
}

ScrollObjectBinderBase *ScrollObjectBinderBase::GetBinder(const orxSTRING _zName, orxBOOL _bAllowDefault)
{
  ScrollObjectBinderBase *poResult = orxNULL;

  // Valid name?
  if(_zName && (_zName != orxSTRING_EMPTY))
  {
    const orxSTRING zSection;

    // Gets associated binder, using config hierarchy
    for(zSection = _zName, poResult = (ScrollObjectBinderBase *)orxHashTable_Get(GetTable(), orxString_Hash(zSection));
        (!poResult) && ((zSection = orxConfig_GetParent(zSection)));
        poResult = (ScrollObjectBinderBase *)orxHashTable_Get(GetTable(), orxString_Hash(zSection)));
  }

  // Not found and default allowed?
  if(!poResult && _bAllowDefault)
  {
    // Gets default binder
    poResult = GetDefaultBinder();
  }

  // Done!
  return poResult;
}

ScrollObjectBinderBase::ScrollObjectBinderBase(orxS32 _s32SegmentSize, orxU32 _u32ElementSize)
{
  // Creates bank
  mpstBank = orxBank_Create((orxU32)_s32SegmentSize, _u32ElementSize, orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

  // Clears variables
  mpoFirstObject = mpoLastObject = orxNULL;
}

ScrollObjectBinderBase::~ScrollObjectBinderBase()
{
  // Deletes bank
  orxBank_Delete(mpstBank);
  mpstBank = orxNULL;
}

ScrollObject *ScrollObjectBinderBase::CreateObject(const orxSTRING _zModelName, const orxSTRING _zInstanceName, ScrollObject::Flag _xFlags)
{
  orxOBJECT    *pstOrxObject;
  ScrollObject *poResult;

  // Creates orx object
  pstOrxObject = orxObject_CreateFromConfig(_zModelName);

  // Valid?
  if(pstOrxObject)
  {
    // Updates result
    poResult = CreateObject(pstOrxObject, _zInstanceName, _xFlags);
  }
  else
  {
    // Updates result
    poResult = orxNULL;
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollObjectBinderBase::CreateObject(orxOBJECT *_pstOrxObject, const orxSTRING _zInstanceName, ScrollObject::Flag _xFlags)
{
  ScrollObject *poResult;

  // Is object valid?
  if(_pstOrxObject)
  {
    ScrollObject::Flag  xFlags;
    orxFLOAT            fRepeatX, fRepeatY;

    // Gets game instance
    ScrollBase &roGame = ScrollBase::GetInstance();

    // Checks
    orxSTRUCTURE_ASSERT(_pstOrxObject);
    orxASSERT(!orxObject_GetUserData(_pstOrxObject));

    // Creates scroll object
    poResult = ConstructObject(mpstBank);

    // Savable or runtime object?
    if(_xFlags & (ScrollObject::FlagSave | ScrollObject::FlagRunTime))
    {
      // First one?
      if(!mpoFirstObject)
      {
        // Stores it
        mpoFirstObject = mpoLastObject = poResult;

        // Adds it to the end of the list
        orxLinkList_AddEnd(&roGame.mstObjectList, &poResult->mstNode);
      }
      else
      {
        // Adds it after last object
        orxLinkList_AddAfter(&mpoLastObject->mstNode, &poResult->mstNode);

        // Stores it
        mpoLastObject = poResult;
      }
    }

    // Adds to chronological list
    orxLinkList_AddEnd(&roGame.mstObjectChronoList, &poResult->mstChronoNode);

    // Stores internal object
    poResult->SetOrxObject(_pstOrxObject);

    // Sets object as user data
    orxObject_SetUserData(_pstOrxObject, poResult);

    // Stores its name
    orxString_NPrint(poResult->macName, sizeof(poResult->macName), "%s", _zInstanceName);

    // Inits flags
    xFlags = _xFlags;

    // Is tiled?
    if((orxOBJECT_GET_STRUCTURE(_pstOrxObject, GRAPHIC))
    && (orxObject_GetRepeat(_pstOrxObject, &fRepeatX, &fRepeatY) != orxSTATUS_FAILURE)
    && ((fRepeatX != orxFLOAT_1)
     || (fRepeatY != orxFLOAT_1)))
    {
      // Updates flags
      xFlags |= ScrollObject::FlagTiled;
    }
    else
    {
      // Updates flags
      xFlags &= ~ScrollObject::FlagTiled;
    }

    // Is smoothed?
    if(orxObject_GetSmoothing(_pstOrxObject) == orxDISPLAY_SMOOTHING_ON)
    {
      // Updates flags
      xFlags |= ScrollObject::FlagSmoothed;
    }
    else
    {
      xFlags &= ~ScrollObject::FlagSmoothed;
    }

    // Not runtime?
    if(!(_xFlags & ScrollObject::FlagRunTime))
    {
      // Creates and protects its instance section
      orxConfig_PushSection(_zInstanceName);
      orxConfig_ProtectSection(_zInstanceName, orxTRUE);
      orxConfig_PopSection();
    }

    // Pushes its section
    poResult->PushConfigSection();

    // Is pausable?
    if((!orxConfig_HasValue(ScrollBase::szConfigScrollObjectPausable))
    || (orxConfig_GetBool(ScrollBase::szConfigScrollObjectPausable)))
    {
      // Updates flags
      xFlags |= ScrollObject::FlagPausable;
    }

    // Stores flags
    poResult->SetFlags(xFlags, ScrollObject::MaskAll);

    // Should use callback?
    if(poResult->TestFlags(ScrollObject::FlagSave | ScrollObject::FlagRunTime))
    {
      // Calls it
      poResult->OnCreate();

      // Is game running?
      if(roGame.IsGameRunning())
      {
        // Calls its start game callback
        poResult->OnStartGame();
      }
    }

    // Pops section
    poResult->PopConfigSection();
  }
  else
  {
    // Updates result
    poResult = orxNULL;
  }

  // Done!
  return poResult;
}

void ScrollObjectBinderBase::DeleteObject(ScrollObject *_poObject)
{
  // Deletes orx object
  if(_poObject)
  {
    orxOBJECT *pstObject;

    // Checks
    orxSTRUCTURE_ASSERT(_poObject->GetOrxObject());

    // Gets internal object
    pstObject = _poObject->GetOrxObject();

    // Deletes object
    DeleteObject(_poObject, orxObject_GetName(pstObject));

    // Deletes internal object
    orxObject_Delete(pstObject);
  }
}

void ScrollObjectBinderBase::DeleteObject(ScrollObject *_poObject, const orxSTRING _zModelName)
{
  const orxSTRING zName;

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Should use callback?
  if(_poObject->TestFlags(ScrollObject::FlagSave | ScrollObject::FlagRunTime))
  {
    orxBOOL bObjectListBlockBackup;

    // Calls game callback
    roGame.OnObjectDelete(_poObject);

    // Blocks object list
    bObjectListBlockBackup = roGame.mbObjectListLocked;
    roGame.mbObjectListLocked = orxTRUE;

    // Calls object callback
    _poObject->OnDelete();

    // Restores object list blocking
    roGame.mbObjectListLocked = bObjectListBlockBackup;
  }

  // Gets its name
  zName = _poObject->GetName();

  // Valid?
  if(zName && (zName[0] != orxCHAR_NULL))
  {
    // Not runtime?
    if(!_poObject->TestFlags(ScrollObject::FlagRunTime))
    {
      // Unprotects instance section
      orxConfig_ProtectSection(zName, orxFALSE);
    }

    // Clears it
    orxConfig_ClearSection(zName);
    orxASSERT(!orxConfig_HasSection(zName));
  }

  // Removes object as user data
  orxObject_SetUserData(_poObject->mpstObject, orxNULL);

  // Clears internal reference
  _poObject->SetOrxObject(orxNULL);

  // Savable or runtime?
  if(_poObject->TestFlags(ScrollObject::FlagSave | ScrollObject::FlagRunTime))
  {
    // First object?
    if(_poObject == mpoFirstObject)
    {
      ScrollObject *poNewFirstObject;

      // Gets new first object
      poNewFirstObject = GetNextObject(_poObject);

      // Last object?
      if(_poObject == mpoLastObject)
      {
        // Updates last object
        mpoLastObject = GetPreviousObject(_poObject);
      }

      // Updates first object
      mpoFirstObject = poNewFirstObject;
    }
    // Last object?
    else if(_poObject == mpoLastObject)
    {
      // Updates last object
      mpoLastObject = GetPreviousObject(_poObject);
    }

    // Checks
    orxASSERT(orxLinkList_GetList(&_poObject->mstNode) == &roGame.mstObjectList);

    // Removes it from chronological list
    orxLinkList_Remove(&_poObject->mstNode);
  }

  // Checks
  orxASSERT(orxLinkList_GetList(&_poObject->mstChronoNode) == &roGame.mstObjectChronoList);

  // Removes it from chronological list
  orxLinkList_Remove(&_poObject->mstChronoNode);

  // Deletes it
  DestructObject(_poObject);
}

void ScrollObjectBinderBase::DestructObject(ScrollObject *_poObject) const
{
  // Deletes object
  _poObject->~ScrollObject();
  operator delete(_poObject, mpstBank);
}

ScrollObject *ScrollObjectBinderBase::GetNextObject(const ScrollObject *_poObject) const
{
    ScrollObject *poResult;

  // None specified?
  if(!_poObject)
  {
    // Updates result
    poResult = mpoFirstObject;
  }
  // Not last one?
  else if(_poObject != mpoLastObject)
  {
    // Updates result
    poResult = ScrollBase::GetInstance().GetNextObject(_poObject);
  }
  else
  {
    // Updates result
    poResult = orxNULL;
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollObjectBinderBase::GetPreviousObject(const ScrollObject *_poObject) const
{
  ScrollObject *poResult;

  // None specified?
  if(!_poObject)
  {
    // Updates result
    poResult = mpoLastObject;
  }
  // Not first one?
  else if(_poObject != mpoFirstObject)
  {
    // Updates result
    poResult = ScrollBase::GetInstance().GetPreviousObject(_poObject);
  }
  else
  {
    // Updates result
    poResult = orxNULL;
  }

  // Done!
  return poResult;
}

#ifdef __orxLLVM__
  #pragma clang diagnostic pop
#endif // __orxLLVM__
