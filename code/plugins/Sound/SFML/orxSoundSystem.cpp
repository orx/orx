/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxSoundSystem.cpp
 * @date 19/07/2008
 * @author iarwain@orx-project.org
 *
 * SFML sound system plugin implementation
 *
 */

#include "orxPluginAPI.h"

#include <SFML/Audio.hpp>


/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */


namespace orxSoundSystem
{
  static const orxFLOAT sfDefaultDimensionRatio = orx2F(0.01f);
}


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal sound structure
 */
struct __orxSOUNDSYSTEM_SOUND_t
{
  union
  {
    sf::Sound *poSound;
    sf::Music *poMusic;
  };
  bool  bIsMusic;
};

/** Static structure
 */
typedef struct __orxSOUNDSYSTEM_STATIC_t
{
  orxFLOAT          fDimensionRatio;    /**< Dimension ratio */
  orxFLOAT          fRecDimensionRatio; /**< Reciprocal dimension ratio */
  sf::SoundBuffer  *poDummyBuffer;      /**< Dummy sound buffer to prevent SFML from crashing on MacOS X 10.4 */
  orxU32            u32Flags;

} orxSOUNDSYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSOUNDSYSTEM_STATIC sstSoundSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY))
  {
    orxFLOAT fRatio;

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

    /* Creates dummy buffer to prevents SFML from crashing on MacOS X 10.4 */
    sstSoundSystem.poDummyBuffer = new sf::SoundBuffer();

    /* Sets 2D listener target */
    sf::Listener::SetTarget(0.0f, 0.0f, -1.0f);

    /* Gets dimension ratio */
    orxConfig_PushSection(orxSOUNDSYSTEM_KZ_CONFIG_SECTION);
    fRatio = orxConfig_GetFloat(orxSOUNDSYSTEM_KZ_CONFIG_RATIO);

    /* Valid? */
    if(fRatio > orxFLOAT_0)
    {
      /* Stores it */
      sstSoundSystem.fDimensionRatio = fRatio;
    }
    else
    {
      /* Stores default one */
      sstSoundSystem.fDimensionRatio = (orxFLOAT)orxSoundSystem::sfDefaultDimensionRatio;
    }

    /* Stores reciprocal dimenstion ratio */
    sstSoundSystem.fRecDimensionRatio = orxFLOAT_1 / sstSoundSystem.fDimensionRatio;

    /* Updates status */
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY, orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL);

    /* Pops config section */
    orxConfig_PopSection();

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxSoundSystem_SFML_Exit()
{
  /* Was initialized? */
  if(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Deletes dummy buffer */
    delete sstSoundSystem.poDummyBuffer;

    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
  }

  return;
}

extern "C" orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_SFML_LoadSample(const orxSTRING _zFilename)
{
  orxSOUNDSYSTEM_SAMPLE *pstResult;
  sf::SoundBuffer       *poBuffer;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Creates empty sound buffer */
  poBuffer = new sf::SoundBuffer();

  /* Loads it from file */
  if(poBuffer->LoadFromFile(_zFilename) != false)
  {
    /* Updates result */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)poBuffer;
  }
  else
  {
    /* Deletes sound buffer */
    delete poBuffer;

    /* Updates result */
    pstResult = (orxSOUNDSYSTEM_SAMPLE *)orxNULL;
  }

  /* Done! */
  return pstResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_UnloadSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  sf::SoundBuffer *poBuffer;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Gets sound buffer */
  poBuffer = (sf::SoundBuffer *)_pstSample;

  /* Deletes it */
  delete poBuffer;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

extern "C" orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_SFML_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  orxSOUNDSYSTEM_SOUND *pstResult;
  sf::SoundBuffer      *poBuffer;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Gets sound buffer */
  poBuffer = (sf::SoundBuffer *)_pstSample;

  /* Creates result */
  pstResult = new orxSOUNDSYSTEM_SOUND();

  /* Creates a sound */
  pstResult->poSound = new sf::Sound(*poBuffer);

  /* Updates its status */
  pstResult->bIsMusic = false;

  /* Done! */
  return pstResult;
}

extern "C" orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_SFML_CreateStreamFromFile(const orxSTRING _zFilename)
{
  orxSOUNDSYSTEM_SOUND *pstResult;
  sf::Music            *poMusic;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Creates empty music */
  poMusic = new sf::Music();

  /* Loads it from file */
  if(poMusic->OpenFromFile(_zFilename) != false)
  {
    /* Creates result */
    pstResult = new orxSOUNDSYSTEM_SOUND();

    /* Stores musics */
    pstResult->poMusic = poMusic;
    
    /* Updates its status */
    pstResult->bIsMusic = true;
  }
  else
  {
    /* Deletes music */
    delete poMusic;

    /* Updates result */
    pstResult = (orxSOUNDSYSTEM_SOUND *)orxNULL;
  }

  /* Done! */
  return pstResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Deletes its music */
    delete _pstSound->poMusic;
  }
  else
  {
    /* Deletes its sound */
    delete _pstSound->poSound;
  }

  /* Deletes it */
  delete _pstSound;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Plays it */
    _pstSound->poMusic->Play();
  }
  else
  {
    /* Plays it */
    _pstSound->poSound->Play();
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Pauses it */
    _pstSound->poMusic->Pause();
  }
  else
  {
    /* Pauses it */
    _pstSound->poSound->Pause();
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Stops it */
    _pstSound->poMusic->Stop();
  }
  else
  {
    /* Stops it */
    _pstSound->poSound->Stop();
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Sets its volume */
    _pstSound->poMusic->SetVolume(100.0f * _fVolume);
  }
  else
  {
    /* Sets its volume */
    _pstSound->poSound->SetVolume(100.0f * _fVolume);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Sets its pitch */
    _pstSound->poMusic->SetPitch(_fPitch);
  }
  else
  {
    /* Sets its pitch */
    _pstSound->poSound->SetPitch(_fPitch);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Sets its position */
    _pstSound->poMusic->SetPosition(sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio *_pvPosition->fY, sstSoundSystem.fDimensionRatio *_pvPosition->fZ);
  }
  else
  {
    /* Sets its position */
    _pstSound->poSound->SetPosition(sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Sets its volume */
    _pstSound->poMusic->SetAttenuation(_fAttenuation);
  }
  else
  {
    /* Sets its volume */
    _pstSound->poSound->SetAttenuation(_fAttenuation);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Sets its volume */
    _pstSound->poMusic->SetMinDistance(_fDistance);
  }
  else
  {
    /* Sets its volume */
    _pstSound->poSound->SetMinDistance(_fDistance);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Sets its looping type */
    _pstSound->poMusic->SetLoop((_bLoop != orxFALSE) ? true : false);
  }
  else
  {
    /* Sets its looping type */
    _pstSound->poSound->SetLoop((_bLoop != orxFALSE) ? true : false);
  }

  /* Done! */
  return eResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_SFML_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Gets its volume */
    fResult = orx2F(0.01f * _pstSound->poMusic->GetVolume());
  }
  else
  {
    /* Gets its volume */
    fResult = orx2F(0.01f * _pstSound->poSound->GetVolume());
  }

  /* Done! */
  return fResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_SFML_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Gets its pitch */
    fResult = _pstSound->poMusic->GetPitch();
  }
  else
  {
    /* Gets its pitch */
    fResult = _pstSound->poSound->GetPitch();
  }

  /* Done! */
  return fResult;
}

extern "C" orxVECTOR *orxFASTCALL orxSoundSystem_SFML_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  sf::Vector3f  vPosition;
  orxVECTOR    *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Gets its position */
    vPosition = _pstSound->poMusic->GetPosition();
  }
  else
  {
    /* Gets its position */
    vPosition = _pstSound->poSound->GetPosition();
  }

  /* Updates result */
  orxVector_Set(pvResult, sstSoundSystem.fRecDimensionRatio * vPosition.x, sstSoundSystem.fRecDimensionRatio * vPosition.y, sstSoundSystem.fRecDimensionRatio * vPosition.z);

  /* Done! */
  return pvResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_SFML_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Gets its volume */
    fResult = orx2F(_pstSound->poMusic->GetAttenuation());
  }
  else
  {
    /* Gets its volume */
    fResult = orx2F(_pstSound->poSound->GetAttenuation());
  }

  /* Done! */
  return fResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_SFML_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Gets its volume */
    fResult = orx2F(_pstSound->poMusic->GetMinDistance());
  }
  else
  {
    /* Gets its volume */
    fResult = orx2F(_pstSound->poSound->GetMinDistance());
  }

  /* Done! */
  return fResult;
}

extern "C" orxBOOL orxFASTCALL orxSoundSystem_SFML_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Gets its looping status */
    bResult = _pstSound->poMusic->GetLoop() ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Gets its pitch */
    bResult = _pstSound->poSound->GetLoop() ? orxTRUE: orxFALSE;
  }

  /* Done! */
  return bResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_SFML_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Gets its duration */
    fResult = _pstSound->poMusic->GetDuration();
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

extern "C" orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_SFML_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxSOUNDSYSTEM_STATUS eResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSound != orxNULL);

  /* Is a music? */
  if(_pstSound->bIsMusic != false)
  {
    /* Depending on music status */
    switch(_pstSound->poMusic->GetStatus())
    {
      case sf::Music::Playing:
      {
        /* Updates result */
        eResult = orxSOUNDSYSTEM_STATUS_PLAY;

        break;
      }

      case sf::Music::Paused:
      {
        /* Updates result */
        eResult = orxSOUNDSYSTEM_STATUS_PAUSE;

        break;
      }

      case sf::Music::Stopped:
      default:
      {
        /* Updates result */
        eResult = orxSOUNDSYSTEM_STATUS_STOP;

        break;
      }
    }
  }
  else
  {
    /* Depending on sound status */
    switch(_pstSound->poSound->GetStatus())
    {
      case sf::Sound::Playing:
      {
        /* Updates result */
        eResult = orxSOUNDSYSTEM_STATUS_PLAY;

        break;
      }

      case sf::Sound::Paused:
      {
        /* Updates result */
        eResult = orxSOUNDSYSTEM_STATUS_PAUSE;

        break;
      }

      case sf::Sound::Stopped:
      default:
      {
        /* Updates result */
        eResult = orxSOUNDSYSTEM_STATUS_STOP;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_SetGlobalVolume(orxFLOAT _fVolume)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Updates listener volume */
  sf::Listener::SetGlobalVolume(100.0f * _fVolume);

  /* Done! */
  return eResult;
}

extern "C" orxFLOAT orxFASTCALL orxSoundSystem_SFML_GetGlobalVolume()
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets listener volume */
  fResult = orx2F(sf::Listener::GetGlobalVolume());

  /* Done! */
  return fResult;
}

extern "C" orxSTATUS orxFASTCALL orxSoundSystem_SFML_SetListenerPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Updates listener position */
  sf::Listener::SetPosition(sstSoundSystem.fDimensionRatio * _pvPosition->fX, sstSoundSystem.fDimensionRatio * _pvPosition->fY, sstSoundSystem.fDimensionRatio * _pvPosition->fZ);

  /* Done! */
  return eResult;
}

extern "C" orxVECTOR *orxFASTCALL orxSoundSystem_SFML_GetListenerPosition(orxVECTOR *_pvPosition)
{
  sf::Vector3f vPosition;
  orxVECTOR    *pvResult;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets listener position */
  vPosition = sf::Listener::GetPosition();

  /* Updates result */
  pvResult = _pvPosition;
  orxVector_Set(pvResult, sstSoundSystem.fRecDimensionRatio * vPosition.x, sstSoundSystem.fRecDimensionRatio * vPosition.y, sstSoundSystem.fRecDimensionRatio * vPosition.z);

  /* Done! */
  return pvResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_LoadSample, SOUNDSYSTEM, LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_UnloadSample, SOUNDSYSTEM, UNLOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_SetPitch, SOUNDSYSTEM, SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_SetAttenuation, SOUNDSYSTEM, SET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_SetReferenceDistance, SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetAttenuation, SOUNDSYSTEM, GET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetReferenceDistance, SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_SetGlobalVolume, SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetGlobalVolume, SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_SetListenerPosition, SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_END();
