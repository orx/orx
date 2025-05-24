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

#ifndef _SCROLLOBJECT_H_
#define _SCROLLOBJECT_H_


//! ScrollObject class
class ScrollObject
{
  friend class ScrollBase;
  friend class ScrollEd;
  friend class ScrollObjectBinderBase;
  template <class O> friend class ScrollObjectBinder;


public:

  enum Flag
  {
    FlagNone          = 0x00000000,

    FlagPausable      = 0x00000001,
    FlagInput         = 0x00000002,
    FlagInputBinding  = 0x00000004,

    MaskAll           = 0xFFFFFFFF
  };


                orxBOOL                 TestFlags(Flag _xTestFlags) const                         {return (mxFlags & _xTestFlags) ? orxTRUE : orxFALSE;}
                orxBOOL                 TestAllFlags(Flag _xTestFlags) const                      {return ((mxFlags & _xTestFlags) == _xTestFlags) ? orxTRUE : orxFALSE;}
                Flag                    GetFlags(Flag _xMask = MaskAll) const                     {return (Flag)(mxFlags & _xMask);}
                orxOBJECT *             GetOrxObject() const                                      {return mpstObject;}
                const orxSTRING         GetName() const                                           {return mzName;}
                const orxSTRING         GetInstanceName() const                                   {return macInstanceName;}
                const orxSTRING         GetInputSet() const                                       {return mzInputSet;}
                orxU64                  GetGUID() const                                           {return orxStructure_GetGUID(mpstObject);}

                orxSTRINGID             GetGroupID() const;
                void                    SetGroupID(orxSTRINGID _stGroupID, orxBOOL _bRecursive = orxTRUE);

                orxBOOL                 IsEnabled() const;
                void                    Enable(orxBOOL _bEnable, orxBOOL _bRecursive = orxTRUE);

                orxBOOL                 IsPaused() const;
                void                    Pause(orxBOOL _bPause, orxBOOL _bRecursive = orxTRUE);

                orxVECTOR &             GetPosition(orxVECTOR &_rvPosition, orxBOOL _bWorld = orxFALSE) const;
                void                    SetPosition(const orxVECTOR &_rvPosition, orxBOOL _bWorld = orxFALSE);

                orxVECTOR &             GetSize(orxVECTOR &_rvSize) const;
                void                    SetSize(const orxVECTOR &_rvSize);

                orxVECTOR &             GetScale(orxVECTOR &_rvScale, orxBOOL _bWorld = orxFALSE) const;
                void                    SetScale(const orxVECTOR &_rvScale, orxBOOL _bWorld = orxFALSE);

                orxFLOAT                GetRotation(orxBOOL _bWorld = orxFALSE) const;
                void                    SetRotation(orxFLOAT _fRotation, orxBOOL _bWorld = orxFALSE);

                orxFLOAT                GetAngularVelocity() const;
                void                    SetAngularVelocity(orxFLOAT _fVelocity);

                orxVECTOR &             GetSpeed(orxVECTOR &_rvSpeed, orxBOOL _bRelative = orxFALSE) const;
                void                    SetSpeed(const orxVECTOR &_rvSpeed, orxBOOL _bRelative = orxFALSE);

                orxCOLOR &              GetColor(orxCOLOR &_rstColor) const;
                void                    SetColor(const orxCOLOR &_rstColor, orxBOOL _bRecursive = orxTRUE);

                orxFLOAT                GetAlpha() const;
                void                    SetAlpha(orxFLOAT _fAlpha, orxBOOL _bRecursive = orxTRUE);

                void                    GetFlip(orxBOOL &_rbFlipX, orxBOOL &_rbFlipY) const;
                void                    SetFlip(orxBOOL _bFlipX, orxBOOL _bFlipY, orxBOOL _bRecursive = orxTRUE);

                const orxSTRING         GetText() const;
                void                    SetText(const orxSTRING _zText);

                orxBOOL                 IsAnim(const orxSTRING _zAnim, orxBOOL _bCurrent = orxFALSE);
                void                    SetAnim(const orxSTRING _zAnim, orxBOOL _bCurrent = orxFALSE, orxBOOL _bRecursive = orxTRUE);

                void                    AddFX(const orxSTRING _zFXName, orxBOOL _bRecursive = orxTRUE, orxFLOAT _fPropagationDelay = orxFLOAT_0);
                void                    RemoveFX(const orxSTRING _zFXName, orxBOOL _bRecursive = orxTRUE);

                const orxSTRING         GetShader() const;
                void                    SetShader(const orxSTRING _zShaderName, orxBOOL _bRecursive = orxTRUE);

                void                    AddSound(const orxSTRING _zSoundName);
                void                    RemoveSound(const orxSTRING _zSoundName);

                void                    AddTrack(const orxSTRING _zTrackName);
                orxSTATUS               AddConditionalTrack(const orxSTRING _zTrackKey, orxS32 _s32Index = -1);
                void                    RemoveTrack(const orxSTRING _zTrackName);

                orxFLOAT                GetLifeTime() const;
                void                    SetLifeTime(orxFLOAT _fLifeTime);

                void                    AddTrigger(const orxSTRING _zTrigger, orxBOOL _bRecursive = orxTRUE);
                void                    RemoveTrigger(const orxSTRING _zTrigger, orxBOOL _bRecursive = orxTRUE);
                void                    FireTrigger(const orxSTRING _zEvent, const orxSTRING *_azRefinementList = orxNULL, orxU32 _u32Count = 0, orxBOOL _bRecursive = orxTRUE);

                ScrollObject *          GetParent() const;
                void                    SetParent(ScrollObject *_poParent);

                ScrollObject *          GetOwner() const;
                void                    SetOwner(ScrollObject *_poOwner);

                ScrollObject *          FindChild(const orxSTRING _zPath) const;
                ScrollObject *          GetChild() const;
                ScrollObject *          GetSibling() const;

                ScrollObject *          FindOwnedChild(const orxSTRING _zPath) const;
                ScrollObject *          GetOwnedChild() const;
                ScrollObject *          GetOwnedSibling() const;


                void                    PushConfigSection(orxBOOL _bPushInstanceSection = orxFALSE) const;
                void                    PopConfigSection() const;


protected:

                                        ScrollObject();
                virtual                ~ScrollObject();


private:

  virtual       void                    OnCreate();
  virtual       void                    OnDelete();
  virtual       void                    Update(const orxCLOCK_INFO &_rstInfo);
  virtual       void                    OnStartGame();
  virtual       void                    OnStopGame();
  virtual       orxBOOL                 OnPauseGame(orxBOOL _bPause);

  virtual       orxBOOL                 OnRender(orxRENDER_EVENT_PAYLOAD &_rstPayload);
  virtual       orxBOOL                 OnShader(orxSHADER_EVENT_PAYLOAD &_rstPayload);

  virtual       void                    OnCollide(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal);
  virtual       void                    OnSeparate(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart);

  virtual       void                    OnSpawn(ScrollObject *_poSpawned);

  virtual       void                    OnNewAnim(const orxSTRING _zOldAnim, const orxSTRING _zNewAnim, orxBOOL _bCut);
  virtual       void                    OnAnimUpdate(const orxSTRING _zAnim);
  virtual       void                    OnAnimEvent(const orxSTRING _zAnim, const orxSTRING _zEvent, orxFLOAT _fTime, orxFLOAT _fValue);

  virtual       void                    OnFXStart(const orxSTRING _zFX, orxFX *_pstFX);
  virtual       void                    OnFXStop(const orxSTRING _zFX, orxFX *_pstFX);
  virtual       void                    OnFXLoop(const orxSTRING _zFX, orxFX *_pstFX);

                void                    SetFlags(Flag _xAddFlags, Flag _xRemoveFlags = FlagNone)  {mxFlags = (Flag)(mxFlags & ~_xRemoveFlags); mxFlags = (Flag)(mxFlags | _xAddFlags);}
                void                    SwapFlags(Flag _xSwapFlags)                               {mxFlags = (Flag)(mxFlags ^ _xSwapFlags);}

                void                    SetOrxObject(orxOBJECT *_pstObject);


//! Variables
private:

                orxOBJECT *             mpstObject;
                const orxSTRING         mzName;
                orxLINKLIST_NODE        mstNode;
                orxLINKLIST_NODE        mstChronoNode;
                const orxSTRING         mzInputSet;
          class ScrollObjectBinderBase *mpoBinder;
                Flag                    mxFlags;
                orxCHAR                 macInstanceName[20];
};


//! Operators
inline ScrollObject::Flag operator|(ScrollObject::Flag _x1, ScrollObject::Flag _x2)
{
  return ScrollObject::Flag(int(_x1) | int(_x2));
}
inline ScrollObject::Flag & operator|=(ScrollObject::Flag &_x1, ScrollObject::Flag _x2)
{
  _x1 = _x1 | _x2;
  return _x1;
}

inline ScrollObject::Flag operator&(ScrollObject::Flag _x1, ScrollObject::Flag _x2)
{
  return ScrollObject::Flag(int(_x1) & int(_x2));
}
inline ScrollObject::Flag & operator&=(ScrollObject::Flag &_x1, ScrollObject::Flag _x2)
{
  _x1 = _x1 & _x2;
  return _x1;
}

inline ScrollObject::Flag operator^(ScrollObject::Flag _x1, ScrollObject::Flag _x2)
{
  return ScrollObject::Flag(int(_x1) ^ int(_x2));
}
inline ScrollObject::Flag & operator^=(ScrollObject::Flag &_x1, ScrollObject::Flag _x2)
{
  _x1 = _x1 ^ _x2;
  return _x1;
}

inline ScrollObject::Flag operator~(ScrollObject::Flag _x1)
{
  return ScrollObject::Flag(~int(_x1));
}


#ifdef __SCROLL_IMPL__

//! Code
ScrollObject::ScrollObject() : mpstObject(orxNULL), mzName(orxNULL), mzInputSet(orxNULL), mxFlags(FlagNone)
{
  // Clears nodes
  orxMemory_Zero(&mstNode, sizeof(orxLINKLIST_NODE));
  orxMemory_Zero(&mstChronoNode, sizeof(orxLINKLIST_NODE));
  orxMemory_Zero(macInstanceName, sizeof(macInstanceName));
}

ScrollObject::~ScrollObject()
{
}

orxSTRINGID ScrollObject::GetGroupID() const
{
  // Done!
  return orxObject_GetGroupID(mpstObject);
}

void ScrollObject::SetGroupID(orxSTRINGID _stGroupID, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Updates object's group ID
    orxObject_SetGroupIDRecursive(mpstObject, _stGroupID);
  }
  else
  {
    // Updates object's group ID
    orxObject_SetGroupID(mpstObject, _stGroupID);
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

orxFLOAT ScrollObject::GetAngularVelocity() const
{
  // Done!
  return orxObject_GetAngularVelocity(mpstObject);
}

void ScrollObject::SetAngularVelocity(orxFLOAT _fVelocity)
{
  // Updates its angular velocity
  orxObject_SetAngularVelocity(mpstObject, _fVelocity);
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

orxFLOAT ScrollObject::GetAlpha() const
{
  orxFLOAT fResult;

  // Updates result
  fResult = orxObject_GetAlpha(mpstObject);

  // Done!
  return fResult;
}

void ScrollObject::SetAlpha(orxFLOAT _fAlpha, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Updates object's alpha
    orxObject_SetAlphaRecursive(mpstObject, _fAlpha);
  }
  else
  {
    // Updates object's alpha
    orxObject_SetAlpha(mpstObject, _fAlpha);
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

const orxSTRING ScrollObject::GetText() const
{
  const orxSTRING zResult;
  
  // Updates result
  zResult = orxObject_GetTextString(mpstObject);
  
  // Done!
  return zResult;
}

void ScrollObject::SetText(const orxSTRING _zText)
{
  // Updates object's text
  orxObject_SetTextString(mpstObject, _zText);
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

const orxSTRING ScrollObject::GetShader() const
{
  const orxSHADER  *pstShader;
  const orxSTRING   zResult;
  
  // Gets shader
  pstShader = orxObject_GetShader(mpstObject);
  
  // Updates result
  zResult = (pstShader != orxNULL) ? orxShader_GetName(pstShader) : orxSTRING_EMPTY;
  
  // Done!
  return zResult;
}

void ScrollObject::SetShader(const orxSTRING _zShaderName, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Sets shader to object
    orxObject_SetShaderFromConfigRecursive(mpstObject, _zShaderName);
  }
  else
  {
    // Sets shader to object
    orxObject_SetShaderFromConfig(mpstObject, _zShaderName);
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

void ScrollObject::AddTrigger(const orxSTRING _zTrigger, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Adds trigger to object
    orxObject_AddTriggerRecursive(mpstObject, _zTrigger);
  }
  else
  {
    // Adds trigger to object
    orxObject_AddTrigger(mpstObject, _zTrigger);
  }
}

void ScrollObject::RemoveTrigger(const orxSTRING _zTrigger, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Removes trigger to object
    orxObject_RemoveTriggerRecursive(mpstObject, _zTrigger);
  }
  else
  {
    // Removes trigger to object
    orxObject_RemoveTrigger(mpstObject, _zTrigger);
  }
}

void ScrollObject::FireTrigger(const orxSTRING _zEvent, const orxSTRING *_azRefinementList, orxU32 _u32Count, orxBOOL _bRecursive)
{
  // Recursive?
  if(_bRecursive)
  {
    // Fires an object's trigger
    orxObject_FireTriggerRecursive(mpstObject, _zEvent, _azRefinementList, _u32Count);
  }
  else
  {
    // Fires an object's trigger
    orxObject_FireTrigger(mpstObject, _zEvent, _azRefinementList, _u32Count);
  }
}

void ScrollObject::PushConfigSection(orxBOOL _bPushInstanceSection) const
{
  // Pushes its section
  orxConfig_PushSection(_bPushInstanceSection ? GetInstanceName() : GetName());
}

ScrollObject *ScrollObject::GetParent() const
{
  orxOBJECT    *pstParent;
  ScrollObject *poResult = orxNULL;

  // Gets parent
  pstParent = orxOBJECT(orxObject_GetParent(mpstObject));

  // Valid?
  if(pstParent)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstParent);
  }

  // Done!
  return poResult;
}

void ScrollObject::SetParent(ScrollObject *_poParent)
{
  orxObject_SetParent(mpstObject, _poParent ? _poParent->mpstObject : orxNULL);
}

ScrollObject *ScrollObject::GetOwner() const
{
  orxOBJECT    *pstOwner;
  ScrollObject *poResult = orxNULL;

  // Gets Owner
  pstOwner = orxOBJECT(orxObject_GetOwner(mpstObject));

  // Valid?
  if(pstOwner)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstOwner);
  }

  // Done!
  return poResult;
}

void ScrollObject::SetOwner(ScrollObject *_poOwner)
{
  orxObject_SetOwner(mpstObject, _poOwner ? _poOwner->mpstObject : orxNULL);
}

ScrollObject *ScrollObject::FindChild(const orxSTRING _zPath) const
{
  orxOBJECT    *pstChild;
  ScrollObject *poResult = orxNULL;

  // Finds child
  pstChild = orxObject_FindChild(mpstObject, _zPath);

  // Valid?
  if(pstChild)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstChild);
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollObject::GetChild() const
{
  orxOBJECT    *pstChild;
  ScrollObject *poResult = orxNULL;

  // Gets child
  pstChild = orxObject_GetChild(mpstObject);

  // Valid?
  if(pstChild)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstChild);
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollObject::GetSibling() const
{
  orxOBJECT    *pstSibling;
  ScrollObject *poResult = orxNULL;

  // Gets sibling
  pstSibling = orxObject_GetSibling(mpstObject);

  // Valid?
  if(pstSibling)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstSibling);
  }

  // Done!
  return poResult;
}

ScrollObject *ScrollObject::FindOwnedChild(const orxSTRING _zPath) const
{
  orxOBJECT    *pstChild;
  ScrollObject *poResult = orxNULL;

  // Finds child
  pstChild = orxObject_FindOwnedChild(mpstObject, _zPath);

  // Valid?
  if(pstChild)
  {
    // Updates result
    poResult = (ScrollObject *)orxObject_GetUserData(pstChild);
  }

  // Done!
  return poResult;
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

void ScrollObject::SetOrxObject(orxOBJECT *_pstObject)
{
  // Had an object?
  if(mpstObject != orxNULL)
  {
    // Checks
    orxASSERT(*macInstanceName != orxCHAR_NULL);

    // Clears its instance section
    orxConfig_ClearSection(macInstanceName);
  }

  // Stores it
  mpstObject = _pstObject;

  // Valid?
  if(_pstObject)
  {
    // Stores its names
    mzName = orxObject_GetName(_pstObject);
    orxString_NPrint(macInstanceName, sizeof(macInstanceName), "0x%016llX", orxStructure_GetGUID(_pstObject));
    
    // Creates its instance section
    orxConfig_SetParent(macInstanceName, mzName);
  }
  else
  {
    // Clears its names
    mzName = orxNULL;
    *macInstanceName = orxCHAR_NULL;
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

void ScrollObject::OnCollide(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal)
{
}

void ScrollObject::OnSeparate(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart)
{
}

void ScrollObject::OnSpawn(ScrollObject *_poSpawned)
{
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

void ScrollObject::OnFXStart(const orxSTRING _zFX, orxFX *_pstFX)
{
}

void ScrollObject::OnFXStop(const orxSTRING _zFX, orxFX *_pstFX)
{
}

void ScrollObject::OnFXLoop(const orxSTRING _zFX, orxFX *_pstFX)
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

#endif // __SCROLL_IMPL__

#endif // _SCROLLOBJECT_H_
