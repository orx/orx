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

#ifndef _SCROLL_H_
#define _SCROLL_H_


//! Includes
#include "ScrollBase.h"

#ifndef __NO_SCROLLED__
  #include "ScrollEd.h"
#endif // __NO_SCROLLED__


//! Scroll template class
template<class G>
class Scroll : public ScrollBase
{
public:

  static        G &             GetInstance();
                void            Execute(int argc, char **argv);

                ScrollObject *  CreateObject(const orxSTRING _zModelName)   {return ScrollBase::CreateObject(_zModelName, ScrollObject::FlagRunTime);}
          template<class O> O * CreateObject(const orxSTRING _zModelName)   {return ScrollCast<O *>(CreateObject(_zModelName));}
                void            DeleteObject(ScrollObject *_poObject)       {ScrollBase::DeleteObject(_poObject);}

                orxSTATUS       SetMapName(const orxSTRING _zMapName);
          const orxSTRING       GetMapName() const;
          const orxSTRING       GetMapShortName() const;
                orxSTATUS       LoadMap();


protected:

                                Scroll<G>()                                 {}
  virtual                      ~Scroll<G>()                                 {}


private:

  virtual       orxSTATUS       Init()                                      {return orxSTATUS_FAILURE;}
  virtual       orxSTATUS       Run()                                       {return orxSTATUS_FAILURE;}
  virtual       void            Exit()                                      {}
  virtual       void            Update(const orxCLOCK_INFO &_rstInfo)       {}
  virtual       void            CameraUpdate(const orxCLOCK_INFO &_rstInfo) {}
  virtual       orxBOOL         MapSaveFilter(const orxSTRING _zSectionName,
                                              const orxSTRING _zKeyName,
                                              const orxSTRING _zFileName,
                                              orxBOOL _bUseEncryption)      {return orxFALSE;}
  virtual       void            BindObjects()                               {}

  virtual       void            OnObjectCreate(ScrollObject *_poObject)     {}
  virtual       void            OnObjectDelete(ScrollObject *_poObject)     {}
  virtual       void            OnMapLoad()                                 {}
  virtual       void            OnMapSave(orxBOOL _bEncrypt)                {}
  virtual       void            OnStartGame()                               {}
  virtual       void            OnStopGame()                                {}
  virtual       void            OnPauseGame(orxBOOL _bPause)                {}

  virtual const orxSTRING       GetEncryptionKey() const;
  virtual       orxSTATUS       Bootstrap() const;


//! Variables
};


//! Templated code
template<class G>
G &Scroll<G>::GetInstance()
{
  if(!spoInstance)
  {
    spoInstance = new G();
  }

  return *ScrollCast<G *>(spoInstance);
}


#ifdef __SCROLL_IMPL__

//! Inline include
#include "Scroll.inl"

#endif // __SCROLL_IMPL__

#endif // _SCROLL_H_
