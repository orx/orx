//! Includes

#ifndef _orxSID_H_
#define _orxSID_H_

#include "orx.h"


//! Prototypes

orxSTATUS orxFASTCALL                     orxSid_Init();
void orxFASTCALL                          orxSid_Exit();


//! Implementation

#ifdef orxSID_IMPL

#ifdef __orxMSVC__
  #pragma warning(push)
  #pragma warning(disable : 4018 4101 4244 4267 4996)
#endif /* __orxMSVC__ */

#define malloc(size)                      orxMemory_Allocate(size, orxMEMORY_TYPE_AUDIO)
#define free(mem)                         orxMemory_Free(mem)

#include "cRSID/libcRSID.c"

#undef free
#undef malloc

#ifdef __orxMSVC__
  #pragma warning(pop)
#endif /* __orxMSVC__ */


#define ORXSID_KU32_SAMPLE_RATE           48000
#define ORXSID_KU32_SAMPLE_BUFFER_LENGTH  8192


//! Variables / Structures

typedef struct SidData
{
  orxOBJECT            *pstObject;
  void                 *pBuffer;
  cRSID_SIDheader      *pstSidHeader;
  cRSID_Interface       stSid;
  cRSID_C64instance     stC64;
  orxBOOL               bFirst;
  char                  cSubTune;

} SidData;

typedef struct __ORXSID_t
{
  orxBANK              *pstSidBank;
  orxTHREAD_SEMAPHORE  *pstSemaphore;

} orxSID;

static orxSID sstSid;


//! Code

static orxINLINE void orxSid_CopyContext(cRSID_Interface *_pstSid, cRSID_C64instance *_pstC64, orxBOOL _bIn)
{
  // Copy in?
  if(_bIn)
  {
    static cRSID_Interface   *spstLastSid = NULL;
    static cRSID_C64instance *spstLastC64 = NULL;

    // Not already there?
    if((_pstSid != spstLastSid)
    || (_pstC64 != spstLastC64))
    {
      // Copies context
      orxMemory_Copy(&cRSID, _pstSid, sizeof(cRSID_Interface));
      orxMemory_Copy(&cRSID_C64, _pstC64, sizeof(cRSID_C64instance));
      spstLastSid = _pstSid;
      spstLastC64 = _pstC64;
    }
  }
  // Copy out
  else
  {
    // Copies context
    orxMemory_Copy(_pstSid, &cRSID, sizeof(cRSID_Interface));
    orxMemory_Copy(_pstC64, &cRSID_C64, sizeof(cRSID_C64instance));
  }

  // Done!
  return;
}

static orxSTATUS orxFASTCALL orxSid_Delete(SidData *_pstSidData)
{
  if(sstSid.pstSidBank)
  {
    // Waits for semaphore
    orxThread_WaitSemaphore(sstSid.pstSemaphore);

    // Deletes it
    _pstSidData->pstObject = orxNULL;
    orxMemory_Free(_pstSidData->pBuffer);
    _pstSidData->pBuffer = orxNULL;
    orxBank_Free(sstSid.pstSidBank, _pstSidData);

    // Signals semaphore
    orxThread_SignalSemaphore(sstSid.pstSemaphore);
  }

  // Done!
  return orxSTATUS_SUCCESS;
}

static orxSTATUS orxFASTCALL orxSid_Decode(const orxEVENT *_pstEvent)
{
  SidData *pstSidData;

  // Gets sid data
  pstSidData = (SidData *)_pstEvent->pContext;

  // Matching object?
  if(pstSidData->pstObject && (pstSidData->pstObject == orxOBJECT(_pstEvent->hSender)))
  {
    orxSOUND_EVENT_PAYLOAD *pstPayload;

    // Gets payload
    pstPayload = (orxSOUND_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    // Valid?
    if(orxObject_IsEnabled(pstSidData->pstObject) && !orxObject_IsPaused(pstSidData->pstObject))
    {
      orxFLOAT  fVolumeCoef;
      orxS32    i;

      // Waits for semaphore
      orxThread_WaitSemaphore(sstSid.pstSemaphore);

      // Restores context
      orxSid_CopyContext(&(pstSidData->stSid), &(pstSidData->stC64), orxTRUE);

      // Not first packet?
      if(!pstSidData->bFirst)
      {
        // Has restarted?
        if(pstPayload->stStream.stPacket.fTime == orxFLOAT_0)
        {
          // Resets replay
          cRSID_initSIDtune(pstSidData->pstSidHeader, pstSidData->cSubTune);

          // Updates status
          pstSidData->bFirst = orxTRUE;
        }
      }
      else
      {
        // Updates status
        pstSidData->bFirst = orxFALSE;
      }

      // Fills it
      fVolumeCoef = orxS2F(cRSID.MainVolume) * orxS2F(cRSID_C64.FadeLevel) / orx2F(32768.0f * 4096.0f);
      for(i = 0; i < (orxS32)pstPayload->stStream.stPacket.u32SampleNumber; i += 2)
      {
        cRSID_Output  stOutput = {0, 0};
        orxCHAR       j;
        for(j = 0; j < cRSID.PlaybackSpeed; j++)
        {
          stOutput = cRSID_generateSample();
        }
        pstPayload->stStream.stPacket.afSampleList[i]     = orxS2F(stOutput.L) * fVolumeCoef;
        pstPayload->stStream.stPacket.afSampleList[i + 1] = orxS2F(stOutput.R) * fVolumeCoef;
      }

      // Backups context
      orxSid_CopyContext(&(pstSidData->stSid), &(pstSidData->stC64), orxFALSE);

      // Signals semaphore
      orxThread_SignalSemaphore(sstSid.pstSemaphore);
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

static orxSTATUS orxFASTCALL orxSid_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Checks
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_OBJECT);

  // New object?
  if(_pstEvent->eID == orxOBJECT_EVENT_PREPARE)
  {
    // Is a sid?
    if(orxConfig_HasValue("Sid"))
    {
      const orxSTRING zSid;
      orxHANDLE       hResource;

      // Has valid resource?
      if((zSid = orxResource_Locate(orxSOUND_KZ_RESOURCE_GROUP, orxConfig_GetListString("Sid", 0)))
      && ((hResource = orxResource_Open(zSid, orxFALSE)) != orxHANDLE_UNDEFINED))
      {
        orxS64 s64Size;
        void   *pBuffer;

        // Gets its size
        s64Size = orxResource_GetSize(hResource);
        orxASSERT(s64Size);

        // Loads its content
        pBuffer = orxMemory_Allocate((orxU32)s64Size, orxMEMORY_TYPE_AUDIO);
        orxASSERT(pBuffer);
        if(orxResource_Read(hResource, s64Size, pBuffer, orxNULL, orxNULL) == s64Size)
        {
          SidData    *pstSidData;
          orxOBJECT  *pstObject;

          // Gets object
          pstObject = orxOBJECT(_pstEvent->hSender);

          // Waits for semaphore
          orxThread_WaitSemaphore(sstSid.pstSemaphore);

          // Creates decoder data
          pstSidData = (SidData *)orxBank_Allocate(sstSid.pstSidBank);
          orxASSERT(pstSidData);
          orxMemory_Zero(pstSidData, sizeof(SidData));

          // Restores context
          orxSid_CopyContext(&(pstSidData->stSid), &(pstSidData->stC64), orxTRUE);

          // Inits SID
          cRSID_init(ORXSID_KU32_SAMPLE_RATE, ORXSID_KU32_SAMPLE_BUFFER_LENGTH);
          cRSID.CLImode = 1;
          cRSID.AutoAdvance = cRSID.FadeOut = 0;
          pstSidData->pstSidHeader = cRSID_processSIDfileData((unsigned char *)pBuffer, (int)s64Size);

          // Success?
          if(pstSidData->pstSidHeader)
          {
            orxCHAR acBuffer[256];

            // Updates sid data & object
            pstSidData->pstObject = pstObject;
            pstSidData->pBuffer   = pBuffer;
            pstSidData->bFirst    = orxTRUE;

            // Inits subtune
            if(orxConfig_GetListCount("Sid") > 1)
            {
              pstSidData->cSubTune = orxCLAMP((char)orxConfig_GetListS32("Sid", 1), 1, pstSidData->pstSidHeader->SubtuneAmount);
            }
            else
            {
              pstSidData->cSubTune = pstSidData->pstSidHeader->DefaultSubtune;
            }
            cRSID_initSIDtune(pstSidData->pstSidHeader, pstSidData->cSubTune);

            // Adds event handlers
            orxEvent_AddHandlerWithContext(orxEVENT_TYPE_SOUND, orxSid_Decode, pstSidData);
            orxEvent_SetHandlerIDFlags(orxSid_Decode, orxEVENT_TYPE_SOUND, pstSidData, orxEVENT_GET_FLAG(orxSOUND_EVENT_PACKET), orxEVENT_KU32_MASK_ID_ALL);
            orxEvent_AddHandlerWithContext(orxEVENT_TYPE_OBJECT, orxSid_EventHandler, pstSidData);
            orxEvent_SetHandlerIDFlags(orxSid_EventHandler, orxEVENT_TYPE_OBJECT, pstSidData, orxEVENT_GET_FLAG(orxOBJECT_EVENT_DELETE), orxEVENT_KU32_MASK_ID_ALL);

            // Creates music stream
            orxString_NPrint(acBuffer, sizeof(acBuffer), "empty 2 %u", ORXSID_KU32_SAMPLE_RATE);
            orxConfig_SetString("SoundList", orxObject_GetName(pstObject));
            orxConfig_SetString("Music", acBuffer);

            // Backups context
            orxSid_CopyContext(&(pstSidData->stSid), &(pstSidData->stC64), orxFALSE);
          }
          else
          {
            // Frees sid data
            orxBank_Free(sstSid.pstSidBank, pstSidData);
          }

          // Signals semaphore
          orxThread_SignalSemaphore(sstSid.pstSemaphore);
        }

        // Closes resource
        orxResource_Close(hResource);
      }
    }
  }
  else
  {
    SidData *pstSidData;

    // Gets sid data
    pstSidData = (SidData *)_pstEvent->pContext;

    // Valid deleted object?
    if(pstSidData->pstObject == orxOBJECT(_pstEvent->hSender))
    {
      // Stops music
      orxObject_Stop(pstSidData->pstObject);

      // Removes event handlers
      orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE_SOUND, orxSid_Decode, _pstEvent->pContext);
      orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE_OBJECT, orxSid_EventHandler, _pstEvent->pContext);

      // Deletes it
      orxSid_Delete(pstSidData);
    }
  }

  // Done!
  return eResult;
}

orxSTATUS orxFASTCALL orxSid_Init()
{
  // Not initialized?
  if(!sstSid.pstSidBank)
  {
    // Creates semaphore
    sstSid.pstSemaphore = orxThread_CreateSemaphore(1);

    // Success?
    if(sstSid.pstSemaphore)
    {
      // Creates bank
      sstSid.pstSidBank = orxBank_Create(16, sizeof(SidData), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_TEMP);

      // Success?
      if(sstSid.pstSidBank)
      {
        // Registers sid handler
        orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxSid_EventHandler);
        orxEvent_SetHandlerIDFlags(orxSid_EventHandler, orxEVENT_TYPE_OBJECT, orxNULL, orxEVENT_GET_FLAG(orxOBJECT_EVENT_PREPARE), orxEVENT_KU32_MASK_ID_ALL);
      }
      else
      {
        // Deletes semaphore
        orxThread_DeleteSemaphore(sstSid.pstSemaphore);
        sstSid.pstSemaphore = orxNULL;
      }
    }
  }

  // Done!
  return orxSTATUS_SUCCESS;
}

void orxFASTCALL orxSid_Exit()
{
  // Was initialized?
  if(sstSid.pstSidBank)
  {
    SidData *pstSidData;

    // Unregisters sid handlers
    orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE_OBJECT, orxSid_EventHandler, orxNULL);
    orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE_SOUND, orxSid_Decode, orxNULL);

    // Deletes bank
    while((pstSidData = (SidData *)orxBank_GetNext(sstSid.pstSidBank, orxNULL)))
    {
      orxSid_Delete(pstSidData);
    }
    orxBank_Delete(sstSid.pstSidBank);
    sstSid.pstSidBank = orxNULL;

    // Deletes semaphore
    orxThread_DeleteSemaphore(sstSid.pstSemaphore);
    sstSid.pstSemaphore = orxNULL;
  }

  // Done!
  return;
}

#endif // orxSID_IMPL

#endif // _orxSID_H_
