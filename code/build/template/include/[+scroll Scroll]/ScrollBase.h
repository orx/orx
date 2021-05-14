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

#ifndef _SCROLLBASE_H_
#define _SCROLLBASE_H_


#include "orx.h"

//! Defines
#if defined(__orxDEBUG__) && !defined(__orxANDROID__) && !defined(__orxANDROID_NATIVE__)

  #define __SCROLL_DEBUG__

#endif // __orxDEBUG__ && !__orxANDROID__ && !__orxANDROID_NATIVE__

#ifdef __SCROLL_DEBUG__
  #include <typeinfo>
#endif // __SCROLL_DEBUG__

#include <new>

#include "ScrollObject.h"


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
  static        ScrollObjectBinderBase *GetBinder(const orxSTRING _zName, orxBOOL _bAllowDefault = orxTRUE);

                                        ScrollObjectBinderBase(orxS32 _s32SegmentSize, orxU32 _u32ElementSize);
  virtual                              ~ScrollObjectBinderBase();


private:

  static        ScrollObjectBinderBase *GetDefaultBinder();

                ScrollObject *          CreateObject(const orxSTRING _zModelName, const orxSTRING _zInstanceName, ScrollObject::Flag _xFlags);
                ScrollObject *          CreateObject(orxOBJECT *_pstOrxObject, const orxSTRING _zInstanceName, ScrollObject::Flag _xFlags);
                void                    DeleteObject(ScrollObject *_poObject);
                void                    DeleteObject(ScrollObject *_poObject, const orxSTRING _zModelName);
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
  static        void                    Register(const orxSTRING _zModelName, orxS32 _s32SegmentSize);


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

template<class O>
ScrollObjectBinder<O> *ScrollObjectBinder<O>::spoInstance = orxNULL;


//! Scroll object bind helper
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

                orxVIEWPORT *   GetMainViewport() const;
                orxCAMERA *     GetMainCamera() const;
                ScrollObject *  GetObject(orxU64 _u64GUID) const;
          template<class O> O * GetObject(orxU64 _u64GUID) const {return ScrollCast<O *>(GetObject(_u64GUID));}

                ScrollObject *  GetNextObject(const ScrollObject *_poObject = orxNULL, orxBOOL _bChronological = orxFALSE) const;
          template<class O> O * GetNextObject(const O *_poObject = orxNULL) const;
                ScrollObject *  GetPreviousObject(const ScrollObject *_poObject = orxNULL, orxBOOL _bChronological = orxFALSE) const;
          template<class O> O * GetPreviousObject(const O *_poObject = orxNULL) const;

                ScrollObject *  PickObject(const orxVECTOR &_rvPosition, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;
                ScrollObject *  PickObject(const orxVECTOR &_rvPosition, const orxVECTOR *_avOffsetList, orxU32 _u32ListSize, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;
                ScrollObject *  PickObject(const orxVECTOR &_rvPosition, const orxVECTOR &_rvExtent, orxSTRINGID _u32GroupID = orxSTRINGID_UNDEFINED) const;
                ScrollObject *  PickObject(const orxOBOX &_rstBox, orxSTRINGID _stGroupID = orxSTRINGID_UNDEFINED) const;



protected:

                                ScrollBase();
  virtual                      ~ScrollBase();

                orxSTATUS       SetMapName(const orxSTRING _zMapName);
          const orxSTRING       GetMapName() const;
          const orxSTRING       GetMapShortName() const;
                orxSTATUS       LoadMap();
                orxSTATUS       SaveMap(orxBOOL _bEncrypt = orxFALSE, const orxCONFIG_SAVE_FUNCTION _pfnMapSaveFilter = orxNULL);
                orxSTATUS       ResetMap();
                ScrollObject *  CreateObject(const orxSTRING _zModelName, ScrollObject::Flag _xFlags = ScrollObject::FlagNone, const orxSTRING _zInstanceName = orxNULL);
                ScrollObject *  CreateObject(orxOBJECT *_pstOrxObject, ScrollObject::Flag _xFlags = ScrollObject::FlagNone, const orxSTRING _zInstanceName = orxNULL);
                void            DeleteObject(ScrollObject *_poObject);

                orxCOLOR *      GetObjectConfigColor(const ScrollObject *_poObject, orxCOLOR &_rstColor);
                orxSTATUS       SetObjectConfigColor(ScrollObject *_poObject, const orxCOLOR &_rstColor);

                void            SetLayerNumber(orxU32 _u32LayerNumber);
                orxU32          GetLayerNumber() const;

                orxU32          GetLayer(const orxVECTOR &_rvPosition) const;
                orxFLOAT        GetLayerDepth(orxU32 _u32Layer = 0) const;

                void            SetEditorMode(orxBOOL _bEnable = orxTRUE);
                orxBOOL         IsEditorMode() const;

                void            SetDifferentialMode(orxBOOL _bEnable = orxTRUE);
                orxBOOL         IsDifferentialMode() const;

  static  const orxSTRING       szConfigClockFrequency;

  static  const orxSTRING       szInputSetGame;
  static  const orxSTRING       szConfigSectionMap;
  static  const orxSTRING       szConfigMainViewport;
  static  const orxSTRING       szConfigCameraPosition;
  static  const orxSTRING       szConfigCameraZoom;
  static  const orxSTRING       szConfigDifferential;
  static  const orxSTRING       szConfigLayerNumber;
  static  const orxSTRING       szConfigNoSave;
  static  const orxSTRING       szConfigSectionScrollObject;
  static  const orxSTRING       szConfigScrollObjectNumber;
  static  const orxSTRING       szConfigScrollObjectPrefix;
  static  const orxSTRING       szConfigScrollObjectFormat;
  static  const orxSTRING       szConfigScrollObjectRuntimeFormat;
  static  const orxSTRING       szConfigScrollObjectName;
  static  const orxSTRING       szConfigScrollObjectPosition;
  static  const orxSTRING       szConfigScrollObjectRotation;
  static  const orxSTRING       szConfigScrollObjectScale;
  static  const orxSTRING       szConfigScrollObjectColor;
  static  const orxSTRING       szConfigScrollObjectAlpha;
  static  const orxSTRING       szConfigScrollObjectSmoothing;
  static  const orxSTRING       szConfigScrollObjectTiling;
  static  const orxSTRING       szConfigScrollObjectPausable;


private:

  virtual       orxSTATUS       Init() = 0;
  virtual       orxSTATUS       Run() = 0;
  virtual       void            Exit() = 0;
  virtual       void            Update(const orxCLOCK_INFO &_rstInfo) = 0;
  virtual       void            CameraUpdate(const orxCLOCK_INFO &_rstInfo) = 0;
  virtual       void            BindObjects() = 0;
  virtual       orxBOOL         MapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption) = 0;

  virtual       void            OnObjectCreate(ScrollObject *_poObject) = 0;
  virtual       void            OnObjectDelete(ScrollObject *_poObject) = 0;
  virtual       void            OnMapLoad() = 0;
  virtual       void            OnMapSave(orxBOOL _bEncrypt) = 0;
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
                orxBOOL         BaseMapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption);
                orxSTRING       GetNewObjectName(orxCHAR _zName[32], orxBOOL bRunTime = orxFALSE);

                orxSTATUS       AddObject(ScrollObject *_poObject);
                orxSTATUS       RemoveObject(ScrollObject *_poObject);

  static  void                  DeleteInstance();

  static  orxSTATUS orxFASTCALL StaticInit();
  static  orxSTATUS orxFASTCALL StaticRun();
  static  void      orxFASTCALL StaticExit();
  static  void      orxFASTCALL StaticUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext);
  static  void      orxFASTCALL StaticCameraUpdate(const orxCLOCK_INFO *_pstInfo, void *_pstContext);
  static  orxBOOL   orxFASTCALL StaticMapSaveFilter(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption);
  static  orxSTATUS orxFASTCALL StaticEventHandler(const orxEVENT *_pstEvent);
  static  orxSTATUS orxFASTCALL StaticBootstrap();


//! Variables
protected:

  static        ScrollBase *    spoInstance;

private:

                orxLINKLIST     mstObjectList;
                orxLINKLIST     mstObjectChronoList;
                orxSTRING       mzMapName;
                orxBOOL         mbEditorMode;
                orxBOOL         mbDifferentialMode;
                orxBOOL         mbObjectListLocked;
                orxBOOL         mbIsRunning;
                orxBOOL         mbIsPaused;
                const orxSTRING mzCurrentObject;
                orxVIEWPORT *   mpstMainViewport;
                orxCAMERA *     mpstMainCamera;
                orxU32          mu32NextObjectID;
                orxU32          mu32RuntimeObjectID;
                orxU32          mu32LayerNumber;
                orxU32          mu32FrameCount;
                orxCONFIG_SAVE_FUNCTION mpfnCustomMapSaveFilter;
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

//! Inline include
#include "ScrollBase.inl"

#endif // __SCROLL_IMPL__

#endif // _SCROLLBASE_H_
