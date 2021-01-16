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
#ifndef __NO_SCROLLED__
static const orxSTRING szParamEditor = "-editor";
#endif // __NO_SCROLLED__

//! Code
template<class G>
orxSTATUS Scroll<G>::SetMapName(const orxSTRING _zMapName)
{
  orxSTATUS eResult;

  // Not in editor mode?
  if(!IsEditorMode())
  {
    // Calls base method
    eResult = ScrollBase::SetMapName(_zMapName);
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

template<class G>
const orxSTRING Scroll<G>::GetMapName() const
{
  // Calls base method
  return ScrollBase::GetMapName();
}

template<class G>
const orxSTRING Scroll<G>::GetMapShortName() const
{
  // Calls base method
  return ScrollBase::GetMapShortName();
}

template<class G>
orxSTATUS Scroll<G>::LoadMap()
{
  orxSTATUS eResult;

  // Not in editor mode?
  if(!IsEditorMode())
  {
    // Calls base method
    eResult = ScrollBase::LoadMap();
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

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
#ifdef __NO_SCROLLED__
  // Executes game
  ScrollBase::GetInstance().Execute(argc, argv);
#else // __NO_SCROLLED__
  orxBOOL bEditor = orxFALSE;

  // For all params
  for(int i = 0; !bEditor && (i < argc); i++)
  {
    // Is editor switch?
    if(!orxString_ICompare(argv[i], szParamEditor))
    {
      // Updates editor status
      bEditor = true;
      break;
    }
  }

  // Editor mode?
  if(bEditor)
  {
    // Executes editor
    ScrollEd::GetInstance().Execute(argc, argv);
  }
  else
  {
    // Executes game
    ScrollBase::GetInstance().Execute(argc, argv);
  }
#endif // __NO_SCROLLED__
}
