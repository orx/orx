/* Orx - Portable Game Engine
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

/**
 * @file orxObject.c
 * @date 01/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxObject.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "memory/orxMemory.h"
#include "anim/orxAnimPointer.h"
#include "display/orxText.h"
#include "physics/orxBody.h"
#include "object/orxFrame.h"
#include "object/orxFXPointer.h"
#include "object/orxSpawner.h"
#include "object/orxTimeLine.h"
#include "render/orxCamera.h"
#include "render/orxShaderPointer.h"
#include "sound/orxSoundPointer.h"


/** Module flags
 */
#define orxOBJECT_KU32_STATIC_FLAG_NONE         0x00000000  /**< No static flag */

#define orxOBJECT_KU32_STATIC_FLAG_READY        0x00000001  /**< Ready static flag */
#define orxOBJECT_KU32_STATIC_FLAG_CLOCK        0x00000002  /**< Clock static flag */
#define orxOBJECT_KU32_STATIC_FLAG_AGE          0x00000004  /**< Age static flag */

#define orxOBJECT_KU32_STATIC_MASK_ALL          0xFFFFFFFF  /**< Internal static mask */


/** Flags
 */
#define orxOBJECT_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxOBJECT_KU32_FLAG_ENABLED             0x10000000  /**< Enabled flag */
#define orxOBJECT_KU32_FLAG_PAUSED              0x20000000  /**< Paused flag */
#define orxOBJECT_KU32_FLAG_HAS_LIFETIME        0x40000000  /**< Has lifetime flag */
#define orxOBJECT_KU32_FLAG_SMOOTHING_ON        0x80000000  /**< Smoothing on flag */
#define orxOBJECT_KU32_FLAG_SMOOTHING_OFF       0x01000000  /**< Smoothing off flag */
#define orxOBJECT_KU32_FLAG_HAS_CHILDREN        0x02000000  /**< Has children flag */
#define orxOBJECT_KU32_FLAG_HAS_JOINT_CHILDREN  0x04000000  /**< Has children flag */
#define orxOBJECT_KU32_FLAG_IS_JOINT_CHILD      0x08000000  /**< Is joint child flag */
#define orxOBJECT_KU32_FLAG_DETACH_JOINT_CHILD  0x00100000  /**< Detach joint child flag */
#define orxOBJECT_KU32_FLAG_DEATH_ROW           0x00200000  /**< Death row flag */
#define orxOBJECT_KU32_FLAG_FX_LIFETIME         0x00400000  /**< FX lifetime flag */
#define orxOBJECT_KU32_FLAG_SOUND_LIFETIME      0x00800000  /**< Sound lifetime flag */
#define orxOBJECT_KU32_FLAG_SPAWNER_LIFETIME    0x00010000  /**< Spawner lifetime flag */
#define orxOBJECT_KU32_FLAG_TIMELINE_LIFETIME   0x00020000  /**< Timeline lifetime flag */
#define orxOBJECT_KU32_FLAG_CHILDREN_LIFETIME   0x00040000  /**< Children lifetime flag */
#define orxOBJECT_KU32_FLAG_ANIM_LIFETIME       0x00080000  /**< Anim lifetime flag */

#define orxOBJECT_KU32_MASK_STRUCTURE_LIFETIME  0x00CF0000  /**< Structure lifetime mask */
#define orxOBJECT_KU32_MASK_LINKED_STRUCTURE    0x0000FFFF  /**< Linked structure mask */

#define orxOBJECT_KU32_MASK_ALL                 0xFFFFFFFF  /**< All mask */


#define orxOBJECT_KU32_STORAGE_FLAG_NONE        0x00000000

#define orxOBJECT_KU32_STORAGE_FLAG_INTERNAL    0x00000001

#define orxOBJECT_KU32_STORAGE_MASK_ALL         0xFFFFFFFF


/** Misc defines
 */
#define orxOBJECT_KU32_NEIGHBOR_LIST_SIZE       128

#define orxOBJECT_KU32_BANK_SIZE                2048
#define orxOBJECT_KU32_AGE_BANK_SIZE            64

#define orxOBJECT_KU32_GROUP_BANK_SIZE          64
#define orxOBJECT_KU32_GROUP_TABLE_SIZE         64

#define orxOBJECT_KZ_CONFIG_GRAPHIC_NAME        "Graphic"
#define orxOBJECT_KZ_CONFIG_ANIMPOINTER_NAME    "AnimationSet"
#define orxOBJECT_KZ_CONFIG_BODY                "Body"
#define orxOBJECT_KZ_CONFIG_CLOCK               "Clock"
#define orxOBJECT_KZ_CONFIG_SPAWNER             "Spawner"
#define orxOBJECT_KZ_CONFIG_AUTO_SCROLL         "AutoScroll"
#define orxOBJECT_KZ_CONFIG_FLIP                "Flip"
#define orxOBJECT_KZ_CONFIG_COLOR               "Color"
#define orxOBJECT_KZ_CONFIG_RGB                 "RGB"
#define orxOBJECT_KZ_CONFIG_HSL                 "HSL"
#define orxOBJECT_KZ_CONFIG_HSV                 "HSV"
#define orxOBJECT_KZ_CONFIG_ALPHA               "Alpha"
#define orxOBJECT_KZ_CONFIG_DEPTH_SCALE         "DepthScale"
#define orxOBJECT_KZ_CONFIG_POSITION            "Position"
#define orxOBJECT_KZ_CONFIG_SPHERICAL_POSITION  "SphericalPosition"
#define orxOBJECT_KZ_CONFIG_SPEED               "Speed"
#define orxOBJECT_KZ_CONFIG_PIVOT               "Pivot"
#define orxOBJECT_KZ_CONFIG_SIZE                "Size"
#define orxOBJECT_KZ_CONFIG_ROTATION            "Rotation"
#define orxOBJECT_KZ_CONFIG_ANGULAR_VELOCITY    "AngularVelocity"
#define orxOBJECT_KZ_CONFIG_SCALE               "Scale"
#define orxOBJECT_KZ_CONFIG_FX_LIST             "FXList"
#define orxOBJECT_KZ_CONFIG_FX_DELAY_LIST       "FXDelayList"
#define orxOBJECT_KZ_CONFIG_SOUND_LIST          "SoundList"
#define orxOBJECT_KZ_CONFIG_SHADER_LIST         "ShaderList"
#define orxOBJECT_KZ_CONFIG_TRACK_LIST          "TrackList"
#define orxOBJECT_KZ_CONFIG_CHILD_LIST          "ChildList"
#define orxOBJECT_KZ_CONFIG_CHILD_JOINT_LIST    "ChildJointList"
#define orxOBJECT_KZ_CONFIG_FREQUENCY           "AnimationFrequency"
#define orxOBJECT_KZ_CONFIG_SMOOTHING           "Smoothing"
#define orxOBJECT_KZ_CONFIG_BLEND_MODE          "BlendMode"
#define orxOBJECT_KZ_CONFIG_REPEAT              "Repeat"
#define orxOBJECT_KZ_CONFIG_LIFETIME            "LifeTime"
#define orxOBJECT_KZ_CONFIG_PARENT_CAMERA       "ParentCamera"
#define orxOBJECT_KZ_CONFIG_USE_RELATIVE_SPEED  "UseRelativeSpeed"
#define orxOBJECT_KZ_CONFIG_USE_PARENT_SPACE    "UseParentSpace"
#define orxOBJECT_KZ_CONFIG_GROUP               "Group"
#define orxOBJECT_KZ_CONFIG_AGE                 "Age"
#define orxOBJECT_KZ_CONFIG_IGNORE_FROM_PARENT  "IgnoreFromParent"
#define orxOBJECT_KZ_CENTERED_PIVOT             "center"
#define orxOBJECT_KZ_TRUNCATE_PIVOT             "truncate"
#define orxOBJECT_KZ_ROUND_PIVOT                "round"
#define orxOBJECT_KZ_TOP_PIVOT                  "top"
#define orxOBJECT_KZ_LEFT_PIVOT                 "left"
#define orxOBJECT_KZ_BOTTOM_PIVOT               "bottom"
#define orxOBJECT_KZ_RIGHT_PIVOT                "right"
#define orxOBJECT_KZ_FX                         "fx"
#define orxOBJECT_KZ_SOUND                      "sound"
#define orxOBJECT_KZ_SPAWN                      "spawn"
#define orxOBJECT_KZ_TRACK                      "track"
#define orxOBJECT_KZ_CHILD                      "child"
#define orxOBJECT_KZ_ANIM                       "anim"


#define orxOBJECT_KZ_X                          "x"
#define orxOBJECT_KZ_Y                          "y"
#define orxOBJECT_KZ_BOTH                       "both"
#define orxOBJECT_KZ_SCALE                      "scale"
#define orxOBJECT_KZ_POSITION                   "position"


/** Helpers
 */
#define orxOBJECT_MAKE_RECURSIVE(FUNCTION, PARAM_TYPE)                                      \
void orxFASTCALL orxObject_##FUNCTION##Recursive(orxOBJECT *_pstObject, PARAM_TYPE _Param)  \
{                                                                                           \
  orxOBJECT *pstChild;                                                                      \
                                                                                            \
  /* Checks */                                                                              \
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);                         \
  orxSTRUCTURE_ASSERT(_pstObject);                                                          \
                                                                                            \
  /* Updates object */                                                                      \
  orxObject_##FUNCTION(_pstObject, _Param);                                                 \
                                                                                            \
  /* For all its children */                                                                \
  for(pstChild = orxObject_GetOwnedChild(_pstObject);                                       \
      pstChild != orxNULL;                                                                  \
      pstChild = orxObject_GetOwnedSibling(pstChild))                                       \
  {                                                                                         \
    /* Updates it */                                                                        \
    orxObject_##FUNCTION##Recursive(pstChild, _Param);                                      \
  }                                                                                         \
                                                                                            \
  /* Done! */                                                                               \
  return;                                                                                   \
}


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Object storage structure
 */
typedef struct __orxOBJECT_STORAGE_t
{
  orxSTRUCTURE *pstStructure;                   /**< Structure pointer : 4 */
  orxU32        u32Flags;                       /**< Flags : 8 */

} orxOBJECT_STORAGE;

/** Object lists
 */
typedef struct __orxOBJECT_LISTS_t
{
  orxLINKLIST       stList;                     /**< List : 12 */
  orxLINKLIST       stEnableList;               /**< Enable list : 24 */
} orxOBJECT_LISTS;

/** Object structure
 */
struct __orxOBJECT_t
{
  orxSTRUCTURE      stStructure;                /**< Public structure, first structure member : 32 */
  orxOBJECT_STORAGE astStructureList[orxSTRUCTURE_ID_LINKABLE_NUMBER]; /**< Stored structures : 88 */
  void             *pUserData;                  /**< User data : 92 */
  const orxSTRING   zReference;                 /**< Config reference : 96 */
  orxSTRINGID       stGroupID;                  /**< Group ID : 100 */
  orxFLOAT          fLifeTime;                  /**< Life time : 104 */
  orxFLOAT          fActiveTime;                /**< Active time : 108 */
  orxFLOAT          fAngularVelocity;           /**< Angular velocity : 112 */
  orxOBJECT        *pstChild;                   /**< Child: 116 */
  orxOBJECT        *pstSibling;                 /**< Sibling: 120 */
  orxVECTOR         vSpeed;                     /**< Object speed : 132 */
  orxVECTOR         vSize;                      /**< Object size : 144 */
  orxVECTOR         vPivot;                     /**< Object pivot : 156 */
  orxLINKLIST_NODE  stGroupNode;                /**< Group node: 168 */
  orxLINKLIST_NODE  stEnableNode;               /**< Enable node: 180 */
  orxLINKLIST_NODE  stEnableGroupNode;          /**< Enable group node: 192 */
};

/** Static structure
 */
typedef struct __orxOBJECT_STATIC_t
{
  orxCLOCK         *pstClock;                   /**< Clock */
  orxBANK          *pstGroupBank;               /**< Group bank */
  orxBANK          *pstAgeBank;                 /**< Age bank */
  orxHASHTABLE     *pstGroupTable;              /**< Group table */
  orxOBJECT_LISTS  *pstCachedGroupLists;        /**< Cached group lists */
  orxOBJECT        *pstCurrentObject;           /**< Current object */
  orxLINKLIST       stEnableList;               /**< Enabled objects list */
  orxSTRINGID       stDefaultGroupID;           /**< Default group ID */
  orxSTRINGID       stCurrentGroupID;           /**< Current group ID */
  orxSTRINGID       stCachedGroupID;            /**< Cached group ID */
  orxU32            u32Flags;                   /**< Control flags */

} orxOBJECT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxOBJECT_STATIC sstObject;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Update body scale
 */
static void orxFASTCALL orxObject_UpdateBodyScale(orxOBJECT *_pstObject)
{
  orxOBJECT *pstChild;

  /* Not a joint child? */
  if(!orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_IS_JOINT_CHILD))
  {
    orxBODY *pstBody;

    /* Gets body */
    pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

    /* Valid? */
    if(pstBody != orxNULL)
    {
      orxFRAME *pstFrame;

      /* Gets frame */
      pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

      /* Valid? */
      if(pstFrame != orxNULL)
      {
        orxVECTOR vScale;

        /* Updates body scale */
        orxBody_SetScale(pstBody, orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vScale));
      }
    }
  }

  /* For all children */
  for(pstChild = orxObject_GetChild(_pstObject);
      pstChild != orxNULL;
      pstChild = orxObject_GetSibling(pstChild))
  {
    /* Updates its body scale */
    orxObject_UpdateBodyScale(pstChild);
  }

  /* Done! */
  return;
}

/** Sets relative pivot
 */
static orxINLINE orxSTATUS orxObject_SetRelativePivot(orxOBJECT *_pstObject, const orxSTRING _zPivot)
{
  orxCHAR   acBuffer[128];
  orxU32    u32AlignFlags = orxGRAPHIC_KU32_FLAG_ALIGN_CENTER;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Gets lower case value */
  acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
  orxString_LowerCase(orxString_NCopy(acBuffer, _zPivot, sizeof(acBuffer) - 1));

  /* Left? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_LEFT_PIVOT) != orxNULL)
  {
    /* Updates alignment flags */
    u32AlignFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_LEFT;
  }
  /* Right? */
  else if(orxString_SearchString(acBuffer, orxOBJECT_KZ_RIGHT_PIVOT) != orxNULL)
  {
    /* Updates alignment flags */
    u32AlignFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_RIGHT;
  }

  /* Top? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_TOP_PIVOT) != orxNULL)
  {
    /* Updates alignment flags */
    u32AlignFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_TOP;
  }
  /* Bottom? */
  else if(orxString_SearchString(acBuffer, orxOBJECT_KZ_BOTTOM_PIVOT) != orxNULL)
  {
    /* Updates alignment flags */
    u32AlignFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_BOTTOM;
  }

  /* Truncate? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_TRUNCATE_PIVOT) != orxNULL)
  {
    /* Updates alignment flags */
    u32AlignFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_TRUNCATE;
  }
  /* Round? */
  else if(orxString_SearchString(acBuffer, orxOBJECT_KZ_ROUND_PIVOT) != orxNULL)
  {
    /* Updates alignment flags */
    u32AlignFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_ROUND;
  }

  /* Valid? */
  if((u32AlignFlags != orxGRAPHIC_KU32_FLAG_ALIGN_CENTER)
  || (orxString_SearchString(acBuffer, orxOBJECT_KZ_CENTERED_PIVOT) != orxNULL))
  {
    orxGRAPHIC *pstGraphic;

    /* Gets graphic */
    pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

    /* Valid? */
    if(pstGraphic != orxNULL)
    {
      /* Updates its relative pivot */
      eResult = orxGraphic_SetRelativePivot(pstGraphic, u32AlignFlags);
    }
    else
    {
      orxVECTOR vSize;

      /* Valid size? */
      if(orxObject_GetSize(_pstObject, &vSize) != orxNULL)
      {
        orxFLOAT fHeight, fWidth;

        /* Gets graphic size */
        fWidth  = vSize.fX;
        fHeight = vSize.fY;

        /* Pivot left? */
        if(orxFLAG_TEST(u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_LEFT))
        {
          /* Updates x position */
          _pstObject->vPivot.fX = orxFLOAT_0;
        }
        /* Align right? */
        else if(orxFLAG_TEST(u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_RIGHT))
        {
          /* Updates x position */
          _pstObject->vPivot.fX = fWidth;
        }
        /* Align center */
        else
        {
          /* Updates x position */
          _pstObject->vPivot.fX = orx2F(0.5f) * fWidth;
        }

        /* Align top? */
        if(orxFLAG_TEST(u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_TOP))
        {
          /* Updates y position */
          _pstObject->vPivot.fY = orxFLOAT_0;
        }
        /* Align bottom? */
        else if(orxFLAG_TEST(u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_BOTTOM))
        {
          /* Updates y position */
          _pstObject->vPivot.fY = fHeight;
        }
        /* Align center */
        else
        {
          /* Updates y position */
          _pstObject->vPivot.fY = orx2F(0.5f) * fHeight;
        }

        /* Truncate? */
        if(orxFLAG_TEST(u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_TRUNCATE))
        {
          /* Updates position */
          orxVector_Floor(&(_pstObject->vPivot), &(_pstObject->vPivot));
        }
        /* Round? */
        else if(orxFLAG_TEST(u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_ROUND))
        {
          /* Updates position */
          orxVector_Round(&(_pstObject->vPivot), &(_pstObject->vPivot));
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid size retrieved from object.");
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Sets literal lifetime
 */
static orxINLINE orxSTATUS orxObject_SetLiteralLifeTime(orxOBJECT *_pstObject, const orxSTRING _zLifeTime)
{
  orxCHAR   acBuffer[128];
  orxU32    u32Flags = orxOBJECT_KU32_FLAG_NONE;
  orxSTATUS eResult;

  /* Gets lower case value */
  acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
  orxString_LowerCase(orxString_NCopy(acBuffer, _zLifeTime, sizeof(acBuffer) - 1));

  /* FX? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_FX) != orxNULL)
  {
    /* Updates flags */
    u32Flags |= orxOBJECT_KU32_FLAG_FX_LIFETIME;
  }

  /* Sound? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_SOUND) != orxNULL)
  {
    /* Updates flags */
    u32Flags |= orxOBJECT_KU32_FLAG_SOUND_LIFETIME;
  }

  /* Spawn? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_SPAWN) != orxNULL)
  {
    /* Updates flags */
    u32Flags |= orxOBJECT_KU32_FLAG_SPAWNER_LIFETIME;
  }

  /* Track? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_TRACK) != orxNULL)
  {
    /* Updates flags */
    u32Flags |= orxOBJECT_KU32_FLAG_TIMELINE_LIFETIME;
  }

  /* Child? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_CHILD) != orxNULL)
  {
    /* Updates flags */
    u32Flags |= orxOBJECT_KU32_FLAG_CHILDREN_LIFETIME;
  }

  /* Anim? */
  if(orxString_SearchString(acBuffer, orxOBJECT_KZ_ANIM) != orxNULL)
  {
    /* Updates flags */
    u32Flags |= orxOBJECT_KU32_FLAG_ANIM_LIFETIME;
  }

  /* Has flags? */
  if(orxFLAG_TEST(u32Flags, orxOBJECT_KU32_MASK_STRUCTURE_LIFETIME))
  {
    /* Clears previous lifetime */
    orxObject_SetLifeTime(_pstObject, -orxFLOAT_1);

    /* Applies them */
    orxStructure_SetFlags(_pstObject, u32Flags, orxOBJECT_KU32_MASK_STRUCTURE_LIFETIME);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Command: Create
 */
void orxFASTCALL orxObject_CommandCreate(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Creates object */
  pstObject = orxObject_CreateFromConfig(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->u64Value = (pstObject != orxNULL) ? orxStructure_GetGUID(pstObject) : orxU64_UNDEFINED;

  /* Done! */
  return;
}

/** Command: Delete
 */
void orxFASTCALL orxObject_CommandDelete(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Marks it for deletion */
    orxObject_SetLifeTime(pstObject, orxFLOAT_0);

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

/** Command: FindNext
 */
void orxFASTCALL orxObject_CommandFindNext(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstPrevious, *pstObject;

  /* Updates result */
  _pstResult->u64Value = orxU64_UNDEFINED;

  /* Gets previous object */
  pstPrevious = (_u32ArgNumber > 1) ? orxOBJECT(orxStructure_Get(_astArgList[1].u64Value)) : orxNULL;

  /* For all next objects */
  for(pstObject = (pstPrevious != orxNULL) ? orxOBJECT(orxStructure_GetNext(pstPrevious)) : orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));
      pstObject != orxNULL;
      pstObject = orxOBJECT(orxStructure_GetNext(pstObject)))
  {
    /* Correct name? */
    if((_u32ArgNumber == 0) || (*_astArgList[0].zValue == '*') || (orxString_Compare(_astArgList[0].zValue, orxObject_GetName(pstObject)) == 0))
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstObject);

      break;
    }
  }

  /* Done! */
  return;
}

/** Command: GetCount
 */
void orxFASTCALL orxObject_CommandGetCount(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxU32      u32Count;
  orxOBJECT  *pstObject;

  /* No name? */
  if((_u32ArgNumber == 0) || (*_astArgList[0].zValue == orxCHAR_NULL))
  {
    /* Enabled only? */
    if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
    {
      /* Updates count */
      u32Count = orxLinkList_GetCount(&(sstObject.stEnableList));
    }
    else
    {
      /* Updates count */
      u32Count = orxStructure_GetCount(orxSTRUCTURE_ID_OBJECT);
    }
  }
  else
  {
    /* Enabled only? */
    if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
    {
      orxLINKLIST_NODE *pstNode;

      /* For all enabled nodes */
      for(pstNode = orxLinkList_GetFirst(&(sstObject.stEnableList)), u32Count = 0;
          pstNode != orxNULL;
          pstNode = orxLinkList_GetNext(pstNode))
      {
        orxOBJECT *pstObject;

        /* Gets associated object */
        pstObject = orxSTRUCT_GET_FROM_FIELD(orxOBJECT, stEnableNode, pstNode);

        /* Match? */
        if(orxString_Compare(orxObject_GetName(pstObject), _astArgList[0].zValue) == 0)
        {
          /* Updates count */
          u32Count++;
        }
      }
    }
    else
    {
      /* For all objects */
      for(pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT)), u32Count = 0;
          pstObject != orxNULL;
          pstObject = orxOBJECT(orxStructure_GetNext(pstObject)))
      {
        /* Match? */
        if(orxString_Compare(orxObject_GetName(pstObject), _astArgList[0].zValue) == 0)
        {
          /* Updates count */
          u32Count++;
        }
      }
    }
  }

  /* Updates result */
  _pstResult->u32Value = u32Count;

  /* Done! */
  return;
}

/** Command: GetID
 */
void orxFASTCALL orxObject_CommandGetID(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
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

/** Command: SetPosition
 */
void orxFASTCALL orxObject_CommandSetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Global? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Sets its position */
      orxObject_SetWorldPosition(pstObject, &(_astArgList[1].vValue));
    }
    else
    {
      /* Sets its position */
      orxObject_SetPosition(pstObject, &(_astArgList[1].vValue));
    }

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

/** Command: SetRotation
 */
void orxFASTCALL orxObject_CommandSetRotation(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Global? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Sets its rotation */
      orxObject_SetWorldRotation(pstObject, orxMATH_KF_DEG_TO_RAD * _astArgList[1].fValue);
    }
    else
    {
      /* Sets its rotation */
      orxObject_SetRotation(pstObject, orxMATH_KF_DEG_TO_RAD * _astArgList[1].fValue);
    }

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

/** Command: SetScale
 */
void orxFASTCALL orxObject_CommandSetScale(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOMMAND_VAR stOperand;

  /* Parses numerical arguments */
  if(orxCommand_ParseNumericalArguments(1, &_astArgList[1], &stOperand) != orxSTATUS_FAILURE)
  {
    orxOBJECT *pstObject;

    /* Gets object */
    pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

    /* Valid? */
    if(pstObject != orxNULL)
    {
      orxVECTOR vScale;

      /* Was single float value? */
      if(stOperand.eType == orxCOMMAND_VAR_TYPE_FLOAT)
      {
        /* Sets vector */
        orxVector_SetAll(&vScale, stOperand.fValue);
      }
      else
      {
        orxASSERT(stOperand.eType == orxCOMMAND_VAR_TYPE_VECTOR);

        /* Copies vector */
        orxVector_Copy(&vScale, &(stOperand.vValue));
      }

      /* Global? */
      if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
      {
        /* Sets its scale */
        orxObject_SetWorldScale(pstObject, &vScale);
      }
      else
      {
        /* Sets its scale */
        orxObject_SetScale(pstObject, &vScale);
      }

      /* Updates result */
      _pstResult->u64Value = _astArgList[0].u64Value;
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetPosition
 */
void orxFASTCALL orxObject_CommandGetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Global? */
    if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
    {
      /* Gets its position */
      orxObject_GetWorldPosition(pstObject, &(_pstResult->vValue));
    }
    else
    {
      /* Gets its position */
      orxObject_GetPosition(pstObject, &(_pstResult->vValue));
    }
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: GetRotation
 */
void orxFASTCALL orxObject_CommandGetRotation(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Global? */
    if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
    {
      /* Gets its rotation */
      _pstResult->fValue = orxMATH_KF_RAD_TO_DEG * orxObject_GetWorldRotation(pstObject);
    }
    else
    {
      /* Gets its position */
      _pstResult->fValue = orxMATH_KF_RAD_TO_DEG * orxObject_GetRotation(pstObject);
    }
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orxFLOAT_0;
  }

  /* Done! */
  return;
}

/** Command: GetScale
 */
void orxFASTCALL orxObject_CommandGetScale(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Global? */
    if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
    {
      /* Gets its scale */
      orxObject_GetWorldScale(pstObject, &(_pstResult->vValue));
    }
    else
    {
      /* Gets its scale */
      orxObject_GetScale(pstObject, &(_pstResult->vValue));
    }
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: SetSpeed
 */
void orxFASTCALL orxObject_CommandSetSpeed(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Relative? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Sets its relative speed */
      orxObject_SetRelativeSpeed(pstObject, &(_astArgList[1].vValue));
    }
    else
    {
      /* Sets its speed */
      orxObject_SetSpeed(pstObject, &(_astArgList[1].vValue));
    }

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

/** Command: SetAngularVelocity
 */
void orxFASTCALL orxObject_CommandSetAngularVelocity(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its angular velocity */
    orxObject_SetAngularVelocity(pstObject, orxMATH_KF_DEG_TO_RAD * _astArgList[1].fValue);

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

/** Command: SetCustomGravity
 */
void orxFASTCALL orxObject_CommandSetCustomGravity(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its custom gravity */
    orxObject_SetCustomGravity(pstObject, &(_astArgList[1].vValue));

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

/** Command: GetSpeed
 */
void orxFASTCALL orxObject_CommandGetSpeed(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Relative? */
    if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
    {
      /* Gets its relative speed */
      orxObject_GetRelativeSpeed(pstObject, &(_pstResult->vValue));
    }
    else
    {
      /* Gets its speed */
      orxObject_GetSpeed(pstObject, &(_pstResult->vValue));
    }
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: GetAngularVelocity
 */
void orxFASTCALL orxObject_CommandGetAngularVelocity(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Gets its angular velocity */
    _pstResult->fValue = orxMATH_KF_RAD_TO_DEG * orxObject_GetAngularVelocity(pstObject);
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orxFLOAT_0;
  }

  /* Done! */
  return;
}

/** Command: GetCustomGravity
 */
void orxFASTCALL orxObject_CommandGetCustomGravity(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Gets its custom gravity */
    orxObject_GetCustomGravity(pstObject, &(_pstResult->vValue));
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: GetMass
 */
void orxFASTCALL orxObject_CommandGetMass(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    _pstResult->fValue = orxObject_GetMass(pstObject);
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orxFLOAT_0;
  }

  /* Done! */
  return;
}

/** Command: GetMassCenter
 */
void orxFASTCALL orxObject_CommandGetMassCenter(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    orxObject_GetMassCenter(pstObject, &(_pstResult->vValue));
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: SetText
 */
void orxFASTCALL orxObject_CommandSetText(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its text string */
    orxObject_SetTextString(pstObject, _astArgList[1].zValue);

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

/** Command: GetText
 */
void orxFASTCALL orxObject_CommandGetText(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    _pstResult->zValue = orxObject_GetTextString(pstObject);
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/** Command: SetRepeat
 */
void orxFASTCALL orxObject_CommandSetRepeat(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its repeat */
    orxObject_SetRepeat(pstObject, _astArgList[1].vValue.fX, _astArgList[1].vValue.fY);

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

/** Command: GetRepeat
 */
void orxFASTCALL orxObject_CommandGetRepeat(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Gets its repeat */
    orxObject_GetRepeat(pstObject, &(_pstResult->vValue.fX), &(_pstResult->vValue.fY));
    _pstResult->vValue.fZ = orxFLOAT_0;
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: SetGroup
 */
void orxFASTCALL orxObject_CommandSetGroup(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_SetGroupIDRecursive(pstObject, orxString_GetID(_astArgList[1].zValue));
    }
    else
    {
      /* Sets its Group */
      orxObject_SetGroupID(pstObject, (_u32ArgNumber > 1) ? orxString_GetID(_astArgList[1].zValue) : sstObject.stDefaultGroupID);
    }

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

/** Command: GetGroup
 */
void orxFASTCALL orxObject_CommandGetGroup(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Updates result */
  _pstResult->zValue = (pstObject != orxNULL) ? orxString_GetFromID(orxObject_GetGroupID(pstObject)) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: GetName
 */
void orxFASTCALL orxObject_CommandGetName(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Updates result */
  _pstResult->zValue = (pstObject != orxNULL) ? orxObject_GetName(pstObject) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: SetLifeTime
 */
void orxFASTCALL orxObject_CommandSetLifeTime(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Set literal lifetime? */
    if(orxObject_SetLiteralLifeTime(pstObject, _astArgList[1].zValue) == orxSTATUS_FAILURE)
    {
      orxFLOAT fLifeTime;

      /* Gets lifetime value */
      if(orxString_ToFloat(_astArgList[1].zValue, &fLifeTime, orxNULL) != orxSTATUS_FAILURE)
      {
        /* Applies it */
        orxObject_SetLifeTime(pstObject, fLifeTime);
      }
      else
      {
        /* Clears it */
        orxObject_SetLifeTime(pstObject, -orxFLOAT_1);
      }
    }

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

/** Command: GetLifeTime
 */
void orxFASTCALL orxObject_CommandGetLifeTime(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    _pstResult->fValue = orxObject_GetLifeTime(pstObject);
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orx2F(-1.0f);
  }

  /* Done! */
  return;
}

/** Command: SetColor
 */
void orxFASTCALL orxObject_CommandSetColor(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxCOLOR stColor;

    /* Inits color */
    orxVector_Mulf(&(stColor.vRGB), &(_astArgList[1].vValue), orxCOLOR_NORMALIZER);
    stColor.fAlpha = orxFLOAT_1;

    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_SetRGBRecursive(pstObject, &(stColor.vRGB));
    }
    else
    {
      /* Updates it */
      orxObject_SetRGB(pstObject, &(stColor.vRGB));
    }

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

/** Command: GetColor
 */
void orxFASTCALL orxObject_CommandGetColor(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxCOLOR stColor;

    /* Gets its color */
    if(orxObject_GetColor(pstObject, &stColor) == orxNULL)
    {
      /* Updates result */
      orxVector_Mulf(&(_pstResult->vValue), &orxVECTOR_WHITE, orxCOLOR_DENORMALIZER);
    }
    else
    {
      /* Updates result */
      orxVector_Mulf(&(_pstResult->vValue), &(stColor.vRGB), orxCOLOR_DENORMALIZER);
    }
  }
  else
  {
    /* Updates result */
    orxVector_Mulf(&(_pstResult->vValue), &orxVECTOR_WHITE, orxCOLOR_DENORMALIZER);
  }

  /* Done! */
  return;
}

/** Command: SetRGB
 */
void orxFASTCALL orxObject_CommandSetRGB(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxCOLOR stColor;

    /* Inits color with RGB values */
    orxColor_Set(&stColor, &(_astArgList[1].vValue), orxFLOAT_1);

    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_SetRGBRecursive(pstObject, &(stColor.vRGB));
    }
    else
    {
      /* Updates it */
      orxObject_SetRGB(pstObject, &(stColor.vRGB));
    }

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

/** Command: GetRGB
 */
void orxFASTCALL orxObject_CommandGetRGB(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOLOR    stColor;
  orxOBJECT  *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Gets its color */
    if(orxObject_GetColor(pstObject, &stColor) != orxNULL)
    {
      /* Updates result */
      orxVector_Copy(&(_pstResult->vValue), &(stColor.vRGB));
    }
    else
    {
      /* Updates result */
      orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_WHITE);
    }
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_WHITE);
  }

  /* Done! */
  return;
}

/** Command: SetHSL
 */
void orxFASTCALL orxObject_CommandSetHSL(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxCOLOR stColor;

    /* Inits color with HSL values */
    orxColor_Set(&stColor, &(_astArgList[1].vValue), orxFLOAT_1);

    /* Converts color to RGB */
    orxColor_FromHSLToRGB(&stColor, &stColor);

    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_SetRGBRecursive(pstObject, &(stColor.vRGB));
    }
    else
    {
      /* Updates it */
      orxObject_SetRGB(pstObject, &(stColor.vRGB));
    }

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

/** Command: GetHSL
 */
void orxFASTCALL orxObject_CommandGetHSL(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOLOR    stColor;
  orxOBJECT  *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Gets its color */
    if(orxObject_GetColor(pstObject, &stColor) == orxNULL)
    {
      /* Uses white */
      orxVector_Copy(&(stColor.vRGB), &orxVECTOR_WHITE);
    }
  }
  else
  {
    /* Uses white */
    orxVector_Copy(&(stColor.vRGB), &orxVECTOR_WHITE);
  }

  /* Converts color to HSL */
  orxColor_FromRGBToHSL(&stColor, &stColor);

  /* Updates result */
  orxVector_Copy(&(_pstResult->vValue), &(stColor.vHSL));

  /* Done! */
  return;
}

/** Command: SetHSV
 */
void orxFASTCALL orxObject_CommandSetHSV(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxCOLOR stColor;

    /* Inits color with HSV values */
    orxColor_Set(&stColor, &(_astArgList[1].vValue), orxFLOAT_1);

    /* Converts color to RGB */
    orxColor_FromHSVToRGB(&stColor, &stColor);

    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_SetRGBRecursive(pstObject, &(stColor.vRGB));
    }
    else
    {
      /* Updates it */
      orxObject_SetRGB(pstObject, &(stColor.vRGB));
    }

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

/** Command: GetHSV
 */
void orxFASTCALL orxObject_CommandGetHSV(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCOLOR    stColor;
  orxOBJECT  *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Gets its color */
    if(orxObject_GetColor(pstObject, &stColor) == orxNULL)
    {
      /* Uses white */
      orxVector_Copy(&(stColor.vRGB), &orxVECTOR_WHITE);
    }
  }
  else
  {
    /* Uses white */
    orxVector_Copy(&(stColor.vRGB), &orxVECTOR_WHITE);
  }

  /* Converts color to HSV */
  orxColor_FromRGBToHSV(&stColor, &stColor);

  /* Updates result */
  orxVector_Copy(&(_pstResult->vValue), &(stColor.vHSV));

  /* Done! */
  return;
}

/** Command: SetAlpha
 */
void orxFASTCALL orxObject_CommandSetAlpha(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_SetAlphaRecursive(pstObject, _astArgList[1].fValue);
    }
    else
    {
      /* Updates it */
      orxObject_SetAlpha(pstObject, _astArgList[1].fValue);
    }

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

/** Command: GetAlpha
 */
void orxFASTCALL orxObject_CommandGetAlpha(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxCOLOR stColor;

    /* Gets its color */
    if(orxObject_GetColor(pstObject, &stColor) == orxNULL)
    {
      /* Updates result */
      _pstResult->fValue = orxFLOAT_1;
    }
    else
    {
      /* Updates result */
      _pstResult->fValue = stColor.fAlpha;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orxFLOAT_1;
  }

  /* Done! */
  return;
}

/** Command: Enable
 */
void orxFASTCALL orxObject_CommandEnable(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_EnableRecursive(pstObject, _astArgList[1].bValue);
    }
    else
    {
      /* Updates it */
      orxObject_Enable(pstObject, (_u32ArgNumber < 2) || (_astArgList[1].bValue != orxFALSE) ? orxTRUE : orxFALSE);
    }

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

/** Command: IsEnabled
 */
void orxFASTCALL orxObject_CommandIsEnabled(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    _pstResult->bValue = orxObject_IsEnabled(pstObject);
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Command: Pause
 */
void orxFASTCALL orxObject_CommandPause(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Updates it */
      orxObject_PauseRecursive(pstObject, _astArgList[1].bValue);
    }
    else
    {
      /* Updates it */
      orxObject_Pause(pstObject, (_u32ArgNumber < 2) || (_astArgList[1].bValue != orxFALSE) ? orxTRUE : orxFALSE);
    }

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

/** Command: IsPaused
 */
void orxFASTCALL orxObject_CommandIsPaused(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    _pstResult->bValue = orxObject_IsPaused(pstObject);
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Command: Play
 */
void orxFASTCALL orxObject_CommandPlay(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates it */
    orxObject_Play(pstObject);

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

/** Command: Stop
 */
void orxFASTCALL orxObject_CommandStop(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates it */
    orxObject_Stop(pstObject);

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

/** Command: SetParent
 */
void orxFASTCALL orxObject_CommandSetParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Has parent? */
    if((_u32ArgNumber > 1) && (_astArgList[1].u64Value != 0))
    {
      orxSTRUCTURE *pstParent;

      /* Gets parent */
      pstParent = orxStructure_Get(_astArgList[1].u64Value);

      /* Valid? */
      if(pstParent != orxNULL)
      {
        /* Updates its parent */
        orxObject_SetParent(pstObject, pstParent);
      }
    }
    else
    {
      /* Removes parent */
      orxObject_SetParent(pstObject, orxNULL);
    }

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

/** Command: GetParent
 */
void orxFASTCALL orxObject_CommandGetParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxSTRUCTURE *pstParent;

    /* Gets its parent */
    pstParent = orxObject_GetParent(pstObject);

    /* Valid? */
    if(pstParent != orxNULL)
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstParent);
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetChild
 */
void orxFASTCALL orxObject_CommandGetChild(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxOBJECT *pstChild;

    /* Gets its child */
    pstChild = orxObject_GetChild(pstObject);

    /* Valid? */
    if(pstChild != orxNULL)
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstChild);
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetSibling
 */
void orxFASTCALL orxObject_CommandGetSibling(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxOBJECT *pstSibling;

    /* Gets its sibling */
    pstSibling = orxObject_GetSibling(pstObject);

    /* Valid? */
    if(pstSibling != orxNULL)
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstSibling);
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: Attach
 */
void orxFASTCALL orxObject_CommandAttach(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Has parent? */
    if((_u32ArgNumber > 1) && (_astArgList[1].u64Value != 0))
    {
      orxSTRUCTURE *pstParent;

      /* Gets parent */
      pstParent = orxStructure_Get(_astArgList[1].u64Value);

      /* Valid? */
      if(pstParent != orxNULL)
      {
        /* Attaches it */
        orxObject_Attach(pstObject, pstParent);
      }
    }
    else
    {
      /* Detaches it */
      orxObject_Detach(pstObject);
    }

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

/** Command: Detach
 */
void orxFASTCALL orxObject_CommandDetach(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Detaches it */
    orxObject_Detach(pstObject);

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

/** Command: LogParents
 */
void orxFASTCALL orxObject_CommandLogParents(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Logs its parents */
    orxObject_LogParents(pstObject);

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

/** Command: SetOwner
 */
void orxFASTCALL orxObject_CommandSetOwner(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Has owner? */
    if((_u32ArgNumber > 1) && (_astArgList[1].u64Value != 0))
    {
      orxSTRUCTURE *pstOwner;

      /* Gets owner */
      pstOwner = orxStructure_Get(_astArgList[1].u64Value);

      /* Valid? */
      if(pstOwner != orxNULL)
      {
        /* Updates its owner */
        orxObject_SetOwner(pstObject, pstOwner);
      }
    }
    else
    {
      /* Removes owner */
      orxObject_SetOwner(pstObject, orxNULL);
    }

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

/** Command: GetOwner
 */
void orxFASTCALL orxObject_CommandGetOwner(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxSTRUCTURE *pstOwner;

    /* Gets its owner */
    pstOwner = orxObject_GetOwner(pstObject);

    /* Valid? */
    if(pstOwner != orxNULL)
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstOwner);
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetOwnedChild
 */
void orxFASTCALL orxObject_CommandGetOwnedChild(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxOBJECT *pstOwnedChild;

    /* Gets its owned child */
    pstOwnedChild = orxObject_GetOwnedChild(pstObject);

    /* Valid? */
    if(pstOwnedChild != orxNULL)
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstOwnedChild);
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetOwnedSibling
 */
void orxFASTCALL orxObject_CommandGetOwnedSibling(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxOBJECT *pstOwnedSibling;

    /* Gets its owned sibling */
    pstOwnedSibling = orxObject_GetOwnedSibling(pstObject);

    /* Valid? */
    if(pstOwnedSibling != orxNULL)
    {
      /* Updates result */
      _pstResult->u64Value = orxStructure_GetGUID(pstOwnedSibling);
    }
    else
    {
      /* Updates result */
      _pstResult->u64Value = orxU64_UNDEFINED;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetClock
 */
void orxFASTCALL orxObject_CommandSetClock(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Has clock? */
    if(_u32ArgNumber > 1)
    {
      orxCLOCK *pstClock;

      /* Creates clock */
      pstClock = orxClock_CreateFromConfig(_astArgList[1].zValue);

      /* Valid? */
      if(pstClock != orxNULL)
      {
        /* Links it */
        if(orxObject_LinkStructure(pstObject, orxSTRUCTURE(pstClock)) != orxSTATUS_FAILURE)
        {
          /* Updates flags */
          orxFLAG_SET(pstObject->astStructureList[orxSTRUCTURE_ID_CLOCK].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

          /* Updates its owner */
          orxStructure_SetOwner(pstClock, pstObject);
        }
        else
        {
          /* Deletes it */
          orxClock_Delete(pstClock);
        }
      }
    }
    else
    {
      /* Removes clock */
      orxObject_SetClock(pstObject, orxNULL);
    }

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

/** Command: GetClock
 */
void orxFASTCALL orxObject_CommandGetClock(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    orxCLOCK *pstClock;

    /* Gets clock */
    pstClock = orxObject_GetClock(pstObject);

    /* Valid? */
    if(pstClock != orxNULL)
    {
      /* Updates result */
      _pstResult->zValue = orxClock_GetName(pstClock);
    }
    else
    {
      /* Updates result */
      _pstResult->zValue = orxSTRING_EMPTY;
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/** Command: AddTrack
 */
void orxFASTCALL orxObject_CommandAddTrack(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Adds time line track */
      orxObject_AddTimeLineTrackRecursive(pstObject, _astArgList[1].zValue);
    }
    else
    {
      /* Adds time line track */
      orxObject_AddTimeLineTrack(pstObject, _astArgList[1].zValue);
    }

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

/** Command: RemoveTrack
 */
void orxFASTCALL orxObject_CommandRemoveTrack(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Removes time line track */
      orxObject_RemoveTimeLineTrackRecursive(pstObject, _astArgList[1].zValue);
    }
    else
    {
      /* Removes time line track */
      orxObject_RemoveTimeLineTrack(pstObject, _astArgList[1].zValue);
    }

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

/** Command: AddFX
 */
void orxFASTCALL orxObject_CommandAddFX(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 3) && (_astArgList[3].bValue != orxFALSE))
    {
      /* Unique? */
      if(_astArgList[2].bValue != orxFALSE)
      {
        /* Adds unique FX */
        orxObject_AddUniqueFXRecursive(pstObject, _astArgList[1].zValue);
      }
      else
      {
        /* Adds FX */
        orxObject_AddFXRecursive(pstObject, _astArgList[1].zValue);
      }
    }
    else
    {
      /* Unique? */
      if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
      {
        /* Adds unique FX */
        orxObject_AddUniqueFX(pstObject, _astArgList[1].zValue);
      }
      else
      {
        /* Adds FX */
        orxObject_AddFX(pstObject, _astArgList[1].zValue);
      }
    }

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

/** Command: RemoveFX
 */
void orxFASTCALL orxObject_CommandRemoveFX(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Removes FX */
      orxObject_RemoveFXRecursive(pstObject, _astArgList[1].zValue);
    }
    else
    {
      /* Removes FX */
      orxObject_RemoveFX(pstObject, _astArgList[1].zValue);
    }

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

/** Command: RemoveAllFXs
 */
void orxFASTCALL orxObject_CommandRemoveAllFXs(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Removes all its FXs */
    orxObject_RemoveAllFXs(pstObject);

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

/** Command: AddShader
 */
void orxFASTCALL orxObject_CommandAddShader(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Adds shader */
      orxObject_AddShaderRecursive(pstObject, _astArgList[1].zValue);
    }
    else
    {
      /* Adds shader */
      orxObject_AddShader(pstObject, _astArgList[1].zValue);
    }

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

/** Command: RemoveShader
 */
void orxFASTCALL orxObject_CommandRemoveShader(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Removes shader */
      orxObject_RemoveShaderRecursive(pstObject, _astArgList[1].zValue);
    }
    else
    {
      /* Removes shader */
      orxObject_RemoveShader(pstObject, _astArgList[1].zValue);
    }

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

/** Command: AddSound
 */
void orxFASTCALL orxObject_CommandAddSound(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Adds sound */
    orxObject_AddSound(pstObject, _astArgList[1].zValue);

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

/** Command: RemoveSound
 */
void orxFASTCALL orxObject_CommandRemoveSound(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Removes sound */
    orxObject_RemoveSound(pstObject, _astArgList[1].zValue);

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

/** Command: SetVolume
 */
void orxFASTCALL orxObject_CommandSetVolume(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its volume */
    orxObject_SetVolume(pstObject, _astArgList[1].fValue);

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

/** Command: SetPitch
 */
void orxFASTCALL orxObject_CommandSetPitch(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its pitch */
    orxObject_SetPitch(pstObject, _astArgList[1].fValue);

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

/** Command: SetAnim
 */
void orxFASTCALL orxObject_CommandSetAnim(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 3) && (_astArgList[3].bValue != orxFALSE))
    {
      /* Is asking for current anim? */
      if(_astArgList[2].bValue != orxFALSE)
      {
        /* Sets its current anim */
        orxObject_SetCurrentAnimRecursive(pstObject, _astArgList[1].zValue);
      }
      else
      {
        /* Sets its target anim */
        orxObject_SetTargetAnimRecursive(pstObject, _astArgList[1].zValue);
      }
    }
    else
    {
      /* Is asking for current anim? */
      if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
      {
        /* Sets its current anim */
        orxObject_SetCurrentAnim(pstObject, _astArgList[1].zValue);
      }
      else
      {
        /* Sets its target anim */
        orxObject_SetTargetAnim(pstObject, (_u32ArgNumber > 1) ? _astArgList[1].zValue : orxNULL);
      }
    }

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

/** Command: GetAnim
 */
void orxFASTCALL orxObject_CommandGetAnim(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Is asking for current anim? */
    if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
    {
      /* Updates result */
      _pstResult->zValue = orxObject_GetCurrentAnim(pstObject);
    }
    else
    {
      /* Updates result */
      _pstResult->zValue = orxObject_GetTargetAnim(pstObject);
    }
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/** Command: SetAnimFrequency
 */
void orxFASTCALL orxObject_CommandSetAnimFrequency(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Recursive? */
    if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
    {
      /* Sets its anim frequency */
      orxObject_SetAnimFrequencyRecursive(pstObject, _astArgList[1].fValue);
    }
    else
    {
      /* Sets its anim frequency */
      orxObject_SetAnimFrequency(pstObject, (_u32ArgNumber > 1) ? _astArgList[1].fValue : orxFLOAT_1);
    }

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

/** Command: GetAnimFrequency
 */
void orxFASTCALL orxObject_CommandGetAnimFrequency(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    _pstResult->fValue = orxObject_GetAnimFrequency(pstObject);
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = -orxFLOAT_1;
  }

  /* Done! */
  return;
}

/** Command: SetOrigin
 */
void orxFASTCALL orxObject_CommandSetOrigin(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its origin */
    orxObject_SetOrigin(pstObject, &(_astArgList[1].vValue));

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

/** Command: GetOrigin
 */
void orxFASTCALL orxObject_CommandGetOrigin(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    orxObject_GetOrigin(pstObject, &(_pstResult->vValue));
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: SetSize
 */
void orxFASTCALL orxObject_CommandSetSize(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets its size */
    orxObject_SetSize(pstObject, &(_astArgList[1].vValue));

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

/** Command: GetSize
 */
void orxFASTCALL orxObject_CommandGetSize(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    orxObject_GetSize(pstObject, &(_pstResult->vValue));
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: SetPivot
 */
void orxFASTCALL orxObject_CommandSetPivot(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT  *pstObject;
  orxU64      u64Result = orxU64_UNDEFINED;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Sets relative pivot? */
    if(orxObject_SetRelativePivot(pstObject, _astArgList[1].zValue) == orxSTATUS_FAILURE)
    {
      orxVECTOR vPivot;

      /* Gets pivot value */
      if(orxString_ToVector(_astArgList[1].zValue, &vPivot, orxNULL) != orxSTATUS_FAILURE)
      {
        /* Applies it */
        orxObject_SetPivot(pstObject, &vPivot);

        /* Updates result */
        u64Result = _astArgList[0].u64Value;
      }
    }
    else
    {
      /* Updates result */
      u64Result = _astArgList[0].u64Value;
    }
  }

  /* Updates result */
  _pstResult->u64Value = u64Result;

  /* Done! */
  return;
}

/** Command: GetPivot
 */
void orxFASTCALL orxObject_CommandGetPivot(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxOBJECT *pstObject;

  /* Gets object */
  pstObject = orxOBJECT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstObject != orxNULL)
  {
    /* Updates result */
    orxObject_GetPivot(pstObject, &(_pstResult->vValue));
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Registers all the object commands
 */
static orxINLINE void orxObject_RegisterCommands()
{
  /* Command: Create */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Create, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Name", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: Delete */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Delete, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: FindNext */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, FindNext, "Object", orxCOMMAND_VAR_TYPE_U64, 0, 2, {"Name = *", orxCOMMAND_VAR_TYPE_STRING}, {"Previous = <none>", orxCOMMAND_VAR_TYPE_U64});

  /* Command: GetCount */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetCount, "Count", orxCOMMAND_VAR_TYPE_U32, 0, 2, {"Name = <empty>", orxCOMMAND_VAR_TYPE_STRING}, {"EnabledOnly = false", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: GetID */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetID, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetPosition, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Position", orxCOMMAND_VAR_TYPE_VECTOR}, {"Global = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: SetRotation */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetRotation, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Rotation", orxCOMMAND_VAR_TYPE_FLOAT}, {"Global = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: SetScale */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetScale, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Scale", orxCOMMAND_VAR_TYPE_NUMERIC}, {"Global = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetPosition, "Position", orxCOMMAND_VAR_TYPE_VECTOR, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Global = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetRotation */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetRotation, "Rotation", orxCOMMAND_VAR_TYPE_FLOAT, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Global = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetScale */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetScale, "Scale", orxCOMMAND_VAR_TYPE_VECTOR, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Global = false", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: SetSpeed */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetSpeed, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Speed", orxCOMMAND_VAR_TYPE_VECTOR}, {"Relative = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: SetAngularVelocity */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetAngularVelocity, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"AngularVelocity", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: SetCustomGravity */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetCustomGravity, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"CustomGravity", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: GetSpeed */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetSpeed, "Speed", orxCOMMAND_VAR_TYPE_VECTOR, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Relative = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetAngularVelocity */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetAngularVelocity, "AngularVelocity", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetCustomGravity */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetCustomGravity, "CustomGravity", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: GetMass */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetMass, "Mass", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetMassCenter */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetMassCenter, "MassCenter", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetText */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetText, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Text", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetText */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetText, "Text", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetRepeat */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetRepeat, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Repeat", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: GetRepeat */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetRepeat, "Repeat", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetGroup */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetGroup, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 2, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Group = <default>", orxCOMMAND_VAR_TYPE_STRING}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetGroup */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetGroup, "Group", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: GetName */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetName, "Name", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetLifeTime */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetLifeTime, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"LifeTime", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetLifeTime */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetLifeTime, "LifeTime", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetColor */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetColor, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Color", orxCOMMAND_VAR_TYPE_VECTOR}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetColor */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetColor, "Color", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: SetRGB */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetRGB, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"RGB", orxCOMMAND_VAR_TYPE_VECTOR}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetRGB */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetRGB, "RGB", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: SetHSL */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetHSL, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"HSL", orxCOMMAND_VAR_TYPE_VECTOR}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetHSL */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetHSL, "HSL", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: SetHSV */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetHSV, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"HSV", orxCOMMAND_VAR_TYPE_VECTOR}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetHSV */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetHSV, "HSV", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: SetAlpha */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetAlpha, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Alpha", orxCOMMAND_VAR_TYPE_FLOAT}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetAlpha */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetAlpha, "Alpha", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Enable */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Enable, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 2, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Enable = true", orxCOMMAND_VAR_TYPE_BOOL}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: IsEnabled */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, IsEnabled, "IsEnabled?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: Pause */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Pause, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 2, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Pause = true", orxCOMMAND_VAR_TYPE_BOOL}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: IsPaused */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, IsPaused, "IsPaused?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Play */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Play, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: Stop */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Stop, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetParent, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Parent = <void>", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetParent, "Parent", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetChild */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetChild, "Child", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetSibling */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetSibling, "Sibling", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Attach */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Attach, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Parent = <void>", orxCOMMAND_VAR_TYPE_U64});
  /* Command: Detach */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, Detach, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: LogParents */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, LogParents, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetOwner */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetOwner, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Owner = <void>", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetOwner */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetOwner, "Owner", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetOwnedChild */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetOwnedChild, "Owned Child", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetOwnedSibling */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetOwnedSibling, "Owned Sibling", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetClock */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetClock, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Clock = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetClock */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetClock, "Clock", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: AddTrack */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, AddTrack, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"TimeLine", orxCOMMAND_VAR_TYPE_STRING}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: RemoveTrack */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, RemoveTrack, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"TimeLine", orxCOMMAND_VAR_TYPE_STRING}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: AddFX */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, AddFX, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 2, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"FX", orxCOMMAND_VAR_TYPE_STRING}, {"Unique = false", orxCOMMAND_VAR_TYPE_BOOL}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: RemoveFX */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, RemoveFX, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"FX", orxCOMMAND_VAR_TYPE_STRING}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: RemoveAllFXs */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, RemoveAllFXs, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: AddShader */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, AddShader, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Shader", orxCOMMAND_VAR_TYPE_STRING}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: RemoveShader */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, RemoveShader, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Shader", orxCOMMAND_VAR_TYPE_STRING}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: AddSound */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, AddSound, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Sound", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: RemoveSound */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, RemoveSound, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Sound", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: SetVolume */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetVolume, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Volume", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: SetPitch */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetPitch, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Pitch", orxCOMMAND_VAR_TYPE_FLOAT});

  /* Command: SetAnim */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetAnim, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 3, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Anim = <void>", orxCOMMAND_VAR_TYPE_STRING}, {"Current = false", orxCOMMAND_VAR_TYPE_BOOL}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetAnim */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetAnim, "Anim", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Current = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: SetAnimFrequency */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetAnimFrequency, "Object", orxCOMMAND_VAR_TYPE_U64, 1, 2, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Frequency = 1.0", orxCOMMAND_VAR_TYPE_FLOAT}, {"Recursive = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: GetAnimFrequency */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetAnimFrequency, "Frequency", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetOrigin */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetOrigin, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Origin", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: GetOrigin */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetOrigin, "Origin", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: SetSize */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetSize, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Size", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: GetSize */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetSize, "Size", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
  /* Command: SetPivot */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, SetPivot, "Object", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Object", orxCOMMAND_VAR_TYPE_U64}, {"Pivot", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetPivot */
  orxCOMMAND_REGISTER_CORE_COMMAND(Object, GetPivot, "Pivot", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Object", orxCOMMAND_VAR_TYPE_U64});
}

/** Unregisters all the object commands
 */
static orxINLINE void orxObject_UnregisterCommands()
{
  /* Command: Create */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Create);
  /* Command: Delete */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Delete);

  /* Command: FindNext */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, FindNext);

  /* Command: GetCount */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetCount);

  /* Command: GetID */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetID);

  /* Command: SetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetPosition);
  /* Command: SetRotation */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetRotation);
  /* Command: SetScale */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetScale);
  /* Command: GetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetPosition);
  /* Command: GetRotation */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetRotation);
  /* Command: GetScale */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetScale);

  /* Command: SetSpeed */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetSpeed);
  /* Command: SetAngularVelocity */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetAngularVelocity);
  /* Command: SetCustomGravity */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetCustomGravity);
  /* Command: GetSpeed */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetSpeed);
  /* Command: GetAngularVelocity */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetAngularVelocity);
  /* Command: GetCustomGravity */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetCustomGravity);

  /* Command: GetMass */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetMass);
  /* Command: GetMassCenter */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetMassCenter);

  /* Command: SetText */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetText);
  /* Command: GetText */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetText);

  /* Command: SetRepeat */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetRepeat);
  /* Command: GetRepeat */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetRepeat);

  /* Command: SetGroup */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetGroup);
  /* Command: GetGroup */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetGroup);

  /* Command: GetName */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetName);

  /* Command: SetLifeTime */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetLifeTime);
  /* Command: GetLifeTime */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetLifeTime);

  /* Command: SetColor */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetColor);
  /* Command: GetColor */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetColor);
  /* Command: SetRGB */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetRGB);
  /* Command: GetRGB */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetRGB);
  /* Command: SetHSL */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetHSL);
  /* Command: GetHSL */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetHSL);
  /* Command: SetHSV */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetHSV);
  /* Command: GetHSV */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetHSV);
  /* Command: SetAlpha */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetAlpha);
  /* Command: GetAlpha */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetAlpha);

  /* Command: Enable */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Enable);
  /* Command: IsEnabled */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, IsEnabled);
  /* Command: Pause */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Pause);
  /* Command: IsPaused */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, IsPaused);

  /* Command: Play */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Play);
  /* Command: Stop */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Stop);

  /* Command: SetParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetParent);
  /* Command: GetParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetParent);
  /* Command: GetChild */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetChild);
  /* Command: GetSibling */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetSibling);

  /* Command: Attach */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Attach);
  /* Command: Detach */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, Detach);

  /* Command: LogParents */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, LogParents);

  /* Command: SetOwner */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetOwner);
  /* Command: GetOwner */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetOwner);
  /* Command: GetOwnedChild */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetOwnedChild);
  /* Command: GetOwnedSibling */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetOwnedSibling);

  /* Command: SetClock */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetClock);
  /* Command: GetClock */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetClock);

  /* Command: AddTrack */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, AddTrack);
  /* Command: RemoveTrack */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, RemoveTrack);

  /* Command: AddFX */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, AddFX);
  /* Command: RemoveFX */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, RemoveFX);
  /* Command: RemoveAllFXs */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, RemoveAllFXs);

  /* Command: AddShader */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, AddShader);
  /* Command: RemoveShader */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, RemoveShader);

  /* Command: AddSound */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, AddSound);
  /* Command: RemoveSound */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, RemoveSound);

  /* Command: SetVolume */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetVolume);
  /* Command: SetPitch */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetPitch);

  /* Command: SetAnim */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetAnim);
  /* Command: GetAnim */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetAnim);
  /* Command: SetAnimFrequency */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetAnimFrequency);
  /* Command: GetAnimFrequency */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetAnimFrequency);

  /* Command: SetOrigin */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetOrigin);
  /* Command: GetOrigin */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetOrigin);
  /* Command: SetSize */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetSize);
  /* Command: GetSize */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetSize);
  /* Command: SetPivot */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, SetPivot);
  /* Command: GetPivot */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Object, GetPivot);
}

/** Creates an empty object
 */
static orxINLINE orxOBJECT *orxObject_CreateInternal()
{
  orxOBJECT *pstResult;

  /* Creates object */
  pstResult = orxOBJECT(orxStructure_Create(orxSTRUCTURE_ID_OBJECT));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxOBJECT_KU32_FLAG_ENABLED, orxOBJECT_KU32_MASK_ALL);

    /* Inits active time */
    pstResult->fActiveTime = orxFLOAT_0;

    /* Sets default group ID */
    orxObject_SetGroupID(pstResult, sstObject.stDefaultGroupID);

    /* Adds it to the enable list */
    orxLinkList_AddEnd(&(sstObject.stEnableList), &(pstResult->stEnableNode));

    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create object.");
  }

  /* Done! */
  return pstResult;
}

/** Deletes an object
 */
static orxINLINE orxSTATUS orxObject_DeleteInternal(orxOBJECT *_pstObject)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Decreases count */
  orxStructure_DecreaseCount(_pstObject);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstObject) == 0)
  {
    orxEVENT stEvent;

    /* Inits event */
    orxEVENT_INIT(stEvent, orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_DELETE, _pstObject, orxNULL, orxNULL);

    /* Sends event */
    if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
    {
      orxU32 i;

      /* Unlink all structures */
      for(i = 0; i < orxSTRUCTURE_ID_LINKABLE_NUMBER; i++)
      {
        orxObject_UnlinkStructure(_pstObject, (orxSTRUCTURE_ID)i);
      }

      /* Has children? */
      if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_HAS_CHILDREN))
      {
        orxOBJECT *pstChild;

        /* For all children */
        for(pstChild = _pstObject->pstChild;
            pstChild != orxNULL;
            pstChild = _pstObject->pstChild)
        {
          /* Removes its owner */
          orxObject_SetOwner(pstChild, orxNULL);

          /* Marks it for deletion */
          orxObject_SetLifeTime(pstChild, orxFLOAT_0);
        }
      }

      /* Removes owner */
      orxObject_SetOwner(_pstObject, orxNULL);

      /* Removes object from its current group */
      if(orxLinkList_GetList(&(_pstObject->stGroupNode)) != orxNULL)
      {
        orxLinkList_Remove(&(_pstObject->stGroupNode));
      }

      /* Removes object from the enable lists */
      if(orxLinkList_GetList(&(_pstObject->stEnableNode)) != orxNULL)
      {
        orxLinkList_Remove(&(_pstObject->stEnableNode));
      }
      if(orxLinkList_GetList(&(_pstObject->stEnableGroupNode)) != orxNULL)
      {
        orxLinkList_Remove(&(_pstObject->stEnableGroupNode));
      }

      /* Deletes structure */
      orxStructure_Delete(_pstObject);
    }
    else
    {
      /* Increases count */
      orxStructure_IncreaseCount(_pstObject);

      /* Resets its active time: going undead */
      _pstObject->fActiveTime = orxFLOAT_0;

      /* Disables it */
      orxObject_Enable(_pstObject, orxFALSE);
    }
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxObject_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_OBJECT);
  orxASSERT(_pstEvent->eID == orxOBJECT_EVENT_PREPARE);

  /* Has current object? */
  if(sstObject.pstCurrentObject != orxNULL)
  {
    /* Stores current object as temporary parent in payload */
    *((orxOBJECT **)_pstEvent->pstPayload) = sstObject.pstCurrentObject;

    /* Sets it as owner */
    orxStructure_SetOwner(orxOBJECT(_pstEvent->hSender), sstObject.pstCurrentObject);
  }

  /* Done! */
  return eResult;
}

/** Deletes all the objects
 */
static orxINLINE void orxObject_DeleteAll()
{
  orxOBJECT *pstObject;

  /* Gets first object */
  pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));

  /* Non empty? */
  while(pstObject != orxNULL)
  {
    /* Deletes object */
    orxObject_Delete(pstObject);

    /* Gets first object */
    pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));
  }

  /* Done! */
  return;
}

/** Updates an object
 * @param[int] _pstObject         Concerned object
 * @param[in] _pstClockInfo       Clock information where this callback has been registered
 */
static void orxFASTCALL orxObject_UpdateInternal(orxOBJECT *_pstObject, const orxCLOCK_INFO *_pstClockInfo)
{
  orxU32        u32UpdateFlags;
  orxSTRUCTURE *pstStructure;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxObject_Update");

  /* Gets object's structure */
  pstStructure = (orxSTRUCTURE *)_pstObject;

  /* Gets object's enabled, paused and death row flags */
  u32UpdateFlags = orxFLAG_GET(pstStructure->u32Flags, orxOBJECT_KU32_FLAG_ENABLED | orxOBJECT_KU32_FLAG_PAUSED | orxOBJECT_KU32_FLAG_DEATH_ROW);

  /* Is object enabled and not paused or in death row? */
  if((u32UpdateFlags == orxOBJECT_KU32_FLAG_ENABLED)
  || (u32UpdateFlags & orxOBJECT_KU32_FLAG_DEATH_ROW))
  {
    const orxCLOCK_INFO  *pstClockInfo;
    orxBOOL               bDeleted = orxFALSE;

    /* Has clock? */
    if(orxStructure_TestFlags(_pstObject, 1 << orxSTRUCTURE_ID_CLOCK))
    {
      /* Uses it */
      pstClockInfo = orxClock_GetInfo((orxCLOCK *)_pstObject->astStructureList[orxSTRUCTURE_ID_CLOCK].pstStructure);
    }
    else
    {
      /* Uses default info */
      pstClockInfo = _pstClockInfo;
    }

    /* Updates its active time */
    _pstObject->fActiveTime += pstClockInfo->fDT;

    /* Has life time? */
    if(orxFLAG_TEST(pstStructure->u32Flags, orxOBJECT_KU32_FLAG_HAS_LIFETIME))
    {
      /* Updates its life time */
      _pstObject->fLifeTime -= pstClockInfo->fDT;

      /* Should die? */
      if(_pstObject->fLifeTime <= orxFLOAT_0)
      {
        /* Deletes it */
        orxObject_Delete(_pstObject);

        /* Marks as deleted */
        bDeleted = orxTRUE;
      }
    }

    /* !!! TODO !!! */
    /* Updates culling info before calling update sub-functions */

    /* Wasn't object deleted? */
    if(bDeleted == orxFALSE)
    {
      /* Has DT? */
      if(pstClockInfo->fDT > orxFLOAT_0)
      {
        orxU32 u32LifeTimeFlags, i;

        /* For all linked structures */
        for(i = 0; i < orxSTRUCTURE_ID_LINKABLE_NUMBER; i++)
        {
          /* Is structure linked? */
          if(orxStructure_TestFlags(_pstObject, 1 << i))
          {
            /* Updates it */
            if(orxStructure_Update(_pstObject->astStructureList[i].pstStructure, _pstObject, pstClockInfo) == orxSTATUS_FAILURE)
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to update structure [%s] for object <%s>.", orxStructure_GetIDString((orxSTRUCTURE_ID)i), orxObject_GetName(_pstObject));
            }
          }
        }

        /* Has no body? */
        if(!orxStructure_TestFlags(_pstObject, 1 << orxSTRUCTURE_ID_BODY))
        {
          /* Has frame? */
          if(orxStructure_TestFlags(_pstObject, 1 << orxSTRUCTURE_ID_FRAME))
          {
            orxFRAME *pstFrame;

            /* Gets it */
            pstFrame = (orxFRAME *)_pstObject->astStructureList[orxSTRUCTURE_ID_FRAME].pstStructure;

            /* Has speed? */
            if(orxVector_IsNull(&(_pstObject->vSpeed)) == orxFALSE)
            {
              orxVECTOR vPosition, vMove;

              /* Gets its position */
              orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_LOCAL, &vPosition);

              /* Computes its move */
              orxVector_Mulf(&vMove, &(_pstObject->vSpeed), pstClockInfo->fDT);

              /* Gets its new position */
              orxVector_Add(&vPosition, &vPosition, &vMove);

              /* Stores it */
              orxFrame_SetPosition(pstFrame, orxFRAME_SPACE_LOCAL, &vPosition);
            }

            /* Has angular velocity? */
            if(_pstObject->fAngularVelocity != orxFLOAT_0)
            {
              /* Updates its rotation */
              orxFrame_SetRotation(pstFrame, orxFRAME_SPACE_LOCAL, orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_LOCAL) + (_pstObject->fAngularVelocity * pstClockInfo->fDT));
            }
          }
        }
        else
        {
          /* Should detach? */
          if(orxFLAG_TEST(pstStructure->u32Flags, orxOBJECT_KU32_FLAG_DETACH_JOINT_CHILD))
          {
            /* Detaches it */
            orxObject_Detach(_pstObject);

            /* Updates status */
            orxFLAG_SET(pstStructure->u32Flags, orxOBJECT_KU32_FLAG_NONE, orxOBJECT_KU32_FLAG_DETACH_JOINT_CHILD);
          }
        }

        /* Has structure-bound lifetime? */
        if((u32LifeTimeFlags = orxStructure_GetFlags(_pstObject, orxOBJECT_KU32_MASK_STRUCTURE_LIFETIME)) != 0)
        {
          /* Not checking FX or no FX left? */
          if((!orxFLAG_TEST(u32LifeTimeFlags, orxOBJECT_KU32_FLAG_FX_LIFETIME))
          || ((_pstObject->astStructureList[orxSTRUCTURE_ID_FXPOINTER].pstStructure != orxNULL)
           && (orxFXPointer_GetCount(orxFXPOINTER(_pstObject->astStructureList[orxSTRUCTURE_ID_FXPOINTER].pstStructure)) == 0)))
          {
            /* Not checking sound or no sound left? */
            if((!orxFLAG_TEST(u32LifeTimeFlags, orxOBJECT_KU32_FLAG_SOUND_LIFETIME))
            || ((_pstObject->astStructureList[orxSTRUCTURE_ID_SOUNDPOINTER].pstStructure != orxNULL)
             && (orxSoundPointer_GetCount(orxSOUNDPOINTER(_pstObject->astStructureList[orxSTRUCTURE_ID_SOUNDPOINTER].pstStructure)) == 0)))
            {
              orxSPAWNER *pstSpawner;

              /* Not checking spawner or no object left to spawn? */
              if((!orxFLAG_TEST(u32LifeTimeFlags, orxOBJECT_KU32_FLAG_SPAWNER_LIFETIME))
              || (((pstSpawner = orxSPAWNER(_pstObject->astStructureList[orxSTRUCTURE_ID_SPAWNER].pstStructure)) != orxNULL)
               && (orxSpawner_GetTotalObjectCount(pstSpawner) == orxSpawner_GetTotalObjectLimit(pstSpawner))))
              {
                /* Not checking timeline or no track left? */
                if((!orxFLAG_TEST(u32LifeTimeFlags, orxOBJECT_KU32_FLAG_TIMELINE_LIFETIME))
                || ((_pstObject->astStructureList[orxSTRUCTURE_ID_TIMELINE].pstStructure != orxNULL)
                 && (orxTimeLine_GetCount(orxTIMELINE(_pstObject->astStructureList[orxSTRUCTURE_ID_TIMELINE].pstStructure)) == 0)))
                {
                  /* Not checking children or no child left? */
                  if((!orxFLAG_TEST(u32LifeTimeFlags, orxOBJECT_KU32_FLAG_CHILDREN_LIFETIME))
                  || (!orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_HAS_CHILDREN)))
                  {
                    /* Not checking anim or no current anim? */
                    if((!orxFLAG_TEST(u32LifeTimeFlags, orxOBJECT_KU32_FLAG_ANIM_LIFETIME))
                    || ((_pstObject->astStructureList[orxSTRUCTURE_ID_ANIMPOINTER].pstStructure != orxNULL)
                     && (orxAnimPointer_GetCurrentAnim(orxANIMPOINTER(_pstObject->astStructureList[orxSTRUCTURE_ID_ANIMPOINTER].pstStructure)) == orxU32_UNDEFINED)))
                    {
                      /* Schedules object's deletion */
                      orxObject_SetLifeTime(_pstObject, orxFLOAT_0);

                      /* Disables it */
                      orxObject_Enable(_pstObject, orxFALSE);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Updates all the objects
 * @param[in] _pstClockInfo       Clock information where this callback has been registered
 * @param[in] _pContext           User defined context
 */
static void orxFASTCALL orxObject_UpdateAll(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxLINKLIST_NODE *pstNode, *pstNextNode;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxObject_UpdateAll");

  /* For all enabled nodes */
  for(pstNode = orxLinkList_GetFirst(&(sstObject.stEnableList));
      pstNode != orxNULL;
      pstNode = pstNextNode)
  {
    orxOBJECT *pstObject;

    /* Gets next node */
    pstNextNode = orxLinkList_GetNext(pstNode);

    /* Gets associated object */
    pstObject = orxSTRUCT_GET_FROM_FIELD(orxOBJECT, stEnableNode, pstNode);

    /* Updates it */
    orxObject_UpdateInternal(pstObject, _pstClockInfo);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Object module setup
 */
void orxFASTCALL orxObject_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_FRAME);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_STRING);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_ANIMPOINTER);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_BODY);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_FXPOINTER);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_GRAPHIC);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_SHADERPOINTER);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_SOUNDPOINTER);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_SPAWNER);
  orxModule_AddOptionalDependency(orxMODULE_ID_OBJECT, orxMODULE_ID_TIMELINE);

  /* Done! */
  return;
}

/** Inits the object module.
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxSTRUCTURE_ID_LINKABLE_NUMBER <= orxMath_GetBitCount(orxOBJECT_KU32_MASK_LINKED_STRUCTURE));

  /* Not already Initialized? */
  if(!(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstObject, sizeof(orxOBJECT_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(OBJECT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxOBJECT_KU32_BANK_SIZE, orxNULL);

    /* Initialized? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Creates objects clock */
      sstObject.pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

      /* Valid? */
      if(sstObject.pstClock != orxNULL)
      {
        /* Adds event handler */
        eResult = orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxObject_EventHandler);

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Filters relevant event IDs */
          orxEvent_SetHandlerIDFlags(orxObject_EventHandler, orxEVENT_TYPE_OBJECT, orxNULL, orxEVENT_GET_FLAG(orxOBJECT_EVENT_PREPARE), orxEVENT_KU32_MASK_ID_ALL);

          /* Registers object update function to clock */
          eResult = orxClock_Register(sstObject.pstClock, orxObject_UpdateAll, orxNULL, orxMODULE_ID_OBJECT, orxCLOCK_PRIORITY_LOW);

          /* Success? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Creates banks */
            sstObject.pstGroupBank  = orxBank_Create(orxOBJECT_KU32_GROUP_BANK_SIZE, sizeof(orxOBJECT_LISTS), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
            sstObject.pstAgeBank    = orxBank_Create(orxOBJECT_KU32_AGE_BANK_SIZE, sizeof(orxOBJECT *), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

            /* Success? */
            if((sstObject.pstGroupBank != orxNULL) && (sstObject.pstAgeBank != orxNULL))
            {
              /* Creates group table */
              sstObject.pstGroupTable = orxHashTable_Create(orxOBJECT_KU32_GROUP_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

              /* Success? */
              if(sstObject.pstGroupTable != orxNULL)
              {
                /* Registers commands */
                orxObject_RegisterCommands();

                /* Stores default group ID */
                sstObject.stDefaultGroupID  = orxString_GetID(orxOBJECT_KZ_DEFAULT_GROUP);
                sstObject.stCurrentGroupID  = sstObject.stDefaultGroupID;

                /* Inits Flags */
                sstObject.u32Flags = orxOBJECT_KU32_STATIC_FLAG_READY | orxOBJECT_KU32_STATIC_FLAG_CLOCK;
              }
              else
              {
                /* Updates result */
                eResult = orxSTATUS_FAILURE;

                /* Removes event handler */
                orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxObject_EventHandler);

                /* Deletes banks */
                orxBank_Delete(sstObject.pstGroupBank);
                orxBank_Delete(sstObject.pstAgeBank);

                /* Unregisters from clock */
                orxClock_Unregister(sstObject.pstClock, orxObject_UpdateAll);

                /* Unregisters structure type */
                orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);
              }
            }
            else
            {
              /* Updates result */
              eResult = orxSTATUS_FAILURE;

              /* Deletes banks */
              if(sstObject.pstGroupBank != orxNULL)
              {
                orxBank_Delete(sstObject.pstGroupBank);
              }
              if(sstObject.pstAgeBank != orxNULL)
              {
                orxBank_Delete(sstObject.pstAgeBank);
              }

              /* Unregisters from clock */
              orxClock_Unregister(sstObject.pstClock, orxObject_UpdateAll);

              /* Removes event handler */
              orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxObject_EventHandler);

              /* Unregisters structure type */
              orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);
            }
          }
          else
          {
            /* Removes event handler */
            orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxObject_EventHandler);

            /* Unregisters structure type */
            orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);
          }
        }
        else
        {
          /* Unregisters structure type */
          orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);
        }
      }
      else
      {
        /* Unregisters structure type */
        orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to register link list structure.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize object module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the object module.
 */
void orxFASTCALL orxObject_Exit()
{
  /* Initialized? */
  if(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxObject_EventHandler);

    /* Unregisters commands */
    orxObject_UnregisterCommands();

    /* Deletes object list */
    orxObject_DeleteAll();

    /* Has clock? */
    if(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_CLOCK)
    {
      /* Unregisters object update all function */
      orxClock_Unregister(sstObject.pstClock, orxObject_UpdateAll);

      /* Removes reference */
      sstObject.pstClock = orxNULL;

      /* Updates flags */
      sstObject.u32Flags &= ~orxOBJECT_KU32_STATIC_FLAG_CLOCK;
    }

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_OBJECT);

    /* Deletes group table */
    orxHashTable_Delete(sstObject.pstGroupTable);

    /* Deletes banks */
    orxBank_Delete(sstObject.pstGroupBank);
    orxBank_Delete(sstObject.pstAgeBank);

    /* Updates flags */
    sstObject.u32Flags &= ~orxOBJECT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit from object module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Creates an empty object.
 * @return      Created orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_Create()
{
  orxOBJECT *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);

  /* Creates object */
  pstResult = orxObject_CreateInternal();

  /* Success? */
  if(pstResult != orxNULL)
  {
    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_CREATE, pstResult, orxNULL, orxNULL);
  }

  /* Done! */
  return pstResult;
}

/** Deletes an object, *unsafe* when called from an event handler: call orxObject_SetLifeTime(orxFLOAT_0) instead.
 * @param[in] _pstObject        Concerned object
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_Delete(orxOBJECT *_pstObject)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((orxEvent_IsSending() == orxFALSE) && "Calling orxObject_Delete() from inside an event handler is *NOT* safe: please consider calling orxObject_SetLifeTime(orxFLOAT_0) instead.");

  /* Deletes it */
  eResult = orxObject_DeleteInternal(_pstObject);

  /* Done! */
  return eResult;
}

/** Updates an object.
 * @param[in] _pstObject        Concerned object
 * @param[in] _pstClockInfo     Clock information used to compute new object's state
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_Update(orxOBJECT *_pstObject, const orxCLOCK_INFO *_pstClockInfo)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstObject != orxNULL);
  orxASSERT(_pstClockInfo != orxNULL);

  /* Updates object */
  orxObject_UpdateInternal(_pstObject, _pstClockInfo);

  /* Done! */
  return eResult;
}

/** Creates an object from config.
 * @param[in]   _zConfigID            Config ID
 * @ return orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxOBJECT *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zConfigID != orxNULL);

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates object */
    pstResult = orxObject_CreateInternal();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxEVENT      stEvent;
      orxSTRUCTURE *pstParent = orxNULL;

      /* Inits event */
      orxEVENT_INIT(stEvent, orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_PREPARE, pstResult, orxNULL, &pstParent);

      /* Sends event */
      if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
      {
        orxVECTOR       vValue, vParentSize, vColor;
        const orxSTRING zGraphicFileName;
        const orxSTRING zAnimPointerName;
        const orxSTRING zAutoScrolling;
        const orxSTRING zFlipping;
        const orxSTRING zBodyName;
        const orxSTRING zClockName;
        const orxSTRING zSpawnerName;
        const orxSTRING zParentName;
        const orxSTRING zIgnoreFromParent;
        orxFRAME       *pstFrame;
        orxBODY        *pstBody;
        orxOBJECT      *pstPreviousObject;
        orxFLOAT        fAge;
        orxU32          u32FrameFlags, u32Flags = orxOBJECT_KU32_FLAG_NONE;
        orxS32          s32Number;
        orxCOLOR        stColor;
        orxBOOL         bUseParentScale = orxFALSE, bUseParentPosition = orxFALSE, bHasColor = orxFALSE, bUseParentSpace = orxFALSE, bHasPosition = orxFALSE;

        /* Backups current spawner */
        pstPreviousObject = sstObject.pstCurrentObject;

        /* Clears current object */
        sstObject.pstCurrentObject = orxNULL;

        /* Gets age */
        fAge = orxConfig_GetFloat(orxOBJECT_KZ_CONFIG_AGE);

        /* Valid? */
        if(fAge > orxFLOAT_0)
        {
          /* No age already defined? */
          if(!orxFLAG_TEST(sstObject.u32Flags, orxOBJECT_KU32_STATIC_FLAG_AGE))
          {
            /* Updates status */
            orxFLAG_SET(sstObject.u32Flags, orxOBJECT_KU32_STATIC_FLAG_AGE, orxOBJECT_KU32_STATIC_FLAG_NONE);
          }
          else
          {
            /* Clears age */
            fAge = orxFLOAT_0;

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "[%s]: Ignoring age as one of its owner is already aging.", _zConfigID);
          }
        }

        /* Has group? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_GROUP) != orxFALSE)
        {
          /* Sets it */
          orxObject_SetGroupID(pstResult, orxString_GetID(orxConfig_GetString(orxOBJECT_KZ_CONFIG_GROUP)));
        }
        /* Has current group ID? */
        else if(sstObject.stCurrentGroupID != sstObject.stDefaultGroupID)
        {
          /* Sets it */
          orxObject_SetGroupID(pstResult, sstObject.stCurrentGroupID);
        }

        /* Stores reference */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* *** Frame *** */

        /* Gets auto scrolling value */
        zAutoScrolling = orxConfig_GetString(orxOBJECT_KZ_CONFIG_AUTO_SCROLL);

        /* X auto scrolling? */
        if(orxString_ICompare(zAutoScrolling, orxOBJECT_KZ_X) == 0)
        {
          /* Updates frame flags */
          u32FrameFlags = orxFRAME_KU32_FLAG_SCROLL_X;
        }
        /* Y auto scrolling? */
        else if(orxString_ICompare(zAutoScrolling, orxOBJECT_KZ_Y) == 0)
        {
          /* Updates frame flags */
          u32FrameFlags = orxFRAME_KU32_FLAG_SCROLL_Y;
        }
        /* Both auto scrolling? */
        else if(orxString_ICompare(zAutoScrolling, orxOBJECT_KZ_BOTH) == 0)
        {
          /* Updates frame flags */
          u32FrameFlags = orxFRAME_KU32_FLAG_SCROLL_X | orxFRAME_KU32_FLAG_SCROLL_Y;
        }
        else
        {
          /* Updates frame flags */
          u32FrameFlags = orxFRAME_KU32_FLAG_NONE;
        }

        /* Gets flipping value */
        zFlipping = orxConfig_GetString(orxOBJECT_KZ_CONFIG_FLIP);

        /* X flipping? */
        if(orxString_ICompare(zFlipping, orxOBJECT_KZ_X) == 0)
        {
          /* Updates frame flags */
          u32FrameFlags  |= orxFRAME_KU32_FLAG_FLIP_X;
        }
        /* Y flipping? */
        else if(orxString_ICompare(zFlipping, orxOBJECT_KZ_Y) == 0)
        {
          /* Updates frame flags */
          u32FrameFlags  |= orxFRAME_KU32_FLAG_FLIP_Y;
        }
        /* Both flipping? */
        else if(orxString_ICompare(zFlipping, orxOBJECT_KZ_BOTH) == 0)
        {
          /* Updates frame flags */
          u32FrameFlags  |= orxFRAME_KU32_FLAG_FLIP_X | orxFRAME_KU32_FLAG_FLIP_Y;
        }

        /* Depth scaling active? */
        if(orxConfig_GetBool(orxOBJECT_KZ_CONFIG_DEPTH_SCALE) != orxFALSE)
        {
          /* Updates frame flags */
          u32FrameFlags  |= orxFRAME_KU32_FLAG_DEPTH_SCALE;
        }

        /* Ignore from parent? */
        if((zIgnoreFromParent = orxConfig_GetString(orxOBJECT_KZ_CONFIG_IGNORE_FROM_PARENT)) != orxSTRING_EMPTY)
        {
          /* Updates flags */
          u32FrameFlags |= orxFrame_GetIgnoreFlagValues(zIgnoreFromParent);
        }

        /* Creates frame */
        pstFrame = orxFrame_Create(u32FrameFlags);

        /* Valid? */
        if(pstFrame != orxNULL)
        {
          /* Links it */
          if(orxObject_LinkStructure(pstResult, orxSTRUCTURE(pstFrame)) != orxSTATUS_FAILURE)
          {
            /* Updates flags */
            orxFLAG_SET(pstResult->astStructureList[orxSTRUCTURE_ID_FRAME].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

            /* Updates its owner */
            orxStructure_SetOwner(pstFrame, pstResult);
          }
          else
          {
            /* Deletes it */
            orxFrame_Delete(pstFrame);
            pstFrame = orxNULL;
          }
        }

        /* *** Parent *** */

        /* Gets camera file name */
        zParentName = orxConfig_GetString(orxOBJECT_KZ_CONFIG_PARENT_CAMERA);

        /* Valid? */
        if((zParentName != orxNULL) && (zParentName != orxSTRING_EMPTY))
        {
          orxCAMERA *pstCamera;

          /* Gets camera */
          pstCamera = orxCamera_CreateFromConfig(zParentName);

          /* Valid? */
          if(pstCamera != orxNULL)
          {
            orxAABOX stFrustum;

            /* No owner? */
            if(orxStructure_GetOwner(pstCamera) == orxNULL)
            {
              /* Sets it as its own owner */
              orxStructure_SetOwner(pstCamera, pstCamera);
            }

            /* Sets it as parent */
            orxObject_SetParent(pstResult, pstCamera);

            /* Updates default status */
            bUseParentSpace = bUseParentScale = bUseParentPosition = orxTRUE;

            /* Gets camera frustum */
            orxCamera_GetFrustum(pstCamera, &stFrustum);

            /* Gets parent size */
            orxVector_Sub(&vParentSize, &(stFrustum.vBR), &(stFrustum.vTL));
          }
        }
        else
        {
          /* Has temporary parent? */
          if(pstParent != orxNULL)
          {
            /* Updates parent space status */
            bUseParentSpace = orxConfig_HasValue(orxOBJECT_KZ_CONFIG_USE_PARENT_SPACE);

            /* Has parent space? */
            if(bUseParentSpace != orxFALSE)
            {
              /* Depending on parent type */
              switch(orxStructure_GetID(pstParent))
              {
                /* Spawner? */
                case orxSTRUCTURE_ID_SPAWNER:
                {
                  /* Gets its owner */
                  pstParent = orxStructure_GetOwner(pstParent);

                  /* Valid? */
                  if((pstParent != orxNULL) && (orxStructure_GetID(pstParent) == orxSTRUCTURE_ID_OBJECT))
                  {
                    /* Fall through */
                  }
                  else
                  {
                    /* Updates status */
                    bUseParentSpace = orxFALSE;
                    break;
                  }
                }

                /* Object? */
                case orxSTRUCTURE_ID_OBJECT:
                {
                  /* Gets its size & name */
                  orxObject_GetSize(orxOBJECT(pstParent), &vParentSize);
                  zParentName = orxObject_GetName(orxOBJECT(pstParent));
                  break;
                }

                default:
                {
                  /* Updates status */
                  bUseParentSpace = orxFALSE;
                  break;
                }
              }
            }
          }
        }

        /* Has parent space? */
        if(bUseParentSpace != orxFALSE)
        {
          const orxSTRING zUseParentSpace;

          /* Gets its literal version */
          zUseParentSpace = orxConfig_GetString(orxOBJECT_KZ_CONFIG_USE_PARENT_SPACE);

          /* Defined? */
          if((zUseParentSpace != orxNULL) && (zUseParentSpace != orxSTRING_EMPTY))
          {
            /* Scale only? */
            if(orxString_ICompare(zUseParentSpace, orxOBJECT_KZ_SCALE) == 0)
            {
              /* Updates status */
              bUseParentScale     = orxTRUE;
              bUseParentPosition  = orxFALSE;
            }
            /* Position only? */
            else if(orxString_ICompare(zUseParentSpace, orxOBJECT_KZ_POSITION) == 0)
            {
              /* Updates status */
              bUseParentScale     = orxFALSE;
              bUseParentPosition  = orxTRUE;
            }
            /* Both? */
            else if(orxString_ICompare(zUseParentSpace, orxOBJECT_KZ_BOTH) == 0)
            {
              /* Updates status */
              bUseParentScale     = orxTRUE;
              bUseParentPosition  = orxTRUE;
            }
            else
            {
              /* Gets boolean value */
              if(orxString_ToBool(zUseParentSpace, &bUseParentSpace, orxNULL) != orxSTATUS_FAILURE)
              {
                /* Updates status */
                bUseParentScale     = bUseParentSpace;
                bUseParentPosition  = bUseParentSpace;
              }
            }
          }
        }

        /* *** Graphic *** */

        /* Gets graphic file name */
        zGraphicFileName = orxConfig_GetString(orxOBJECT_KZ_CONFIG_GRAPHIC_NAME);

        /* Valid? */
        if((zGraphicFileName != orxNULL) && (zGraphicFileName != orxSTRING_EMPTY))
        {
          orxGRAPHIC *pstGraphic;

          /* Creates graphic */
          pstGraphic = orxGraphic_CreateFromConfig(zGraphicFileName);

          /* Valid? */
          if(pstGraphic != orxNULL)
          {
            /* Links it structures */
            if(orxObject_LinkStructure(pstResult, orxSTRUCTURE(pstGraphic)) != orxSTATUS_FAILURE)
            {
              /* Updates flags */
              orxFLAG_SET(pstResult->astStructureList[orxSTRUCTURE_ID_GRAPHIC].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

              /* Updates its owner */
              orxStructure_SetOwner(pstGraphic, pstResult);
            }
            else
            {
              /* Deletes it */
              orxGraphic_Delete(pstGraphic);
              pstGraphic = orxNULL;
            }
          }
        }

        /* *** Animation *** */

        /* Gets animation set name */
        zAnimPointerName = orxConfig_GetString(orxOBJECT_KZ_CONFIG_ANIMPOINTER_NAME);

        /* Valid? */
        if((zAnimPointerName != orxNULL) && (zAnimPointerName != orxSTRING_EMPTY))
        {
          orxANIMPOINTER *pstAnimPointer;

          /* Creates animation pointer from it */
          pstAnimPointer = orxAnimPointer_CreateFromConfig(zAnimPointerName);

          /* Valid? */
          if(pstAnimPointer != orxNULL)
          {
            /* Links it structures */
            if(orxObject_LinkStructure(pstResult, orxSTRUCTURE(pstAnimPointer)) != orxSTATUS_FAILURE)
            {
              /* Updates flags */
              orxFLAG_SET(pstResult->astStructureList[orxSTRUCTURE_ID_ANIMPOINTER].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

              /* Updates its owner */
              orxStructure_SetOwner(pstAnimPointer, pstResult);

              /* Has frequency? */
              if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_FREQUENCY) != orxFALSE)
              {
                /* Updates animation pointer frequency */
                orxObject_SetAnimFrequency(pstResult, orxConfig_GetFloat(orxOBJECT_KZ_CONFIG_FREQUENCY));
              }

              /* Doesn't have a graphic? */
              if(pstResult->astStructureList[orxSTRUCTURE_ID_GRAPHIC].pstStructure == orxNULL)
              {
                orxGRAPHIC *pstGraphic;

                /* Gets current graphic */
                pstGraphic = orxObject_GetWorkingGraphic(pstResult);

                /* Valid? */
                if(pstGraphic != orxNULL)
                {
                  /* Creates a clone */
                  pstGraphic = orxGraphic_Clone(pstGraphic);

                  /* Valid? */
                  if(pstGraphic != orxNULL)
                  {
                    /* Links it structures */
                    if(orxObject_LinkStructure(pstResult, orxSTRUCTURE(pstGraphic)) != orxSTATUS_FAILURE)
                    {
                      /* Updates flags */
                      orxFLAG_SET(pstResult->astStructureList[orxSTRUCTURE_ID_GRAPHIC].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

                      /* Updates its owner */
                      orxStructure_SetOwner(pstGraphic, pstResult);
                    }
                    else
                    {
                      /* Deletes it */
                      orxGraphic_Delete(pstGraphic);
                    }
                  }
                }
              }
            }
            else
            {
              /* Deletes it */
              orxAnimPointer_Delete(pstAnimPointer);
              pstAnimPointer = orxNULL;
            }
          }
        }

        /* *** Pivot/Size *** */

        /* Has size? */
        if(orxConfig_GetVector(orxOBJECT_KZ_CONFIG_SIZE, &vValue) != orxNULL)
        {
          /* Updates object size */
          orxObject_SetSize(pstResult, &vValue);
        }

        /* Has pivot? */
        if(orxConfig_GetVector(orxOBJECT_KZ_CONFIG_PIVOT, &vValue) != orxNULL)
        {
          /* Updates object pivot */
          orxObject_SetPivot(pstResult, &vValue);
        }
        /* Has relative pivot point? */
        else if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_PIVOT) != orxFALSE)
        {
          /* Sets relative pivot */
          orxObject_SetRelativePivot(pstResult, orxConfig_GetString(orxOBJECT_KZ_CONFIG_PIVOT));
        }

        /* *** Scale *** */

        /* Has scale? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_SCALE) != orxFALSE)
        {
          /* Is config scale not a vector? */
          if(orxConfig_GetVector(orxOBJECT_KZ_CONFIG_SCALE, &vValue) == orxNULL)
          {
            orxFLOAT fScale;

            /* Gets config uniformed scale */
            fScale = orxConfig_GetFloat(orxOBJECT_KZ_CONFIG_SCALE);

            /* Updates vector */
            orxVector_SetAll(&vValue, fScale);
          }

          /* Uses parent's scale? */
          if(bUseParentScale != orxFALSE)
          {
            orxVECTOR vSize;

            /* Gets object's size */
            orxObject_GetSize(pstResult, &vSize);

            /* Invalid? */
            if((vSize.fX <= orxFLOAT_0) || (vSize.fY <= orxFLOAT_0))
            {
              /* Uses default size */
              orxVector_SetAll(&vSize, orxFLOAT_1);

              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Warning, object <%s> can't use relative scale from parent <%s> as it doesn't have any size. Assuming size (1, 1, 1).", _zConfigID, zParentName);
            }
            else if((vParentSize.fX <= orxFLOAT_0) || (vParentSize.fY <= orxFLOAT_0))
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Warning, object <%s> won't have a size as its parent <%s> doesn't have any size.", _zConfigID, zParentName);
            }

            /* Gets world space values */
            orxVector_Mul(&vValue, orxVector_Div(&vValue, &vValue, &vSize), &vParentSize);
          }

          /* Updates object scale */
          orxObject_SetScale(pstResult, &vValue);
        }

        /* *** Color *** */

        /* Inits color */
        orxColor_Set(&stColor, &orxVECTOR_WHITE, orxFLOAT_1);

        /* Has color? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_COLOR) != orxFALSE)
        {
          /* Is a vector value? */
          if(orxConfig_GetVector(orxOBJECT_KZ_CONFIG_COLOR, &vColor) != orxNULL)
          {
            /* Normalizes it */
            orxVector_Mulf(&(stColor.vRGB), &vColor, orxCOLOR_NORMALIZER);

            /* Updates status */
            bHasColor = orxTRUE;
          }
          /* Color literal */
          else
          {
            const orxSTRING zColor;

            /* Gets color name */
            zColor = orxConfig_GetString(orxOBJECT_KZ_CONFIG_COLOR);

            /* Pushes color section */
            orxConfig_PushSection(orxCOLOR_KZ_CONFIG_SECTION);

            /* Retrieves its value */
            if(orxConfig_GetVector(zColor, &vColor) != orxNULL)
            {
              /* Normalizes it */
              orxVector_Mulf(&(stColor.vRGB), &vColor, orxCOLOR_NORMALIZER);

              /* Updates status */
              bHasColor = orxTRUE;
            }

            /* Pops config section */
            orxConfig_PopSection();
          }
        }
        /* Has RGB values? */
        else if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_RGB) != orxFALSE)
        {
          /* Gets its value */
          orxConfig_GetVector(orxOBJECT_KZ_CONFIG_RGB, &(stColor.vRGB));

          /* Updates status */
          bHasColor = orxTRUE;
        }
        /* Has HSL values? */
        else if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_HSL) != orxFALSE)
        {
          /* Gets its value */
          orxConfig_GetVector(orxOBJECT_KZ_CONFIG_HSL, &(stColor.vHSL));

          /* Stores its RGB equivalent */
          orxColor_FromHSLToRGB(&stColor, &stColor);

          /* Updates status */
          bHasColor = orxTRUE;
        }
        /* Has HSV values? */
        else if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_HSV) != orxFALSE)
        {
          /* Gets its value */
          orxConfig_GetVector(orxOBJECT_KZ_CONFIG_HSV, &(stColor.vHSV));

          /* Stores its RGB equivalent */
          orxColor_FromHSVToRGB(&stColor, &stColor);

          /* Updates status */
          bHasColor = orxTRUE;
        }

        /* Has alpha? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_ALPHA) != orxFALSE)
        {
          /* Doesn't have any color? */
          if(bHasColor == orxFALSE)
          {
            orxGRAPHIC* pstGraphic;

            /* Gets current graphic */
            pstGraphic = orxObject_GetWorkingGraphic(pstResult);

            /* Valid? */
            if(pstGraphic != orxNULL)
            {
              /* Has color? */
              if(orxGraphic_HasColor(pstGraphic) != orxFALSE)
              {
                /* Retrieves it */
                orxGraphic_GetColor(pstGraphic, &stColor);
              }
            }
          }

          /* Applies it */
          orxColor_SetAlpha(&stColor, orxConfig_GetFloat(orxOBJECT_KZ_CONFIG_ALPHA));

          /* Updates color */
          orxObject_SetColor(pstResult, &stColor);
        }
        /* Should apply color? */
        else if(bHasColor != orxFALSE)
        {
          /* Updates color */
          orxObject_SetColor(pstResult, &stColor);
        }

        /* *** Body *** */

        /* Gets body name */
        zBodyName = orxConfig_GetString(orxOBJECT_KZ_CONFIG_BODY);

        /* Valid? */
        if((zBodyName != orxNULL) && (zBodyName != orxSTRING_EMPTY))
        {
          /* Creates body */
          pstBody = orxBody_CreateFromConfig(orxSTRUCTURE(pstResult), zBodyName);

          /* Valid? */
          if(pstBody != orxNULL)
          {
            /* Links it */
            if(orxObject_LinkStructure(pstResult, orxSTRUCTURE(pstBody)) != orxSTATUS_FAILURE)
            {
              /* Updates flags */
              orxFLAG_SET(pstResult->astStructureList[orxSTRUCTURE_ID_BODY].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

              /* Updates its owner */
              orxStructure_SetOwner(pstBody, pstResult);

              /* Using depth scale xor auto scroll? */
              if(orxFLAG_TEST(u32FrameFlags, orxFRAME_KU32_FLAG_DEPTH_SCALE | orxFRAME_KU32_MASK_SCROLL_BOTH)
              && !orxFLAG_TEST_ALL(u32FrameFlags, orxFRAME_KU32_FLAG_DEPTH_SCALE | orxFRAME_KU32_MASK_SCROLL_BOTH))
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Warning, object <%s> is using physics along with either DepthScale or AutoScroll properties. Either all properties or none should be used on this object otherwise this will result in incorrect object rendering.", _zConfigID);
              }
            }
            else
            {
              /* Deletes it */
              orxBody_Delete(pstBody);
              pstBody = orxNULL;
            }
          }
        }
        else
        {
          /* Clears body */
          pstBody = orxNULL;
        }

        /* *** Clock *** */

        /* Gets clock name */
        zClockName = orxConfig_GetString(orxOBJECT_KZ_CONFIG_CLOCK);

        /* Valid? */
        if((zClockName != orxNULL) && (zClockName != orxSTRING_EMPTY))
        {
          orxCLOCK *pstClock;

          /* Creates clock */
          pstClock = orxClock_CreateFromConfig(zClockName);

          /* Valid? */
          if(pstClock != orxNULL)
          {
            /* Links it */
            if(orxObject_LinkStructure(pstResult, orxSTRUCTURE(pstClock)) != orxSTATUS_FAILURE)
            {
              /* Updates flags */
              orxFLAG_SET(pstResult->astStructureList[orxSTRUCTURE_ID_CLOCK].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

              /* Updates its owner */
              orxStructure_SetOwner(pstClock, pstResult);
            }
            else
            {
              /* Deletes it */
              orxClock_Delete(pstClock);
              pstClock = orxNULL;
            }
          }
        }

        /* *** Position & rotation */

        /* Has cartesian position? */
        if(orxConfig_GetVector(orxOBJECT_KZ_CONFIG_POSITION, &vValue) != orxNULL)
        {
          /* Updates status */
          bHasPosition = orxTRUE;
        }
        /* Has spherical position? */
        else if(orxConfig_GetVector(orxOBJECT_KZ_CONFIG_SPHERICAL_POSITION, &vValue) != orxNULL)
        {
          /* Transforms it */
          vValue.fTheta *= orxMATH_KF_DEG_TO_RAD;
          vValue.fPhi   *= orxMATH_KF_DEG_TO_RAD;
          orxVector_FromSphericalToCartesian(&vValue, &vValue);

          /* Updates status */
          bHasPosition = orxTRUE;
        }

        /* Has position? */
        if(bHasPosition != orxFALSE)
        {
          /* Uses parent's position? */
          if(bUseParentPosition != orxFALSE)
          {
            /* Gets world space values */
            orxVector_Mul(&vValue, &vValue, &vParentSize);
          }

          /* Updates object position */
          orxObject_SetPosition(pstResult, &vValue);
        }

        /* Updates object rotation */
        orxObject_SetRotation(pstResult, orxMATH_KF_DEG_TO_RAD * orxConfig_GetFloat(orxOBJECT_KZ_CONFIG_ROTATION));

        /* *** Children *** */

        /* Has child list? */
        if((s32Number = orxConfig_GetListCount(orxOBJECT_KZ_CONFIG_CHILD_LIST)) > 0)
        {
          orxS32      i, s32JointNumber;
          orxOBJECT  *pstLastChild;

          /* Stores current object */
          sstObject.pstCurrentObject = pstResult;

          /* Gets child joint list number */
          s32JointNumber = orxConfig_GetListCount(orxOBJECT_KZ_CONFIG_CHILD_JOINT_LIST);

          /* For all defined objects */
          for(i = 0, pstLastChild = orxNULL; i < s32Number; i++)
          {
            const orxSTRING zChild;

            /* Gets its name */
            zChild = orxConfig_GetListString(orxOBJECT_KZ_CONFIG_CHILD_LIST, i);

            /* Valid? */
            if(zChild != orxSTRING_EMPTY)
            {
              orxOBJECT *pstChild;

              /* Stores current group ID */
              sstObject.stCurrentGroupID = pstResult->stGroupID;

              /* Creates it */
              pstChild = orxObject_CreateFromConfig(zChild);

              /* Clears current group ID */
              sstObject.stCurrentGroupID = sstObject.stDefaultGroupID;

              /* Valid? */
              if(pstChild != orxNULL)
              {
                /* Has last child? */
                if(pstLastChild != orxNULL)
                {
                  /* Sets its sibling */
                  pstLastChild->pstSibling = pstChild;
                }
                else
                {
                  /* Sets first child */
                  pstResult->pstChild = pstChild;
                }

                /* Stores last child */
                pstLastChild = pstChild;

                /* Doesn't already have a parent? */
                if(orxFrame_IsRootChild(orxOBJECT_GET_STRUCTURE(pstChild, FRAME)) != orxFALSE)
                {
                  orxBODY *pstChildBody;

                  /* Gets its body */
                  pstChildBody = orxOBJECT_GET_STRUCTURE(pstChild, BODY);

                  /* Valid joint can be added? */
                  if((pstBody != orxNULL)
                  && (pstChildBody != orxNULL)
                  && (i < s32JointNumber)
                  && (orxBody_AddJointFromConfig(pstBody, pstChildBody, orxConfig_GetListString(orxOBJECT_KZ_CONFIG_CHILD_JOINT_LIST, i)) != orxNULL))
                  {
                    /* Marks it as a joint child */
                    orxStructure_SetFlags(pstChild, orxOBJECT_KU32_FLAG_IS_JOINT_CHILD | orxOBJECT_KU32_FLAG_DETACH_JOINT_CHILD, orxOBJECT_KU32_FLAG_NONE);

                    /* Updates flags */
                    u32Flags |= orxOBJECT_KU32_FLAG_HAS_JOINT_CHILDREN;
                  }

                  /* Sets its parent */
                  orxObject_SetParent(pstChild, pstResult);
                }

                /* Updates flags */
                u32Flags |= orxOBJECT_KU32_FLAG_HAS_CHILDREN;
              }
            }
          }
        }

        /* *** Speed *** */

        /* Has speed? */
        if(orxConfig_GetVector(orxOBJECT_KZ_CONFIG_SPEED, &vValue) != orxNULL)
        {
          /* Uses relative speed? */
          if(orxConfig_GetBool(orxOBJECT_KZ_CONFIG_USE_RELATIVE_SPEED) != orxFALSE)
          {
            /* Updates object relative speed */
            orxObject_SetRelativeSpeed(pstResult, &vValue);
          }
          else
          {
            /* Updates object speed */
            orxObject_SetSpeed(pstResult, &vValue);
          }
        }

        /* *** Angular velocity *** */

        /* Sets angular velocity? */
        orxObject_SetAngularVelocity(pstResult, orxMATH_KF_DEG_TO_RAD * orxConfig_GetFloat(orxOBJECT_KZ_CONFIG_ANGULAR_VELOCITY));

        /* *** FX *** */

        /* Has FX? */
        if((s32Number = orxConfig_GetListCount(orxOBJECT_KZ_CONFIG_FX_LIST)) > 0)
        {
          orxS32 i, s32DelayNumber;

          /* Gets number of delays */
          s32DelayNumber = orxConfig_GetListCount(orxOBJECT_KZ_CONFIG_FX_DELAY_LIST);

          /* For all defined FXs */
          for(i = 0; i < s32Number; i++)
          {
            const orxSTRING zFX;
            orxFLOAT        fDelay;

            /* Gets its name */
            zFX = orxConfig_GetListString(orxOBJECT_KZ_CONFIG_FX_LIST, i);

            /* Valid? */
            if(zFX != orxSTRING_EMPTY)
            {
              /* Gets its delay */
              fDelay = (i < s32DelayNumber) ? orxConfig_GetListFloat(orxOBJECT_KZ_CONFIG_FX_DELAY_LIST, i) : orxFLOAT_0;
              fDelay = orxMAX(fDelay, orxFLOAT_0);

              /* Adds it */
              orxObject_AddDelayedFX(pstResult, zFX, fDelay);
            }
          }

          /* Success? */
          if(pstResult->astStructureList[orxSTRUCTURE_ID_FXPOINTER].pstStructure != orxNULL)
          {
            orxCLOCK_INFO stClockInfo;

            /* Applies FXs directly to prevent any potential 1-frame visual glitches */
            orxMemory_Zero(&stClockInfo, sizeof(orxCLOCK_INFO));
            stClockInfo.fDT = orxMATH_KF_EPSILON;
            orxStructure_Update(pstResult->astStructureList[orxSTRUCTURE_ID_FXPOINTER].pstStructure, pstResult, &stClockInfo);
          }
        }

        /* *** Spawner *** */

        /* Gets spawner name */
        zSpawnerName = orxConfig_GetString(orxOBJECT_KZ_CONFIG_SPAWNER);

        /* Valid? */
        if((zSpawnerName != orxNULL) && (zSpawnerName != orxSTRING_EMPTY))
        {
          orxSPAWNER *pstSpawner;

          /* Creates spawner */
          pstSpawner = orxSpawner_CreateFromConfig(zSpawnerName);

          /* Valid? */
          if(pstSpawner != orxNULL)
          {
            /* Links it */
            if(orxObject_LinkStructure(pstResult, orxSTRUCTURE(pstSpawner)) != orxSTATUS_FAILURE)
            {
              /* Sets object as parent */
              orxSpawner_SetParent(pstSpawner, pstResult);

              /* Updates flags */
              orxFLAG_SET(pstResult->astStructureList[orxSTRUCTURE_ID_SPAWNER].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

              /* Updates its owner */
              orxStructure_SetOwner(pstSpawner, pstResult);
            }
            else
            {
              /* Deletes it */
              orxSpawner_Delete(pstSpawner);
              pstSpawner = orxNULL;
            }
          }
        }

        /* *** Sound *** */

        /* Has sound? */
        if((s32Number = orxConfig_GetListCount(orxOBJECT_KZ_CONFIG_SOUND_LIST)) > 0)
        {
          orxS32 i;

          /* For all defined sounds */
          for(i = 0; i < s32Number; i++)
          {
            const orxSTRING zSound;

            /* Gets its name */
            zSound = orxConfig_GetListString(orxOBJECT_KZ_CONFIG_SOUND_LIST, i);

            /* Valid? */
            if(zSound != orxSTRING_EMPTY)
            {
              /* Adds it */
              orxObject_AddSound(pstResult, zSound);
            }
          }
        }

        /* *** Shader *** */

        /* Has shader? */
        if((s32Number = orxConfig_GetListCount(orxOBJECT_KZ_CONFIG_SHADER_LIST)) > 0)
        {
          orxS32 i;

          /* For all defined shaders */
          for(i = 0; i < s32Number; i++)
          {
            const orxSTRING zShader;

            /* Gets its name */
            zShader = orxConfig_GetListString(orxOBJECT_KZ_CONFIG_SHADER_LIST, i);

            /* Valid? */
            if(zShader != orxSTRING_EMPTY)
            {
              /* Adds it */
              orxObject_AddShader(pstResult, zShader);
            }
          }
        }

        /* *** Timeline *** */

        /* Has TimeLine tracks? */
        if((s32Number = orxConfig_GetListCount(orxOBJECT_KZ_CONFIG_TRACK_LIST)) > 0)
        {
          orxS32 i;

          /* For all defined tracks */
          for(i = 0; i < s32Number; i++)
          {
            const orxSTRING zTrack;

            /* Gets its name */
            zTrack = orxConfig_GetListString(orxOBJECT_KZ_CONFIG_TRACK_LIST, i);

            /* Valid? */
            if(zTrack != orxSTRING_EMPTY)
            {
              /* Adds it */
              orxObject_AddTimeLineTrack(pstResult, zTrack);
            }
          }
        }

        /* *** Misc *** */

        /* Has smoothing value? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_SMOOTHING) != orxFALSE)
        {
          /* Updates flags */
          u32Flags |= (orxConfig_GetBool(orxOBJECT_KZ_CONFIG_SMOOTHING) != orxFALSE) ? orxOBJECT_KU32_FLAG_SMOOTHING_ON : orxOBJECT_KU32_FLAG_SMOOTHING_OFF;
        }

        /* Has blend mode? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_BLEND_MODE) != orxFALSE)
        {
          const orxSTRING zBlendMode;

          /* Gets blend mode value */
          zBlendMode = orxConfig_GetString(orxOBJECT_KZ_CONFIG_BLEND_MODE);

          /* Updates object's blend mode */
          orxObject_SetBlendMode(pstResult, orxDisplay_GetBlendModeFromString(zBlendMode));
        }

        /* Should repeat? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_REPEAT) != orxFALSE)
        {
          orxVECTOR vRepeat;

          /* Gets its value */
          orxConfig_GetVector(orxOBJECT_KZ_CONFIG_REPEAT, &vRepeat);

          /* Stores it */
          orxObject_SetRepeat(pstResult, vRepeat.fX, vRepeat.fY);
        }

        /* Has life time? */
        if(orxConfig_HasValue(orxOBJECT_KZ_CONFIG_LIFETIME) != orxFALSE)
        {
          /* Sets literal lifetime? */
          if(orxObject_SetLiteralLifeTime(pstResult, orxConfig_GetString(orxOBJECT_KZ_CONFIG_LIFETIME)) == orxSTATUS_FAILURE)
          {
            /* Stores lifetime's numerical value */
            orxObject_SetLifeTime(pstResult, orxConfig_GetFloat(orxOBJECT_KZ_CONFIG_LIFETIME));
          }
        }

        /* Updates flags */
        orxStructure_SetFlags(pstResult, u32Flags, orxOBJECT_KU32_FLAG_NONE);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_CREATE, pstResult, orxNULL, orxNULL);

        /* Should age? */
        if(orxFLAG_TEST(sstObject.u32Flags, orxOBJECT_KU32_STATIC_FLAG_AGE))
        {
          orxOBJECT **ppstObject;

          /* Adds it to the bank */
          ppstObject = (orxOBJECT **)orxBank_Allocate(sstObject.pstAgeBank);
          orxASSERT(ppstObject != orxNULL);
          *ppstObject = pstResult;

          /* Should apply age? */
          if(fAge > orxFLOAT_0)
          {
            orxCLOCK_INFO         stAgeClockInfo;
            orxCLOCK             *pstClock;
            const orxCLOCK_INFO  *pstClockInfo;

            /* Gets object clock */
            pstClock = orxObject_GetClock(pstResult);

            /* Not found? */
            if(pstClock == orxNULL)
            {
              /* Gets core clock */
              pstClock = orxClock_Get(orxCLOCK_KZ_CORE);
              orxASSERT(pstClock != orxNULL);
            }

            /* Gets its info */
            pstClockInfo = orxClock_GetInfo(pstClock);
            orxASSERT(pstClockInfo != orxNULL);

            /* Copies it */
            orxMemory_Copy(&stAgeClockInfo, pstClockInfo, sizeof(orxCLOCK_INFO));

            /* Has multiply modifier? */
            if(stAgeClockInfo.afModifierList[orxCLOCK_MODIFIER_MULTIPLY] != orxFLOAT_0)
            {
              /* Updates age */
              fAge *= stAgeClockInfo.afModifierList[orxCLOCK_MODIFIER_MULTIPLY];
            }

            /* For all time slices */
            for(; fAge > orxFLOAT_0; fAge -= stAgeClockInfo.fDT)
            {
              /* Computes age DT */
              stAgeClockInfo.fDT = orxCLAMP(stAgeClockInfo.fDT, orx2F(0.01f), fAge);

              /* For all aging objects */
              for(ppstObject = (orxOBJECT **)orxBank_GetNext(sstObject.pstAgeBank, orxNULL);
                  ppstObject != orxNULL;
                  ppstObject = (orxOBJECT **)orxBank_GetNext(sstObject.pstAgeBank, ppstObject))
              {
                /* Still valid? */
                if(orxOBJECT(*ppstObject) != orxNULL)
                {
                  /* Updates it */
                  orxObject_UpdateInternal(*ppstObject, &stAgeClockInfo);
                }
              }
            }

            /* Clears bank */
            orxBank_Clear(sstObject.pstAgeBank);

            /* Updates status */
            orxFLAG_SET(sstObject.u32Flags, orxOBJECT_KU32_STATIC_FLAG_NONE, orxOBJECT_KU32_STATIC_FLAG_AGE);
          }
        }

        /* Restores previous object */
        sstObject.pstCurrentObject = pstPreviousObject;
      }
      else
      {
        /* Deletes object */
        orxObject_DeleteInternal(pstResult);
        pstResult = orxNULL;
      }
    }

    /* Pops section */
    orxConfig_PopSection();
  }
  else
  {
    /* Not empty? */
    if(*_zConfigID != orxCHAR_NULL)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to find config section named %s.", _zConfigID);
    }

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Links a structure to an object.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstStructure   Structure to link
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_LinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE *_pstStructure)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxSTRUCTURE_ID eStructureID;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxSTRUCTURE_ASSERT(_pstStructure);

  /* Gets structure id & offset */
  eStructureID = orxStructure_GetID(_pstStructure);

  /* Valid? */
  if(eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER)
  {
    /* Unlink previous structure if needed */
    orxObject_UnlinkStructure(_pstObject, eStructureID);

    /* Updates structure reference count */
    orxStructure_IncreaseCount(_pstStructure);

    /* Links new structure to object */
    _pstObject->astStructureList[eStructureID].pstStructure = _pstStructure;
    _pstObject->astStructureList[eStructureID].u32Flags     = orxOBJECT_KU32_STORAGE_FLAG_NONE;

    /* Updates flags */
    orxStructure_SetFlags(_pstObject, 1 << eStructureID, orxOBJECT_KU32_FLAG_NONE);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Can't link structure ID <%d> to object <%s>.", eStructureID, _pstObject->zReference);

    /* Wrong structure ID */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Unlinks structure from an object, given its structure ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eStructureID   ID of structure to unlink
 */
void orxFASTCALL orxObject_UnlinkStructure(orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER);

  /* Needs to be processed? */
  if(orxStructure_TestFlags(_pstObject, 1 << _eStructureID))
  {
    orxSTRUCTURE *pstStructure;

    /* Gets referenced structure */
    pstStructure = _pstObject->astStructureList[_eStructureID].pstStructure;

    /* Decreases structure reference count */
    orxStructure_DecreaseCount(pstStructure);

    /* Was internally handled? */
    if(orxFLAG_TEST(_pstObject->astStructureList[_eStructureID].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL))
    {
      /* Removes its owner */
      orxStructure_SetOwner(pstStructure, orxNULL);

      /* Depending on structure ID */
      switch(_eStructureID)
      {
        case orxSTRUCTURE_ID_ANIMPOINTER:
        {
          orxAnimPointer_Delete(orxANIMPOINTER(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_BODY:
        {
          orxBody_Delete(orxBODY(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_CLOCK:
        {
          orxClock_Delete(orxCLOCK(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_FRAME:
        {
          orxFrame_Delete(orxFRAME(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_FXPOINTER:
        {
          orxFXPointer_Delete(orxFXPOINTER(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_GRAPHIC:
        {
          orxGraphic_Delete(orxGRAPHIC(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_SHADERPOINTER:
        {
          orxShaderPointer_Delete(orxSHADERPOINTER(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_SOUNDPOINTER:
        {
          orxSoundPointer_Delete(orxSOUNDPOINTER(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_SPAWNER:
        {
          orxSpawner_Delete(orxSPAWNER(pstStructure));
          break;
        }

        case orxSTRUCTURE_ID_TIMELINE:
        {
          orxTimeLine_Delete(orxTIMELINE(pstStructure));
          break;
        }

        default:
        {
          orxASSERT(orxFALSE && "Can't destroy this structure type directly from an object.");

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid parent's structure id.");
          break;
        }
      }
    }

    /* Cleans it */
    orxMemory_Zero(&(_pstObject->astStructureList[_eStructureID]), sizeof(orxOBJECT_STORAGE));

    /* Updates flags */
    orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_NONE, 1 << _eStructureID);
  }

  return;
}


/* *** Structure accessors *** */


/** Structure used by an object get accessor, given its structure ID. Structure must then be cast correctly. (see helper macro
 * #orxOBJECT_GET_STRUCTURE())
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eStructureID   ID of the structure to get
 * @return orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL _orxObject_GetStructure(const orxOBJECT *_pstObject, orxSTRUCTURE_ID _eStructureID)
{
  orxSTRUCTURE *pstStructure = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Offset is valid? */
  if(_eStructureID < orxSTRUCTURE_ID_LINKABLE_NUMBER)
  {
    /* Gets requested structure */
    pstStructure = _pstObject->astStructureList[_eStructureID].pstStructure;
  }

  /* Done ! */
  return pstStructure;
}

/** Enables/disables an object. Note that enabling/disabling an object is not recursive, so its children will not be affected, see orxObject_EnableRecursive().
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _bEnable      Enable / disable
 */
void orxFASTCALL orxObject_Enable(orxOBJECT *_pstObject, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Wasn't enabled? */
    if(!orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
    {
      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_ENABLE, _pstObject, orxNULL, orxNULL);

      /* Updates status flags */
      orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED, orxOBJECT_KU32_FLAG_NONE);

      /* Isn't already part of the enable list? */
      if(orxLinkList_GetList(&(_pstObject->stEnableNode)) == orxNULL)
      {
        orxOBJECT_LISTS *pstGroupLists;

        /* Adds it to enable list */
        orxLinkList_AddEnd(&(sstObject.stEnableList), &(_pstObject->stEnableNode));

        /* Is cached group list? */
        if(_pstObject->stGroupID == sstObject.stCachedGroupID)
        {
          /* Gets it */
          pstGroupLists = sstObject.pstCachedGroupLists;
        }
        else
        {
          /* Gets group list */
          pstGroupLists = (orxOBJECT_LISTS *)orxHashTable_Get(sstObject.pstGroupTable, _pstObject->stGroupID);

          /* Checks */
          orxASSERT(pstGroupLists != orxNULL);

          /* Caches it */
          sstObject.pstCachedGroupLists = pstGroupLists;
          sstObject.stCachedGroupID     = _pstObject->stGroupID;
        }

        /* Adds object to enable group list */
        orxLinkList_AddEnd(&(pstGroupLists->stEnableList), &(_pstObject->stEnableGroupNode));
      }
    }
  }
  else
  {
    /* Was enabled? */
    if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
    {
      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_DISABLE, _pstObject, orxNULL, orxNULL);

      /* Updates status flags */
      orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_NONE, orxOBJECT_KU32_FLAG_ENABLED);

      /* Isn't on death row? */
      if(!orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_DEATH_ROW))
      {
        /* Removes it from enable lists */
        orxLinkList_Remove(&(_pstObject->stEnableNode));
        orxLinkList_Remove(&(_pstObject->stEnableGroupNode));
      }
    }
  }

  /* Done! */
  return;
}

/** Enables/disables an object and all its children.
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _bEnable      Enable / disable
 */
orxOBJECT_MAKE_RECURSIVE(Enable, orxBOOL);

/** Is object enabled?
 * @param[in]   _pstObject    Concerned object
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxObject_IsEnabled(const orxOBJECT *_pstObject)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Done! */
  return(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED));
}

/** Pauses/unpauses an object. Note that pausing an object is not recursive, so its children will not be affected, see orxObject_PauseRecursive().
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _bPause       Pause / unpause
 */
void orxFASTCALL orxObject_Pause(orxOBJECT *_pstObject, orxBOOL _bPause)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Pause? */
  if(_bPause != orxFALSE)
  {
    /* Wasn't paused? */
    if(!orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_PAUSED))
    {
      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_PAUSE, _pstObject, orxNULL, orxNULL);

      /* Updates status flags */
      orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_PAUSED, orxOBJECT_KU32_FLAG_NONE);
    }
  }
  else
  {
    /* Was paused? */
    if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_PAUSED))
    {
      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_OBJECT, orxOBJECT_EVENT_UNPAUSE, _pstObject, orxNULL, orxNULL);

      /* Updates status flags */
      orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_NONE, orxOBJECT_KU32_FLAG_PAUSED);
    }
  }

  return;
}

/** Pauses/unpauses an object and all its children.
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _bPause       Pause / unpause
 */
orxOBJECT_MAKE_RECURSIVE(Pause, orxBOOL);

/** Is object paused?
 * @param[in]   _pstObject    Concerned object
 * @return      orxTRUE if paused, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxObject_IsPaused(const orxOBJECT *_pstObject)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Done! */
  return(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_PAUSED));
}

/** Sets user data for an object. Orx ignores the user data, this is a mechanism for attaching custom
 * data to be used later by user code.
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _pUserData    User data to store / orxNULL
 */
void orxFASTCALL    orxObject_SetUserData(orxOBJECT *_pstObject, void *_pUserData)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Stores it */
  _pstObject->pUserData = _pUserData;

  return;
}

/** Gets object's user data.
 * @param[in]   _pstObject    Concerned object
 * @return      Storeduser data / orxNULL
 */
void *orxFASTCALL orxObject_GetUserData(const orxOBJECT *_pstObject)
{
  void *pResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets user data */
  pResult = _pstObject->pUserData;

  /* Done! */
  return pResult;
}

/** Sets owner for an object. Ownership in Orx is only about lifetime management. That is, when an object
 * dies, it also kills its children. Compare this with orxObject_SetParent().
 *
 * Note that the "ChildList" field of an object's config section implies two things; that the object is both
 * the owner (orxObject_SetOwner()) and the parent (orxObject_SetParent()) of its children. There is an
 * exception to this though; when an object's child has a parent camera, the object is only the owner, and
 * the camera is the parent.
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _pOwner       Owner to set / orxNULL, if owner is an orxOBJECT, the owned object will be added to it as a children
 */
void orxFASTCALL orxObject_SetOwner(orxOBJECT *_pstObject, void *_pOwner)
{
  orxOBJECT *pstOwner;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_pOwner == orxNULL) || (orxStructure_GetID((orxSTRUCTURE *)_pOwner) < orxSTRUCTURE_ID_NUMBER));

  /* Had a previous object owner? */
  if((pstOwner = orxOBJECT(orxStructure_GetOwner(_pstObject))) != orxNULL)
  {
    /* Is it the first child? */
    if(pstOwner->pstChild == _pstObject)
    {
      /* Was last child? */
      if(_pstObject->pstSibling == orxNULL)
      {
        /* Updates previous owner */
        pstOwner->pstChild = orxNULL;
        orxStructure_SetFlags(pstOwner, orxOBJECT_KU32_FLAG_NONE, orxOBJECT_KU32_FLAG_HAS_CHILDREN);
      }
      else
      {
        /* Stores sibling as first child */
        pstOwner->pstChild = _pstObject->pstSibling;
      }
    }
    else
    {
      orxOBJECT *pstChild;

      /* Finds previous child */
      for(pstChild = pstOwner->pstChild; pstChild->pstSibling != _pstObject; pstChild = pstChild->pstSibling);

      /* Updates it */
      pstChild->pstSibling = _pstObject->pstSibling;
    }
  }

  /* Sets new owner */
  orxStructure_SetOwner(_pstObject, _pOwner);
  _pstObject->pstSibling = orxNULL;

  /* Is new owner an object? */
  if((pstOwner = orxOBJECT(_pOwner)) != orxNULL)
  {
    /* Has a child? */
    if(pstOwner->pstChild != orxNULL)
    {
      orxOBJECT *pstChild;

      /* Gets the last child */
      for(pstChild = pstOwner->pstChild; pstChild->pstSibling != orxNULL; pstChild = pstChild->pstSibling);

      /* Adds object as last child */
      pstChild->pstSibling = _pstObject;
    }
    else
    {
      /* Adds it as first child */
      pstOwner->pstChild = _pstObject;

      /* Updates its status */
      orxStructure_SetFlags(pstOwner, orxOBJECT_KU32_FLAG_HAS_CHILDREN, orxOBJECT_KU32_FLAG_NONE);
    }
  }

  /* Done! */
  return;
}

/** Gets object's owner. See orxObject_SetOwner().
 * @param[in]   _pstObject    Concerned object
 * @return      Owner / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxObject_GetOwner(const orxOBJECT *_pstObject)
{
  orxSTRUCTURE *pResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets owner */
  pResult = orxStructure_GetOwner(_pstObject);

  /* Done! */
  return pResult;
}

/** Gets object's first owned child (only if created with a config ChildList / has an owner set with orxObject_SetOwner)
 * see orxObject_SetOwner() and orxObject_SetParent() for a comparison of ownership and parenthood in Orx.
 *
 * This function is typically used to iterate over the owned children of an object. For example;
 * @code
 * for(orxOBJECT * pstChild = orxObject_GetOwnedChild(pstObject);
 *     pstChild;
 *     pstChild = orxObject_GetOwnedSibling(pstChild))
 * {
 *     do_something(pstChild);
 * } @endcode
 * @param[in]   _pstObject    Concerned object
 * @return      First owned child object / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_GetOwnedChild(const orxOBJECT *_pstObject)
{
  orxOBJECT *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Has children? */
  if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_HAS_CHILDREN))
  {
    /* Updates result */
    pstResult = _pstObject->pstChild;
  }

  /* Done! */
  return pstResult;
}

/** Gets object's next owned sibling (only if created with a config ChildList / has an owner set with orxObject_SetOwner)
 * This function is typically used to iterate over the owned children of an object, see orxObject_GetOwnedChild() for an example.
 * @param[in]   _pstObject    Concerned object
 * @return      Next sibling object / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_GetOwnedSibling(const orxOBJECT *_pstObject)
{
  orxOBJECT *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(orxStructure_TestFlags(orxOBJECT(orxStructure_GetOwner(_pstObject)), orxOBJECT_KU32_FLAG_HAS_CHILDREN));

  /* Updates result */
  pstResult = _pstObject->pstSibling;

  /* Done! */
  return pstResult;
}

/** Sets associated clock for an object.
 * @param[in]   _pstObject    Concerned object
 * @param[in]   _pOwner       Clock to associate / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetClock(orxOBJECT *_pstObject, orxCLOCK *_pstClock)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Removes old one */
  orxObject_UnlinkStructure(_pstObject, orxSTRUCTURE_ID_CLOCK);

  /* Has new one? */
  if(_pstClock != orxNULL)
  {
    /* Links it */
    eResult = orxObject_LinkStructure(_pstObject, orxSTRUCTURE(_pstClock));
  }

  /* Done! */
  return eResult;
}

/** Gets object's clock.
 * @param[in]   _pstObject    Concerned object
 * @return      Associated clock / orxNULL
 */
orxCLOCK *orxFASTCALL orxObject_GetClock(const orxOBJECT *_pstObject)
{
  orxCLOCK *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Updates result */
  pstResult = orxOBJECT_GET_STRUCTURE(_pstObject, CLOCK);

  /* Done! */
  return pstResult;
}

/** Sets object flipping.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _bFlipX         Flip it on X axis
 * @param[in]   _bFlipY         Flip it on Y axis
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetFlip(orxOBJECT *_pstObject, orxBOOL _bFlipX, orxBOOL _bFlipY)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxU32 u32Flags;

    /* Updates flags */
    u32Flags  = (_bFlipX != orxFALSE) ? orxFRAME_KU32_FLAG_FLIP_X : orxFRAME_KU32_FLAG_NONE;
    u32Flags |= (_bFlipY != orxFALSE) ? orxFRAME_KU32_FLAG_FLIP_Y : orxFRAME_KU32_FLAG_NONE;

    /* Updates frame */
    orxStructure_SetFlags(pstFrame, u32Flags, orxFRAME_KU32_FLAG_FLIP_X | orxFRAME_KU32_FLAG_FLIP_Y);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets object flipping.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pbFlipX        X axis flipping
 * @param[in]   _pbFlipY        Y axis flipping
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_GetFlip(const orxOBJECT *_pstObject, orxBOOL *_pbFlipX, orxBOOL *_pbFlipY)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pbFlipX != orxNULL);
  orxASSERT(_pbFlipY != orxNULL);

  /* Gets its frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Updates flipping mode */
    *_pbFlipX = orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_FLIP_X) ? orxTRUE : orxFALSE;
    *_pbFlipY = orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_FLIP_Y) ? orxTRUE : orxFALSE;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object pivot. This is a convenience wrapper around orxGraphic_SetPivot(). The "pivot" is essentially
 * what is indicated by the "Pivot" field of a config graphic section.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvPivot        Object pivot
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetPivot(orxOBJECT *_pstObject, const orxVECTOR *_pvPivot)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPivot != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Sets object pivot */
    orxGraphic_SetPivot(pstGraphic, _pvPivot);
  }
  else
  {
    /* Stores it */
    orxVector_Copy(&(_pstObject->vPivot), _pvPivot);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Sets object origin. This is a convenience wrapper around orxGraphic_SetOrigin(). The "origin" of a graphic is
 * essentially what is indicated by the "TextureOrigin" field of a config graphic section. The "origin" together with
 * "size" (see orxObject_SetSize()) defines the sprite of an object.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvOrigin       Object origin
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetOrigin(orxOBJECT *_pstObject, const orxVECTOR *_pvOrigin)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvOrigin != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Sets object origin */
    orxGraphic_SetOrigin(pstGraphic, _pvOrigin);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object size. For objects that have a graphic attached it's simply a convenience wrapper for orxGraphic_SetSize(),
 * but an object can also have a size without a graphic.
 *
 * Note the difference between "Scale" and "Size". The size of an object with a non-text graphic is the sprite size in
 * pixels on its texture. The object's effective size for rendering and intersection purposes (see orxObject_Pick()
 * and friends) is proportional to its "size" multiplied by its "scale". Another important distinction is that the
 * scale of an object also affects its children (see orxObject_SetParent() and note the distinction between
 * parenthood and ownership).
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvSize         Object size
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetSize(orxOBJECT *_pstObject, const orxVECTOR *_pvSize)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvSize != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Sets object size */
    orxGraphic_SetSize(pstGraphic, _pvSize);
  }
  else
  {
    /* Stores it */
    orxVector_Copy(&(_pstObject->vSize), _pvSize);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Get object pivot. See orxObject_SetPivot() for a more detailed explanation.
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvPivot        Object pivot
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetPivot(const orxOBJECT *_pstObject, orxVECTOR *_pvPivot)
{
  orxGRAPHIC  *pstGraphic;
  orxVECTOR   *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPivot != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets its pivot */
     pvResult = orxGraphic_GetPivot(pstGraphic, _pvPivot);
  }
  else
  {
    /* Stores value */
    orxVector_Copy(_pvPivot, &(_pstObject->vPivot));

    /* Updates result */
    pvResult = _pvPivot;
  }

  /* Done! */
  return pvResult;
}

/** Get object origin. See orxObject_SetOrigin() for a more detailed explanation.
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvOrigin       Object origin
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetOrigin(const orxOBJECT *_pstObject, orxVECTOR *_pvOrigin)
{
  orxGRAPHIC  *pstGraphic;
  orxVECTOR   *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvOrigin != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets its origin */
     pvResult = orxGraphic_GetOrigin(pstGraphic, _pvOrigin);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets object size. See orxObject_SetSize() for a more detailed explanation.
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvSize         Object's size
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetSize(const orxOBJECT *_pstObject, orxVECTOR *_pvSize)
{
  orxGRAPHIC *pstGraphic;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvSize != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets its size */
    pvResult = orxGraphic_GetSize(pstGraphic, _pvSize);

    /* Invalid Z? */
    if(pvResult->fZ <= orxFLOAT_0)
    {
      /* Defaults to 1 */
      pvResult->fZ = orxFLOAT_1;
    }
  }
  else
  {
    /* Stores value */
    orxVector_Copy(_pvSize, &(_pstObject->vSize));

    /* Updates result */
    pvResult = _pvSize;
  }

  /* Done! */
  return pvResult;
}

/** Sets object position in its parent's reference frame. See orxObject_SetWorldPosition() for setting an object's
 * position in the global reference frame.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvPosition     Object position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetPosition(orxOBJECT *_pstObject, const orxVECTOR *_pvPosition)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxBODY *pstBody;

    /* Sets frame position */
    orxFrame_SetPosition(pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition);

    /* Gets body */
    pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

    /* Valid? */
    if(pstBody != orxNULL)
    {
      orxVECTOR vPos;

      /* Updates body position */
      orxBody_SetPosition(pstBody, orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vPos));
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object position in the global reference frame. See orxObject_SetPosition() for setting an object's position
 * in its parent's reference frame.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvPosition     Object world position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetWorldPosition(orxOBJECT *_pstObject, const orxVECTOR *_pvPosition)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxBODY *pstBody;

    /* Gets body */
    pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

    /* Valid? */
    if(pstBody != orxNULL)
    {
      /* Sets frame position */
      orxFrame_SetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, _pvPosition);

      /* Updates body position */
      orxBody_SetPosition(pstBody, _pvPosition);
    }
    else
    {
      /* Sets frame position */
      orxFrame_SetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, _pvPosition);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object rotation in its parent's reference frame. See orxObject_SetWorldRotation() for setting an object's
 * rotation in the global reference frame.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fRotation      Object rotation (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetRotation(orxOBJECT *_pstObject, orxFLOAT _fRotation)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxBODY *pstBody;

    /* Gets body */
    pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

    /* Valid? */
    if(pstBody != orxNULL)
    {
      /* Sets frame rotation */
      orxFrame_SetRotation(pstFrame, orxFRAME_SPACE_LOCAL, _fRotation);

      /* Updates body rotation */
      orxBody_SetRotation(pstBody, orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL));
    }
    else
    {
      /* Sets frame rotation */
      orxFrame_SetRotation(pstFrame, orxFRAME_SPACE_LOCAL, _fRotation);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object rotation in the global reference frame. See orxObject_SetRotation() for setting an object's rotation
 * in its parent's reference frame.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fRotation      Object world rotation (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetWorldRotation(orxOBJECT *_pstObject, orxFLOAT _fRotation)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxBODY *pstBody;

    /* Gets body */
    pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

    /* Valid? */
    if(pstBody != orxNULL)
    {
      /* Sets frame rotation */
      orxFrame_SetRotation(pstFrame, orxFRAME_SPACE_GLOBAL, _fRotation);

      /* Updates body rotation */
      orxBody_SetRotation(pstBody, _fRotation);
    }
    else
    {
      /* Sets object rotation */
      orxFrame_SetRotation(pstFrame, orxFRAME_SPACE_GLOBAL, _fRotation);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object scale in its parent's reference frame. See orxObject_SetWorldScale() for setting an object's scale
 * in the global reference frame.
 * See orxObject_SetSize() for a deeper explanation of the "size" of an object.
 * @param[in]   _pstObject      Concerned Object
 * @param[in]   _pvScale        Object scale vector
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetScale(orxOBJECT *_pstObject, const orxVECTOR *_pvScale)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvScale != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Sets frame scale */
    orxFrame_SetScale(pstFrame, orxFRAME_SPACE_LOCAL, _pvScale);

    /* Updates body scale */
    orxObject_UpdateBodyScale(_pstObject);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object scale in the global reference frame. See orxObject_SetScale() for setting an object's scale in its
 * parent's reference frame.
 * @param[in]   _pstObject      Concerned Object
 * @param[in]   _pvScale        Object world scale vector
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetWorldScale(orxOBJECT *_pstObject, const orxVECTOR *_pvScale)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvScale != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Sets frame scale */
    orxFrame_SetScale(pstFrame, orxFRAME_SPACE_GLOBAL, _pvScale);

    /* Updates body scale */
    orxObject_UpdateBodyScale(_pstObject);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Get object position. See orxObject_SetPosition().
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvPosition     Object position
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetPosition(const orxOBJECT *_pstObject, orxVECTOR *_pvPosition)
{
  orxFRAME  *pstFrame;
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object position */
    pvResult = orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Get object world position. See orxObject_SetWorldPosition().
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvPosition     Object world position
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetWorldPosition(const orxOBJECT *_pstObject, orxVECTOR *_pvPosition)
{
  orxFRAME  *pstFrame;
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object position */
    pvResult = orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, _pvPosition);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Get object rotation. See orxObject_SetRotation().
 * @param[in]   _pstObject      Concerned object
 * @return      orxFLOAT (radians)
 */
orxFLOAT orxFASTCALL orxObject_GetRotation(const orxOBJECT *_pstObject)
{
  orxFRAME *pstFrame;
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object rotation */
    fResult = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_LOCAL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Get object world rotation. See orxObject_SetWorldRotation().
 * @param[in]   _pstObject      Concerned object
 * @return      orxFLOAT (radians)
 */
orxFLOAT orxFASTCALL orxObject_GetWorldRotation(const orxOBJECT *_pstObject)
{
  orxFRAME *pstFrame;
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object rotation */
    fResult = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Get object scale. See orxObject_SetScale().
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvScale        Object scale vector
 * @return      Scale vector
 */
orxVECTOR *orxFASTCALL orxObject_GetScale(const orxOBJECT *_pstObject, orxVECTOR *_pvScale)
{
  orxFRAME *pstFrame;
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvScale != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object scale */
    orxFrame_GetScale(pstFrame, orxFRAME_SPACE_LOCAL, _pvScale);

    /* Updates result */
    pvResult = _pvScale;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Clears vector */
    orxVector_SetAll(_pvScale, orxFLOAT_0);

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets object world scale. See orxObject_SetWorldScale().
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvScale        Object world scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxVECTOR *orxFASTCALL orxObject_GetWorldScale(const orxOBJECT *_pstObject, orxVECTOR *_pvScale)
{
  orxFRAME  *pstFrame;
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvScale != orxNULL);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Gets object scale */
    pvResult = orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, _pvScale);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get frame object.");

    /* Clears scale */
    orxVector_Copy(_pvScale, &orxVECTOR_0);

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Sets an object parent (in the frame hierarchy). Parenthood in orx is about the transformation (position,
 * rotation, scale) of objects. Transformation of objects are compounded in a frame hierarchy. Compare this
 * with orxObject_SetOwner()
 *
 * Note that the "ChildList" field of an object's config section implies two things; that the object is both
 * the owner (orxObject_SetOwner()) and the parent (orxObject_SetParent()) of its children. There is an
 * exception to this though; when an object's child has a parent camera, the object is only the owner, and
 * the camera is the parent.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pParent        Parent structure to set (object, spawner, camera or frame) / orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetParent(orxOBJECT *_pstObject, void *_pParent)
{
  orxFRAME   *pstFrame;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_pParent == orxNULL) || (orxStructure_GetID((orxSTRUCTURE *)_pParent) < orxSTRUCTURE_ID_NUMBER));

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Checks */
  orxSTRUCTURE_ASSERT(pstFrame);

  /* No parent? */
  if(_pParent == orxNULL)
  {
    /* Removes parent */
    orxFrame_SetParent(pstFrame, orxNULL);
  }
  else
  {
#ifdef __orxDEBUG__

    /* Does object have a body and isn't a jointed child? */
    if((orxOBJECT_GET_STRUCTURE(_pstObject, BODY) != orxNULL)
    && (!orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_DETACH_JOINT_CHILD)))
    {
      orxSTRUCTURE *pstParent;

      /* For all parents */
      for(pstParent = orxSTRUCTURE(_pParent); pstParent != orxNULL;)
      {
        /* Is an object with a body? */
        if((orxOBJECT(pstParent) != orxNULL)
        && (orxOBJECT_GET_STRUCTURE(orxOBJECT(pstParent), BODY) != orxNULL))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, orxANSI_KZ_COLOR_FG_GREEN "[%s]" orxANSI_KZ_COLOR_FG_DEFAULT ": object has been set as part of the child hierarchy of object " orxANSI_KZ_COLOR_FG_YELLOW "<%s>" orxANSI_KZ_COLOR_FG_DEFAULT ", " orxANSI_KZ_COLOR_BLINK_ON orxANSI_KZ_COLOR_FG_RED "this will result in undefined behavior as both have physics bodies.", orxObject_GetName(_pstObject), orxObject_GetName(orxOBJECT(pstParent)));

          break;
        }

        /* Depending on type */
        switch(orxStructure_GetID(pstParent))
        {
          case orxSTRUCTURE_ID_CAMERA:
          {
            /* Gets its parent */
            pstParent = orxCamera_GetParent(orxCAMERA(pstParent));

            break;
          }

          case orxSTRUCTURE_ID_FRAME:
          {
            /* Gets its parent */
            pstParent = orxSTRUCTURE(orxFrame_GetParent(orxFRAME(pstParent)));

            break;
          }

          case orxSTRUCTURE_ID_OBJECT:
          {
            /* Gets its parent */
            pstParent = orxObject_GetParent(orxOBJECT(pstParent));

            break;
          }

          case orxSTRUCTURE_ID_SPAWNER:
          {
            /* Gets its parent */
            pstParent = orxSpawner_GetParent(orxSPAWNER(pstParent));

            break;
          }

          default:
          {
            /* No more parents */
            pstParent = orxNULL;
            break;
          }
        }
      }
    }

#endif /* __orxDEBUG__ */

    /* Depending on parent ID */
    switch(orxStructure_GetID(_pParent))
    {
      case orxSTRUCTURE_ID_CAMERA:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxCamera_GetFrame(orxCAMERA(_pParent)));

        break;
      }

      case orxSTRUCTURE_ID_FRAME:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxFRAME(_pParent));

        break;
      }

      case orxSTRUCTURE_ID_OBJECT:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxOBJECT_GET_STRUCTURE(orxOBJECT(_pParent), FRAME));

        break;
      }

      case orxSTRUCTURE_ID_SPAWNER:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxSpawner_GetFrame(orxSPAWNER(_pParent)));

        break;
      }

      default:
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid parent's structure id.");

        /* Updates result */
        eResult = orxSTATUS_FAILURE;

        break;
      }
    }
  }

  /* Updates body scale */
  orxObject_UpdateBodyScale(_pstObject);

  /* Done! */
  return eResult;
}

/** Gets object's parent. See orxObject_SetParent() for a more detailed explanation.
 * @param[in]   _pstObject      Concerned object
 * @return      Parent (object, spawner, camera or frame) / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxObject_GetParent(const orxOBJECT *_pstObject)
{
  orxFRAME     *pstFrame, *pstParentFrame;
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Checks */
  orxSTRUCTURE_ASSERT(pstFrame);

  /* Gets frame's parent */
  pstParentFrame = orxFrame_GetParent(pstFrame);

  /* Valid? */
  if(pstParentFrame != orxNULL)
  {
    /* Gets its owner */
    pstResult = orxStructure_GetOwner(pstParentFrame);

    /* No owner? */
    if(pstResult == orxNULL)
    {
      /* Updates result with frame itself */
      pstResult = (orxSTRUCTURE *)pstParentFrame;
    }
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets object's first child object. See orxObject_SetOwner() and orxObject_SetParent() for a comparison of
 * ownership and parenthood in Orx.
 * Note: this function will filter out any camera or spawner and retrieve the first child object.
 * This function is typically used to iterate over the children objects of an object.
 * For example:
 * @code
 * for(orxOBJECT *pstChild = orxObject_GetChild(pstObject);
 *     pstChild != orxNULL;
 *     pstChild = orxObject_GetSibling(pstChild))
 * {
 *     DoSomething(pstChild); // DoSomething() can recurse into the children of pstChild for a depth-first traversal
 * }
 * @endcode
 * @param[in]   _pstObject      Concerned object
 * @return      First child object / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_GetChild(const orxOBJECT *_pstObject)
{
  orxFRAME   *pstFrame, *pstChildFrame;
  orxOBJECT  *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Checks */
  orxSTRUCTURE_ASSERT(pstFrame);

  /* Gets frame's child */
  pstChildFrame = orxFrame_GetChild(pstFrame);

  /* Valid? */
  if(pstChildFrame != orxNULL)
  {
    orxSTRUCTURE *pstChild;

    /* Skips all non-objects */
    for(pstChild = orxStructure_GetOwner(pstChildFrame);
        (pstChild == orxNULL) || (orxStructure_GetID(pstChild) != orxSTRUCTURE_ID_OBJECT);
        pstChild = orxStructure_GetOwner(pstChildFrame))
    {
      /* Gets next sibling frame */
      pstChildFrame = orxFrame_GetSibling(pstChildFrame);

      /* No more siblings? */
      if(pstChildFrame == orxNULL)
      {
        /* Updates child */
        pstChild = orxNULL;

        /* Stops */
        break;
      }
    }

    /* Updates result */
    pstResult = orxOBJECT(pstChild);
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets object's next sibling object. This function is typically used for iterating over the children objects of an object,
 * see orxObject_GetChild() for an iteration example.
 * Note: this function will filter out any camera or spawner and retrieve the next sibling object.
 * @param[in]   _pstObject      Concerned object
 * @return      Next sibling object / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_GetSibling(const orxOBJECT *_pstObject)
{
  orxFRAME   *pstFrame, *pstSiblingFrame;
  orxOBJECT  *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Checks */
  orxSTRUCTURE_ASSERT(pstFrame);

  /* Gets frame's sibling */
  pstSiblingFrame = orxFrame_GetSibling(pstFrame);

  /* Valid? */
  if(pstSiblingFrame != orxNULL)
  {
    orxSTRUCTURE *pstSibling;

    /* Skips all non-objects */
    for(pstSibling = orxStructure_GetOwner(pstSiblingFrame);
        (pstSibling == orxNULL) || (orxStructure_GetID(pstSibling) != orxSTRUCTURE_ID_OBJECT);
        pstSibling = orxStructure_GetOwner(pstSiblingFrame))
    {
      /* Gets next sibling frame */
      pstSiblingFrame = orxFrame_GetSibling(pstSiblingFrame);

      /* No more siblings? */
      if(pstSiblingFrame == orxNULL)
      {
        /* Updates sibling */
        pstSibling = orxNULL;

        /* Stops */
        break;
      }
    }

    /* Updates result */
    pstResult = orxOBJECT(pstSibling);
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets object's next child structure of a given type (camera, object or spawner).
 * See orxObject_SetOwner() and orxObject_SetParent() for a comparison of
 * ownership and parenthood in Orx.
 * See orxObject_GetChild()/orxObject_GetSibling() if you want to only consider children objects.
 * This function is typically used to iterate over the children of an object.
 * For example, iterating over the immediate children cameras:
 * @code
 * for(orxCAMERA *pstChild = orxCAMERA(orxObject_GetNextChild(pstObject, orxNULL, orxSTRUCTURE_ID_CAMERA));
 *     pstChild != orxNULL;
 *     pstChild = orxCAMERA(orxObject_GetNextChild(pstObject, pstChild, orxSTRUCTURE_ID_CAMERA)))
 * {
 *     DoSomethingWithCamera(pstChild);
 * }
 * @endcode
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pChild         Concerned child to retrieve the next sibling, orxNULL to retrieve the first child
 * @param[in]   _eStructureID   ID of the structure to consider (camera, spawner, object or frame)
 * @return      Next child/sibling structure (camera, spawner, object or frame) / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxObject_GetNextChild(const orxOBJECT *_pstObject, void *_pChild, orxSTRUCTURE_ID _eStructureID)
{
  orxFRAME     *pstFrame, *pstChildFrame;
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT((_eStructureID == orxSTRUCTURE_ID_CAMERA) || (_eStructureID == orxSTRUCTURE_ID_FRAME) || (_eStructureID == orxSTRUCTURE_ID_OBJECT) || (_eStructureID == orxSTRUCTURE_ID_SPAWNER));
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Checks */
  orxSTRUCTURE_ASSERT(pstFrame);

  /* Gets frame's child */
  pstChildFrame = orxFrame_GetChild(pstFrame);

  /* Valid and has child? */
  if((pstChildFrame != orxNULL) && (_pChild != orxNULL))
  {
    orxSTRUCTURE *pstChild;

    /* Skips to it */
    for(pstChild = orxStructure_GetOwner(pstChildFrame);
        pstChild != _pChild;
        pstChild = orxStructure_GetOwner(pstChildFrame))
    {
      /* Gets next sibling frame */
      pstChildFrame = orxFrame_GetSibling(pstChildFrame);

      /* No more siblings? */
      if(pstChildFrame == orxNULL)
      {
        /* Stops */
        break;
      }
    }

    /* Found? */
    if(pstChild == _pChild)
    {
      /* Gets its sibling frame */
      pstChildFrame = orxFrame_GetSibling(pstChildFrame);
    }
  }

  /* Valid? */
  if(pstChildFrame != orxNULL)
  {
    /* Skips all non-related structures */
    for(pstResult = orxStructure_GetOwner(pstChildFrame);
        (pstResult == orxNULL) || (orxStructure_GetID(pstResult) != _eStructureID);
        pstResult = orxStructure_GetOwner(pstChildFrame))
    {
      /* Gets next sibling frame */
      pstChildFrame = orxFrame_GetSibling(pstChildFrame);

      /* No more siblings? */
      if(pstChildFrame == orxNULL)
      {
        /* Updates result */
        pstResult = orxNULL;

        /* Stops */
        break;
      }
    }
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Attaches an object to a parent while maintaining the object's world position.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pParent        Parent structure to attach to (object, spawner, camera or frame)
 * @return      orsSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_Attach(orxOBJECT *_pstObject, void *_pParent)
{
  orxFRAME *pstFrame;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(_pstObject, FRAME);

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    orxVECTOR vPosition, vScale;
    orxFLOAT  fRotation;

    /* Gets object's world position, rotation and scale */
    orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vPosition);
    fRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL);
    orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vScale);

    /* Updates its parent */
    eResult = orxObject_SetParent(_pstObject, _pParent);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      orxBODY *pstBody;

      /* Restores object's world position, rotation and scale */
      orxFrame_SetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vScale);
      orxFrame_SetRotation(pstFrame, orxFRAME_SPACE_GLOBAL, fRotation);
      orxFrame_SetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vPosition);

      /* Gets body */
      pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

      /* Valid? */
      if(pstBody != orxNULL)
      {
        /* Updates body's position, rotation and scale */
        orxBody_SetScale(pstBody, &vScale);
        orxBody_SetRotation(pstBody, fRotation);
        orxBody_SetPosition(pstBody, &vPosition);
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Detaches an object from a parent while maintaining the object's world position.
 * @param[in]   _pstObject      Concerned object
 * @return      orsSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_Detach(orxOBJECT *_pstObject)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Detaches it */
  eResult = orxObject_Attach(_pstObject, orxNULL);

  /* Done! */
  return eResult;
}

/** Logs all parents of an object, including their frame data.
 * @param[in]   _pstObject      Concerned object
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_LogParents(const orxOBJECT *_pstObject)
{
  orxU32    u32DebugFlags;
  orxCHAR   acBuffer[512], acTempBuffer[128];
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Clears buffers */
  orxMemory_Zero(&acBuffer, sizeof(acBuffer));
  orxMemory_Zero(&acTempBuffer, sizeof(acTempBuffer));

  /* Backups debug flags */
  u32DebugFlags = orxDEBUG_GET_FLAGS();

  /* Sets new debug flags */
  orxDEBUG_SET_FLAGS(orxDEBUG_KU32_STATIC_FLAG_NONE,
                     orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP | orxDEBUG_KU32_STATIC_FLAG_TYPE);

  /* Logs header */
  orxLOG("*** BEGIN PARENTS LOG: \"" orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_FG_DEFAULT "\" [" orxANSI_KZ_COLOR_FG_MAGENTA "0x%016llX" orxANSI_KZ_COLOR_FG_DEFAULT "] ***\n", orxObject_GetName(_pstObject), orxStructure_GetGUID(_pstObject));
  orxLOG("             NAME                     GUID             [LOCAL]     POSITION          ROTATION             SCALE        =>    [WORLD]     POSITION          ROTATION             SCALE        [IGNORE FLAGS]");

  /* For all structures in parent hierarchy */
  for(orxSTRUCTURE *pstParent = orxSTRUCTURE(_pstObject); pstParent != orxNULL;)
  {
    const orxSTRING zName;
    orxU64          u64GUID;
    orxFRAME       *pstFrame;
    orxCAMERA      *pstCamera;
    orxOBJECT      *pstObject;
    orxSPAWNER     *pstSpawner;

    /* Gets its GUID */
    u64GUID = orxStructure_GetGUID(pstParent);

    /* Depending on its type */
    switch(orxStructure_GetID(pstParent))
    {
      case orxSTRUCTURE_ID_CAMERA:
      {
        /* Gets it */
        pstCamera = orxCAMERA(pstParent);

        /* Gets its name */
        zName = orxCamera_GetName(pstCamera);

        /* Gets its frame */
        pstFrame = orxCamera_GetFrame(pstCamera);

        /* Gets its parent */
        pstParent = orxCamera_GetParent(pstCamera);

        break;
      }

      case orxSTRUCTURE_ID_FRAME:
      {
        /* Uses default name */
        zName = "[FRAME]";

        /* Gets it */
        pstFrame = orxFRAME(pstParent);

        /* Gets its parent */
        pstParent = orxSTRUCTURE(orxFrame_GetParent(pstFrame));

        break;
      }

      case orxSTRUCTURE_ID_OBJECT:
      {
        /* Gets it */
        pstObject = orxOBJECT(pstParent);

        /* Gets its name */
        zName = orxObject_GetName(pstObject);

        /* Gets its frame */
        pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

        /* Gets its parent */
        pstParent = orxObject_GetParent(pstObject);

        break;
      }

      case orxSTRUCTURE_ID_SPAWNER:
      {
        /* Gets it */
        pstSpawner = orxSPAWNER(pstParent);

        /* Gets its name */
        zName = orxSpawner_GetName(pstSpawner);

        /* Gets its frame */
        pstFrame = orxSpawner_GetFrame(pstSpawner);

        /* Gets its parent */
        pstParent = orxSpawner_GetParent(pstSpawner);

        break;
      }

      default:
      {
        /* No more parents */
        zName     = orxNULL;
        pstFrame  = orxNULL;
        pstParent = orxNULL;
        break;
      }
    }

    /* Has frame? */
    if(pstFrame != orxNULL)
    {
      orxVECTOR vTemp;
      orxCHAR  *pc = acBuffer;
      orxFLOAT  fRotation;

      /* Prints name */
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "\"" orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_FG_DEFAULT "\"", zName);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), "%-40s", acTempBuffer);

      /* Prints GUID */
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "[" orxANSI_KZ_COLOR_FG_MAGENTA "0x%016llX" orxANSI_KZ_COLOR_FG_DEFAULT "]", u64GUID);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), "%-32s", acTempBuffer);

      /* Prints local position */
      orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_LOCAL, &vTemp);
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "(%.6g, %.6g, %.6g)", vTemp.fX, vTemp.fY, vTemp.fZ);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_GREEN "%28s", acTempBuffer);

      /* Prints local rotation */
      fRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_LOCAL);
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "%.6g", fRotation);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_GREEN "%10s", acTempBuffer);

      /* Prints local scale */
      orxFrame_GetScale(pstFrame, orxFRAME_SPACE_LOCAL, &vTemp);
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "(%.4g, %.4g, %.4g)", vTemp.fX, vTemp.fY, vTemp.fZ);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_GREEN "%28s", acTempBuffer);

      /* Prints separator */
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_DEFAULT " => ");

      /* Prints world position */
      orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vTemp);
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "(%.6g, %.6g, %.6g)", vTemp.fX, vTemp.fY, vTemp.fZ);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_GREEN "%28s", acTempBuffer);

      /* Prints world rotation */
      fRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL);
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "%.6g", fRotation);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_GREEN "%10s", acTempBuffer);

      /* Prints world scale */
      orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vTemp);
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, "(%.4g, %.4g, %.4g)", vTemp.fX, vTemp.fY, vTemp.fZ);
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_GREEN "%28s", acTempBuffer);

      /* Prints ignore flags */
      orxString_NPrint(acTempBuffer, sizeof(acTempBuffer) - 1, orxANSI_KZ_COLOR_FG_DEFAULT " [" orxANSI_KZ_COLOR_FG_YELLOW "%s" orxANSI_KZ_COLOR_FG_DEFAULT "]", orxFrame_GetIgnoreFlagNames(orxStructure_GetFlags(pstFrame, orxFRAME_KU32_MASK_IGNORE_ALL)));
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - 1 - (pc - acBuffer)), orxANSI_KZ_COLOR_FG_GREEN "%s", acTempBuffer);

      /* Logs it */
      orxLOG(acBuffer);
    }
  }

  /* Logs footer */
  orxLOG("\n*** END PARENTS LOG ***");

  /* Restores debug flags */
  orxDEBUG_SET_FLAGS(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);

  /* Done! */
  return eResult;
}

/** Sets an object animset.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstAnimSet     Animation set to set / orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetAnimSet(orxOBJECT *_pstObject, orxANIMSET *_pstAnimSet)
{
  orxANIMPOINTER *pstAnimPointer;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Creates animation pointer from animation set */
  pstAnimPointer = orxAnimPointer_Create(_pstAnimSet);

  /* Valid? */
  if(pstAnimPointer != orxNULL)
  {
    /* Links it to the object */
    eResult = orxObject_LinkStructure(_pstObject, orxSTRUCTURE(pstAnimPointer));

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates internal flag */
      orxFLAG_SET(_pstObject->astStructureList[orxSTRUCTURE_ID_ANIMPOINTER].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

      /* Updates its owner */
      orxStructure_SetOwner(pstAnimPointer, _pstObject);
    }
    else
    {
      /* Deletes it */
      orxAnimPointer_Delete(pstAnimPointer);
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets an object's relative animation frequency.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fFrequency     Frequency to set: < 1.0 for slower than initial, > 1.0 for faster than initial
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetAnimFrequency(orxOBJECT *_pstObject, orxFLOAT _fFrequency)
{
  orxANIMPOINTER *pstAnimPointer;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_fFrequency >= orxFLOAT_0);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != orxNULL)
  {
    /* Updates result */
    eResult = orxAnimPointer_SetFrequency(pstAnimPointer, _fFrequency);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets the relative animation frequency for an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fFrequency     Frequency to set: < 1.0 for slower than initial, > 1.0 for faster than initial
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxOBJECT_MAKE_RECURSIVE(SetAnimFrequency, orxFLOAT);

/** Gets an object's relative animation frequency.
 * @param[in]   _pstObject      Concerned object
 * @return Animation frequency / -orxFLOAT_1
 */
orxFLOAT orxFASTCALL orxObject_GetAnimFrequency(const orxOBJECT *_pstObject)
{
  orxANIMPOINTER *pstAnimPointer;
  orxFLOAT       fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != orxNULL)
  {
    /* Updates result */
    fResult = orxAnimPointer_GetFrequency(pstAnimPointer);
  }
  else
  {
    /* Updates result */
    fResult = -orxFLOAT_1;
  }

  /* Done! */
  return fResult;
}

/** Sets current animation for object. This function switches the currently displayed animation of the object
 * immediately. Compare this with orxObject_SetTargetAnim().
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zAnimName      Animation name (config's one) to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetCurrentAnim(orxOBJECT *_pstObject, const orxSTRING _zAnimName)
{
  orxANIMPOINTER *pstAnimPointer;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != orxNULL)
  {
    /* Sets current animation */
    eResult = orxAnimPointer_SetCurrentAnimFromName(pstAnimPointer, _zAnimName);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets current animation for an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zAnimName      Animation name (config's one) to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxOBJECT_MAKE_RECURSIVE(SetCurrentAnim, const orxSTRING);

/** Sets target animation for object. The animations are sequenced on an object according to the animation link graph
 * defined by its AnimationSet. The sequence follows the graph and tries to reach the target animation. Use
 * orxObject_SetCurrentAnim() to switch the animation without using the link graph.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zAnimName      Animation name (config's one) to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetTargetAnim(orxOBJECT *_pstObject, const orxSTRING _zAnimName)
{
  orxANIMPOINTER *pstAnimPointer;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != orxNULL)
  {
    /* Sets target animation */
    eResult = orxAnimPointer_SetTargetAnimFromName(pstAnimPointer, _zAnimName);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets target animation for an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zAnimName      Animation name (config's one) to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxOBJECT_MAKE_RECURSIVE(SetTargetAnim, const orxSTRING);

/** Gets current animation.
 * @param[in]   _pstObject      Concerned object
 * @return      Current animation / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxObject_GetCurrentAnim(const orxOBJECT *_pstObject)
{
  orxANIMPOINTER *pstAnimPointer;
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != orxNULL)
  {
    /* Gets current animation */
    zResult = orxAnimPointer_GetCurrentAnimName(pstAnimPointer);
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Gets target animation.
 * @param[in]   _pstObject      Concerned object
 * @return      Target animation / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxObject_GetTargetAnim(const orxOBJECT *_pstObject)
{
  orxANIMPOINTER *pstAnimPointer;
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Valid? */
  if(pstAnimPointer != orxNULL)
  {
    /* Gets target animation */
    zResult = orxAnimPointer_GetTargetAnimName(pstAnimPointer);
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Is current animation test.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zAnimName      Animation name (config's one) to test
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxObject_IsCurrentAnim(const orxOBJECT *_pstObject, const orxSTRING _zAnimName)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zAnimName != orxNULL) && (_zAnimName != orxSTRING_EMPTY));

  /* Updates result */
  bResult = (orxString_Compare(orxObject_GetCurrentAnim(_pstObject), _zAnimName) == 0) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Is target animation test.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zAnimName      Animation name (config's one) to test
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxObject_IsTargetAnim(const orxOBJECT *_pstObject, const orxSTRING _zAnimName)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zAnimName != orxNULL) && (_zAnimName != orxSTRING_EMPTY));

  /* Updates result */
  bResult = (orxString_Compare(orxObject_GetTargetAnim(_pstObject), _zAnimName) == 0) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Sets an object speed.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvSpeed        Speed to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetSpeed(orxOBJECT *_pstObject, const orxVECTOR *_pvSpeed)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Updates its speed */
    eResult = orxBody_SetSpeed(pstBody, _pvSpeed);
  }
  else
  {
    /* Stores it */
    orxVector_Copy(&(_pstObject->vSpeed), _pvSpeed);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Sets an object speed relative to its rotation/scale.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvSpeed        Relative speed to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetRelativeSpeed(orxOBJECT *_pstObject, const orxVECTOR *_pvRelativeSpeed)
{
  orxVECTOR vSpeed, vObjectScale;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvRelativeSpeed != orxNULL);

  /* Gets global speed */
  orxVector_Mul(&vSpeed, orxVector_2DRotate(&vSpeed, _pvRelativeSpeed, orxObject_GetRotation(_pstObject)), orxObject_GetScale(_pstObject, &vObjectScale));

  /* Applies it */
  eResult = orxObject_SetSpeed(_pstObject, &vSpeed);

  /* Done! */
  return eResult;
}

/** Sets an object angular velocity.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fVelocity      Angular velocity to set (radians/seconds)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetAngularVelocity(orxOBJECT *_pstObject, orxFLOAT _fVelocity)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Updates its angular velocity */
    eResult = orxBody_SetAngularVelocity(pstBody, _fVelocity);
  }
  else
  {
    /* Stores it */
    _pstObject->fAngularVelocity = _fVelocity;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Sets an object custom gravity.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _pvCustomGravity  Custom gravity to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetCustomGravity(orxOBJECT *_pstObject, const orxVECTOR *_pvCustomGravity)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Updates its custom gravity */
    eResult = orxBody_SetCustomGravity(pstBody, _pvCustomGravity);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets an object speed.
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvSpeed        Speed to get
 * @return      Object speed / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetSpeed(const orxOBJECT *_pstObject, orxVECTOR *_pvSpeed)
{
  orxBODY    *pstBody;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvSpeed != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Gets its speed */
    pvResult = orxBody_GetSpeed(pstBody, _pvSpeed);
  }
  else
  {
    /* Stores value */
    orxVector_Copy(_pvSpeed, &(_pstObject->vSpeed));

    /* Updates result */
    pvResult = _pvSpeed;
  }

  /* Done! */
  return pvResult;
}

/** Gets an object relative speed.
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvRelativeSpeed Relative speed to get
 * @return      Object relative speed / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetRelativeSpeed(const orxOBJECT *_pstObject, orxVECTOR *_pvRelativeSpeed)
{
  orxVECTOR vObjectScale, *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvRelativeSpeed != orxNULL);

  /* Gets objects speed */
  pvResult = orxObject_GetSpeed(_pstObject, _pvRelativeSpeed);

  /* Valid? */
  if(pvResult != orxNULL)
  {
    /* Gets relative speed */
    orxVector_Div(pvResult, orxVector_2DRotate(pvResult, pvResult, -orxObject_GetRotation(_pstObject)), orxObject_GetScale(_pstObject, &vObjectScale));
  }

  /* Done! */
  return pvResult;
}

/** Gets an object angular velocity.
 * @param[in]   _pstObject      Concerned object
 * @return      Object angular velocity (radians/seconds)
 */
orxFLOAT orxFASTCALL orxObject_GetAngularVelocity(const orxOBJECT *_pstObject)
{
  orxBODY  *pstBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Gets its angular velocity */
    fResult = orxBody_GetAngularVelocity(pstBody);
  }
  else
  {
    /* Updates result */
    fResult = _pstObject->fAngularVelocity;
  }

  /* Done! */
  return fResult;
}

/** Gets an object custom gravity.
 * @param[in]   _pstObject        Concerned object
 * @param[out]  _pvCustomGravity  Custom gravity to get
 * @return      Object custom gravity / orxNULL is object doesn't have any
 */
orxVECTOR *orxFASTCALL orxObject_GetCustomGravity(const orxOBJECT *_pstObject, orxVECTOR *_pvCustomGravity)
{
  orxBODY    *pstBody;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvCustomGravity != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Updates result */
    pvResult = orxBody_GetCustomGravity(pstBody, _pvCustomGravity);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets an object mass.
 * @param[in]   _pstObject      Concerned object
 * @return      Object mass
 */
orxFLOAT orxFASTCALL orxObject_GetMass(const orxOBJECT *_pstObject)
{
  orxBODY  *pstBody;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Gets its mass */
    fResult = orxBody_GetMass(pstBody);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get body object.");

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets an object center of mass (object space).
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pvMassCenter   Mass center to get
 * @return      Mass center / orxNULL
 */
orxVECTOR *orxFASTCALL orxObject_GetMassCenter(const orxOBJECT *_pstObject, orxVECTOR *_pvMassCenter)
{
  orxBODY    *pstBody;
  orxVECTOR  *pvResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvMassCenter != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Gets its center of mass */
    pvResult = orxBody_GetMassCenter(pstBody, _pvMassCenter);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to get body object.");

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Applies a torque.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fTorque        Torque to apply
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_ApplyTorque(orxOBJECT *_pstObject, orxFLOAT _fTorque)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Applies torque */
    eResult = orxBody_ApplyTorque(pstBody, _fTorque);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No body on object <%s>, can't apply torque.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Applies a force.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvForce        Force to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the force will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_ApplyForce(orxOBJECT *_pstObject, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvForce != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Applies force */
    eResult = orxBody_ApplyForce(pstBody, _pvForce, _pvPoint);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No body on object <%s>, can't apply force.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Applies an impulse.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _pvImpulse      Impulse to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the impulse will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_ApplyImpulse(orxOBJECT *_pstObject, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint)
{
  orxBODY  *pstBody;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvImpulse != orxNULL);

  /* Gets body */
  pstBody = orxOBJECT_GET_STRUCTURE(_pstObject, BODY);

  /* Valid? */
  if(pstBody != orxNULL)
  {
    /* Applies impulse */
    eResult = orxBody_ApplyImpulse(pstBody, _pvImpulse, _pvPoint);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No body on object <%s>, can't apply impulse.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Issues a raycast to test for potential objects in the way.
 * @param[in]   _pvBegin        Beginning of raycast
 * @param[in]   _pvEnd          End of raycast
 * @param[in]   _u16SelfFlags   Selfs flags used for filtering (0xFFFF for no filtering)
 * @param[in]   _u16CheckMask   Check mask used for filtering (0xFFFF for no filtering)
 * @param[in]   _bEarlyExit     Should stop as soon as an object has been hit (which might not be the closest)
 * @param[in]   _pvContact      If non-null and a contact is found it will be stored here
 * @param[in]   _pvNormal       If non-null and a contact is found, its normal will be stored here
 * @return Colliding orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_Raycast(const orxVECTOR *_pvBegin, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  orxBODY    *pstRaycastResult;
  orxOBJECT  *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvBegin != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Issues raycast */
  pstRaycastResult = orxBody_Raycast(_pvBegin, _pvEnd, _u16SelfFlags, _u16CheckMask, _bEarlyExit, _pvContact, _pvNormal);

  /* Found? */
  if(pstRaycastResult != orxNULL)
  {
    /* Updates result */
    pstResult = orxOBJECT(orxStructure_GetOwner(pstRaycastResult));
  }

  /* Done! */
  return pstResult;
}

/** Sets object text string, if object is associated to a text.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zString        String to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetTextString(orxOBJECT *_pstObject, const orxSTRING _zString)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxGRAPHIC(_pstObject->astStructureList[orxSTRUCTURE_ID_GRAPHIC].pstStructure);

  /* Valid text graphic? */
  if((pstGraphic != orxNULL) && (orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT)))
  {
    orxTEXT *pstText;

    /* Gets text */
    pstText = orxTEXT(orxGraphic_GetData(pstGraphic));

    /* Valid? */
    if(pstText != orxNULL)
    {
      /* Updates its string */
      eResult = orxText_SetString(pstText, _zString);

      /* Valid */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates graphic */
        orxGraphic_UpdateSize(pstGraphic);
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets object text string, if object is associated to a text.
 * @param[in]   _pstObject      Concerned object
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxObject_GetTextString(orxOBJECT *_pstObject)
{
  orxGRAPHIC     *pstGraphic;
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxGRAPHIC(_pstObject->astStructureList[orxSTRUCTURE_ID_GRAPHIC].pstStructure);

  /* Valid text graphic? */
  if((pstGraphic != orxNULL) && (orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT)))
  {
    orxTEXT *pstText;

    /* Gets text */
    pstText = orxTEXT(orxGraphic_GetData(pstGraphic));

    /* Valid? */
    if(pstText != orxNULL)
    {
      /* Updates result */
      zResult = orxText_GetString(pstText);
    }
  }

  /* Done! */
  return zResult;
}

/** Gets object's bounding box (OBB).
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pstBoundingBox Bounding box result
 * @return      Bounding box / orxNULL
 */
orxOBOX *orxFASTCALL orxObject_GetBoundingBox(const orxOBJECT *_pstObject, orxOBOX *_pstBoundingBox)
{
  orxVECTOR vSize, vPivot, vPosition, vScale;
  orxFLOAT  fAngle;
  orxOBOX  *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pstBoundingBox != orxNULL);

  /* Gets size, pivot, position, scale & rotation */
  orxObject_GetSize(_pstObject, &vSize);
  orxObject_GetPivot(_pstObject, &vPivot);
  orxObject_GetWorldPosition(_pstObject, &vPosition);
  orxObject_GetWorldScale(_pstObject, &vScale);
  fAngle = orxObject_GetWorldRotation(_pstObject);

  /* Updates pivot & size */
  orxVector_Mul(&vSize, &vSize, &vScale);
  orxVector_Mul(&vPivot, &vPivot, &vScale);

  /* Updates box */
  orxOBox_2DSet(_pstBoundingBox, &vPosition, &vPivot, &vSize, fAngle);

  /* Updates result */
  pstResult = _pstBoundingBox;

  /* Done! */
  return pstResult;
}

/** Adds an FX using its config ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_AddFX(orxOBJECT *_pstObject, const orxSTRING _zFXConfigID)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));

  /* Adds FX */
  eResult = orxObject_AddDelayedFX(_pstObject, _zFXConfigID, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** Adds an FX to an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 */
orxOBJECT_MAKE_RECURSIVE(AddFX, const orxSTRING);

/** Adds a unique FX using its config ID. Refer to orxObject_AddUniqueDelayedFX() for details, since this
 * function is the same as it with the delay argument set to 0.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_AddUniqueFX(orxOBJECT *_pstObject, const orxSTRING _zFXConfigID)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));

  /* Adds FX */
  eResult = orxObject_AddUniqueDelayedFX(_pstObject, _zFXConfigID, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** Adds a unique FX to an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 */
orxOBJECT_MAKE_RECURSIVE(AddUniqueFX, const orxSTRING);

/** Adds a delayed FX using its config ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 * @param[in]   _fDelay         Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_AddDelayedFX(orxOBJECT *_pstObject, const orxSTRING _zFXConfigID, orxFLOAT _fDelay)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));
  orxASSERT(_fDelay >= orxFLOAT_0);

  /* Is object active? */
  if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
  {
    orxFXPOINTER *pstFXPointer;

    /* Gets its FXPointer */
    pstFXPointer = orxOBJECT_GET_STRUCTURE(_pstObject, FXPOINTER);

    /* Doesn't exist? */
    if(pstFXPointer == orxNULL)
    {
      /* Creates one */
      pstFXPointer = orxFXPointer_Create();

      /* Valid? */
      if(pstFXPointer != orxNULL)
      {
        /* Links it */
        eResult = orxObject_LinkStructure(_pstObject, orxSTRUCTURE(pstFXPointer));

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Updates flags */
          orxFLAG_SET(_pstObject->astStructureList[orxSTRUCTURE_ID_FXPOINTER].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

          /* Updates its owner */
          orxStructure_SetOwner(pstFXPointer, _pstObject);

          /* Adds FX from config */
          eResult = orxFXPointer_AddDelayedFXFromConfig(pstFXPointer, _zFXConfigID, _fDelay);
        }
        else
        {
          /* Deletes it */
          orxFXPointer_Delete(pstFXPointer);
        }
      }
    }
    else
    {
      /* Adds FX from config */
      eResult = orxFXPointer_AddDelayedFXFromConfig(pstFXPointer, _zFXConfigID, _fDelay);
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a delayed FX to an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 * @param[in]   _fDelay         Delay time
 * @param[in]   _bPropagate     Should the delay be incremented with each child application?
 */
void orxFASTCALL orxObject_AddDelayedFXRecursive(orxOBJECT *_pstObject, const orxSTRING _zFXConfigID, orxFLOAT _fDelay, orxBOOL _bPropagate)
{
  orxFLOAT    fDelay;
  orxOBJECT  *pstChild;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Sets initial delay */
  fDelay = _fDelay;

  /* Updates object */
  orxObject_AddDelayedFX(_pstObject, _zFXConfigID, fDelay);

  /* For all its children */
  for(pstChild = orxObject_GetOwnedChild(_pstObject);
      pstChild != orxNULL;
      pstChild = orxObject_GetOwnedSibling(pstChild))
  {
    /* Should propagate? */
    if(_bPropagate != orxFALSE)
    {
      /* Updates delay */
      fDelay += _fDelay;
    }

    /* Updates it */
    orxObject_AddDelayedFXRecursive(pstChild, _zFXConfigID, fDelay, _bPropagate);
  }

  /* Done! */
  return;
}

/** Adds a unique delayed FX using its config ID. The difference between this function and orxObject_AddDelayedFX()
 * is that this one does not add the specified FX, if the object already has an FX with the same config ID attached.
 * note that the "uniqueness" is determined immediately at the time of this function call, not at the time of the
 * FX start (i.e. after the delay).
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 * @param[in]   _fDelay         Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_AddUniqueDelayedFX(orxOBJECT *_pstObject, const orxSTRING _zFXConfigID, orxFLOAT _fDelay)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));
  orxASSERT(_fDelay >= orxFLOAT_0);

  /* Is object active? */
  if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
  {
    orxFXPOINTER *pstFXPointer;

    /* Gets its FXPointer */
    pstFXPointer = orxOBJECT_GET_STRUCTURE(_pstObject, FXPOINTER);

    /* Doesn't exist? */
    if(pstFXPointer == orxNULL)
    {
      /* Creates one */
      pstFXPointer = orxFXPointer_Create();

      /* Valid? */
      if(pstFXPointer != orxNULL)
      {
        /* Links it */
        eResult = orxObject_LinkStructure(_pstObject, orxSTRUCTURE(pstFXPointer));

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Updates flags */
          orxFLAG_SET(_pstObject->astStructureList[orxSTRUCTURE_ID_FXPOINTER].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

          /* Updates its owner */
          orxStructure_SetOwner(pstFXPointer, _pstObject);

          /* Adds FX from config */
          eResult = orxFXPointer_AddUniqueDelayedFXFromConfig(pstFXPointer, _zFXConfigID, _fDelay);
        }
        else
        {
          /* Deletes it */
          orxFXPointer_Delete(pstFXPointer);
        }
      }
    }
    else
    {
      /* Adds FX from config */
      eResult = orxFXPointer_AddUniqueDelayedFXFromConfig(pstFXPointer, _zFXConfigID, _fDelay);
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a unique delayed FX to an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to add
 * @param[in]   _fDelay         Delay time
 * @param[in]   _bPropagate    Should the delay be incremented with each child application?
 */
void orxFASTCALL orxObject_AddUniqueDelayedFXRecursive(orxOBJECT *_pstObject, const orxSTRING _zFXConfigID, orxFLOAT _fDelay, orxBOOL _bPropagate)
{
  orxFLOAT    fDelay;
  orxOBJECT  *pstChild;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Sets initial delay */
  fDelay = _fDelay;

  /* Updates object */
  orxObject_AddUniqueDelayedFX(_pstObject, _zFXConfigID, fDelay);

  /* For all its children */
  for(pstChild = orxObject_GetOwnedChild(_pstObject);
      pstChild != orxNULL;
      pstChild = orxObject_GetOwnedSibling(pstChild))
  {
    /* Should propagate? */
    if(_bPropagate != orxFALSE)
    {
      /* Updates delay */
      fDelay += _fDelay;
    }

    /* Updates it */
    orxObject_AddUniqueDelayedFXRecursive(pstChild, _zFXConfigID, fDelay, _bPropagate);
  }

  /* Done! */
  return;
}

/** Removes an FX using its config ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_RemoveFX(orxOBJECT *_pstObject, const orxSTRING _zFXConfigID)
{
  orxFXPOINTER *pstFXPointer;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its FXPointer */
  pstFXPointer = orxOBJECT_GET_STRUCTURE(_pstObject, FXPOINTER);

  /* Valid? */
  if(pstFXPointer != orxNULL)
  {
    /* Removes FX from config */
    eResult = orxFXPointer_RemoveFXFromConfig(pstFXPointer, _zFXConfigID);
  }

  /* Done! */
  return eResult;
}

/** Removes an FX from an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zFXConfigID    Config ID of the FX to remove
 */
orxOBJECT_MAKE_RECURSIVE(RemoveFX, const orxSTRING);

/** Removes all FXs.
 * @param[in]   _pstObject      Concerned object
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_RemoveAllFXs(orxOBJECT *_pstObject)
{
  orxFXPOINTER *pstFXPointer;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its FXPointer */
  pstFXPointer = orxOBJECT_GET_STRUCTURE(_pstObject, FXPOINTER);

  /* Valid? */
  if(pstFXPointer != orxNULL)
  {
    /* Removes all its FXs */
    eResult = orxFXPointer_RemoveAllFXs(pstFXPointer);
  }

  /* Done! */
  return eResult;
}

/** Synchronizes FXs with another object's ones (if FXs are not matching on both objects the behavior is undefined).
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstModel       Model object on which to synchronize FXs
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SynchronizeFX(orxOBJECT *_pstObject, const orxOBJECT *_pstModel)
{
  orxFXPOINTER *pstFXPointer;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxSTRUCTURE_ASSERT(_pstModel);

  /* Gets FX pointer */
  pstFXPointer = orxOBJECT_GET_STRUCTURE(_pstObject, FXPOINTER);

  /* Valid? */
  if(pstFXPointer != orxNULL)
  {
    const orxFXPOINTER *pstModelFXPointer;

    /* Gets model FX pointer */
    pstModelFXPointer = orxOBJECT_GET_STRUCTURE(_pstModel, FXPOINTER);

    /* Valid? */
    if(pstModelFXPointer != orxNULL)
    {
      /* Synchronizes them */
      eResult = orxFXPointer_Synchronize(pstFXPointer, pstModelFXPointer);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Can't synchronize FX for object <%s> with model object <%s>: no FXs found on model.", orxObject_GetName(_pstObject), orxObject_GetName(_pstModel));
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Can't synchronize FX for object <%s>: no FXs found.", orxObject_GetName(_pstObject));
  }

  /* Done! */
  return eResult;
}

/** Adds a sound using its config ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zSoundConfigID Config ID of the sound to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_AddSound(orxOBJECT *_pstObject, const orxSTRING _zSoundConfigID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zSoundConfigID != orxNULL) && (_zSoundConfigID != orxSTRING_EMPTY));

  /* Is object active? */
  if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
  {
    orxSOUNDPOINTER *pstSoundPointer;

    /* Gets its SoundPointer */
    pstSoundPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SOUNDPOINTER);

    /* Doesn't exist? */
    if(pstSoundPointer == orxNULL)
    {
      /* Creates one */
      pstSoundPointer = orxSoundPointer_Create();

      /* Valid? */
      if(pstSoundPointer != orxNULL)
      {
        /* Links it */
        eResult = orxObject_LinkStructure(_pstObject, orxSTRUCTURE(pstSoundPointer));

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Updates flags */
          orxFLAG_SET(_pstObject->astStructureList[orxSTRUCTURE_ID_SOUNDPOINTER].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

          /* Updates its owner */
          orxStructure_SetOwner(pstSoundPointer, _pstObject);

          /* Adds sound from config */
          eResult = orxSoundPointer_AddSoundFromConfig(pstSoundPointer, _zSoundConfigID);
        }
        else
        {
          /* Deletes it */
          orxSoundPointer_Delete(pstSoundPointer);
        }
      }
    }
    else
    {
      /* Adds sound from config */
      eResult = orxSoundPointer_AddSoundFromConfig(pstSoundPointer, _zSoundConfigID);
    }
  }

  /* Done! */
  return eResult;
}

/** Removes a sound using its config ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zSoundConfigID Config ID of the sound to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_RemoveSound(orxOBJECT *_pstObject, const orxSTRING _zSoundConfigID)
{
  orxSOUNDPOINTER  *pstSoundPointer;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its SoundPointer */
  pstSoundPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SOUNDPOINTER);

  /* Valid? */
  if(pstSoundPointer != orxNULL)
  {
    /* Removes sound from config */
    eResult = orxSoundPointer_RemoveSoundFromConfig(pstSoundPointer, _zSoundConfigID);
  }

  /* Done! */
  return eResult;
}

/** Gets last added sound (Do *NOT* destroy it directly before removing it!!!).
 * @param[in]   _pstObject      Concerned object
 * @return      orxSOUND / orxNULL
 */
orxSOUND *orxFASTCALL orxObject_GetLastAddedSound(const orxOBJECT *_pstObject)
{
  orxSOUNDPOINTER  *pstSoundPointer;
  orxSOUND         *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its SoundPointer */
  pstSoundPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SOUNDPOINTER);

  /* Valid? */
  if(pstSoundPointer != orxNULL)
  {
    /* Updates result */
    pstResult = orxSoundPointer_GetLastAddedSound(pstSoundPointer);
  }

  /* Done! */
  return pstResult;
}

/** Sets volume for all sounds of an object.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fVolume        Desired volume (0.0 - 1.0)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetVolume(orxOBJECT *_pstObject, orxFLOAT _fVolume)
{
  orxSOUNDPOINTER  *pstSoundPointer;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its SoundPointer */
  pstSoundPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SOUNDPOINTER);

  /* Valid? */
  if(pstSoundPointer != orxNULL)
  {
    /* Set volume to all sounds */
    eResult = orxSoundPointer_SetVolume(pstSoundPointer, _fVolume);
  }

  /* Done! */
  return eResult;
}

/** Sets pitch for all sounds of an object.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fVolume        Desired pitch (0.0 - 1.0)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetPitch(orxOBJECT *_pstObject, orxFLOAT _fPitch)
{
  orxSOUNDPOINTER  *pstSoundPointer;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its SoundPointer */
  pstSoundPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SOUNDPOINTER);

  /* Valid? */
  if(pstSoundPointer != orxNULL)
  {
    /* Set pitch to all sounds */
    eResult = orxSoundPointer_SetPitch(pstSoundPointer, _fPitch);
  }

  /* Done! */
  return eResult;
}

/** Plays all the sounds of an object.
 * @param[in]   _pstObject      Concerned object
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_Play(orxOBJECT *_pstObject)
{
  orxSOUNDPOINTER  *pstSoundPointer;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its SoundPointer */
  pstSoundPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SOUNDPOINTER);

  /* Valid? */
  if(pstSoundPointer != orxNULL)
  {
    /* Plays all the sounds */
    eResult = orxSoundPointer_Play(pstSoundPointer);
  }

  /* Done! */
  return eResult;
}

/** Stops all the sounds of an object.
 * @param[in]   _pstObject      Concerned object
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_Stop(orxOBJECT *_pstObject)
{
  orxSOUNDPOINTER  *pstSoundPointer;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its SoundPointer */
  pstSoundPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SOUNDPOINTER);

  /* Valid? */
  if(pstSoundPointer != orxNULL)
  {
    /* Stops all the sounds */
    eResult = orxSoundPointer_Stop(pstSoundPointer);
  }

  /* Done! */
  return eResult;
}

/** Adds a shader to an object using its config ID.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _zShaderConfigID  Config ID of the shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_AddShader(orxOBJECT *_pstObject, const orxSTRING _zShaderConfigID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zShaderConfigID != orxNULL) && (_zShaderConfigID != orxSTRING_EMPTY));

  /* Is object active? */
  if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
  {
    orxSHADERPOINTER *pstShaderPointer;

    /* Gets its ShaderPointer */
    pstShaderPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SHADERPOINTER);

    /* Doesn't exist? */
    if(pstShaderPointer == orxNULL)
    {
      /* Creates one */
      pstShaderPointer = orxShaderPointer_Create();

      /* Valid? */
      if(pstShaderPointer != orxNULL)
      {
        /* Links it */
        eResult = orxObject_LinkStructure(_pstObject, orxSTRUCTURE(pstShaderPointer));

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Updates flags */
          orxFLAG_SET(_pstObject->astStructureList[orxSTRUCTURE_ID_SHADERPOINTER].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

          /* Updates its owner */
          orxStructure_SetOwner(pstShaderPointer, _pstObject);

          /* Adds shader from config */
          eResult = orxShaderPointer_AddShaderFromConfig(pstShaderPointer, _zShaderConfigID);
        }
        else
        {
          /* Deletes it */
          orxShaderPointer_Delete(pstShaderPointer);
        }
      }
    }
    else
    {
      /* Adds shader from config */
      eResult = orxShaderPointer_AddShaderFromConfig(pstShaderPointer, _zShaderConfigID);
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a shader to an object and its children.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _zShaderConfigID  Config ID of the shader to add
 */
orxOBJECT_MAKE_RECURSIVE(AddShader, const orxSTRING);

/** Removes a shader using its config ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zShaderConfigID Config ID of the shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_RemoveShader(orxOBJECT *_pstObject, const orxSTRING _zShaderConfigID)
{
  orxSHADERPOINTER *pstShaderPointer;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its ShaderPointer */
  pstShaderPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SHADERPOINTER);

  /* Valid? */
  if(pstShaderPointer != orxNULL)
  {
    /* Removes shader from config */
    eResult = orxShaderPointer_RemoveShaderFromConfig(pstShaderPointer, _zShaderConfigID);
  }

  /* Done! */
  return eResult;
}

/** Removes a shader from an object and its children.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _zShaderConfigID  Config ID of the shader to remove
 */
orxOBJECT_MAKE_RECURSIVE(RemoveShader, const orxSTRING);

/** Enables an object's shader.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _bEnable          Enable / disable
 */
void orxFASTCALL orxObject_EnableShader(orxOBJECT *_pstObject, orxBOOL _bEnable)
{
  orxSHADERPOINTER *pstShaderPointer;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its ShaderPointer */
  pstShaderPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SHADERPOINTER);

  /* Valid? */
  if(pstShaderPointer != orxNULL)
  {
    /* Enables it */
    orxShaderPointer_Enable(pstShaderPointer, _bEnable);
  }

  /* Done! */
  return;
}

/** Is an object's shader enabled?
 * @param[in]   _pstObject        Concerned object
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxObject_IsShaderEnabled(const orxOBJECT *_pstObject)
{
  orxSHADERPOINTER *pstShaderPointer;
  orxBOOL           bResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its ShaderPointer */
  pstShaderPointer = orxOBJECT_GET_STRUCTURE(_pstObject, SHADERPOINTER);

  /* Valid? */
  if(pstShaderPointer != orxNULL)
  {
    /* Updates result */
    bResult = orxShaderPointer_IsEnabled(pstShaderPointer);
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Adds a timeline track to an object using its config ID.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _zTrackConfigID   Config ID of the timeline track to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_AddTimeLineTrack(orxOBJECT *_pstObject, const orxSTRING _zTrackConfigID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_zTrackConfigID != orxNULL) && (_zTrackConfigID != orxSTRING_EMPTY));

  /* Is object active? */
  if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
  {
    orxTIMELINE *pstTimeLine;

    /* Gets its TimeLine */
    pstTimeLine = orxOBJECT_GET_STRUCTURE(_pstObject, TIMELINE);

    /* Doesn't exist? */
    if(pstTimeLine == orxNULL)
    {
      /* Creates one */
      pstTimeLine = orxTimeLine_Create();

      /* Valid? */
      if(pstTimeLine != orxNULL)
      {
        /* Links it */
        eResult = orxObject_LinkStructure(_pstObject, orxSTRUCTURE(pstTimeLine));

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Updates flags */
          orxFLAG_SET(_pstObject->astStructureList[orxSTRUCTURE_ID_TIMELINE].u32Flags, orxOBJECT_KU32_STORAGE_FLAG_INTERNAL, orxOBJECT_KU32_STORAGE_MASK_ALL);

          /* Updates its owner */
          orxStructure_SetOwner(pstTimeLine, _pstObject);

          /* Adds timeline track from config */
          eResult = orxTimeLine_AddTrackFromConfig(pstTimeLine, _zTrackConfigID);
        }
        else
        {
          /* Deletes it */
          orxTimeLine_Delete(pstTimeLine);
        }
      }
    }
    else
    {
      /* Adds timeline track from config */
      eResult = orxTimeLine_AddTrackFromConfig(pstTimeLine, _zTrackConfigID);
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a timeline track to an object and its children.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _zTrackConfigID   Config ID of the timeline track to add
 */
orxOBJECT_MAKE_RECURSIVE(AddTimeLineTrack, const orxSTRING);

/** Removes a timeline track using its config ID.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _zTrackConfigID Config ID of the timeline track to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_RemoveTimeLineTrack(orxOBJECT *_pstObject, const orxSTRING _zTrackConfigID)
{
  orxTIMELINE  *pstTimeLine;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its TimeLine */
  pstTimeLine = orxOBJECT_GET_STRUCTURE(_pstObject, TIMELINE);

  /* Valid? */
  if(pstTimeLine != orxNULL)
  {
    /* Removes timeline track from config */
    eResult = orxTimeLine_RemoveTrackFromConfig(pstTimeLine, _zTrackConfigID);
  }

  /* Done! */
  return eResult;
}

/** Removes a timeline track from an object and its children.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _zTrackConfigID   Config ID of the timeline track to remove
 */
orxOBJECT_MAKE_RECURSIVE(RemoveTimeLineTrack, const orxSTRING);

/** Enables an object's timeline.
 * @param[in]   _pstObject        Concerned object
 * @param[in]   _bEnable          Enable / disable
 */
void orxFASTCALL orxObject_EnableTimeLine(orxOBJECT *_pstObject, orxBOOL _bEnable)
{
  orxTIMELINE *pstTimeLine;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its TimeLine */
  pstTimeLine = orxOBJECT_GET_STRUCTURE(_pstObject, TIMELINE);

  /* Valid? */
  if(pstTimeLine != orxNULL)
  {
    /* Enables it */
    orxTimeLine_Enable(pstTimeLine, _bEnable);
  }

  /* Done! */
  return;
}

/** Is an object's timeline enabled?
 * @param[in]   _pstObject        Concerned object
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxObject_IsTimeLineEnabled(const orxOBJECT *_pstObject)
{
  orxTIMELINE  *pstTimeLine;
  orxBOOL       bResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its TimeLine */
  pstTimeLine = orxOBJECT_GET_STRUCTURE(_pstObject, TIMELINE);

  /* Valid? */
  if(pstTimeLine != orxNULL)
  {
    /* Updates result */
    bResult = orxTimeLine_IsEnabled(pstTimeLine);
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Gets object config name.
 * @param[in]   _pstObject      Concerned object
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxObject_GetName(const orxOBJECT *_pstObject)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Updates result */
  zResult = (_pstObject->zReference != orxNULL) ? _pstObject->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Creates a list of object at neighboring of the given box (ie. whose bounding volume intersects this box).
 * The following is an example for iterating over a neighbor list:
 * @code
 * orxVECTOR vPosition; // The world position of the neighborhood area
 * // set_position(vPosition);
 * orxVECTOR vSize; // The size of the neighborhood area
 * // set_size(vSize);
 * orxVECTOR vPivot; // The pivot of the neighborhood area
 * // set_pivot(vPivot);
 *
 * orxOBOX stBox;
 * orxOBox_2DSet(&stBox, &vPosition, &vPivot, &vSize, 0);
 *
 * orxBANK * pstBank = orxObject_CreateNeighborList(&stBox, orxU32_UNDEFINED);
 * if(pstBank) {
 *     for(int i=0; i < orxBank_GetCount(pstBank); ++i)
 *     {
 *         orxOBJECT * pstObject = *((orxOBJECT **) orxBank_GetAtIndex(pstBank, i));
 *         do_something_with(pstObject);
 *     }
 *     orxObject_DeleteNeighborList(pstBank);
 * }
 * @endcode
 * @param[in]   _pstCheckBox    Box to check intersection with
 * @param[in]   _stGroupID      Group ID to consider, orxSTRINGID_UNDEFINED for all
 * @return      orxBANK / orxNULL
 */
orxBANK *orxFASTCALL orxObject_CreateNeighborList(const orxOBOX *_pstCheckBox, orxSTRINGID _stGroupID)
{
  orxOBOX    stObjectBox;
  orxOBJECT  *pstObject;
  orxBANK    *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstCheckBox != orxNULL);

  /* Creates bank */
  pstResult = orxBank_Create(orxOBJECT_KU32_NEIGHBOR_LIST_SIZE, sizeof(orxOBJECT *), orxBANK_KU32_FLAG_NOT_EXPANDABLE, orxMEMORY_TYPE_TEMP);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    orxU32 u32Count;

    /* For all objects */
    for(u32Count = 0, pstObject = orxObject_GetNext(orxNULL, _stGroupID);
        (u32Count < orxOBJECT_KU32_NEIGHBOR_LIST_SIZE) && (pstObject != orxNULL);
        pstObject = orxObject_GetNext(pstObject, _stGroupID))
    {
      /* Gets its bounding box */
      if(orxObject_GetBoundingBox(pstObject, &stObjectBox) != orxNULL)
      {
        /* Is intersecting? */
        if(orxOBox_ZAlignedTestIntersection(_pstCheckBox, &stObjectBox) != orxFALSE)
        {
          orxOBJECT **ppstObject;

          /* Creates a new cell */
          ppstObject = (orxOBJECT **)orxBank_Allocate(pstResult);

          /* Valid? */
          if(ppstObject != orxNULL)
          {
            /* Adds object */
            *ppstObject = pstObject;

            /* Updates count */
            u32Count++;
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to allocate new object neighbor cell.");
            break;
          }
        }
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes an object list created with orxObject_CreateNeigborList.
 * @param[in]   _astObjectList  Concerned object list
 */
void orxFASTCALL orxObject_DeleteNeighborList(orxBANK *_pstObjectList)
{
  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);

  /* Non null? */
  if(_pstObjectList != orxNULL)
  {
    /* Deletes it */
    orxBank_Delete(_pstObjectList);
  }
}

/** Sets object smoothing.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eSmoothing     Smoothing type (enabled, default or none)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetSmoothing(orxOBJECT *_pstObject, orxDISPLAY_SMOOTHING _eSmoothing)
{
  orxU32    u32Flags;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Depending on smoothing type */
  switch(_eSmoothing)
  {
    case orxDISPLAY_SMOOTHING_ON:
    {
      /* Updates flags */
      u32Flags = orxOBJECT_KU32_FLAG_SMOOTHING_ON;

      break;
    }

    case orxDISPLAY_SMOOTHING_OFF:
    {
      /* Updates flags */
      u32Flags = orxOBJECT_KU32_FLAG_SMOOTHING_OFF;

      break;
    }

    default:
    case orxDISPLAY_SMOOTHING_DEFAULT:
    {
      /* Updates flags */
      u32Flags = orxOBJECT_KU32_FLAG_NONE;

      break;
    }
  }

  /* Updates status */
  orxStructure_SetFlags(_pstObject, u32Flags, orxOBJECT_KU32_FLAG_SMOOTHING_ON | orxOBJECT_KU32_FLAG_SMOOTHING_OFF);

  /* Done! */
  return eResult;
}

/** Sets smoothing for an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eSmoothing     Smoothing type (enabled, default or none)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxOBJECT_MAKE_RECURSIVE(SetSmoothing, orxDISPLAY_SMOOTHING);

/** Gets object smoothing.
 * @param[in]   _pstObject     Concerned object
 * @return Smoothing type (enabled, default or none)
 */
orxDISPLAY_SMOOTHING orxFASTCALL orxObject_GetSmoothing(const orxOBJECT *_pstObject)
{
  orxDISPLAY_SMOOTHING eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Updates result */
  eResult = orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_SMOOTHING_ON)
            ? orxDISPLAY_SMOOTHING_ON
            : orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_SMOOTHING_OFF)
              ? orxDISPLAY_SMOOTHING_OFF
              : orxDISPLAY_SMOOTHING_DEFAULT;

  /* Done! */
  return eResult;
}

/** Gets object working texture.
 * @param[in]   _pstObject     Concerned object
 * @return orxTEXTURE / orxNULL
 */
orxTEXTURE *orxFASTCALL orxObject_GetWorkingTexture(const orxOBJECT *_pstObject)
{
  orxGRAPHIC *pstGraphic;
  orxTEXTURE *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets its working graphic */
  pstGraphic = orxObject_GetWorkingGraphic(_pstObject);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Text? */
    if(orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT))
    {
      /* Updates result */
      pstResult = orxFont_GetTexture(orxText_GetFont(orxTEXT(orxGraphic_GetData(pstGraphic))));
    }
    else
    {
      /* Updates result */
      pstResult = orxTEXTURE(orxGraphic_GetData(pstGraphic));
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets object working graphic.
 * @param[in]   _pstObject     Concerned object
 * @return orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxFASTCALL orxObject_GetWorkingGraphic(const orxOBJECT *_pstObject)
{
  orxANIMPOINTER *pstAnimPointer;
  orxGRAPHIC     *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets animation pointer */
  pstAnimPointer = orxOBJECT_GET_STRUCTURE(_pstObject, ANIMPOINTER);

  /* Invalid? */
  if((pstAnimPointer == orxNULL)
  || ((pstResult = orxGRAPHIC(orxAnimPointer_GetCurrentAnimData(pstAnimPointer))) == orxNULL))
  {
    /* Gets its graphic */
    pstResult = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstObject), GRAPHIC);
  }

  /* Done! */
  return pstResult;
}

/** Sets object color.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstColor       Color to set, orxNULL to remove any specific color
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetColor(orxOBJECT *_pstObject, const orxCOLOR *_pstColor)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Sets its color */
    eResult = (_pstColor != orxNULL) ? orxGraphic_SetColor(pstGraphic, _pstColor) : orxGraphic_ClearColor(pstGraphic);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No graphic on object <%s>, can't set color.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets color of an object and all its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pstColor       Color to set, orxNULL to remove any specific color
 */
orxOBJECT_MAKE_RECURSIVE(SetColor, const orxCOLOR *);

/** Object has color accessor?
 * @param[in]   _pstObject      Concerned object
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxObject_HasColor(const orxOBJECT *_pstObject)
{
  orxGRAPHIC *pstGraphic;
  orxBOOL     bResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Has color? */
    bResult = orxGraphic_HasColor(pstGraphic);
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Gets object color.
 * @param[in]   _pstObject      Concerned object
 * @param[out]  _pstColor       Object's color
 * @return      orxCOLOR / orxNULL
 */
orxCOLOR *orxFASTCALL orxObject_GetColor(const orxOBJECT *_pstObject, orxCOLOR *_pstColor)
{
  orxGRAPHIC *pstGraphic;
  orxCOLOR   *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets its color */
    pstResult = orxGraphic_GetColor(pstGraphic, _pstColor);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No graphic on object <%s>, can't get color.", orxObject_GetName(_pstObject));

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Sets object RGB values.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvRGB          RGB values to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetRGB(orxOBJECT *_pstObject, const orxVECTOR *_pvRGB)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pvRGB != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    orxCOLOR stColor;

    /* Has color? */
    if(orxGraphic_HasColor(pstGraphic) != orxFALSE)
    {
      /* Gets it */
      orxGraphic_GetColor(pstGraphic, &stColor);
    }
    else
    {
      /* Sets its alpha to opaque */
      stColor.fAlpha = orxFLOAT_1;
    }

    /* Updates its RGB values */
    orxVector_Copy(&(stColor.vRGB), _pvRGB);

    /* Sets graphic's color */
    eResult = orxGraphic_SetColor(pstGraphic, &stColor);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No graphic on object <%s>, can't set RGB values.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets color of an object and all its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _pvRGB          RGB values to set
 */
orxOBJECT_MAKE_RECURSIVE(SetRGB, const orxVECTOR *);

/** Sets object alpha.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fAlpha         Alpha value to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetAlpha(orxOBJECT *_pstObject, orxFLOAT _fAlpha)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    orxCOLOR stColor;

    /* Has color? */
    if(orxGraphic_HasColor(pstGraphic) != orxFALSE)
    {
      /* Gets it */
      orxGraphic_GetColor(pstGraphic, &stColor);
    }
    else
    {
      /* Sets its color to white */
      orxVector_Copy(&(stColor.vRGB), &orxVECTOR_WHITE);
    }

    /* Updates its alpha */
    stColor.fAlpha = _fAlpha;

    /* Sets graphic's color */
    eResult = orxGraphic_SetColor(pstGraphic, &stColor);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No graphic on object <%s>, can't set alpha values.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets alpha of an object and all its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fAlpha         Alpha value to set
 */
orxOBJECT_MAKE_RECURSIVE(SetAlpha, orxFLOAT);

/** Sets object repeat (wrap) values.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fRepeatX       X-axis repeat value
 * @param[in]   _fRepeatY       Y-axis repeat value
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetRepeat(orxOBJECT *_pstObject, orxFLOAT _fRepeatX, orxFLOAT _fRepeatY)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Sets its repeat */
    eResult = orxGraphic_SetRepeat(pstGraphic, _fRepeatX, _fRepeatY);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No graphic on object <%s>, can't set repeat.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets object repeat (wrap) values.
 * @param[in]   _pstObject     Concerned object
 * @param[out]  _pfRepeatX      X-axis repeat value
 * @param[out]  _pfRepeatY      Y-axis repeat value
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_GetRepeat(const orxOBJECT *_pstObject, orxFLOAT *_pfRepeatX, orxFLOAT *_pfRepeatY)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_pfRepeatX != orxNULL);
  orxASSERT(_pfRepeatY != orxNULL);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets its repeat */
    eResult = orxGraphic_GetRepeat(pstGraphic, _pfRepeatX, _pfRepeatY);
  }
  else
  {
    /* Updates result */
    *_pfRepeatX = *_pfRepeatY = orxFLOAT_1;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets object blend mode.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eBlendMode     Blend mode (alpha, multiply, add or none), orxDISPLAY_BLEND_MODE_NUMBER to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetBlendMode(orxOBJECT *_pstObject, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Sets its blend mode */
    eResult = (_eBlendMode != orxDISPLAY_BLEND_MODE_NUMBER) ? orxGraphic_SetBlendMode(pstGraphic, _eBlendMode) : orxGraphic_ClearBlendMode(pstGraphic);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No graphic on object <%s>, can't set blend mode.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets blend mode of an object and its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _eBlendMode     Blend mode (alpha, multiply, add or none)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxOBJECT_MAKE_RECURSIVE(SetBlendMode, orxDISPLAY_BLEND_MODE);

/** Object has blend mode accessor?
 * @param[in]   _pstObject      Concerned object
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxObject_HasBlendMode(const orxOBJECT *_pstObject)
{
  orxGRAPHIC *pstGraphic;
  orxBOOL     bResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Has blend mode? */
    bResult = orxGraphic_HasBlendMode(pstGraphic);
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Gets object blend mode.
 * @param[in]   _pstObject     Concerned object
 * @return Blend mode (alpha, multiply, add or none)
 */
orxDISPLAY_BLEND_MODE orxFASTCALL orxObject_GetBlendMode(const orxOBJECT *_pstObject)
{
  orxGRAPHIC           *pstGraphic;
  orxDISPLAY_BLEND_MODE eResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Gets graphic */
  pstGraphic = orxOBJECT_GET_STRUCTURE(_pstObject, GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Gets its repeat */
    eResult = orxGraphic_GetBlendMode(pstGraphic);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No graphic on object <%s>, can't get blend mode.", orxObject_GetName(_pstObject));

    /* Updates result */
    eResult = orxDISPLAY_BLEND_MODE_NONE;
  }

  /* Done! */
  return eResult;
}

/** Sets object lifetime.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _fLifeTime      Lifetime to set, negative value to disable it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxObject_SetLifeTime(orxOBJECT *_pstObject, orxFLOAT _fLifeTime)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Is valid? */
  if(_fLifeTime >= orxFLOAT_0)
  {
    /* Stores it */
    _pstObject->fLifeTime = _fLifeTime;

    /* Updates status */
    orxStructure_SetFlags(_pstObject, (_fLifeTime == orxFLOAT_0) ? orxOBJECT_KU32_FLAG_HAS_LIFETIME | orxOBJECT_KU32_FLAG_DEATH_ROW : orxOBJECT_KU32_FLAG_HAS_LIFETIME, orxOBJECT_KU32_FLAG_DEATH_ROW);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstObject, orxOBJECT_KU32_FLAG_NONE, orxOBJECT_KU32_FLAG_HAS_LIFETIME | orxOBJECT_KU32_FLAG_DEATH_ROW);
  }

  /* Should die? */
  if(_fLifeTime == orxFLOAT_0)
  {
    /* Isn't already part of the enable list? */
    if(orxLinkList_GetList(&(_pstObject->stEnableNode)) == orxNULL)
    {
      orxOBJECT_LISTS *pstGroupLists;

      /* Adds it to enable list */
      orxLinkList_AddEnd(&(sstObject.stEnableList), &(_pstObject->stEnableNode));

      /* Is cached group list? */
      if(_pstObject->stGroupID == sstObject.stCachedGroupID)
      {
        /* Gets it */
        pstGroupLists = sstObject.pstCachedGroupLists;
      }
      else
      {
        /* Gets group list */
        pstGroupLists = (orxOBJECT_LISTS *)orxHashTable_Get(sstObject.pstGroupTable, _pstObject->stGroupID);

        /* Checks */
        orxASSERT(pstGroupLists != orxNULL);

        /* Caches it */
        sstObject.pstCachedGroupLists = pstGroupLists;
        sstObject.stCachedGroupID     = _pstObject->stGroupID;
      }

      /* Adds object to enable group list */
      orxLinkList_AddEnd(&(pstGroupLists->stEnableList), &(_pstObject->stEnableGroupNode));
    }
  }
  else
  {
    /* Is not enabled and part of the enable lists? */
    if(!orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED)
    && (orxLinkList_GetList(&(_pstObject->stEnableNode)) != orxNULL))
    {
      /* Removes it from enable lists */
      orxLinkList_Remove(&(_pstObject->stEnableNode));
      orxLinkList_Remove(&(_pstObject->stEnableGroupNode));
    }
  }

  /* Done! */
  return eResult;
}

/** Gets object lifetime.
 * @param[in]   _pstObject      Concerned object
 * @return      Lifetime / negative value if none
 */
orxFLOAT orxFASTCALL orxObject_GetLifeTime(const orxOBJECT *_pstObject)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Updates result */
  fResult = orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_HAS_LIFETIME) ? _pstObject->fLifeTime : orx2F(-1.0f);

  /* Done! */
  return fResult;
}

/** Gets object active time, i.e. the amount of time that the object has been alive taking into account
 * the object's clock multiplier and object's periods of pause.
 * @param[in]   _pstObject      Concerned object
 * @return      Active time
 */
orxFLOAT orxFASTCALL orxObject_GetActiveTime(const orxOBJECT *_pstObject)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Updates result */
  fResult = _pstObject->fActiveTime;

  /* Done! */
  return fResult;
}

/** Gets default group ID.
 * @return      Default group ID
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL orxObject_GetDefaultGroupID()
{
  orxSTRINGID stResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);

  /* Updates result */
  stResult = sstObject.stDefaultGroupID;

  /* Done! */
  return stResult;
}

/** Gets object's group ID
 * @param[in]   _pstObject      Concerned object
 * @return      Object's group ID. This is the string ID (see orxString_GetFromID()) of the object's group name.
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL orxObject_GetGroupID(const orxOBJECT *_pstObject)
{
  orxSTRINGID stResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);

  /* Updates result */
  stResult = _pstObject->stGroupID;

  /* Done! */
  return stResult;
}

/** Sets object's group ID
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _stGroupID      Group ID to set. This is the string ID (see orxString_GetID()) of the object's group name.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL orxObject_SetGroupID(orxOBJECT *_pstObject, orxSTRINGID _stGroupID)
{
  orxOBJECT_LISTS **ppstBucket;
  orxOBJECT_LISTS  *pstGroupLists;
  orxSTATUS               eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT((_stGroupID != 0) && (_stGroupID != orxSTRINGID_UNDEFINED));

  /* Removes object from its current group */
  if(orxLinkList_GetList(&(_pstObject->stGroupNode)) != orxNULL)
  {
    orxLinkList_Remove(&(_pstObject->stGroupNode));
  }
  /* Removes object from its current enable group */
  if(orxLinkList_GetList(&(_pstObject->stEnableGroupNode)) != orxNULL)
  {
    orxLinkList_Remove(&(_pstObject->stEnableGroupNode));
  }

  /* Gets group list bucker*/
  ppstBucket = (orxOBJECT_LISTS **)orxHashTable_Retrieve(sstObject.pstGroupTable, _stGroupID);

  /* Checks */
  orxASSERT(ppstBucket != orxNULL);

  /* Not found? */
  if(*ppstBucket == orxNULL)
  {
    /* Allocates it */
    pstGroupLists = (orxOBJECT_LISTS *)orxBank_Allocate(sstObject.pstGroupBank);

    /* Checks */
    orxASSERT(pstGroupLists != orxNULL);

    /* Clears it */
    orxMemory_Zero(pstGroupLists, sizeof(orxOBJECT_LISTS));

    /* Stores it */
    *ppstBucket = pstGroupLists;
  }
  else
  {
    /* Gets it */
    pstGroupLists = *ppstBucket;
  }

  /* Adds object to end of group list */
  orxLinkList_AddEnd(&(pstGroupLists->stList), &(_pstObject->stGroupNode));

  /* Is enabled? */
  if(orxStructure_TestFlags(_pstObject, orxOBJECT_KU32_FLAG_ENABLED))
  {
    /* Adds object to end of enable group list */
    orxLinkList_AddEnd(&(pstGroupLists->stEnableList), &(_pstObject->stEnableGroupNode));
  }

  /* Stores group ID */
  _pstObject->stGroupID = _stGroupID;

  /* Done! */
  return eResult;
}

/** Sets group ID of an object and all its children.
 * @param[in]   _pstObject      Concerned object
 * @param[in]   _stGroupID      Group ID to set. This is the string ID (see orxString_GetID()) of the object's group name.
 */
orxOBJECT_MAKE_RECURSIVE(SetGroupID, orxSTRINGID);

/** Gets next object in group.
 * @param[in]   _pstObject      Concerned object, orxNULL to get the first one
 * @param[in]   _stGroupID      Group ID to consider, orxSTRINGID_UNDEFINED for all
 * @return      orxOBJECT / orxNULL
 */
extern orxDLLAPI orxOBJECT *orxFASTCALL orxObject_GetNext(const orxOBJECT *_pstObject, orxSTRINGID _stGroupID)
{
  orxOBJECT *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstObject == orxNULL) || (orxStructure_GetID((orxSTRUCTURE *)_pstObject) < orxSTRUCTURE_ID_NUMBER));
  orxASSERT((_pstObject == orxNULL) || (_stGroupID == orxSTRINGID_UNDEFINED) || (orxLinkList_GetList(&(_pstObject->stGroupNode)) == &(((orxOBJECT_LISTS *)orxHashTable_Get(sstObject.pstGroupTable, _stGroupID))->stList)));

  /* Has group? */
  if(_stGroupID != orxSTRINGID_UNDEFINED)
  {
    orxOBJECT_LISTS *pstGroupLists;

    /* Is cached one? */
    if(_stGroupID == sstObject.stCachedGroupID)
    {
      /* Gets group list */
      pstGroupLists = sstObject.pstCachedGroupLists;
    }
    else
    {
      /* Gets group list */
      pstGroupLists = (orxOBJECT_LISTS *)orxHashTable_Get(sstObject.pstGroupTable, _stGroupID);
    }

    /* Valid? */
    if(pstGroupLists != orxNULL)
    {
      orxLINKLIST_NODE *pstNode;

      /* Gets node */
      pstNode = (_pstObject == orxNULL) ? orxLinkList_GetFirst(&(pstGroupLists->stList)) : orxLinkList_GetNext(&(_pstObject->stGroupNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Updates result */
        pstResult = orxSTRUCT_GET_FROM_FIELD(orxOBJECT, stGroupNode, pstNode);
      }
      else
      {
        /* Updates result */
        pstResult = orxNULL;
      }

      /* Caches group list */
      sstObject.pstCachedGroupLists = pstGroupLists;
      sstObject.stCachedGroupID     = _stGroupID;
    }
    else
    {
      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Updates result */
    pstResult = (_pstObject == orxNULL) ? orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT)) : orxOBJECT(orxStructure_GetNext(_pstObject));
  }

  /* Done! */
  return pstResult;
}

/** Gets next enabled object.
 * @param[in]   _pstObject      Concerned object, orxNULL to get the first one
 * @param[in]   _stGroupID      Group ID to consider, orxSTRINGID_UNDEFINED for all
 * @return      orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_GetNextEnabled(const orxOBJECT *_pstObject, orxSTRINGID _stGroupID)
{
  orxOBJECT *pstResult;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstObject == orxNULL) || (orxStructure_GetID((orxSTRUCTURE *)_pstObject) < orxSTRUCTURE_ID_NUMBER));
  orxASSERT((_pstObject == orxNULL) || (_stGroupID == orxSTRINGID_UNDEFINED) || (orxLinkList_GetList(&(_pstObject->stEnableGroupNode)) == &(((orxOBJECT_LISTS *)orxHashTable_Get(sstObject.pstGroupTable, _stGroupID))->stEnableList)));

  /* Has group? */
  if(_stGroupID != orxSTRINGID_UNDEFINED)
  {
    orxOBJECT_LISTS *pstGroupLists;

    /* Is cached one? */
    if(_stGroupID == sstObject.stCachedGroupID)
    {
      /* Gets group list */
      pstGroupLists = sstObject.pstCachedGroupLists;
    }
    else
    {
      /* Gets group list */
      pstGroupLists = (orxOBJECT_LISTS *)orxHashTable_Get(sstObject.pstGroupTable, _stGroupID);
    }

    /* Valid? */
    if(pstGroupLists != orxNULL)
    {
      orxLINKLIST_NODE *pstNode;

      /* Gets node */
      pstNode = (_pstObject == orxNULL) ? orxLinkList_GetFirst(&(pstGroupLists->stEnableList)) : orxLinkList_GetNext(&(_pstObject->stEnableGroupNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Updates result */
        pstResult = orxSTRUCT_GET_FROM_FIELD(orxOBJECT, stEnableGroupNode, pstNode);
      }
      else
      {
        /* Updates result */
        pstResult = orxNULL;
      }

      /* Caches group list */
      sstObject.pstCachedGroupLists = pstGroupLists;
      sstObject.stCachedGroupID     = _stGroupID;
    }
    else
    {
      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    orxLINKLIST_NODE *pstNode;

    /* Gets node */
    pstNode = (_pstObject == orxNULL) ? orxLinkList_GetFirst(&(sstObject.stEnableList)) : orxLinkList_GetNext(&(_pstObject->stEnableNode));

    /* Valid? */
    if(pstNode != orxNULL)
    {
      /* Updates result */
      pstResult = orxSTRUCT_GET_FROM_FIELD(orxOBJECT, stEnableNode, pstNode);
    }
    else
    {
      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/** Picks the first active object with size "under" the given position, within a given group. See
 * orxObject_BoxPick(), orxObject_CreateNeighborList() and orxObject_Raycast for other ways of picking
 * objects.
 * @param[in]   _pvPosition     Position to pick from
 * @param[in]   _stGroupID      Group ID to consider, orxSTRINGID_UNDEFINED for all
 * @return      orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_Pick(const orxVECTOR *_pvPosition, orxSTRINGID _stGroupID)
{
  orxFLOAT    fSelectedZ;
  orxOBJECT  *pstResult = orxNULL, *pstObject;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* For all objects */
  for(pstObject = orxObject_GetNext(orxNULL, _stGroupID), fSelectedZ = _pvPosition->fZ;
      pstObject != orxNULL;
      pstObject = orxObject_GetNext(pstObject, _stGroupID))
  {
    /* Is enabled? */
    if(orxObject_IsEnabled(pstObject) != orxFALSE)
    {
      orxVECTOR vObjectPos;

      /* Gets object position */
      if(orxObject_GetWorldPosition(pstObject, &vObjectPos) != orxNULL)
      {
        /* Is under position? */
        if(vObjectPos.fZ >= _pvPosition->fZ)
        {
          /* No selection or above it? */
          if((pstResult == orxNULL) || (vObjectPos.fZ <= fSelectedZ))
          {
            orxOBOX stObjectBox;

            /* Gets its bounding box */
            if(orxObject_GetBoundingBox(pstObject, &stObjectBox) != orxNULL)
            {
              /* Is position in 2D box? */
              if(orxOBox_2DIsInside(&stObjectBox, _pvPosition) != orxFALSE)
              {
                /* Updates result */
                pstResult = pstObject;

                /* Updates selected position */
                fSelectedZ = vObjectPos.fZ;
              }
            }
          }
        }
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Picks the first active object with size in contact with the given box, withing a given group. Use
 * orxObject_CreateNeighborList() to get all the objects in the box.
 * @param[in]   _pstBox         Box to use for picking
 * @param[in]   _stGroupID      Group ID to consider, orxSTRINGID_UNDEFINED for all
 * @return      orxOBJECT / orxNULL
 */
orxOBJECT *orxFASTCALL orxObject_BoxPick(const orxOBOX *_pstBox, orxSTRINGID _stGroupID)
{
  orxFLOAT    fSelectedZ;
  orxOBJECT  *pstResult = orxNULL, *pstObject;

  /* Checks */
  orxASSERT(sstObject.u32Flags & orxOBJECT_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBox != orxNULL);

  /* For all objects */
  for(pstObject = orxObject_GetNext(orxNULL, _stGroupID), fSelectedZ = _pstBox->vPosition.fZ;
      pstObject != orxNULL;
      pstObject = orxObject_GetNext(pstObject, _stGroupID))
  {
    /* Is enabled? */
    if(orxObject_IsEnabled(pstObject) != orxFALSE)
    {
      orxVECTOR vObjectPos;

      /* Gets object position */
      if(orxObject_GetWorldPosition(pstObject, &vObjectPos) != orxNULL)
      {
        /* No selection or above it? */
        if((pstResult == orxNULL) || (vObjectPos.fZ <= fSelectedZ))
        {
          orxOBOX stObjectBox;

          /* Gets its bounding box */
          if(orxObject_GetBoundingBox(pstObject, &stObjectBox) != orxNULL)
          {
            /* Does it intersect with box? */
            if(orxOBox_ZAlignedTestIntersection(_pstBox, &stObjectBox) != orxFALSE)
            {
              /* Updates result */
              pstResult = pstObject;

              /* Updates selected position */
              fSelectedZ = vObjectPos.fZ;
            }
          }
        }
      }
    }
  }

  /* Done! */
  return pstResult;
}
