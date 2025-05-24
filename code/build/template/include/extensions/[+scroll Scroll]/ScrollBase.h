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

#ifndef _SCROLLBASE_H_
#define _SCROLLBASE_H_

#include "orx.h"

//! Defines
#if defined(__orxDEBUG__) && !defined(__orxANDROID__)

  #define __SCROLL_DEBUG__

#endif // __orxDEBUG__ && !__orxANDROID__

#ifdef __SCROLL_DEBUG__
  #include <typeinfo>
#endif // __SCROLL_DEBUG__

#include <new>

#include "ScrollObject.h"


//! Code
inline void *operator new(size_t _Size, orxBANK *_pstBank)
{
  // Done!
  return orxBank_Allocate(_pstBank);
}

inline void operator delete(void *_p, orxBANK *_pstBank)
{
  // Done!
  orxBank_Free(_pstBank, _p);
}


//! Template helpers
template<class T, class U>
inline static T ScrollCast(U _p)
{
#ifdef __SCROLL_DEBUG__
  T pDummy = orxNULL;

  // Valid?
  if(_p)
  {
    // Calls dynamic cast
    pDummy = dynamic_cast<T>(_p);

    // Checks
    orxASSERT(pDummy && "Impossible cast from %s to %s!", typeid(U).name(), typeid(T).name());
  }

  // Done!
  return pDummy;

#else // __SCROLL_DEBUG__

  // Done!
  return static_cast<T>(_p);

#endif // __SCROLL_DEBUG__
}

template<class Child, class Parent>
struct ScrollIsA
{
  struct stNo
  {
  };

  struct stYes
  {
    orxU32 u32Dummy;
  };

  static stNo   Convert(...);
  static stYes  Convert(const Parent &);
  enum {Value = (sizeof(Convert(*(Child *)0)) == sizeof(stYes))};
};

template <class T>
struct ScrollIsPolymorphic
{
  struct Dummy1 : public T
  {
                  Dummy1();
            char  au8Padding[256];
  };
  struct Dummy2 : public T
  {
                  Dummy2();
    virtual void  DummyVirt(void);
            char  au8Padding[256];
  };

  enum {Value = (sizeof(Dummy1) == sizeof(Dummy2))};
};


//! Object binder abstract class
class ScrollObjectBinderBase
{
  friend class ScrollBase;

protected:

  static        orxHASHTABLE *          GetTable();
  static        void                    DeleteTable();
  static        ScrollObjectBinderBase *GetBinder(const orxSTRING _zName);

                                        ScrollObjectBinderBase(orxS32 _s32SegmentSize, orxU32 _u32ElementSize);
  virtual                              ~ScrollObjectBinderBase();


private:

  static        ScrollObjectBinderBase *GetDefaultBinder();

                ScrollObject *          CreateObject(orxOBJECT *_pstObject);
                void                    DeleteObject(ScrollObject *_poObject);
  virtual       ScrollObject *          ConstructObject(orxBANK * _pstBank) const = 0;
                void                    DestructObject(ScrollObject *_poObject) const;

                ScrollObject *          GetNextObject(const ScrollObject *_poObject = orxNULL) const;
                ScrollObject *          GetPreviousObject(const ScrollObject *_poObject = orxNULL) const;


//! Variables
private:

  static  const orxU32                  su32TableSize;

                orxBANK *               mpstBank;
                ScrollObject *          mpoFirstObject;
                ScrollObject *          mpoLastObject;
  static        orxHASHTABLE *          spstTable;
};


//! Object binder template class
template<class O>
class ScrollObjectBinder : public ScrollObjectBinderBase
{
  friend class ScrollBase;

public:

  static        ScrollObjectBinder<O> * GetInstance(orxS32 _s32SegmentSize = -1);
  static        void                    Register(const orxSTRING _zName, orxS32 _s32SegmentSize);


protected:

                                        ScrollObjectBinder(orxS32 _s32SegmentSize);
  virtual                              ~ScrollObjectBinder();


private:

  virtual       ScrollObject *          ConstructObject(orxBANK *_pstBank) const;


//! Variables
private:

  static        ScrollObjectBinder<O> * spoInstance;
};

template<class O>
ScrollObjectBinder<O> *ScrollObjectBinder<O>::GetInstance(orxS32 _s32SegmentSize)
{
  // First call?
  if(!spoInstance)
  {
    // Valid segment size?
    if(_s32SegmentSize > 0)
    {
      // Creates instance
      spoInstance = new ScrollObjectBinder<O>(_s32SegmentSize);
    }
  }

  // Done!
  return spoInstance;
}

template<class O>
void ScrollObjectBinder<O>::Register(const orxSTRING _zName, orxS32 _s32SegmentSize)
{
  // Checks
  orxASSERT(!orxHashTable_Get(ScrollObjectBinderBase::GetTable(), orxString_Hash(_zName)));
  orxASSERT(_s32SegmentSize > 0);

  // Adds binder to table
  orxHashTable_Add(GetTable(), orxString_Hash(_zName ? _zName : orxSTRING_EMPTY), GetInstance(_s32SegmentSize));
}

template<class O>
ScrollObjectBinder<O>::ScrollObjectBinder(orxS32 _s32SegmentSize) : ScrollObjectBinderBase(_s32SegmentSize, sizeof(O))
{
}

template<class O>
ScrollObjectBinder<O>::~ScrollObjectBinder()
{
  // Removes instance
  spoInstance = orxNULL;
}

template<class O>
ScrollObject *ScrollObjectBinder<O>::ConstructObject(orxBANK *_pstBank) const
{
  // Done!
  return new(_pstBank) O();
}


//! Scroll object bind helper
#if defined(__orxGCC__) || defined(__orxLLVM__)
  #define BindObject(OBJECT, ...)       ScrollBindObject<OBJECT>(#OBJECT, ##__VA_ARGS__)
#else // __orxGCC__ || __orxLLVM__
  #define BindObject(OBJECT, ...)       ScrollBindObject<OBJECT>(#OBJECT, __VA_ARGS__)
#endif // __orxGCC__ || __orxLLVM__
template<class O>
inline static void ScrollBindObject(const orxSTRING _zName, orxS32 _s32SegmentSize = 128)
{
#ifdef __SCROLL_DEBUG__
  if(!ScrollIsA<O, ScrollObject>::Value)
  {
    orxLOG("[ERROR] Binding object <%s>: class %s has to derive from class ScrollObject!", _zName, typeid(O).name());
  }
#endif // __SCROLL_DEBUG__

  // Instances corresponding binder
  ScrollObjectBinder<O>::Register(_zName, _s32SegmentSize);
}


//! ScrollBase abstract class
class ScrollBase
{
  friend class ScrollEd;
  friend class ScrollObjectBinderBase;

public:

  static        ScrollBase &    GetInstance();
                void            Execute(int argc, char **argv);

                orxSTATUS       StartGame();
                orxSTATUS       StopGame();
                orxBOOL         IsGameRunning() const;
                orxSTATUS       PauseGame(orxBOOL _bPause);
                orxBOOL         IsGamePaused() const;

                orxU32          GetFrameCount() const;

                ScrollObject *  GetObject(orxU64 _u64GUID) const;
          template<class O> O * GetObject(orxU64 _u64GUID) const {return ScrollCast<O *>(GetObject(_u64GUID));}
                ScrollObject *  GetObject(const orxSTRING _zSection, const orxSTRING _zKey = ScrollBase::szConfigScrollObjectID /* = ID */) const;
          template<class O> O * GetObject(const orxSTRING _zSection, const orxSTRING _zKey = ScrollBase::szConfigScrollObjectID /* = ID */) const {return ScrollCast<O *>(GetObject(_zSection, _zKey));}

                ScrollObject *  GetNextObject(const ScrollObject *_poObject = orxNULL, orxBOOL _bChronological = orxFALSE) const;
          template<class O> O * GetNextObject(const O *_poObject = orxNULL) const;
                ScrollObject *  GetPreviousObject(const ScrollObject *_poObject = orxNULL, orxBOOL _bChronological = orxFALSE) const;
          template<class O> O * GetPreviousObject(const O *_poObject = orxNULL) const;

                ScrollObject *  PickObject(const orxVECTOR &_rvPosition, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;
                ScrollObject *  PickObject(const orxVECTOR &_rvPosition, const orxVECTOR *_avOffsetList, orxU32 _u32ListSize, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;
                ScrollObject *  PickObject(const orxVECTOR &_rvPosition, const orxVECTOR &_rvExtent, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;
                ScrollObject *  PickObject(const orxOBOX &_rstBox, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;



protected:

                                ScrollBase();
  virtual                      ~ScrollBase();

                ScrollObject *  CreateObject(const orxSTRING _zName);
                void            DeleteObject(ScrollObject *_poObject);

  static  const orxSTRING       szConfigScrollObjectPausable;
  static  const orxSTRING       szConfigScrollObjectInput;
  static  const orxSTRING       szConfigScrollObjectUnique;
  static  const orxSTRING       szConfigScrollObjectID;
  static  const orxCHAR         scConfigScrollObjectInstantMarker   = '.';
  static  const orxCHAR         scConfigScrollObjectNegativeMarker  = '-';


private:

  virtual       orxSTATUS       Init() = 0;
  virtual       orxSTATUS       Run() = 0;
  virtual       void            Exit() = 0;
  virtual       void            Update(const orxCLOCK_INFO &_rstInfo) = 0;
  virtual       void            CameraUpdate(const orxCLOCK_INFO &_rstInfo) = 0;
  virtual       void            BindObjects() = 0;

  virtual       void            OnObjectCreate(ScrollObject *_poObject) = 0;
  virtual       void            OnObjectDelete(ScrollObject *_poObject) = 0;
  virtual       void            OnStartGame() = 0;
  virtual       void            OnStopGame() = 0;
  virtual       void            OnPauseGame(orxBOOL _bPause) = 0;

  virtual const orxSTRING       GetEncryptionKey() const = 0;
  virtual       orxSTATUS       Bootstrap() const = 0;


                orxSTATUS       BaseInit();
                orxSTATUS       BaseRun();
                void            BaseExit();
                void            BaseUpdate(const orxCLOCK_INFO &_rstInfo);
                void            BaseCameraUpdate(const orxCLOCK_INFO &_rstInfo);


  static  void                  DeleteInstance();

  static  orxSTATUS orxFASTCALL StaticInit();
  static  orxSTATUS orxFASTCALL StaticRun();
  static  void      orxFASTCALL StaticExit();
  static  void      orxFASTCALL StaticUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext);
  static  void      orxFASTCALL StaticCameraUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext);
  static  orxSTATUS orxFASTCALL StaticEventHandler(const orxEVENT *_pstEvent);
  static  orxSTATUS orxFASTCALL StaticBootstrap();


//! Variables
protected:

  static        ScrollBase *    spoInstance;


private:

                orxLINKLIST     mstObjectList;
                orxLINKLIST     mstObjectChronoList;
                orxU32          mu32FrameCount;
                orxBOOL         mbObjectListLocked;
                orxBOOL         mbIsRunning;
                orxBOOL         mbIsPaused;
};


//! Template function definitions
template<class O>
O *ScrollBase::GetNextObject(const O *_poObject) const
{
  const ScrollObjectBinder<O> *poBinder;
  O                           *poResult = orxNULL;

  // Gets binder
  poBinder = ScrollObjectBinder<O>::GetInstance();

  // Valid?
  if(poBinder)
  {
    // Updates result
    poResult = ScrollCast<O *>(poBinder->GetNextObject(_poObject));
  }
#ifdef __SCROLL_DEBUG__
  else
  {
    // Logs message
    orxLOG("Couldn't get next object of <%s>: no registered binder found!", typeid(O).name());
  }
#endif // __SCROLL_DEBUG__

  // Done!
  return poResult;
}

template<class O>
O *ScrollBase::GetPreviousObject(const O *_poObject) const
{
  const ScrollObjectBinder<O> *poBinder;
  O                           *poResult = orxNULL;

  // Gets binder
  poBinder = ScrollObjectBinder<O>::GetInstance();

  // Valid?
  if(poBinder)
  {
    // Updates result
    poResult = ScrollCast<O *>(poBinder->GetPreviousObject(_poObject));
  }
#ifdef __SCROLL_DEBUG__
  else
  {
    // Logs message
    orxLOG("Couldn't get previous object of <%s>: no registered binder found!", typeid(O).name());
  }
#endif // __SCROLL_DEBUG__

  // Done!
  return poResult;
}


#ifdef __SCROLL_IMPL__

#include <stddef.h>


// Deactivates invalid offsetof warnings for GCC & Clang for the rest of this file
#if defined(__orxGCC__)
  #pragma GCC system_header
#elif defined(__orxLLVM__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif


//! Constants
const orxSTRING ScrollBase::szConfigScrollObjectPausable      = "Pausable";
const orxSTRING ScrollBase::szConfigScrollObjectInput         = "Input";
const orxSTRING ScrollBase::szConfigScrollObjectUnique        = "Unique";
const orxSTRING ScrollBase::szConfigScrollObjectID            = "ID";


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

ScrollBase::ScrollBase() : mu32FrameCount(0), mbObjectListLocked(orxFALSE), mbIsRunning(orxFALSE), mbIsPaused(orxFALSE)
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

ScrollObject *ScrollBase::CreateObject(const orxSTRING _zName)
{
  ScrollObject *poResult = orxNULL;

  // Valid?
  if(_zName && (_zName != orxSTRING_EMPTY))
  {
    orxOBJECT *pstObject;

    // Creates object
    pstObject = orxObject_CreateFromConfig(_zName);

    // Valid?
    if(pstObject)
    {
      // Updates result
      poResult = (ScrollObject *)orxObject_GetUserData(pstObject);

      // Calls create callback
      OnObjectCreate(poResult);
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
      // Deletes it
      orxObject_Delete(_poObject->GetOrxObject());
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

ScrollObject *ScrollBase::GetObject(const orxSTRING _zSection, const orxSTRING _zKey) const
{
  orxOBJECT    *pstObject;
  ScrollObject *poResult = orxNULL;

  // Pushes config section
  orxConfig_PushSection(_zSection);

  // Gets object
  pstObject = orxOBJECT(orxStructure_Get(orxConfig_GetU64(_zKey)));

  // Pops config section
  orxConfig_PopSection();

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

orxSTATUS ScrollBase::BaseInit()
{
  orxCLOCK *pstClock;
  orxSTATUS eResult;

  // Binds ScrollObject
  BindObject(ScrollObject);

  // Binds objects
  BindObjects();

  // Gets core clock
  pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

  // Registers update function
  eResult = ((orxClock_Register(pstClock, StaticUpdate, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL) != orxSTATUS_FAILURE)
          && (orxClock_Register(pstClock, StaticCameraUpdate, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_LOWER) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  // Successful?
  if(eResult != orxSTATUS_FAILURE)
  {
    // Adds event handler
    eResult = ((orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_ANIM, StaticEventHandler) != orxSTATUS_FAILURE)
            && (orxEvent_AddHandler(orxEVENT_TYPE_SPAWNER, StaticEventHandler) != orxSTATUS_FAILURE)
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
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_SPAWNER, orxNULL, orxEVENT_GET_FLAG(orxSPAWNER_EVENT_SPAWN), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_OBJECT_START), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_SHADER, orxNULL, orxEVENT_GET_FLAG(orxSHADER_EVENT_SET_PARAM), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_PHYSICS, orxNULL, orxEVENT_GET_FLAG(orxPHYSICS_EVENT_CONTACT_ADD) | orxEVENT_GET_FLAG(orxPHYSICS_EVENT_CONTACT_REMOVE), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(StaticEventHandler, orxEVENT_TYPE_FX, orxNULL, orxEVENT_GET_FLAG(orxFX_EVENT_START) | orxEVENT_GET_FLAG(orxFX_EVENT_STOP) | orxEVENT_GET_FLAG(orxFX_EVENT_LOOP), orxEVENT_KU32_MASK_ID_ALL);

      // Clears object lists
      orxMemory_Zero(&mstObjectList, sizeof(orxLINKLIST));
      orxMemory_Zero(&mstObjectChronoList, sizeof(orxLINKLIST));

      // Updates running status
      mbIsRunning = orxFALSE;

      // Calls child init
      eResult = Init();

      // Successful?
      if(eResult != orxSTATUS_FAILURE)
      {
        // Starts game
        StartGame();
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
  orxEvent_RemoveHandler(orxEVENT_TYPE_SPAWNER, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_SHADER, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_PHYSICS, StaticEventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_FX, StaticEventHandler);

  // Deletes binder's table
  ScrollObjectBinderBase::DeleteTable();
}

void ScrollBase::BaseUpdate(const orxCLOCK_INFO &_rstInfo)
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

        // Has input set?
        if(poObject->mzInputSet != orxNULL)
        {
          // Pushes input set
          orxInput_PushSet(poObject->mzInputSet);

          // Has trigger?
          if(orxOBJECT_GET_STRUCTURE(pstObject, TRIGGER))
          {
            // For all inputs
            for(const orxSTRING zInput = orxInput_GetNext(orxNULL); zInput; zInput = orxInput_GetNext(zInput))
            {
              orxCHAR acBuffer[256], *pc = acBuffer;
              orxBOOL bInstant = orxFALSE;

              // Adds propagation stop marker
              *pc++ = orxTRIGGER_KC_STOP_MARKER;

              // Has new status?
              if(orxInput_HasNewStatus(zInput))
              {
                // Adds instant marker
                *pc++ = scConfigScrollObjectInstantMarker;

                // Updates status
                bInstant = orxTRUE;
              }

              // Is inactive?
              if(!orxInput_IsActive(zInput))
              {
                // Adds negative marker
                *pc++ = scConfigScrollObjectNegativeMarker;
              }

              // Adds input name
              orxString_NPrint(pc, sizeof(acBuffer) - (orxU32)(pc - acBuffer), "%s", zInput);
              pc = acBuffer;

              // Fires trigger
              if((orxObject_FireTrigger(pstObject, szConfigScrollObjectInput, (const orxSTRING *)&pc, 1) == orxSTATUS_FAILURE) && (bInstant != orxFALSE))
              {
                // Gets non-instant trigger event
                for(pc += 2; *pc != orxCHAR_NULL; pc++)
                {
                  *(pc - 1) = *pc;
                }
                *(pc - 1) = orxCHAR_NULL;
                pc = acBuffer;

                // Fires it
                orxObject_FireTrigger(pstObject, szConfigScrollObjectInput, (const orxSTRING *)&pc, 1);
              }
            }
          }
        }

        // Gets its clock
        pstClock = orxObject_GetClock(pstObject);

        // Valid?
        if(pstClock)
        {
          // Not paused?
          if(!orxClock_IsPaused(pstClock))
          {
            orxCLOCK_INFO stClockInfo;

            // Copies its info
            orxMemory_Copy(&stClockInfo, orxClock_GetInfo(pstClock), sizeof(orxCLOCK_INFO));

            // Computes its DT
            stClockInfo.fDT = orxClock_ComputeDT(pstClock, _rstInfo.fDT);

            // Updates object
            poObject->Update(stClockInfo);
          }
        }
        else
        {
          // Updates object
          poObject->Update(_rstInfo);
        }

        // Has input set?
        if(poObject->mzInputSet != orxNULL)
        {
          // Pops input set
          orxInput_PopSet();
        }
      }
    }
  }

  // Unlocks object list
  mbObjectListLocked = orxFALSE;

  // Calls child update
  Update(_rstInfo);
}

void ScrollBase::BaseCameraUpdate(const orxCLOCK_INFO &_rstInfo)
{
  // Calls child update
  CameraUpdate(_rstInfo);
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
        ScrollObjectBinderBase *poBinder;

        // Gets binder
        poBinder = ScrollObjectBinderBase::GetBinder(orxObject_GetName(pstObject));

        // Found?
        if(poBinder)
        {
          // Uses it
          poBinder->CreateObject(pstObject);
        }
      }
      // Delete?
      else if(_pstEvent->eID == orxOBJECT_EVENT_DELETE)
      {
        ScrollObject *poObject;

        // Gets scroll object
        poObject = (ScrollObject *)orxObject_GetUserData(pstObject);

        // Checks
        orxASSERT((!poObject) || (!poObject->mpstObject) || (poObject->mpstObject == pstObject));

        // Valid object (first deletion)?
        if(poObject && poObject->mpstObject)
        {
          // Deletes it through its binder
          orxASSERT(poObject->mpoBinder);
          poObject->mpoBinder->DeleteObject(poObject);
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

    // Spawner event
    case orxEVENT_TYPE_SPAWNER:
    {
      ScrollObject *poSender;

      // Gets sender object
      poSender = (ScrollObject *)orxObject_GetUserData(orxOBJECT(orxStructure_GetOwner(_pstEvent->hSender)));

      // Valid?
      if(poSender)
      {
        // Calls its callback
        poSender->OnSpawn((ScrollObject *)orxObject_GetUserData(orxOBJECT(_pstEvent->hRecipient)));
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
      orxOBJECT *pstSender;

      // Gets sender object
      pstSender = orxOBJECT(_pstEvent->hSender);

      // Valid?
      if(pstSender)
      {
        ScrollObject *poSender;

        // Gets sender object
        poSender = (ScrollObject *)orxObject_GetUserData(pstSender);

        // Valid?
        if(poSender)
        {
          // Calls object callback
          eResult = poSender->OnShader(*(orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
        }
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

ScrollObjectBinderBase *ScrollObjectBinderBase::GetBinder(const orxSTRING _zName)
{
  ScrollObjectBinderBase *poResult = orxNULL;

  // Valid name?
  if(_zName && (_zName != orxSTRING_EMPTY))
  {
    const orxSTRING zSection;

    // Gets associated binder, using config hierarchy
    for(poResult = (ScrollObjectBinderBase *)orxHashTable_Get(GetTable(), orxString_Hash(_zName)), zSection = orxConfig_GetParent(_zName);
        (poResult == orxNULL) && (zSection != orxNULL) && (zSection != orxSTRING_EMPTY);
        poResult = (ScrollObjectBinderBase *)orxHashTable_Get(GetTable(), orxString_Hash(zSection)), zSection = orxConfig_GetParent(zSection));

    // Not found and not explicitly rejected?
    if(!poResult && (zSection != orxSTRING_EMPTY))
    {
      // Gets default binder
      poResult = GetDefaultBinder();
    }
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

ScrollObject *ScrollObjectBinderBase::CreateObject(orxOBJECT *_pstObject)
{
  ScrollObject::Flag  xFlags = ScrollObject::FlagNone;
  ScrollObject       *poResult;
  const orxSTRING     zInputSet;
  const orxSTRING     zUnique;

  // Checks
  orxSTRUCTURE_ASSERT(_pstObject);

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Checks
  orxASSERT(!orxObject_GetUserData(_pstObject));

  // Creates scroll object
  poResult = ConstructObject(mpstBank);

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

  // Adds to chronological list
  orxLinkList_AddEnd(&roGame.mstObjectChronoList, &poResult->mstChronoNode);

  // Stores internal object
  poResult->SetOrxObject(_pstObject);

  // Sets object as user data
  orxObject_SetUserData(_pstObject, poResult);

  // Pushes its section
  poResult->PushConfigSection();

  // Is pausable?
  if((!orxConfig_HasValue(ScrollBase::szConfigScrollObjectPausable))
  || (orxConfig_GetBool(ScrollBase::szConfigScrollObjectPausable)))
  {
    // Updates flags
    xFlags |= ScrollObject::FlagPausable;
  }

  // Gets unique identifier
  zUnique = orxConfig_GetString(ScrollBase::szConfigScrollObjectUnique);
  
  // Valid?
  if(*zUnique != orxCHAR_NULL)
  {
    const orxSTRING zRemaining;
    const orxSTRING zKey = orxNULL;
    orxBOOL         bUnique;
    
    /* Is a bool? */
    if((orxString_ToBool(zUnique, &bUnique, &zRemaining) != orxSTATUS_FAILURE)
    && (*zRemaining == orxCHAR_NULL))
    {
      /* Unique? */
      if(bUnique != orxFALSE)
      {
        /* Uses default ID key */
        zKey = ScrollBase::szConfigScrollObjectID;
      }
    }
    else
    {
      /* Uses it as key */
      zKey = zUnique;
    }
    
    /* Has key? */
    if(zKey != orxNULL)
    {
      /* Stores its GUID */
      orxConfig_SetString(zKey, poResult->GetInstanceName());
    }
  }

  // Gets input set
  zInputSet = orxConfig_GetString(ScrollBase::szConfigScrollObjectInput);

  // Valid?
  if(*zInputSet != orxCHAR_NULL)
  {
    // Instance?
    if((*zInputSet == orxCOMMAND_KC_GUID_MARKER) && (*(zInputSet + 1) == orxCHAR_NULL))
    {
      // Updates it
      zInputSet = poResult->GetInstanceName();
    }

    // Enables it & pushes it
    if((orxInput_EnableSet(zInputSet, orxTRUE) != orxSTATUS_FAILURE)
    && (orxInput_PushSet(zInputSet) != orxSTATUS_FAILURE))
    {
      // Updates flags
      xFlags |= ScrollObject::FlagInput;
      
      // No defined input?
      if(!orxInput_GetNext(orxNULL))
      {
        // Updates its type
        orxInput_SetTypeFlags(orxINPUT_KU32_FLAG_TYPE_NONE, orxINPUT_KU32_MASK_TYPE_ALL);
      }
      else
      {
        // Updates flags
        xFlags |= ScrollObject::FlagInputBinding;
      }

      // Stores its name
      poResult->mzInputSet = (zInputSet == poResult->GetInstanceName()) ? zInputSet : orxInput_GetCurrentSet();

      // Pops set
      orxInput_PopSet();
    }
  }

  // Stores its binder
  poResult->mpoBinder = this;

  // Stores flags
  poResult->SetFlags(xFlags, ScrollObject::MaskAll);

  // Calls it
  poResult->OnCreate();

  // Is game running?
  if(roGame.IsGameRunning())
  {
    // Calls its start game callback
    poResult->OnStartGame();
  }

  // Is game paused?
  if(roGame.IsGamePaused())
  {
    // Calls its pause game callback
    poResult->OnPauseGame(orxTRUE);
  }

  // Pops section
  poResult->PopConfigSection();

  // Done!
  return poResult;
}

void ScrollObjectBinderBase::DeleteObject(ScrollObject *_poObject)
{
  orxBOOL bObjectListBlockBackup;

  // Checks
  orxSTRUCTURE_ASSERT(_poObject->GetOrxObject());

  // Gets game instance
  ScrollBase &roGame = ScrollBase::GetInstance();

  // Calls game callback
  roGame.OnObjectDelete(_poObject);

  // Blocks object list
  bObjectListBlockBackup = roGame.mbObjectListLocked;
  roGame.mbObjectListLocked = orxTRUE;

  // Calls object callback
  _poObject->OnDelete();

  // Restores object list blocking
  roGame.mbObjectListLocked = bObjectListBlockBackup;

  // Has instance input set?
  if(_poObject->mzInputSet == _poObject->GetInstanceName())
  {
    // Removes it
    orxInput_RemoveSet(_poObject->mzInputSet);
  }

  // Removes object as user data
  orxObject_SetUserData(_poObject->GetOrxObject(), orxNULL);

  // Clears internal reference
  _poObject->SetOrxObject(orxNULL);

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

  // Removes it from list
  orxLinkList_Remove(&_poObject->mstNode);

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

#endif // __SCROLL_IMPL__

#endif // _SCROLLBASE_H_
