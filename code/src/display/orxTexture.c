/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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

/**
 * @file orxTexture.c
 * @date 07/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "display/orxTexture.h"

#include "core/orxCommand.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "display/orxDisplay.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"


/** Module flags
 */
#define orxTEXTURE_KU32_STATIC_FLAG_NONE        0x00000000

#define orxTEXTURE_KU32_STATIC_FLAG_READY       0x00000001
#define orxTEXTURE_KU32_STATIC_FLAG_INTERNAL    0x00000002  /**< Internal flag */

#define orxTEXTURE_KU32_STATIC_MASK_ALL         0xFFFFFFFF

/** Defines
 */
#define orxTEXTURE_KU32_FLAG_NONE               0x00000000

#define orxTEXTURE_KU32_FLAG_BITMAP             0x00000010
#define orxTEXTURE_KU32_FLAG_INTERNAL           0x00000020
#define orxTEXTURE_KU32_FLAG_REF_COORD          0x00000100
#define orxTEXTURE_KU32_FLAG_SIZE               0x00000200

#define orxTEXTURE_KU32_MASK_ALL                0xFFFFFFFF

#define orxTEXTURE_KU32_TABLE_SIZE              128

#define orxTEXTURE_KU32_BANK_SIZE               128

#define orxTEXTURE_KU32_HOTLOAD_DELAY           orx2F(0.01f)
#define orxTEXTURE_KU32_HOTLOAD_TRY_NUMBER      10

#define orxTEXTURE_KZ_DEFAULT_EXTENSION         "png"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Texture structure
 */
struct __orxTEXTURE_t
{
  orxSTRUCTURE    stStructure;                  /**< Public structure, first structure member : 32 */
  orxU32          u32ID;                        /**< Associated name ID : 36 */
  orxFLOAT        fWidth;                       /**< Width : 40 */
  orxFLOAT        fHeight;                      /**< Height : 44 */
  orxHANDLE       hData;                        /**< Data : 48 */
};

/** Static structure
 */
typedef struct __orxTEXTURE_STATIC_t
{
  orxHASHTABLE   *pstTable;                     /**< Bitmap hashtable */
  orxTEXTURE     *pstScreen;                    /**< Screen texture */
  orxTEXTURE     *pstPixel;                     /**< Pixel texture */
  orxU32          u32ResourceGroupID;           /**< Resource group ID */
  orxU32          u32LoadCounter;               /**< Load counter */
  orxU32          u32Flags;                     /**< Control flags */

} orxTEXTURE_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

static orxTEXTURE_STATIC sstTexture;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 */
static orxSTATUS orxFASTCALL orxTexture_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Resource? */
  if(_pstEvent->eType == orxEVENT_TYPE_RESOURCE)
  {
    /* Add or update? */
    if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
    {
      orxRESOURCE_EVENT_PAYLOAD *pstPayload;

      /* Gets payload */
      pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Is texture group? */
      if(pstPayload->u32GroupID == sstTexture.u32ResourceGroupID)
      {
        orxTEXTURE *pstTexture;

        /* Gets texture */
        pstTexture = (orxTEXTURE *)orxHashTable_Get(sstTexture.pstTable, pstPayload->u32NameID);

        /* Found? */
        if(pstTexture != orxNULL)
        {
          orxBITMAP      *pstBackupBitmap, *pstBitmap;
          const orxSTRING zName;
          orxBOOL         bInternal;
          orxU32          i;

          /* Profiles */
          orxPROFILER_PUSH_MARKER("orxTexture_CreateFromFile");

          /* Stores status */
          bInternal = orxStructure_TestFlags(pstTexture, orxTEXTURE_KU32_FLAG_INTERNAL);

          /* Gets current texture name */
          zName = orxTexture_GetName(pstTexture);

          /* Resets internal status */
          orxStructure_SetFlags(pstTexture, orxTEXTURE_KU32_FLAG_NONE, orxTEXTURE_KU32_FLAG_INTERNAL);

          /* Backups current bitmap */
          pstBackupBitmap = orxTexture_GetBitmap(pstTexture);

          /* Unlinks it */
          orxTexture_UnlinkBitmap(pstTexture);

          /* Re-loads bitmap */
          pstBitmap = orxDisplay_LoadBitmap(zName);

          /* Failure? */
          for(i = 0; (pstBitmap == orxNULL) && (i < orxTEXTURE_KU32_HOTLOAD_TRY_NUMBER); i++)
          {
            /* Waits a bit */
            orxSystem_Delay(orxTEXTURE_KU32_HOTLOAD_DELAY);

            /* Tries again */
            pstBitmap = orxDisplay_LoadBitmap(zName);
          }

          /* Success? */
          if(pstBitmap != orxNULL)
          {
            /* Was internal? */
            if(bInternal != orxFALSE)
            {
              /* Deletes backup */
              orxDisplay_DeleteBitmap(pstBackupBitmap);
            }
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't hotload texture <%s> after %u tries, reverting to former version.", zName, orxTEXTURE_KU32_HOTLOAD_TRY_NUMBER);

            /* Restores backup */
            pstBitmap = pstBackupBitmap;
          }

          /* Assigns given bitmap to it */
          if(orxTexture_LinkBitmap(pstTexture, pstBitmap, zName) != orxSTATUS_FAILURE)
          {
            /* Was internal? */
            if(bInternal != orxFALSE)
            {
              /* Updates flags */
              orxStructure_SetFlags(pstTexture, orxTEXTURE_KU32_FLAG_INTERNAL, orxTEXTURE_KU32_FLAG_NONE);
            }

            /* Asynchronous loading? */
            if(orxDisplay_GetTempBitmap() != orxNULL)
            {
              /* Updates load counter */
              sstTexture.u32LoadCounter++;
            }
            else
            {
              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_TEXTURE, orxTEXTURE_EVENT_LOAD, pstTexture, orxNULL, orxNULL);
            }
          }

          /* Profiles */
          orxPROFILER_POP_MARKER();
        }
      }
    }
  }
  /* Display? */
  else if(_pstEvent->eType == orxEVENT_TYPE_DISPLAY)
  {
    /* Load? */
    if(_pstEvent->eID == orxDISPLAY_EVENT_LOAD_BITMAP)
    {
      orxDISPLAY_EVENT_PAYLOAD *pstPayload;
      orxTEXTURE               *pstTexture;

      /* Gets payload */
      pstPayload = (orxDISPLAY_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Gets texture */
      pstTexture = (orxTEXTURE *)orxHashTable_Get(sstTexture.pstTable, pstPayload->stBitmap.u32FilenameID);

      /* Found? */
      if(pstTexture != orxNULL)
      {
        /* Updates load counter */
        sstTexture.u32LoadCounter--;

        /* Success? */
        if(pstPayload->stBitmap.u32ID != orxU32_UNDEFINED)
        {
          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_TEXTURE, orxTEXTURE_EVENT_LOAD, pstTexture, orxNULL, orxNULL);
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all textures
 */
static orxINLINE void orxTexture_DeleteAll()
{
  orxTEXTURE *pstTexture;

  /* Gets first texture */
  pstTexture = orxTEXTURE(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXTURE));

  /* Non empty? */
  while(pstTexture != orxNULL)
  {
    /* Deletes texture */
    orxTexture_Delete(pstTexture);

    /* Gets first texture */
    pstTexture = orxTEXTURE(orxStructure_GetFirst(orxSTRUCTURE_ID_TEXTURE));
  }

  return;
}

/** Finds a texture by name
 * @param[in]   _wDataName    Name of the texture to find
 * @return      orxTEXTURE / orxNULL
 */
static orxINLINE orxTEXTURE *orxTexture_FindByName(const orxSTRING _zDataName)
{
  register orxTEXTURE *pstTexture;

  /* Gets texture from hash table */
  pstTexture = (orxTEXTURE *)orxHashTable_Get(sstTexture.pstTable, orxString_ToCRC(_zDataName));

  /* Done! */
  return pstTexture;
}

/** Command: Create
 */
void orxFASTCALL orxTexture_CommandCreate(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxTEXTURE *pstTexture;

  /* Has size? */
  if(_u32ArgNumber > 1)
  {
    /* Clears result */
    _pstResult->u64Value = orxU64_UNDEFINED;

    /* Creates texture */
    pstTexture = orxTexture_Create();

    /* Success? */
    if(pstTexture != orxNULL)
    {
      orxBITMAP *pstBitmap;

      /* Creates bitmap */
      pstBitmap = orxDisplay_CreateBitmap(orxF2U(_astArgList[1].vValue.fX), orxF2U(_astArgList[2].vValue.fY));

      /* Success? */
      if(pstBitmap != orxNULL)
      {
        /* Links them */
        if(orxTexture_LinkBitmap(pstTexture, pstBitmap, _astArgList[0].zValue) != orxSTATUS_FAILURE)
        {
          /* Updates its flag */
          orxStructure_SetFlags(pstTexture, orxTEXTURE_KU32_FLAG_INTERNAL, orxTEXTURE_KU32_FLAG_NONE);

          /* Updates result */
          _pstResult->u64Value = orxStructure_GetGUID(pstTexture);
        }
        else
        {
          /* Deletes bitmap */
          orxDisplay_DeleteBitmap(pstBitmap);

          /* Deletes texture */
          orxTexture_Delete(pstTexture);
        }
      }
      else
      {
        /* Deletes texture */
        orxTexture_Delete(pstTexture);
      }
    }
  }
  else
  {
    /* Creates it */
    pstTexture = orxTexture_CreateFromFile(_astArgList[0].zValue);

    /* Updates result */
    _pstResult->u64Value = (pstTexture != orxNULL) ? orxStructure_GetGUID(pstTexture) : orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: Delete
 */
void orxFASTCALL orxTexture_CommandDelete(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxTEXTURE *pstTexture;

  /* Gets texture */
  pstTexture = orxTEXTURE(orxStructure_Get(_astArgList[0].u64Value));

  /* Success? */
  if(pstTexture != orxNULL)
  {
    /* Deletes it */
    orxTexture_Delete(pstTexture);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: Find
 */
void orxFASTCALL orxTexture_CommandFind(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxTEXTURE *pstTexture;

  /* Gets texture */
  pstTexture = orxTexture_FindByName(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->u64Value = (pstTexture != orxNULL) ? orxStructure_GetGUID(pstTexture) : orxU64_UNDEFINED;

  /* Done! */
  return;
}

/** Command: GetName
 */
void orxFASTCALL orxTexture_CommandGetName(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxTEXTURE *pstTexture;

  /* Gets texture */
  pstTexture = orxTEXTURE(orxStructure_Get(_astArgList[0].u64Value));

  /* Updates result */
  _pstResult->zValue = (pstTexture != orxNULL) ? orxTexture_GetName(pstTexture) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: Save
 */
void orxFASTCALL orxTexture_CommandSave(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxTEXTURE *pstTexture;

  /* Gets texture */
  pstTexture = orxTexture_FindByName(_astArgList[0].zValue);

  /* Not found? */
  if(pstTexture == orxNULL)
  {
    orxU64 u64ID;

    /* Is argument an ID? */
    if(orxString_ToU64(_astArgList[0].zValue, &u64ID, orxNULL) != orxSTATUS_FAILURE)
    {
      /* Gets texture */
      pstTexture = orxTEXTURE(orxStructure_Get(u64ID));
    }
  }

  /* Success? */
  if(pstTexture != orxNULL)
  {
    const orxSTRING zName;
    orxCHAR         acBuffer[256];

    /* Was target name specified? */
    if(_u32ArgNumber > 1)
    {
      /* Uses it */
      zName = _astArgList[1].zValue;
    }
    else
    {
      /* Gets texture name */
      zName = orxTexture_GetName(pstTexture);

      /* Doesn't include extension? */
      if(orxString_GetExtension(zName) == orxSTRING_EMPTY)
      {
        /* Appends default extension */
        orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s.%s", zName, orxTEXTURE_KZ_DEFAULT_EXTENSION);
        acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

        /* Uses it */
        zName = acBuffer;
      }
    }

    /* Saves it */
    _pstResult->bValue = (orxDisplay_SaveBitmap(orxTexture_GetBitmap(pstTexture), zName) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Command: GetLoadCounter
 */
void orxFASTCALL orxTexture_CommandGetLoadCounter(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->u32Value = sstTexture.u32LoadCounter;

  /* Done! */
  return;
}

/** Registers all the texture commands
 */
static orxINLINE void orxTexture_RegisterCommands()
{
  /* Command: Create */
  orxCOMMAND_REGISTER_CORE_COMMAND(Texture, Create, "Texture", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Name", orxCOMMAND_VAR_TYPE_STRING}, {"Size = <void>", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: Delete */
  orxCOMMAND_REGISTER_CORE_COMMAND(Texture, Delete, "Texture", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Texture", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Find */
  orxCOMMAND_REGISTER_CORE_COMMAND(Texture, Find, "Texture", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Name", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetName */
  orxCOMMAND_REGISTER_CORE_COMMAND(Texture, GetName, "Name", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Texture", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Save */
  orxCOMMAND_REGISTER_CORE_COMMAND(Texture, Save, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 1, {"Texture|Name", orxCOMMAND_VAR_TYPE_STRING}, {"File = Name.png", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetLoadCounter */
  orxCOMMAND_REGISTER_CORE_COMMAND(Texture, GetLoadCounter, "Load Counter", orxCOMMAND_VAR_TYPE_U32, 0, 0);
}

/** Unregisters all the texture commands
 */
static orxINLINE void orxTexture_UnregisterCommands()
{
  /* Command: Create */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Texture, Create);
  /* Command: Delete */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Texture, Delete);

  /* Command: Find */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Texture, Find);

  /* Command: GetName */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Texture, GetName);

  /* Command: Save */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Texture, Save);

  /* Command: GetLoadCounter */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Texture, GetLoadCounter);
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups the texture module
 */
void orxFASTCALL orxTexture_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_DISPLAY);

  return;
}

/** Inits the texture module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTexture_Init()
{
  orxSTATUS eResult;

  /* Not already Initialized? */
  if(!(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstTexture, sizeof(orxTEXTURE_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(TEXTURE, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxTEXTURE_KU32_BANK_SIZE, orxNULL);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Creates hash table */
      sstTexture.pstTable = orxHashTable_Create(orxTEXTURE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if(sstTexture.pstTable != orxNULL)
      {
        /* Updates flags for screen texture creation */
        sstTexture.u32Flags = orxTEXTURE_KU32_STATIC_FLAG_READY;

        /* Registers commands */
        orxTexture_RegisterCommands();

        /* Creates screen & pixel textures */
        sstTexture.pstScreen  = orxTexture_Create();
        sstTexture.pstPixel   = orxTexture_Create();

        /* Valid? */
        if((sstTexture.pstScreen != orxNULL) && (sstTexture.pstPixel != orxNULL))
        {
          /* Sets them as their own owners */
          orxStructure_SetOwner(sstTexture.pstScreen, sstTexture.pstScreen);
          orxStructure_SetOwner(sstTexture.pstPixel, sstTexture.pstPixel);

          /* Links screen bitmap */
          eResult = orxTexture_LinkBitmap(sstTexture.pstScreen, orxDisplay_GetScreenBitmap(), orxTEXTURE_KZ_SCREEN_NAME);

          /* Success? */
          if(eResult != orxSTATUS_FAILURE)
          {
            orxBITMAP *pstBitmap;

            /* Creates empty bitmap */
            pstBitmap = orxDisplay_CreateBitmap(1, 1);

            /* Success? */
            if(pstBitmap != orxNULL)
            {
              orxU8 au8PixelBuffer[] = {0xFF, 0xFF, 0xFF, 0xFF};

              /* Sets it data */
              if(orxDisplay_SetBitmapData(pstBitmap, au8PixelBuffer, 4 * sizeof(orxU8)) != orxSTATUS_FAILURE)
              {
                /* Links it */
                if(orxTexture_LinkBitmap(sstTexture.pstPixel, pstBitmap, orxTEXTURE_KZ_PIXEL) != orxSTATUS_FAILURE)
                {
                  /* Updates its flag */
                  orxStructure_SetFlags(sstTexture.pstPixel, orxTEXTURE_KU32_FLAG_INTERNAL, orxTEXTURE_KU32_FLAG_NONE);

                  /* Sets it as temp bitmap for asynchronous operations */
                  orxDisplay_SetTempBitmap(pstBitmap);

                  /* Inits values */
                  sstTexture.u32ResourceGroupID = orxString_GetID(orxTEXTURE_KZ_RESOURCE_GROUP);

                  /* Adds event handler */
                  orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxTexture_EventHandler);
                  orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxTexture_EventHandler);
                }
                else
                {
                  /* Deletes bitmap */
                  orxDisplay_DeleteBitmap(pstBitmap);

                  /* Updates result */
                  eResult = orxSTATUS_FAILURE;
                }
              }
              else
              {
                /* Deletes bitmap */
                orxDisplay_DeleteBitmap(pstBitmap);

                /* Updates result */
                eResult = orxSTATUS_FAILURE;
              }
            }
            else
            {
              /* Updates result */
              eResult = orxSTATUS_FAILURE;
            }
          }
        }
        else
        {
          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to initialize texture module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Not initialized? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Initializing texture module failed.");

    /* Deletes screen & pixel textures */
    if(sstTexture.pstScreen != orxNULL)
    {
      orxTexture_Delete(sstTexture.pstScreen);
      sstTexture.pstScreen = orxNULL;
    }
    if(sstTexture.pstPixel != orxNULL)
    {
      orxTexture_Delete(sstTexture.pstPixel);
      sstTexture.pstPixel = orxNULL;
    }

    /* Deletes hash table */
    if(sstTexture.pstTable != orxNULL)
    {
      orxHashTable_Delete(sstTexture.pstTable);
      sstTexture.pstTable = orxNULL;
    }

    /* Updates Flags */
    sstTexture.u32Flags &= ~orxTEXTURE_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the texture module
 */
void orxFASTCALL orxTexture_Exit()
{
  /* Initialized? */
  if(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxTexture_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxTexture_EventHandler);

    /* Unregisters commands */
    orxTexture_UnregisterCommands();

    /* Deletes screen & pixel textures */
    orxTexture_Delete(sstTexture.pstScreen);
    orxTexture_Delete(sstTexture.pstPixel);

    /* Deletes texture list */
    orxTexture_DeleteAll();

    /* Deletes hash table */
    orxHashTable_Delete(sstTexture.pstTable);
    sstTexture.pstTable = orxNULL;

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_TEXTURE);

    /* Updates flags */
    sstTexture.u32Flags &= ~orxTEXTURE_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to exit texture module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty texture
 * @return      orxTEXTURE / orxNULL
 */
orxTEXTURE *orxFASTCALL orxTexture_Create()
{
  orxTEXTURE *pstTexture;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);

  /* Creates texture */
  pstTexture = orxTEXTURE(orxStructure_Create(orxSTRUCTURE_ID_TEXTURE));

  /* Created? */
  if(pstTexture != orxNULL)
  {
    /* Increases counter */
    orxStructure_IncreaseCounter(pstTexture);

    /* Not creating it internally? */
    if(!orxFLAG_TEST(sstTexture.u32Flags, orxTEXTURE_KU32_STATIC_FLAG_INTERNAL))
    {
      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_TEXTURE, orxTEXTURE_EVENT_CREATE, pstTexture, orxNULL, orxNULL);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create structure for texture.");
  }

  /* Done! */
  return pstTexture;
}

/** Creates a texture from a bitmap file
 * @param[in]   _zBitmapFileName  Name of the bitmap
 * @return      orxTEXTURE / orxNULL
 */
orxTEXTURE *orxFASTCALL orxTexture_CreateFromFile(const orxSTRING _zBitmapFileName)
{
  orxTEXTURE *pstTexture;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxASSERT(_zBitmapFileName != orxNULL);

  /* Search for a texture using this bitmap */
  pstTexture = orxTexture_FindByName(_zBitmapFileName);

  /* Found? */
  if(pstTexture != orxNULL)
  {
    /* Increases counter */
    orxStructure_IncreaseCounter(pstTexture);
  }
  else
  {
    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxTexture_CreateFromFile");

    /* Sets internal flag */
    orxFLAG_SET(sstTexture.u32Flags, orxTEXTURE_KU32_STATIC_FLAG_INTERNAL, orxTEXTURE_KU32_STATIC_FLAG_NONE);

    /* Creates an empty texture */
    pstTexture = orxTexture_Create();

    /* Removes internal flag */
    orxFLAG_SET(sstTexture.u32Flags, orxTEXTURE_KU32_STATIC_FLAG_NONE, orxTEXTURE_KU32_STATIC_FLAG_INTERNAL);

    /* Valid? */
    if(pstTexture != orxNULL)
    {
      orxBITMAP *pstBitmap;

      /* Loads bitmap */
      pstBitmap = orxDisplay_LoadBitmap(_zBitmapFileName);

      /* Assigns given bitmap to it */
      if((pstBitmap != orxNULL)
      && (orxTexture_LinkBitmap(pstTexture, pstBitmap, _zBitmapFileName) != orxSTATUS_FAILURE))
      {
        /* Inits it */
        orxStructure_SetFlags(pstTexture, orxTEXTURE_KU32_FLAG_INTERNAL, orxTEXTURE_KU32_FLAG_NONE);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_TEXTURE, orxTEXTURE_EVENT_CREATE, pstTexture, orxNULL, orxNULL);

        /* Asynchronous loading? */
        if(orxDisplay_GetTempBitmap() != orxNULL)
        {
          /* Updates load counter */
          sstTexture.u32LoadCounter++;
        }
        else
        {
          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_TEXTURE, orxTEXTURE_EVENT_LOAD, pstTexture, orxNULL, orxNULL);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to load bitmap [%s] and link it to texture.", _zBitmapFileName);

        /* Frees allocated texture */
        orxTexture_Delete(pstTexture);

        /* Not created */
        pstTexture = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Invalid texture created.");
    }

    /* Profiles */
    orxPROFILER_POP_MARKER();
  }

  /* Done! */
  return pstTexture;
}

/** Deletes a texture (and its referenced bitmap)
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTexture_Delete(orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Decreases reference counter */
  orxStructure_DecreaseCounter(_pstTexture);

  /* Is the last reference? */
  if(orxStructure_GetRefCounter(_pstTexture) == 0)
  {
    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_TEXTURE, orxTEXTURE_EVENT_DELETE, _pstTexture, orxNULL, orxNULL);

    /* Cleans bitmap reference */
    orxTexture_UnlinkBitmap(_pstTexture);

    /* Deletes structure */
    orxStructure_Delete(_pstTexture);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Links a bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @param[in]   _pstBitmap      Bitmap to link
 * @param[in]   _zDataName      Name associated with the bitmap (usually filename)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTexture_LinkBitmap(orxTEXTURE *_pstTexture, const orxBITMAP *_pstBitmap, const orxSTRING _zDataName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);
  orxASSERT(_pstBitmap != orxNULL);

  /* Unlink previous bitmap if needed */
  orxTexture_UnlinkBitmap(_pstTexture);

  /* Has no texture now? */
  if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP) == orxFALSE)
  {
    orxTEXTURE *pstTexture;
    orxU32      u32ID;

    /* Gets ID */
    u32ID = orxString_GetID(_zDataName);

    /* Gets texture from hash table */
    pstTexture = (orxTEXTURE *)orxHashTable_Get(sstTexture.pstTable, u32ID);

    /* Not found? */
    if(pstTexture == orxNULL)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP | orxTEXTURE_KU32_FLAG_SIZE, orxTEXTURE_KU32_FLAG_NONE);

      /* References bitmap */
      _pstTexture->hData = (orxHANDLE)_pstBitmap;

      /* Gets bitmap size */
      orxDisplay_GetBitmapSize(_pstBitmap, &(_pstTexture->fWidth), &(_pstTexture->fHeight));

      /* Updates texture ID */
      _pstTexture->u32ID = u32ID;

      /* Adds it to hash table */
      orxHashTable_Add(sstTexture.pstTable, u32ID, _pstTexture);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Bitmap <%s> is already linked to another texture, aborting.", _zDataName);

      /* Already linked */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Texture is already linked.");

    /* Already linked */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Unlinks (and deletes if not used anymore) a bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTexture_UnlinkBitmap(orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Has bitmap */
  if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP) != orxFALSE)
  {
    /* Internally handled? */
    if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_INTERNAL))
    {
      /* Deletes bitmap */
      orxDisplay_DeleteBitmap((orxBITMAP *)(_pstTexture->hData));
    }

    /* Updates flags */
    orxStructure_SetFlags(_pstTexture, orxTEXTURE_KU32_FLAG_NONE, (orxTEXTURE_KU32_FLAG_BITMAP | orxTEXTURE_KU32_FLAG_SIZE | orxTEXTURE_KU32_FLAG_INTERNAL));

    /* Cleans data */
    _pstTexture->hData = orxHANDLE_UNDEFINED;

    /* Removes from hash table */
    orxHashTable_Remove(sstTexture.pstTable, _pstTexture->u32ID);

    /* Clears ID */
    _pstTexture->u32ID = 0;
  }
  else
  {
    /* No bitmap to unlink from */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets texture bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxBITMAP / orxNULL
 */
orxBITMAP *orxFASTCALL orxTexture_GetBitmap(const orxTEXTURE *_pstTexture)
{
  register orxBITMAP *pstBitmap = orxNULL;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Has bitmap? */
  if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP) != orxFALSE)
  {
    /* Updates result */
    pstBitmap = (orxBITMAP *)_pstTexture->hData;
  }

  /* Done! */
  return pstBitmap;
}

/** Gets texture size
 * @param[in]   _pstTexture     Concerned texture
 * @param[out]  _pfWidth        Texture's width
 * @param[out]  _pfHeight       Texture's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTexture_GetSize(const orxTEXTURE *_pstTexture, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Is screen? */
  if(_pstTexture == sstTexture.pstScreen)
  {
    /* Updates its size (screen might have changed) */
    orxDisplay_GetBitmapSize(orxDisplay_GetScreenBitmap(), &(((orxTEXTURE *)_pstTexture)->fWidth), &(((orxTEXTURE *)_pstTexture)->fHeight));
  }

  /* Has size? */
  if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_SIZE) != orxFALSE)
  {
    /* Stores values */
    *_pfWidth   = _pstTexture->fWidth;
    *_pfHeight  = _pstTexture->fHeight;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Texture's width is not set.");

    /* No size */
    *_pfWidth = *_pfHeight = orx2F(-1.0f);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets texture name
 * @param[in]   _pstTexture   Concerned texture
 * @return      Texture name / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxTexture_GetName(const orxTEXTURE *_pstTexture)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Updates result */
  zResult = (_pstTexture->u32ID != 0) ? orxString_GetFromID(_pstTexture->u32ID) : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Sets texture color
 * @param[in]   _pstTexture     Concerned texture
 * @param[in]   _pstColor       Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTexture_SetColor(orxTEXTURE *_pstTexture, const orxCOLOR *_pstColor)
{
  orxBITMAP  *pstBitmap;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);
  orxASSERT(_pstColor != orxNULL);

  /* Gets bitmap */
  pstBitmap = orxTexture_GetBitmap(_pstTexture);

  /* Valid? */
  if(pstBitmap != orxNULL)
  {
    /* Updates its color */
    eResult = orxDisplay_SetBitmapColor(pstBitmap, orxColor_ToRGBA(_pstColor));
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Invalid bitmap in texture.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets screen texture
 * @return      Screen texture / orxNULL
 */
orxTEXTURE *orxFASTCALL orxTexture_GetScreenTexture()
{
  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstTexture.pstScreen;
}

/** Gets pending load counter
 * @return      Pending load counter
 */
orxU32 orxFASTCALL orxTexture_GetLoadCounter()
{
  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstTexture.u32LoadCounter;
}
