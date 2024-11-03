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

#ifndef _SCROLL_H_
#define _SCROLL_H_


//! Includes
#include "ScrollBase.h"


//! Scroll template class
template<class G>
class Scroll : public ScrollBase
{
public:

  static        G &             GetInstance();
                void            Execute(int argc, char **argv);

                ScrollObject *  CreateObject(const orxSTRING _zName)        {return ScrollBase::CreateObject(_zName);}
          template<class O> O * CreateObject(const orxSTRING _zName)        {return ScrollCast<O *>(CreateObject(_zName));}
                void            DeleteObject(ScrollObject *_poObject)       {ScrollBase::DeleteObject(_poObject);}


protected:

                                Scroll<G>()                                 {}
  virtual                      ~Scroll<G>()                                 {}


private:

  virtual       orxSTATUS       Init()                                      {return orxSTATUS_FAILURE;}
  virtual       orxSTATUS       Run()                                       {return orxSTATUS_FAILURE;}
  virtual       void            Exit()                                      {}
  virtual       void            Update(const orxCLOCK_INFO &_rstInfo)       {}
  virtual       void            CameraUpdate(const orxCLOCK_INFO &_rstInfo) {}
  virtual       void            BindObjects()                               {}

  virtual       void            OnObjectCreate(ScrollObject *_poObject)     {}
  virtual       void            OnObjectDelete(ScrollObject *_poObject)     {}
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

//! Constants


//! Code
template<class G>
const orxSTRING Scroll<G>::GetEncryptionKey() const
{
  return "This is Scroll's default encryption key!";
}

template<class G>
orxSTATUS Scroll<G>::Bootstrap() const
{
  return orxSTATUS_SUCCESS;
}

template<class G>
void Scroll<G>::Execute(int argc, char **argv)
{
  // Executes the game
  ScrollBase::GetInstance().Execute(argc, argv);
}

#endif // __SCROLL_IMPL__

#endif // _SCROLL_H_
