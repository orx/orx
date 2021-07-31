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
    FlagNone      = 0x00000000,

    FlagSave      = 0x00000001,
    FlagSmoothed  = 0x00000002,
    FlagTiled     = 0x00000004,
    FlagRunTime   = 0x00000008,
    FlagPausable  = 0x00000010,

    MaskAll       = 0xFFFFFFFF
  };


                void                    SetFlags(Flag _xAddFlags, Flag _xRemoveFlags = FlagNone)  {mxFlags = (Flag)(mxFlags & ~_xRemoveFlags); mxFlags = (Flag)(mxFlags | _xAddFlags);}
                void                    SwapFlags(Flag _xSwapFlags)                               {mxFlags = (Flag)(mxFlags ^ _xSwapFlags);}
                orxBOOL                 TestFlags(Flag _xTestFlags) const                         {return (mxFlags & _xTestFlags) ? orxTRUE : orxFALSE;}
                orxBOOL                 TestAllFlags(Flag _xTestFlags) const                      {return ((mxFlags & _xTestFlags) == _xTestFlags) ? orxTRUE : orxFALSE;}
                Flag                    GetFlags(Flag _xMask = MaskAll) const                     {return (Flag)(mxFlags & _xMask);}
                orxOBJECT *             GetOrxObject() const                                      {return mpstObject;}
                const orxSTRING         GetName() const                                           {return macName;}
                const orxSTRING         GetModelName() const                                      {return mzModelName;}
                orxU64                  GetGUID() const                                           {return orxStructure_GetGUID(mpstObject);}

                orxSTRINGID             GetGroupID() const;
                void                    SetGroupID(orxU32 _u32GroupID, orxBOOL _bRecursive = orxTRUE);

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

                orxVECTOR &             GetSpeed(orxVECTOR &_rvSpeed, orxBOOL _bRelative = orxFALSE) const;
                void                    SetSpeed(const orxVECTOR &_rvSpeed, orxBOOL _bRelative = orxFALSE);

                orxCOLOR &              GetColor(orxCOLOR &_rstColor) const;
                void                    SetColor(const orxCOLOR &_rstColor, orxBOOL _bRecursive = orxTRUE);

                void                    GetFlip(orxBOOL &_rbFlipX, orxBOOL &_rbFlipY) const;
                void                    SetFlip(orxBOOL _bFlipX, orxBOOL _bFlipY, orxBOOL _bRecursive = orxTRUE);

                orxBOOL                 IsAnim(const orxSTRING _zAnim, orxBOOL _bCurrent = orxFALSE);
                void                    SetAnim(const orxSTRING _zAnim, orxBOOL _bCurrent = orxFALSE, orxBOOL _bRecursive = orxFALSE);

                void                    AddFX(const orxSTRING _zFXName, orxBOOL _bRecursive = orxTRUE, orxFLOAT _fPropagationDelay = orxFLOAT_0);
                void                    RemoveFX(const orxSTRING _zFXName, orxBOOL _bRecursive = orxTRUE);

                void                    AddShader(const orxSTRING _zShaderName, orxBOOL _bRecursive = orxTRUE);
                void                    RemoveShader(const orxSTRING _zShaderName, orxBOOL _bRecursive = orxTRUE);

                void                    AddSound(const orxSTRING _zSoundName);
                void                    RemoveSound(const orxSTRING _zSoundName);

                void                    AddTrack(const orxSTRING _zTrackName);
                orxSTATUS               AddConditionalTrack(const orxSTRING _zTrackKey, orxS32 _s32Index = -1);
                void                    RemoveTrack(const orxSTRING _zTrackName);

                orxFLOAT                GetLifeTime() const;
                void                    SetLifeTime(orxFLOAT _fLifeTime);

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

  virtual       orxBOOL                 OnCollide(ScrollObject *_poCollider, const orxSTRING _zPartName, const orxSTRING _zColliderPartName, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal);
  virtual       orxBOOL                 OnSeparate(ScrollObject *_poCollider);

  virtual       void                    OnNewAnim(const orxSTRING _zOldAnim, const orxSTRING _zNewAnim, orxBOOL _bCut);
  virtual       void                    OnAnimUpdate(const orxSTRING _zAnim);
  virtual       void                    OnAnimEvent(const orxSTRING _zAnim, const orxSTRING _zEvent, orxFLOAT _fTime, orxFLOAT _fValue);

                void                    SetDifferentialMode(orxBOOL _bDifferential = orxTRUE);

                void                    SetOrxObject(orxOBJECT *_pstObject);


//! Variables
private:

                orxOBJECT *             mpstObject;
                const orxSTRING         mzModelName;
                Flag                    mxFlags;
                orxLINKLIST_NODE        mstNode;
                orxLINKLIST_NODE        mstChronoNode;
                orxCHAR                 macName[16];
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

//! Inline include
#include "ScrollObject.inl"

#endif // __SCROLL_IMPL__

#endif // _SCROLLOBJECT_H_
