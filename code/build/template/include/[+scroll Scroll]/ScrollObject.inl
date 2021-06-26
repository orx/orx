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

//! Code
ScrollObject::ScrollObject() : mpstObject(orxNULL), mzModelName(orxNULL), mxFlags(FlagNone)
{
  // Clears nodes
  orxMemory_Zero(&mstNode, sizeof(orxLINKLIST_NODE));
  orxMemory_Zero(&mstChronoNode, sizeof(orxLINKLIST_NODE));
  orxMemory_Zero(macName, sizeof(macName));
}

ScrollObject::~ScrollObject()
{
}

orxSTRINGID ScrollObject::GetGroupID() const
{
  // Done!
  return orxObject_GetGroupID(mpstObject);
}

void ScrollObject::SetGroupID(orxU32 _u32GroupID, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Updates object's group ID
    orxObject_SetGroupIDRecursive(mpstObject, _u32GroupID);
  }
  else
  {
    // Updates object's group ID
    orxObject_SetGroupID(mpstObject, _u32GroupID);
  }
}

orxBOOL ScrollObject::IsEnabled() const
{
  // Done!
  return orxObject_IsEnabled(mpstObject);
}

void ScrollObject::Enable(orxBOOL _bEnable, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Updates its status
    orxObject_EnableRecursive(mpstObject, _bEnable);
  }
  else
  {
    // Updates its status
    orxObject_Enable(mpstObject, _bEnable);
  }
}

orxBOOL ScrollObject::IsPaused() const
{
  // Done!
  return orxObject_IsPaused(mpstObject);
}

void ScrollObject::Pause(orxBOOL _bPause, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Updates its status
    orxObject_PauseRecursive(mpstObject, _bPause);
  }
  else
  {
    // Updates its status
    orxObject_Pause(mpstObject, _bPause);
  }
}

orxVECTOR &ScrollObject::GetPosition(orxVECTOR &_rvPosition, orxBOOL _bWorld) const
{
  // Local?
  if(!_bWorld)
  {
    // Updates result
    orxObject_GetPosition(mpstObject, &_rvPosition);
  }
  else
  {
    // Updates result
    orxObject_GetWorldPosition(mpstObject, &_rvPosition);
  }

  // Done!
  return _rvPosition;
}

void ScrollObject::SetPosition(const orxVECTOR &_rvPosition, orxBOOL _bWorld)
{
  // Local?
  if(!_bWorld)
  {
    // Updates its position
    orxObject_SetPosition(mpstObject, &_rvPosition);
  }
  else
  {
    // Updates its position
    orxObject_SetWorldPosition(mpstObject, &_rvPosition);
  }
}

orxVECTOR &ScrollObject::GetSize(orxVECTOR &_rvSize) const
{
  // Updates result
  orxObject_GetSize(mpstObject, &_rvSize);

  // Done!
  return _rvSize;
}

void ScrollObject::SetSize(const orxVECTOR &_rvSize)
{
  // Updates its Size
  orxObject_SetSize(mpstObject, &_rvSize);
}

orxVECTOR &ScrollObject::GetScale(orxVECTOR &_rvScale, orxBOOL _bWorld) const
{
  // Local?
  if(!_bWorld)
  {
    // Updates result
    orxObject_GetScale(mpstObject, &_rvScale);
  }
  else
  {
    // Updates result
    orxObject_GetWorldScale(mpstObject, &_rvScale);
  }

  // Done!
  return _rvScale;
}

void ScrollObject::SetScale(const orxVECTOR &_rvScale, orxBOOL _bWorld)
{
  // Local?
  if(!_bWorld)
  {
    // Updates its scale
    orxObject_SetScale(mpstObject, &_rvScale);
  }
  else
  {
    // Updates its scale
    orxObject_SetWorldScale(mpstObject, &_rvScale);
  }
}

orxFLOAT ScrollObject::GetRotation(orxBOOL _bWorld) const
{
  orxFLOAT fResult;

  // Local?
  if(!_bWorld)
  {
    // Updates result
    fResult = orxObject_GetRotation(mpstObject);
  }
  else
  {
    // Updates result
    fResult = orxObject_GetWorldRotation(mpstObject);
  }

  // Done!
  return fResult;
}

void ScrollObject::SetRotation(orxFLOAT _fRotation, orxBOOL _bWorld)
{
  // Local?
  if(!_bWorld)
  {
    // Updates its rotation
    orxObject_SetRotation(mpstObject, _fRotation);
  }
  else
  {
    // Updates its rotation
    orxObject_SetWorldRotation(mpstObject, _fRotation);
  }
}

orxVECTOR &ScrollObject::GetSpeed(orxVECTOR &_rvSpeed, orxBOOL _bRelative) const
{
  // Relative?
  if(_bRelative)
  {
    // Updates result
    orxObject_GetRelativeSpeed(mpstObject, &_rvSpeed);
  }
  else
  {
    // Updates result
    orxObject_GetSpeed(mpstObject, &_rvSpeed);
  }

  // Done!
  return _rvSpeed;
}

void ScrollObject::SetSpeed(const orxVECTOR &_rvSpeed, orxBOOL _bRelative)
{
  // Relative?
  if(_bRelative)
  {
    // Updates its speed
    orxObject_SetRelativeSpeed(mpstObject, &_rvSpeed);
  }
  else
  {
    // Updates its speed
    orxObject_SetSpeed(mpstObject, &_rvSpeed);
  }
}

orxCOLOR &ScrollObject::GetColor(orxCOLOR &_rstColor) const
{
  // Updates result
  orxObject_GetColor(mpstObject, &_rstColor);

  // Done!
  return _rstColor;
}

void ScrollObject::SetColor(const orxCOLOR &_rstColor, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Updates object's color
    orxObject_SetColorRecursive(mpstObject, &_rstColor);
  }
  else
  {
    // Updates object's color
    orxObject_SetColor(mpstObject, &_rstColor);
  }
}

void ScrollObject::GetFlip(orxBOOL &_rbFlipX, orxBOOL &_rbFlipY) const
{
  // Updates result
  orxObject_GetFlip(mpstObject, &_rbFlipX, &_rbFlipY);
}

void ScrollObject::SetFlip(orxBOOL _bFlipX, orxBOOL _bFlipY, orxBOOL _bRecursive)
{
  // Updates its flip
  orxObject_SetFlip(mpstObject, _bFlipX, _bFlipY);

  // Recursive?
  if(_bRecursive)
  {
    // For all children
    for(orxOBJECT *pstChild = orxObject_GetOwnedChild(mpstObject);
        pstChild;
        pstChild = orxObject_GetOwnedSibling(pstChild))
    {
      ScrollObject *poChild;

      // Gets its scroll object
      poChild = (ScrollObject *)orxObject_GetUserData(pstChild);

      // Valid?
      if(poChild)
      {
        // Updates its flip
        poChild->SetFlip(_bFlipX, _bFlipY, _bRecursive);
      }
    }
  }
}

orxBOOL ScrollObject::IsAnim(const orxSTRING _zAnim, orxBOOL _bCurrent)
{
  orxBOOL bResult;

  // Target anim?
  if(!_bCurrent)
  {
    // Updates result
    bResult = orxObject_IsTargetAnim(mpstObject, _zAnim);
  }
  else
  {
    // Updates result
    bResult = orxObject_IsCurrentAnim(mpstObject, _zAnim);
  }

  // Done!
  return bResult;
}

void ScrollObject::SetAnim(const orxSTRING _zAnim, orxBOOL _bCurrent, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Target anim?
    if(!_bCurrent)
    {
      // Sets it
      orxObject_SetTargetAnimRecursive(mpstObject, _zAnim);
    }
    else
    {
      // Sets it
      orxObject_SetCurrentAnimRecursive(mpstObject, _zAnim);
    }
  }
  else
  {
    // Target anim?
    if(!_bCurrent)
    {
      // Sets it
      orxObject_SetTargetAnim(mpstObject, _zAnim);
    }
    else
    {
      // Sets it
      orxObject_SetCurrentAnim(mpstObject, _zAnim);
    }
  }
}

void ScrollObject::AddFX(const orxSTRING _zFXName, orxBOOL _bRecursive, orxFLOAT _fPropagationDelay)
{
  // Recursive?
  if(_bRecursive)
  {
    // Adds FX to object
    orxObject_AddFXRecursive(mpstObject, _zFXName, _fPropagationDelay);
  }
  else
  {
    // Adds FX to object
    orxObject_AddFX(mpstObject, _zFXName);
  }
}

void ScrollObject::RemoveFX(const orxSTRING _zFXName, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Removes FX from object
    orxObject_RemoveFXRecursive(mpstObject, _zFXName);
  }
  else
  {
    // Removes FX from object
    orxObject_RemoveFX(mpstObject, _zFXName);
  }
}

void ScrollObject::AddShader(const orxSTRING _zShaderName, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Adds shader to object
    orxObject_AddShaderRecursive(mpstObject, _zShaderName);
  }
  else
  {
    // Adds shader to object
    orxObject_AddShader(mpstObject, _zShaderName);
  }
}

void ScrollObject::RemoveShader(const orxSTRING _zShaderName, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Removes shader from object
    orxObject_RemoveShaderRecursive(mpstObject, _zShaderName);
  }
  else
  {
    // Removes shader from object
    orxObject_RemoveShader(mpstObject, _zShaderName);
  }
}

void ScrollObject::AddSound(const orxSTRING _zSoundName)
{
  // Adds sound to object
  orxObject_AddSound(mpstObject, _zSoundName);
}

void ScrollObject::RemoveSound(const orxSTRING _zSoundName)
{
  // Removes sound from object
  orxObject_RemoveSound(mpstObject, _zSoundName);
}

void ScrollObject::AddTrack(const orxSTRING _zTrackName)
{
  // Adds track to object
  orxObject_AddTimeLineTrack(mpstObject, _zTrackName);
}

orxSTATUS ScrollObject::AddConditionalTrack(const orxSTRING _zTrackKey, orxS32 _s32Index)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  // Pushes config section
  PushConfigSection();

  // Has Track key?
  if(orxConfig_HasValue(_zTrackKey))
  {
    // Is index valid?
    if(_s32Index < orxConfig_GetListCount(_zTrackKey))
    {
      const orxSTRING zTrack;

      // Gets track
      zTrack = orxConfig_GetListString(_zTrackKey, _s32Index);

      // Valid?
      if(*zTrack != orxCHAR_NULL)
      {
        // Adds track
        eResult = orxObject_AddTimeLineTrack(mpstObject, zTrack);
      }
    }
  }

  // Pops config section
  PopConfigSection();

  // Done!
  return eResult;
}

void ScrollObject::RemoveTrack(const orxSTRING _zTrackName)
{
  // Removes track from object
  orxObject_RemoveTimeLineTrack(mpstObject, _zTrackName);
}

orxFLOAT ScrollObject::GetLifeTime() const
{
  orxFLOAT fResult;

  // Updates result
  fResult = orxObject_GetLifeTime(mpstObject);

  // Done!
  return fResult;
}

void ScrollObject::SetLifeTime(orxFLOAT _fLifeTime)
{
  // Updates its lifetime
  orxObject_SetLifeTime(mpstObject, _fLifeTime);
}

void ScrollObject::PushConfigSection(orxBOOL _bPushInstanceSection) const
{
  // Pushes its model section
  orxConfig_PushSection(_bPushInstanceSection ? macName : mzModelName);
}

ScrollObject *ScrollObject::GetOwnedChild() const
{
  orxOBJECT    *pstChild;
  ScrollObject *poResult = orxNULL;

  // Gets child
  pstChild = orxObject_GetOwnedChild(mpstObject);

  // Valid?
  if(pstChild)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstChild);
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollObject::GetOwnedSibling() const
{
  orxOBJECT    *pstSibling;
  ScrollObject *poResult = orxNULL;

  // Gets sibling
  pstSibling = orxObject_GetOwnedSibling(mpstObject);

  // Valid?
  if(pstSibling)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstSibling);
  }

  // Done!
  return poResult;
}

void ScrollObject::PopConfigSection() const
{
  // Pops config section
  orxConfig_PopSection();
}

void ScrollObject::SetDifferentialMode(orxBOOL _bDifferential)
{
  // Uses differential scrolling?
  if(_bDifferential)
  {
    // Enforces object's differential flags
    orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstObject, FRAME), orxFRAME_KU32_FLAG_DEPTH_SCALE|orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y, orxFRAME_KU32_FLAG_NONE);

    // For all children
    for(orxOBJECT *pstChild = orxObject_GetOwnedChild(mpstObject);
        pstChild;
        pstChild = orxObject_GetOwnedSibling(pstChild))
    {
      // Enforces its differential flags
      orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(pstChild, FRAME), orxFRAME_KU32_FLAG_DEPTH_SCALE|orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y, orxFRAME_KU32_FLAG_NONE);
    }
  }
  else
  {
    // Removes object's differential flags
    orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(mpstObject, FRAME), orxFRAME_KU32_FLAG_NONE, orxFRAME_KU32_FLAG_DEPTH_SCALE|orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y);

    // For all children
    for(orxOBJECT *pstChild = orxObject_GetOwnedChild(mpstObject);
        pstChild;
        pstChild = orxObject_GetOwnedSibling(pstChild))
    {
      // Removes its differential flags
      orxStructure_SetFlags(orxOBJECT_GET_STRUCTURE(pstChild, FRAME), orxFRAME_KU32_FLAG_NONE, orxFRAME_KU32_FLAG_DEPTH_SCALE|orxFRAME_KU32_FLAG_SCROLL_X|orxFRAME_KU32_FLAG_SCROLL_Y);
    }
  }
}

void ScrollObject::SetOrxObject(orxOBJECT *_pstObject)
{
  // Stores it
  mpstObject = _pstObject;

  // Valid?
  if(_pstObject)
  {
    // Stores model name
    mzModelName = orxObject_GetName(_pstObject);
  }
  else
  {
    // Clears model name
    mzModelName = orxNULL;
  }
}

void ScrollObject::OnCreate()
{
#ifdef __SCROLL_DEBUG__
  // Is a derived class?
  if(typeid(ScrollObject) != typeid(*this))
  {
    // Has to override this method in the derived class!
    orxLOG("[ERROR] You need to override the ScrollObject::OnCreate() method for derived class %s!", typeid(*this).name());
    orxBREAK();
  }
#endif // __SCROLL_DEBUG__
}

void ScrollObject::OnDelete()
{
#ifdef __SCROLL_DEBUG__
  // Is a derived class?
  if(typeid(ScrollObject) != typeid(*this))
  {
    // Has to override this method in the derived class!
    orxLOG("[ERROR] You need to override the ScrollObject::OnDelete() method for derived class %s!", typeid(*this).name());
    orxBREAK();
  }
#endif // __SCROLL_DEBUG__
}

void ScrollObject::Update(const orxCLOCK_INFO &_rstInfo)
{
}

orxBOOL ScrollObject::OnCollide(ScrollObject *_poCollider, const orxSTRING _zPartName, const orxSTRING _zColliderPartName, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal)
{
  return orxTRUE;
}

orxBOOL ScrollObject::OnSeparate(ScrollObject *_poCollider)
{
  return orxTRUE;
}

void ScrollObject::OnNewAnim(const orxSTRING _zOldAnim, const orxSTRING _zNewAnim, orxBOOL _bCut)
{
}

void ScrollObject::OnAnimUpdate(const orxSTRING _zAnim)
{
}

void ScrollObject::OnAnimEvent(const orxSTRING _zAnim, const orxSTRING _zEvent, orxFLOAT _fTime, orxFLOAT _fValue)
{
}

void ScrollObject::OnStartGame()
{
}

void ScrollObject::OnStopGame()
{
}

orxBOOL ScrollObject::OnPauseGame(orxBOOL _bPause)
{
  return orxTRUE;
}

orxBOOL ScrollObject::OnRender(orxRENDER_EVENT_PAYLOAD &_rstPayload)
{
  return orxTRUE;
}

orxBOOL ScrollObject::OnShader(orxSHADER_EVENT_PAYLOAD &_rstPayload)
{
  return orxTRUE;
}
