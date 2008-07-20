/**
 * @file orxSoundSystem.cpp
 *
 * SFML Sound System plugin
 */

 /***************************************************************************
 begin                : 19/07/2008
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
  #include "orxInclude.h"

  #include "plugin/orxPluginUser.h"
  #include "plugin/orxPlugin.h"

  #include "sound/orxSoundSystem.h"
}

#include <SFML/Audio.hpp>


/** Module flags
 */
#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_NONE      0x00000000 /**< No flags */

#define orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY     0x00000001 /**< Ready flag */

#define orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL       0xFFFFFFFF /**< All mask */


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
  orxU32            u32Flags;

} orxSOUNDSYSTEM_STATIC;

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxSOUNDSYSTEM_STATIC sstSoundSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

extern "C" orxSTATUS orxSoundSystem_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));

    /* Updates status */
    orxFLAG_SET(sstSoundSystem.u32Flags, orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY, orxSOUNDSYSTEM_KU32_STATIC_MASK_ALL);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

extern "C" orxVOID orxSoundSystem_SFML_Exit()
{
  /* Was initialized? */
  if(sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSoundSystem, sizeof(orxSOUNDSYSTEM_STATIC));
  }

  return;
}

extern "C" orxSOUNDSYSTEM_SAMPLE *orxSoundSystem_SFML_LoadSample(orxCONST orxSTRING _zFilename)
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

extern "C" orxVOID orxSoundSystem_SFML_UnloadSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  sf::SoundBuffer *poBuffer;

  /* Checks */
  orxASSERT((sstSoundSystem.u32Flags & orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY) == orxSOUNDSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Gets sound buffer */
  poBuffer = (sf::SoundBuffer *)_pstSample;

  /* Deletes it */
  delete poBuffer;

  return;
}

extern "C" orxSOUNDSYSTEM_SOUND *orxSoundSystem_SFML_CreateFromSample(orxCONST orxSOUNDSYSTEM_SAMPLE *_pstSample)
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

extern "C" orxSOUNDSYSTEM_SOUND *orxSoundSystem_SFML_CreateStreamFromFile(orxCONST orxSTRING _zFilename)
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

extern "C" orxVOID orxSoundSystem_SFML_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
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

  return;
}

extern "C" orxSTATUS orxSoundSystem_SFML_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
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

extern "C" orxSTATUS orxSoundSystem_SFML_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
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

extern "C" orxSTATUS orxSoundSystem_SFML_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
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

extern "C" orxSTATUS orxSoundSystem_SFML_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
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

extern "C" orxSTATUS orxSoundSystem_SFML_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
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

extern "C" orxSTATUS orxSoundSystem_SFML_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, orxCONST orxVECTOR *_pvPosition)
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
    _pstSound->poMusic->SetPosition(_pvPosition->fX, _pvPosition->fY, _pvPosition->fZ);
  }
  else
  {
    /* Sets its position */
    _pstSound->poSound->SetPosition(_pvPosition->fX, _pvPosition->fY, _pvPosition->fZ);
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxSoundSystem_SFML_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
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

extern "C" orxFLOAT orxSoundSystem_SFML_GetVolume(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
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

extern "C" orxFLOAT orxSoundSystem_SFML_GetPitch(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
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

extern "C" orxVECTOR *orxSoundSystem_SFML_GetPosition(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
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
  orxVector_Set(pvResult, vPosition.x, vPosition.y, vPosition.z);

  /* Done! */
  return pvResult;
}

extern "C" orxBOOL orxSoundSystem_SFML_IsLooping(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
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

extern "C" orxFLOAT orxSoundSystem_SFML_GetDuration(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
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

extern "C" orxSOUNDSYSTEM_STATUS orxSoundSystem_SFML_GetStatus(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_SFML_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_END();
