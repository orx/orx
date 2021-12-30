//! Includes

#ifndef _orxMOD_H_
#define _orxMOD_H_

#include "orx.h"


//! Prototypes

orxSTATUS orxFASTCALL orxMod_Init();
void orxFASTCALL      orxMod_Exit();


//! Implementation

#ifdef orxMOD_IMPL

// #define POCKETMOD_NO_INTERPOLATION // Uncomment this define if you want to deactivate interpolation
#define POCKETMOD_IMPLEMENTATION
#include "pocketmod/pocketmod.h"
#undef POCKETMOD_IMPLEMENTATION
#undef POCKETMOD_NO_INTERPOLATION

#define orxMOD_KU32_SAMPLE_RATE           48000


//! Variables / Structures

typedef struct ModData
{
  pocketmod_context stContext;
  orxOBJECT        *pstObject;
  orxS64            s64Size;
  void             *pBuffer;
  int               iLoopCount;
  orxBOOL           bFirst;
  volatile orxBOOL  bLock;

} ModData;

static orxBANK *spstModBank = orxNULL;


//! Code

static orxSTATUS orxFASTCALL orxMod_Delete(ModData *_pstModData)
{
  // Wait for locked decoder
  while(_pstModData->bLock)
    ;

  // Deletes it
  orxMemory_Free(_pstModData->pBuffer);
  if(spstModBank)
  {
    orxBank_Free(spstModBank, _pstModData);
  }

  // Done!
  return orxSTATUS_SUCCESS;
}

static orxSTATUS orxFASTCALL orxMod_Decode(const orxEVENT *_pstEvent)
{
  ModData *pstModData;

  // Gets movie data
  pstModData = (ModData *)_pstEvent->pContext;

  // Matching object?
  if(pstModData->pstObject && pstModData->pstObject == orxOBJECT(_pstEvent->hSender))
  {
    orxSOUND_EVENT_PAYLOAD *pstPayload;

    // Gets payload
    pstPayload = (orxSOUND_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    // Valid?
    if(orxObject_IsEnabled(pstModData->pstObject) && !orxObject_IsPaused(pstModData->pstObject))
    {
      int       iSize, iLoopCount;
      orxCHAR  *pBuffer;

      // Locks decoders
      pstModData->bLock = orxTRUE;
      orxMEMORY_BARRIER();

      // Not first packet?
      if(!pstModData->bFirst)
      {
        // Has restarted?
        if(pstPayload->stStream.stPacket.fTime == orxFLOAT_0)
        {
          int iResult;

          // Reinits context
          iResult = pocketmod_init(&(pstModData->stContext), pstModData->pBuffer, (int)pstModData->s64Size, orxMOD_KU32_SAMPLE_RATE);
          orxASSERT(iResult);

          // Updates status
          pstModData->bFirst = orxTRUE;
        }
      }
      else
      {
        // Updates status
        pstModData->bFirst = orxFALSE;
      }

      // Gets buffer info
      iSize   = (int)(pstPayload->stStream.stPacket.u32SampleNumber * sizeof(orxFLOAT));
      pBuffer = (orxCHAR *)pstPayload->stStream.stPacket.afSampleList;

      // Fills it
      while(iSize > 0)
      {
        int iWritten;
        iWritten  = pocketmod_render(&(pstModData->stContext), pBuffer, iSize);
        iSize    -= iWritten;
        pBuffer  += iWritten;
      }
      orxASSERT(iSize == 0);

      // Looped?
      if((iLoopCount = pocketmod_loop_count(&(pstModData->stContext))) != pstModData->iLoopCount)
      {
        pstPayload->stStream.stPacket.bLast = orxTRUE;
        pstModData->iLoopCount              = iLoopCount;
        pstModData->bFirst                  = orxTRUE;
      }

      // Unlocks decoders
      orxMEMORY_BARRIER();
      pstModData->bLock = orxFALSE;
    }
    else
    {
      // Updates packet
      pstPayload->stStream.stPacket.bDiscard  = orxTRUE;
      pstPayload->stStream.stPacket.bLast     = orxFALSE;
    }
  }

  // Done!
  return orxSTATUS_SUCCESS;
}

static orxSTATUS orxFASTCALL orxMod_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // New object?
  if(_pstEvent->eID == orxOBJECT_EVENT_PREPARE)
  {
    // Is a movie?
    if(orxConfig_HasValue("Mod"))
    {
      const orxSTRING zMod;
      orxHANDLE       hResource;

      // Has valid resource?
      if((zMod = orxResource_Locate(orxSOUND_KZ_RESOURCE_GROUP, orxConfig_GetString("Mod")))
      && ((hResource = orxResource_Open(zMod, orxFALSE)) != orxHANDLE_UNDEFINED))
      {
        orxS64  s64Size;
        void   *pBuffer;

        // Gets its size
        s64Size = orxResource_GetSize(hResource);
        orxASSERT(s64Size);

        // Loads its content
        pBuffer = orxMemory_Allocate((orxU32)s64Size, orxMEMORY_TYPE_AUDIO);
        orxASSERT(pBuffer);
        if(orxResource_Read(hResource, s64Size, pBuffer, orxNULL, orxNULL) == s64Size)
        {
          ModData    *pstModData;
          orxOBJECT  *pstObject;
          orxCHAR     acBuffer[256];

          // Gets object
          pstObject = orxOBJECT(_pstEvent->hSender);

          // Creates decoder data
          pstModData = (ModData *)orxBank_Allocate(spstModBank);
          orxASSERT(pstModData);
          orxMemory_Zero(pstModData, sizeof(ModData));

          // Inits decoder
          if(pocketmod_init(&(pstModData->stContext), pBuffer, (int)s64Size, orxMOD_KU32_SAMPLE_RATE) != 0)
          {
            // Updates mod data & object
            pstModData->pstObject = pstObject;
            pstModData->s64Size   = s64Size;
            pstModData->pBuffer   = pBuffer;
            pstModData->bFirst    = orxTRUE;
            orxConfig_SetParent(orxObject_GetName(pstObject), "Mod");

            // Adds event handlers
            orxEvent_AddHandlerWithContext(orxEVENT_TYPE_SOUND, orxMod_Decode, pstModData);
            orxEvent_SetHandlerIDFlags(orxMod_Decode, orxEVENT_TYPE_SOUND, pstModData, orxEVENT_GET_FLAG(orxSOUND_EVENT_PACKET), orxEVENT_KU32_MASK_ID_ALL);
            orxEvent_AddHandlerWithContext(orxEVENT_TYPE_OBJECT, orxMod_EventHandler, pstModData);
            orxEvent_SetHandlerIDFlags(orxMod_EventHandler, orxEVENT_TYPE_OBJECT, pstModData, orxEVENT_GET_FLAG(orxOBJECT_EVENT_DELETE), orxEVENT_KU32_MASK_ID_ALL);

            // Creates music stream
            orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "empty 2 %u", orxMOD_KU32_SAMPLE_RATE);
            orxConfig_SetString("SoundList", orxObject_GetName(pstObject));
            orxConfig_SetString("Music", acBuffer);
          }
          else
          {
            // Frees mod data
            orxBank_Free(spstModBank, pstModData);

            // Frees buffer
            orxMemory_Free(pBuffer);
          }
        }
        else
        {
          // Frees buffer
          orxMemory_Free(pBuffer);
        }
      }
    }
  }
  else
  {
    ModData *pstModData;

    // Gets movie data
    pstModData = (ModData *)_pstEvent->pContext;

    // Valid deleted object?
    if(pstModData->pstObject == orxOBJECT(_pstEvent->hSender))
    {
      // Stops music
      orxObject_Stop(pstModData->pstObject);

      // Removes event handlers
      orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE_SOUND, orxMod_Decode, _pstEvent->pContext);
      orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE_OBJECT, orxMod_EventHandler, _pstEvent->pContext);

      // Deletes it
      orxMod_Delete(pstModData);
    }
  }

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL orxMod_Init()
{
  // Not initialized?
  if(!spstModBank)
  {
    // Creates bank
    spstModBank = orxBank_Create(16, sizeof(ModData), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_TEMP);

    // Success?
    if(spstModBank)
    {
      // Registers movie handler
      orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxMod_EventHandler);
      orxEvent_SetHandlerIDFlags(orxMod_EventHandler, orxEVENT_TYPE_OBJECT, orxNULL, orxEVENT_GET_FLAG(orxOBJECT_EVENT_PREPARE), orxEVENT_KU32_MASK_ID_ALL);
    }
  }

  // Done!
  return orxSTATUS_SUCCESS;
}

void orxFASTCALL orxMod_Exit()
{
  // Was initialized?
  if(spstModBank)
  {
    // Unregisters movie handler
    orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxMod_EventHandler);

    // Deletes bank
    orxBank_Delete(spstModBank);
    spstModBank = orxNULL;
  }

  // Done!
  return;
}

#endif // orxMOD_IMPL

#endif // _orxMOD_H_
