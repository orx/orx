/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxRender.c
 * @date 25/09/2007
 * @author iarwain@orx-project.org
 *
 * Home render plugin implementation
 *
 */


#include "orxPluginAPI.h"


#ifdef __orxGCC__

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#endif /* __orxGCC__ */


/** Module flags
 */
#define orxRENDER_KU32_STATIC_FLAG_NONE             0x00000000 /**< No flags */

#define orxRENDER_KU32_STATIC_FLAG_READY            0x00000001 /**< Ready flag */
#define orxRENDER_KU32_STATIC_FLAG_RESET_MAXIMA     0x00000002 /**< Reset maxima flag */
#define orxRENDER_KU32_STATIC_FLAG_REGISTERED       0x00000004 /**< Rendering function registered flag */
#define orxRENDER_KU32_STATIC_FLAG_CONSOLE_BLINK    0x00000008 /**< Console blink flag */
#define orxRENDER_KU32_STATIC_FLAG_PROFILER         0x00000010 /**< Profiler flag */
#define orxRENDER_KU32_STATIC_FLAG_PROFILER_HISTORY 0x00000020 /**< Profiler history flag */
#define orxRENDER_KU32_STATIC_FLAG_PRESENT_REQUEST  0x00000040 /**< Present request flag */

#define orxRENDER_KU32_STATIC_MASK_ALL              0xFFFFFFFF /**< All mask */


/** Defines
 */
#define orxRENDER_KF_TICK_SIZE                      orx2F(1.0f / 10.0f)
#define orxRENDER_KU32_ORDER_BANK_SIZE              1024
#define orxRENDER_KST_DEFAULT_COLOR                 orx2RGBA(255, 0, 0, 255)
#define orxRENDER_KZ_FPS_FORMAT                     "FPS: %d"
#define orxRENDER_KF_CONSOLE_BLINK_DELAY            orx2F(0.5f)

#define orxRENDER_KF_PROFILER_BORDER                orx2F(0.01f)
#define orxRENDER_KF_PROFILER_SEPARATOR_WIDTH       orx2F(0.5f)
#define orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT      orx2F(0.25f)
#define orxRENDER_KF_PROFILER_BAR_MIN_HEIGHT        orx2F(5.0f)
#define orxRENDER_KF_PROFILER_BAR_MAX_HEIGHT        orx2F(24.0f)
#define orxRENDER_KF_PROFILER_BAR_ALPHA             orx2F(0.8f)
#define orxRENDER_KF_PROFILER_BAR_HIGH_L            orx2F(0.7f)
#define orxRENDER_KF_PROFILER_BAR_LOW_L             orx2F(0.3f)
#define orxRENDER_KF_PROFILER_TEXT_MIN_HEIGHT       orx2F(0.5f)
#define orxRENDER_KF_PROFILER_TEXT_MAX_HEIGHT       orx2F(1.0f)
#define orxRENDER_KF_PROFILER_TEXT_DEFAULT_WIDTH    orx2F(800.0f)
#define orxRENDER_KF_PROFILER_HISTOGRAM_ALPHA       orx2F(0.4f)
#define orxRENDER_KF_PROFILER_HUE_STACK_RANGE       orx2F(2.0f)
#define orxRENDER_KF_PROFILER_HUE_UNSTACK_RANGE     orx2F(0.8f/3.0f)
#define orxRENDER_KC_PROFILER_DEPTH_MARKER          '*'

#define orxRENDER_KST_CONSOLE_BACKGROUND_COLOR      orx2RGBA(0x11, 0x55, 0x11, 0x99)
#define orxRENDER_KST_CONSOLE_SEPARATOR_COLOR       orx2RGBA(0x88, 0x11, 0x11, 0xFF)
#define orxRENDER_KST_CONSOLE_LOG_COLOR             orx2RGBA(0xAA, 0xAA, 0xAA, 0xFF)
#define orxRENDER_KST_CONSOLE_INPUT_COLOR           orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF)
#define orxRENDER_KST_CONSOLE_COMPLETION_COLOR      orx2RGBA(0x88, 0x88, 0x88, 0xFF)
#define orxRENDER_KF_CONSOLE_MARGIN_WIDTH           orx2F(0.02f)
#define orxRENDER_KF_CONSOLE_MARGIN_HEIGHT          orx2F(0.05f)
#define orxRENDER_KF_CONSOLE_SPEED                  orx2F(3000.0f)
#define orxRENDER_KF_CONSOLE_FONT_SCALE             orx2F(1.0f)

#define orxRENDER_KE_KEY_PROFILER_TOGGLE_HISTORY    orxKEYBOARD_KEY_SCROLL_LOCK
#define orxRENDER_KE_KEY_PROFILER_PAUSE             orxKEYBOARD_KEY_SPACE
#define orxRENDER_KE_KEY_PROFILER_PREVIOUS_FRAME    orxKEYBOARD_KEY_LEFT
#define orxRENDER_KE_KEY_PROFILER_NEXT_FRAME        orxKEYBOARD_KEY_RIGHT
#define orxRENDER_KE_KEY_PROFILER_PREVIOUS_DEPTH    orxKEYBOARD_KEY_UP
#define orxRENDER_KE_KEY_PROFILER_NEXT_DEPTH        orxKEYBOARD_KEY_DOWN
#define orxRENDER_KE_KEY_PROFILER_PREVIOUS_THREAD   orxKEYBOARD_KEY_PAGE_UP
#define orxRENDER_KE_KEY_PROFILER_NEXT_THREAD       orxKEYBOARD_KEY_PAGE_DOWN

#define orxRENDER_KF_INPUT_RESET_FIRST_DELAY        orx2F(0.25f)
#define orxRENDER_KF_INPUT_RESET_DELAY              orx2F(0.02f)

#define orxRENDER_KU32_MAX_MARKER_DEPTH             16

#define orxRENDER_KC_CONSOLE_INSERT_MARKER          '_'
#define orxRENDER_KC_CONSOLE_OVERTYPE_MARKER        '#'

#define orxRENDER_KZ_PORTRAIT                       "portrait"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxRENDER_RENDER_NODE_t
{
  orxLINKLIST_NODE      stNode;                     /**< Linklist node : 12 */
  orxTEXTURE           *pstTexture;                 /**< Texture pointer : 16 */
  const orxSHADER      *pstShader;                  /**< Shader pointer : 20 */
  orxOBJECT            *pstObject;                  /**< Object pointer : 24 */
  orxFLOAT              fZ;                         /**< Z coordinate : 28 */
  orxDISPLAY_BLEND_MODE eBlendMode;                 /**< Blend mode : 32 */
  orxDISPLAY_SMOOTHING  eSmoothing;                 /**< Smoothing : 36 */
  orxFLOAT              fDepthCoef;                 /**< Depth coef : 40 */

} orxRENDER_NODE;

/** Static structure
 */
typedef struct __orxRENDER_STATIC_t
{
  orxU32        u32Flags;                           /**< Control flags */
  orxCLOCK     *pstClock;                           /**< Rendering clock pointer */
  orxFRAME     *pstFrame;                           /**< Conversion frame */
  orxBANK      *pstRenderBank;                      /**< Rendering bank */
  orxLINKLIST   stRenderList;                       /**< Rendering list */
  orxFLOAT      fDefaultConsoleOffset;              /**< Default console offset */
  orxFLOAT      fConsoleOffset;                     /**< Console offset */
  orxFLOAT      fConsoleFontScale;                  /**< Console font scale */
  orxU32        u32SelectedFrame;                   /**< Selected frame */
  orxU32        u32SelectedThread;                  /**< Selected thread */
  orxU32        u32SelectedMarkerDepth;             /**< Selected marker depth */
  orxU32        u32MaxMarkerDepth;                  /**< Maximum marker depth */

} orxRENDER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxRENDER_STATIC sstRender;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Updates console
 */
static orxINLINE void orxRender_Home_UpdateConsole(orxFLOAT _fScreenWidth, orxFLOAT _fScreenHeight)
{
  const orxFONT  *pstFont;
  orxFLOAT        fConsoleWidth, fConsoleFontScale;

  /* Pushes config section */
  orxConfig_PushSection(orxRENDER_KZ_CONFIG_SECTION);

  /* Gets font scale */
  if((fConsoleFontScale = orxConfig_GetFloat(orxRENDER_KZ_CONFIG_CONSOLE_FONT_SCALE)) > orxFLOAT_0)
  {
    /* Gets it */
    sstRender.fConsoleFontScale = fConsoleFontScale;
  }
  else
  {
    /* Stores it */
    orxConfig_SetFloat(orxRENDER_KZ_CONFIG_CONSOLE_FONT_SCALE, sstRender.fConsoleFontScale);
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Use default screen size? */
  if((_fScreenWidth <= orxFLOAT_0) || (_fScreenHeight <= orxFLOAT_0))
  {
    /* Gets screen size */
    orxDisplay_GetScreenSize(&_fScreenWidth, &_fScreenHeight);
  }

  /* Gets console width */
  fConsoleWidth = _fScreenWidth * (orxFLOAT_1 - orx2F(2.0f) * orxRENDER_KF_CONSOLE_MARGIN_WIDTH);

  /* Gets console font */
  pstFont = orxConsole_GetFont();

  /* Updates console log line length */
  orxConsole_SetLogLineLength(orxF2U(fConsoleWidth / (orxFont_GetCharacterWidth(pstFont, (orxU32)' ') * sstRender.fConsoleFontScale)));

  /* Sets default console offset */
  sstRender.fDefaultConsoleOffset = -_fScreenHeight;

  /* Done! */
  return;
}

/** Inits console
 */
static orxINLINE void orxRender_Home_InitConsole()
{
  /* Inits console scale */
  sstRender.fConsoleFontScale = orxRENDER_KF_CONSOLE_FONT_SCALE;

  /* Updates console */
  orxRender_Home_UpdateConsole(orxFLOAT_0, orxFLOAT_0);

  /* Updates current console offset */
  sstRender.fConsoleOffset = sstRender.fDefaultConsoleOffset;

  /* Done! */
  return;
}

/** Blink timer
 */
static void orxFASTCALL orxRender_Home_BlinkTimer(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  /* Updates blink status */
  orxFLAG_SWAP(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_CONSOLE_BLINK);

  /* Done! */
  return;
}

/** Resets profiler's maxima
 */
static void orxFASTCALL orxRender_Home_ResetProfilerMaxima(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  /* Resets profiler's maxima */
  orxProfiler_ResetAllMaxima();

  /* Done! */
  return;
}

/** Resets input callback
 */
static void orxFASTCALL orxRender_ResetInput(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  const orxSTRING zPreviousSet;
  orxSTRING       zInput;

  /* Backups previous input set */
  zPreviousSet = orxInput_GetCurrentSet();

  /* Selects render input set */
  orxInput_SelectSet(orxRENDER_KZ_INPUT_SET);

  /* Gets input */
  zInput = (orxSTRING)_pContext;

  /* Is input still active? */
  if(orxInput_IsActive(zInput) != orxFALSE)
  {
    /* Resets it */
    orxInput_SetValue(zInput, orxFLOAT_0);

    /* Re-adds input reset timer */
    orxClock_AddGlobalTimer(orxRender_ResetInput, orxRENDER_KF_INPUT_RESET_DELAY, 1, zInput);
  }

  /* Restores previous input set */
  orxInput_SelectSet(zPreviousSet);
}

/** Renders FPS count
 */
static orxINLINE void orxRender_Home_RenderFPS()
{
  const orxFONT *pstFont;

  /* Disables marker operations */
  orxProfiler_EnableMarkerOperations(orxFALSE);

  /* Gets default font */
  pstFont = orxFont_GetDefaultFont();

  /* Valid? */
  if(pstFont != orxNULL)
  {
    orxVIEWPORT          *pstViewport;
    orxBITMAP            *pstBitmap;
    orxDISPLAY_TRANSFORM  stTextTransform;
    orxCHAR               acBuffer[16];

    /* Gets its bitmap */
    pstBitmap = orxTexture_GetBitmap(orxFont_GetTexture(pstFont));

    /* Clears text transform */
    orxMemory_Zero(&stTextTransform, sizeof(orxDISPLAY_TRANSFORM));

    /* Gets last viewport */
    pstViewport = orxVIEWPORT(orxStructure_GetLast(orxSTRUCTURE_ID_VIEWPORT));

    /* Valid? */
    if(pstViewport != orxNULL)
    {
      orxAABOX  stBox;
      orxFLOAT  fWidth, fHeight, fCorrectionRatio;

      /* Gets its box & size */
      orxViewport_GetBox(pstViewport, &stBox);
      orxViewport_GetRelativeSize(pstViewport, &fWidth, &fHeight);

      /* Gets current correction ratio */
      fCorrectionRatio = orxViewport_GetCorrectionRatio(pstViewport);

      /* Has correction ratio? */
      if(fCorrectionRatio != orxFLOAT_1)
      {
        /* X axis? */
        if(fCorrectionRatio < orxFLOAT_1)
        {
          orxFLOAT fDelta;

          /* Gets rendering limit delta using correction ratio */
          fDelta = orx2F(0.5f) * (orxFLOAT_1 - fCorrectionRatio) * (stBox.vBR.fX - stBox.vTL.fX);

          /* Updates viewport */
          stBox.vTL.fX += fDelta;
          stBox.vBR.fX -= fDelta;
        }
        /* Y axis */
        else
        {
          orxFLOAT fDelta;

          /* Gets rendering limit delta using correction ratio */
          fDelta = orx2F(0.5f) * (orxFLOAT_1 - (orxFLOAT_1 / fCorrectionRatio)) * (stBox.vBR.fY - stBox.vTL.fY);

          /* Updates viewport */
          stBox.vTL.fY += fDelta;
          stBox.vBR.fY -= fDelta;
        }
      }

      /* Inits transform's scale */
      stTextTransform.fScaleX = orx2F(2.0f) * fWidth;
      stTextTransform.fScaleY = orx2F(2.0f) * fHeight;

      /* Inits transform's destination */
      stTextTransform.fDstX = stBox.vTL.fX + orx2F(10.0f);
      stTextTransform.fDstY = stBox.vTL.fY + orx2F(10.0f);
    }
    else
    {
      /* Inits transform's scale */
      stTextTransform.fScaleX = stTextTransform.fScaleY = orx2F(2.0f);

      /* Inits transform's position */
      stTextTransform.fDstX = stTextTransform.fDstY = orx2F(10.0f);
    }

    /* Writes string */
    orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, orxRENDER_KZ_FPS_FORMAT, orxFPS_GetFPS());
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

    /* Displays it */
    orxDisplay_TransformText(acBuffer, pstBitmap, orxFont_GetMap(pstFont), &stTextTransform, orxRENDER_KST_DEFAULT_COLOR, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);
  }

  /* Re-enables marker operations */
  orxProfiler_EnableMarkerOperations(orxTRUE);

  /* Done! */
  return;
}

/** Renders profiler info
 */
static orxINLINE void orxRender_Home_RenderProfiler()
{
  orxDISPLAY_TRANSFORM    stTransform;
  orxTEXTURE             *pstTexture;
  orxBITMAP              *pstBitmap, *pstFontBitmap;
  orxS32                  s32MarkerCount, s32UniqueCount, s32MarkerID;
  orxU32                  u32CurrentDepth, i;
  orxFLOAT                fScreenWidth, fScreenHeight, fWidth, fHeight, fBorder, fHueDelta, fTextScale;
  orxDOUBLE               dFrameStartTime = orxDOUBLE_0, dTotalTime, dRecTotalTime;
  orxCOLOR                stColor;
  orxBOOL                 bLandscape;
  const orxFONT          *pstFont;
  const orxCHARACTER_MAP *pstMap;
  orxFLOAT                fMarkerWidth;
  orxCHAR                 acLabel[64];
  orxDOUBLE               adDepthBlockEndTime[orxRENDER_KU32_MAX_MARKER_DEPTH];

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxRender_RenderProfiler");

  /* Disables marker operations */
  orxProfiler_EnableMarkerOperations(orxFALSE);

  /* Resets frame selection */
  orxProfiler_SelectQueryFrame(sstRender.u32SelectedFrame, sstRender.u32SelectedThread);

  /* Inits buffer */
  acLabel[sizeof(acLabel) - 1] = orxCHAR_NULL;

  /* Gets default font */
  pstFont = orxFont_GetDefaultFont();

  /* Gets its bitmap */
  pstFontBitmap = orxTexture_GetBitmap(orxFont_GetTexture(pstFont));

  /* Gets its map */
  pstMap = orxFont_GetMap(pstFont);

  /* Gets its marker width */
  fMarkerWidth = ((orxCHARACTER_GLYPH *)orxHashTable_Get(pstMap->pstCharacterTable, orxRENDER_KC_PROFILER_DEPTH_MARKER))->fWidth;

  /* Creates pixel texture */
  pstTexture = orxTexture_Get(orxTEXTURE_KZ_PIXEL);

  /* Gets its bitmap */
  pstBitmap = orxTexture_GetBitmap(pstTexture);

  /* Resets its clipping */
  orxDisplay_SetBitmapClipping(pstBitmap, 0, 0, 1, 1);

  /* Gets marker count */
  s32MarkerCount = orxProfiler_GetMarkerCount();

  /* For all markers */
  for(s32UniqueCount = 0, sstRender.u32MaxMarkerDepth = 0, s32MarkerID = orxProfiler_GetNextMarkerID(orxPROFILER_KS32_MARKER_ID_NONE);
      s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE;
      s32MarkerID = orxProfiler_GetNextMarkerID(s32MarkerID))
  {
    /* Is unique? */
    if(orxProfiler_IsUniqueMarker(s32MarkerID) != orxFALSE)
    {
      orxU32 u32Depth;

      /* Gets depth */
      u32Depth = orxProfiler_GetUniqueMarkerDepth(s32MarkerID);

      /* Deeper than previous? */
      if(u32Depth > sstRender.u32MaxMarkerDepth)
      {
        /* Stores it */
        sstRender.u32MaxMarkerDepth = u32Depth;
      }

      /* Updates count */
      s32UniqueCount++;
    }
  }

  /* Inits array for storing block ends at each depth */
  orxMemory_Zero(adDepthBlockEndTime, orxRENDER_KU32_MAX_MARKER_DEPTH * sizeof(orxDOUBLE));

  /* Gets marker total time, reciprocal total time and start time */
  dTotalTime    = orxProfiler_GetResetTime();
  dRecTotalTime = orxDOUBLE_1 / dTotalTime;

  /* Gets screen size */
  orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

  /* Updates orientation */
  bLandscape = (fScreenWidth >= fScreenHeight) ? orxTRUE: orxFALSE;

  /* Pushes config section */
  orxConfig_PushSection(orxRENDER_KZ_CONFIG_SECTION);

  /* Has orientation value? */
  if(orxConfig_HasValue(orxRENDER_KZ_CONFIG_PROFILER_ORIENTATION) != orxFALSE)
  {
    orxBOOL bPortrait = orxFALSE;
    orxCHAR acBuffer[16];

    /* Gets lower case value */
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
    orxString_LowerCase(orxString_NCopy(acBuffer, orxConfig_GetString(orxRENDER_KZ_CONFIG_PROFILER_ORIENTATION), sizeof(acBuffer) - 1));

    /* Portrait? */
    if((orxString_SearchString(acBuffer, orxRENDER_KZ_PORTRAIT) != orxNULL)
    || ((orxString_ToBool(acBuffer, &bPortrait, orxNULL) != orxSTATUS_FAILURE)
     && (bPortrait != orxFALSE)))
    {
      /* Updates landscape status */
      bLandscape = !bLandscape;
    }
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Gets border */
  fBorder = orxMath_Floor(orxRENDER_KF_PROFILER_BORDER * fScreenWidth);

  /* Gets full marker size and text scale */
  if(bLandscape != orxFALSE)
  {
    fWidth      = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth - orx2F(2.0f) * fBorder;
    fHeight     = orxMath_Floor(orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT * fScreenHeight / orxU2F(sstRender.u32MaxMarkerDepth + 2));
    fTextScale  = orxMIN(fScreenWidth / orxRENDER_KF_PROFILER_TEXT_DEFAULT_WIDTH, orxFLOAT_1);
  }
  else
  {
    fWidth      = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight - orx2F(2.0f) * fBorder;
    fHeight     = orxMath_Floor(orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT * fScreenWidth / orxU2F(sstRender.u32MaxMarkerDepth + 2));
    fTextScale  = orxMIN(fScreenHeight / orxRENDER_KF_PROFILER_TEXT_DEFAULT_WIDTH, orxFLOAT_1);
  }
  fHeight = orxCLAMP(fHeight, orxRENDER_KF_PROFILER_BAR_MIN_HEIGHT, orxRENDER_KF_PROFILER_BAR_MAX_HEIGHT);

  /* Gets hue delta */
  fHueDelta = orxRENDER_KF_PROFILER_HUE_STACK_RANGE / orxS2F(s32MarkerCount + 1);

  /* Inits transform */
  stTransform.fSrcX     = stTransform.fSrcY     = orxFLOAT_0;
  stTransform.fRepeatX  = stTransform.fRepeatY  = orxFLOAT_1;
  stTransform.fRotation = orxFLOAT_0;

  /* Draws background */
  stTransform.fDstX   = orxFLOAT_0;
  stTransform.fDstY   = orxFLOAT_0;
  stTransform.fScaleX = fScreenWidth;
  stTransform.fScaleY = fScreenHeight;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, orx2RGBA(0x00, 0x00, 0x00, 0x99), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

  /* Should render history? */
  if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PROFILER_HISTORY))
  {
    orxDISPLAY_VERTEX astVertexList[2 * (orxPROFILER_KU32_HISTORY_LENGTH - 1)];
    orxU16            au16IndexList[2 * (orxPROFILER_KU32_HISTORY_LENGTH - 1)];
    orxDOUBLE         adStartTimeList[orxPROFILER_KU32_HISTORY_LENGTH - 1], dFrameRecDuration = orxDOUBLE_0;
    orxBOOL           bFirst;

    /* Inits color */
    orxColor_Set(&stColor, &orxVECTOR_GREEN, orxRENDER_KF_PROFILER_HISTOGRAM_ALPHA);
    orxColor_FromRGBToHSV(&stColor, &stColor);

    /* For all vertices */
    for(i = 0; i < orxPROFILER_KU32_HISTORY_LENGTH - 1; i++)
    {
      /* Inits both vertices */
      astVertexList[2 * i].fX     =
      astVertexList[2 * i + 1].fX = (bLandscape != orxFALSE) ? (orxFLOAT_1 - orxU2F(i) / orxU2F(orxPROFILER_KU32_HISTORY_LENGTH)) * (fScreenWidth - fBorder) : fScreenWidth - fBorder;
      astVertexList[2 * i].fY     =
      astVertexList[2 * i + 1].fY = (bLandscape != orxFALSE) ? fScreenHeight - fBorder : fScreenHeight - (orxFLOAT_1 - orxU2F(i) / orxU2F(orxPROFILER_KU32_HISTORY_LENGTH)) * (fScreenHeight - fBorder);
      astVertexList[2 * i].fU     =
      astVertexList[2 * i + 1].fU =
      astVertexList[2 * i].fV     =
      astVertexList[2 * i + 1].fV = orxFLOAT_0;
    }

    /* For all indices */
    for(i = 0; i < orxARRAY_GET_ITEM_COUNT(au16IndexList); i++)
    {
      /* Inits indices */
      au16IndexList[i] = (orxU16)i;
    }

    /* For all sorted markers */
    for(bFirst = orxTRUE, s32MarkerID = orxProfiler_GetNextSortedMarkerID(orxPROFILER_KS32_MARKER_ID_NONE);
        s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE;
        s32MarkerID = orxProfiler_GetNextSortedMarkerID(s32MarkerID))
    {
      /* Is unique? */
      if(orxProfiler_IsUniqueMarker(s32MarkerID) != orxFALSE)
      {
        /* Has been pushed? */
        if(orxProfiler_GetMarkerPushCount(s32MarkerID) > 0)
        {
          /* First marker? */
          if(bFirst != orxFALSE)
          {
            orxDOUBLE dAccDuration = orxDOUBLE_0, dDurationSampleNumber = orxDOUBLE_0;

            /* For all past frames */
            for(i = 0; i < orxPROFILER_KU32_HISTORY_LENGTH - 1; i++)
            {
              orxDOUBLE dDuration;

              /* Selects it */
              orxProfiler_SelectQueryFrame(i, sstRender.u32SelectedThread);

              /* Stores its frame start time */
              adStartTimeList[i] = orxProfiler_GetUniqueMarkerStartTime(s32MarkerID);

              /* Gets frame duration */
              dDuration = orxProfiler_GetResetTime();

              /* Valid? */
              if(dDuration != orxDOUBLE_0)
              {
                /* Updates accumulators */
                dAccDuration           += dDuration;
                dDurationSampleNumber  += orxDOUBLE_1;
              }
            }

            /* Gets averaged frame reciprocal duration */
            dFrameRecDuration = orx2D(dDurationSampleNumber) / dAccDuration;

            /* Resets query frame */
            orxProfiler_SelectQueryFrame(0, sstRender.u32SelectedThread);

            /* Clears first status */
            bFirst = orxFALSE;
          }

          /* Desired depth? */
          if(orxProfiler_GetUniqueMarkerDepth(s32MarkerID) == sstRender.u32SelectedMarkerDepth)
          {
            orxDISPLAY_MESH stMesh;
            orxCOLOR        stBarColor, stTempColor;
            orxRGBA         stLowRGBA, stHighRGBA;

            /* Gets associated colors */
            stBarColor.fAlpha   = orxRENDER_KF_PROFILER_HISTOGRAM_ALPHA;
            stBarColor.vHSL.fH  = orxMath_Mod(fHueDelta * orxS2F((s32MarkerID & 0xFF) % s32MarkerCount), orxFLOAT_1);
            stBarColor.vHSL.fS  = orxFLOAT_1;
            stBarColor.vHSL.fL  = orxRENDER_KF_PROFILER_BAR_LOW_L;
            stLowRGBA           = orxColor_ToRGBA(orxColor_FromHSLToRGB(&stTempColor, &stBarColor));
            stBarColor.vHSL.fL  = orxRENDER_KF_PROFILER_BAR_HIGH_L;
            stHighRGBA          = orxColor_ToRGBA(orxColor_FromHSLToRGB(&stTempColor, &stBarColor));

            /* For all past frames */
            for(i = 0; i < orxPROFILER_KU32_HISTORY_LENGTH - 1; i++)
            {
              /* Selects it */
              orxProfiler_SelectQueryFrame(i, sstRender.u32SelectedThread);

              /* Landscape? */
              if(bLandscape != orxFALSE)
              {
                /* Updates bottom vertex with previous top one */
                astVertexList[2 * i].fY     = fScreenHeight - fBorder - orx2F((orxProfiler_GetUniqueMarkerStartTime(s32MarkerID) - adStartTimeList[i]) * dFrameRecDuration) * (orx2F(0.5f) * fScreenHeight - fBorder);
                astVertexList[2 * i].stRGBA = (i == sstRender.u32SelectedFrame) ? orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF) : stLowRGBA;

                /* Updates top vertex */
                astVertexList[2 * i + 1].fY     = astVertexList[2 * i].fY - orx2F(orxProfiler_GetMarkerTime(s32MarkerID) * dFrameRecDuration) * (orx2F(0.5f) * fScreenHeight - fBorder);
                astVertexList[2 * i + 1].stRGBA = (i == sstRender.u32SelectedFrame) ? orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF) : stHighRGBA;
              }
              else
              {
                /* Updates bottom vertex with previous top one */
                astVertexList[2 * i].fX     = fScreenWidth - fBorder - orx2F((orxProfiler_GetUniqueMarkerStartTime(s32MarkerID) - adStartTimeList[i]) * dFrameRecDuration) * (orx2F(0.5f) * fScreenWidth - fBorder);
                astVertexList[2 * i].stRGBA = (i == sstRender.u32SelectedFrame) ? orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF) : stLowRGBA;

                /* Updates top vertex */
                astVertexList[2 * i + 1].fX     = astVertexList[2 * i].fX - orx2F(orxProfiler_GetMarkerTime(s32MarkerID) * dFrameRecDuration) * (orx2F(0.5f) * fScreenWidth - fBorder);
                astVertexList[2 * i + 1].stRGBA = (i == sstRender.u32SelectedFrame) ? orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF) : stHighRGBA;
              }
            }

            /* Resets query frame */
            orxProfiler_SelectQueryFrame(0, sstRender.u32SelectedThread);

            /* Inits mesh */
            orxMemory_Zero(&stMesh, sizeof(orxDISPLAY_MESH));
            stMesh.astVertexList    = astVertexList;
            stMesh.u32VertexNumber  = orxARRAY_GET_ITEM_COUNT(astVertexList);
            stMesh.au16IndexList    = au16IndexList;
            stMesh.u32IndexNumber   = orxARRAY_GET_ITEM_COUNT(au16IndexList);
            stMesh.ePrimitive       = orxDISPLAY_PRIMITIVE_TRIANGLE_STRIP;

            /* Draws it */
            orxDisplay_DrawMesh(&stMesh, pstBitmap, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
          }
        }
      }
    }
  }

  /* Resets frame selection */
  orxProfiler_SelectQueryFrame(sstRender.u32SelectedFrame, sstRender.u32SelectedThread);

  /* Inits color */
  orxColor_Set(&stColor, &orxVECTOR_GREEN, orxRENDER_KF_PROFILER_BAR_ALPHA);
  orxColor_FromRGBToHSV(&stColor, &stColor);

  /* Draws top bar */
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX     = fBorder;
    stTransform.fDstY     = orxFLOAT_1;
    stTransform.fRotation = orxFLOAT_0;
  }
  else
  {
    stTransform.fDstX     = orxFLOAT_1;
    stTransform.fDstY     = fScreenHeight - fBorder;
    stTransform.fRotation = -orxMATH_KF_PI_BY_2;
  }
  stTransform.fScaleX   = fWidth;
  stTransform.fScaleY   = fHeight - orx2F(2.0f);
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, orx2RGBA(0xCC, 0xCC, 0xCC, 0xCC), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

  /* Displays its label */
  orxString_NPrint(acLabel, sizeof(acLabel) - 1, "-=%s Thread=-  Frame[%.2f|%.2fms]", orxThread_GetName(sstRender.u32SelectedThread), orx2D(1000.0) * dTotalTime, orx2D(1000.0) * orxProfiler_GetMaxResetTime());
  stTransform.fScaleX = fHeight / pstMap->fCharacterHeight;
  stTransform.fScaleX = orxMIN(fTextScale, stTransform.fScaleX);
  stTransform.fScaleY = stTransform.fScaleX = orxCLAMP(stTransform.fScaleX, orxRENDER_KF_PROFILER_TEXT_MIN_HEIGHT, orxRENDER_KF_PROFILER_TEXT_MAX_HEIGHT);
  orxDisplay_TransformText(acLabel, pstFontBitmap, pstMap, &stTransform, orx2RGBA(0xFF, 0xFF, 0xFF, 0xCC), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

  /* Draws separators */
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX   = orxFLOAT_0;
    stTransform.fDstY   = orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT * fScreenHeight;
    stTransform.fScaleX = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth;
  }
  else
  {
    stTransform.fDstX   = orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT * fScreenWidth;
    stTransform.fDstY   = fScreenHeight;
    stTransform.fScaleX = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight;
  }
  stTransform.fScaleY = orxFLOAT_1;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, orx2RGBA(0xFF, 0xFF, 0xFF, 0xCC), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX   = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth;
    stTransform.fDstY   = orxFLOAT_0;
    stTransform.fScaleY = fScreenHeight;
  }
  else
  {
    stTransform.fDstX   = orxFLOAT_0;
    stTransform.fDstY   = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight;
    stTransform.fScaleY = fScreenWidth;
  }
  stTransform.fScaleX = orxFLOAT_1;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, orx2RGBA(0xFF, 0xFF, 0xFF, 0xCC), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

  /* Updates vertical values */
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstY = fHeight + orxFLOAT_1;
  }
  else
  {
    stTransform.fDstX = fHeight + orxFLOAT_1;
  }
  stTransform.fScaleY = fHeight - orx2F(2.0f);

  /* For all sorted markers */
  for(u32CurrentDepth = 0, s32MarkerID = orxProfiler_GetNextSortedMarkerID(orxPROFILER_KS32_MARKER_ID_NONE);
      s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE;
      s32MarkerID = orxProfiler_GetNextSortedMarkerID(s32MarkerID))
  {
    /* Is unique and has been pushed? */
    if((orxProfiler_GetMarkerPushCount(s32MarkerID) > 0) && (orxProfiler_IsUniqueMarker(s32MarkerID) != orxFALSE))
    {
      orxDOUBLE dTime, dStartTime;
      orxCOLOR  stBarColor;
      orxU32    u32Depth;

      /* Gets its depth */
      u32Depth = orxProfiler_GetUniqueMarkerDepth(s32MarkerID) - 1;

      /* Updates start time */
      dStartTime = orxProfiler_GetUniqueMarkerStartTime(s32MarkerID);

      /* First one? */
      if(dFrameStartTime == orxDOUBLE_0)
      {
        /* Updates start time */
        dFrameStartTime = dStartTime;
      }

      /* Checks */
      orxASSERT(u32Depth < orxRENDER_KU32_MAX_MARKER_DEPTH);

      /* Adjusts start time to prevent block overlap at this level */
      dStartTime = orxMAX(dStartTime, adDepthBlockEndTime[u32Depth]);

      /* Gets its time */
      dTime = orxProfiler_GetMarkerTime(s32MarkerID);

      /* Updates block end time for this level */
      adDepthBlockEndTime[u32Depth] = dStartTime + dTime;

      /* Updates its horizontal scale */
      stTransform.fScaleX = (orxFLOAT)(dTime * dRecTotalTime) * fWidth;

      /* Updates its position */
      if(bLandscape != orxFALSE)
      {
        stTransform.fDstX   = fBorder + (orxFLOAT)((dStartTime - dFrameStartTime) * dRecTotalTime) * fWidth;
        stTransform.fDstY  += fHeight * orxS2F((orxS32)u32Depth - (orxS32)u32CurrentDepth);
      }
      else
      {
        stTransform.fDstX  += fHeight * orxS2F((orxS32)u32Depth - (orxS32)u32CurrentDepth);
        stTransform.fDstY   = fScreenHeight - (fBorder + (orxFLOAT)((dStartTime - dFrameStartTime) * dRecTotalTime) * fWidth);
      }

      /* Updates current depth */
      u32CurrentDepth = u32Depth;

      /* Updates pixel color */
      stColor.vHSV.fH = orxMath_Mod(fHueDelta * orxS2F((s32MarkerID & 0xFF) % s32MarkerCount), orxFLOAT_1);

      /* Draws bar */
      orxDisplay_TransformBitmap(pstBitmap, &stTransform, orxColor_ToRGBA(orxColor_FromHSVToRGB(&stBarColor, &stColor)), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

      /* Is selected depth for history? */
      if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PROFILER_HISTORY) && (u32Depth + 1 == sstRender.u32SelectedMarkerDepth))
      {
        /* Updates is width */
        stTransform.fScaleX = fBorder - orx2F(2.0f);

        /* Updates its position */
        if(bLandscape != orxFALSE)
        {
          stTransform.fDstX = orxFLOAT_1;
        }
        else
        {
          stTransform.fDstY = fScreenHeight - orxFLOAT_1;
        }

        /* Draws marker */
        orxDisplay_TransformBitmap(pstBitmap, &stTransform, orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
      }
    }
  }

  /* Updates vertical position & marker height */
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX = fBorder;
    stTransform.fDstY = orxMath_Ceil(orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT * fScreenHeight + orxFLOAT_1);
    fHeight           = orxMath_Floor((orxFLOAT_1 - orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT) * fScreenHeight / ((s32UniqueCount) ? orxS2F(s32UniqueCount) : orxFLOAT_1));
  }
  else
  {
    stTransform.fDstX = orxMath_Ceil(orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT * fScreenWidth + orxFLOAT_1);
    stTransform.fDstY = fScreenHeight - fBorder;
    fHeight           = orxMath_Floor((orxFLOAT_1 - orxRENDER_KF_PROFILER_SEPARATOR_HEIGHT) * fScreenWidth / ((s32UniqueCount) ? orxS2F(s32UniqueCount) : orxFLOAT_1));
  }
  fHeight = orxCLAMP(fHeight, orxRENDER_KF_PROFILER_BAR_MIN_HEIGHT, orxRENDER_KF_PROFILER_BAR_MAX_HEIGHT);

  /* Reinits text scale */
  fTextScale          = orxMIN(fTextScale, fHeight / pstMap->fCharacterHeight);
  fTextScale          = orxCLAMP(fTextScale, orxRENDER_KF_PROFILER_TEXT_MIN_HEIGHT, orxRENDER_KF_PROFILER_TEXT_MAX_HEIGHT);
  stTransform.fScaleY = stTransform.fScaleX = fTextScale;

  /* Inits color */
  orxColor_Set(&stColor, &orxVECTOR_GREEN, orxRENDER_KF_PROFILER_BAR_ALPHA);
  orxColor_FromRGBToHSV(&stColor, &stColor);

  /* For all sorted markers */
  for(s32MarkerID = orxProfiler_GetNextSortedMarkerID(orxPROFILER_KS32_MARKER_ID_NONE);
      s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE;
      s32MarkerID = orxProfiler_GetNextSortedMarkerID(s32MarkerID))
  {
    /* Is unique and has been pushed? */
    if((orxProfiler_IsUniqueMarker(s32MarkerID) != orxFALSE)
    && (orxProfiler_GetMarkerPushCount(s32MarkerID) > 0))
    {
      orxDOUBLE dTime;
      orxCOLOR  stLabelColor;
      orxU32    u32Depth;

      /* Gets its time */
      dTime = orxProfiler_GetMarkerTime(s32MarkerID);

      /* Gets its depth */
      u32Depth = orxProfiler_GetUniqueMarkerDepth(s32MarkerID);

      /* Sets font's color */
      stColor.vHSV.fH = orxMath_Mod(fHueDelta * orxS2F((s32MarkerID & 0xFF) % s32MarkerCount), orxFLOAT_1);

      /* Is selected depth for history? */
      if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PROFILER_HISTORY) && (u32Depth == sstRender.u32SelectedMarkerDepth))
      {
        /* Adds marker */
        acLabel[0] = orxRENDER_KC_PROFILER_DEPTH_MARKER;

        /* Updates depth and iterator for text display */
        u32Depth++;
        i = 1;

        /* Updates position */
        if(bLandscape != orxFALSE)
        {
          stTransform.fDstX -= fMarkerWidth * fTextScale;
        }
        else
        {
          stTransform.fDstY += fMarkerWidth * fTextScale;
        }
      }
      else
      {
        /* Inits iterator */
        i = 0;
      }

      /* Adds depth markers */
      for(; i < u32Depth; i++)
      {
        acLabel[i] = '+';
      }

      /* Draws its label */
      orxString_NPrint(acLabel + u32Depth, sizeof(acLabel) - 1 - u32Depth, " %s [%.2f|%.2fms][%dx]", orxProfiler_GetMarkerName(s32MarkerID), orx2D(1000.0) * dTime, orx2D(1000.0) * orxProfiler_GetMarkerMaxTime(s32MarkerID), orxProfiler_GetMarkerPushCount(s32MarkerID));
      orxDisplay_TransformText(acLabel, pstFontBitmap, orxFont_GetMap(pstFont), &stTransform, orxColor_ToRGBA(orxColor_FromHSVToRGB(&stLabelColor, &stColor)), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

      /* Updates position */
      if(bLandscape != orxFALSE)
      {
        stTransform.fDstX   = fBorder;
        stTransform.fDstY  += fHeight;
      }
      else
      {
        stTransform.fDstX  += fHeight;
        stTransform.fDstY   = fScreenHeight - fBorder;
      }
    }
  }

  /* For all sorted markers */
  for(s32MarkerID = orxProfiler_GetNextSortedMarkerID(orxPROFILER_KS32_MARKER_ID_NONE);
      s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE;
      s32MarkerID = orxProfiler_GetNextSortedMarkerID(s32MarkerID))
  {
    /* Is unique and hasn't been pushed? */
    if((orxProfiler_IsUniqueMarker(s32MarkerID) != orxFALSE)
    && (orxProfiler_GetMarkerPushCount(s32MarkerID) == 0))
    {
      orxDOUBLE dTime;
      orxU32    u32Depth;

      /* Gets its time */
      dTime = orxProfiler_GetMarkerTime(s32MarkerID);

      /* Adds depth markers */
      acLabel[0] = '-';

      /* Updates depth */
      u32Depth = 1;

      /* Draws its label */
      orxString_NPrint(acLabel + u32Depth, sizeof(acLabel) - 1 - u32Depth, " %s [%.2f|%.2fms][%dx]", orxProfiler_GetMarkerName(s32MarkerID), orx2D(1000.0) * dTime, orx2D(1000.0) * orxProfiler_GetMarkerMaxTime(s32MarkerID), orxProfiler_GetMarkerPushCount(s32MarkerID));
      orxDisplay_TransformText(acLabel, pstFontBitmap, orxFont_GetMap(pstFont), &stTransform, orx2RGBA(0x66, 0x66, 0x66, 0xCC), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

      /* Updates position */
      if(bLandscape != orxFALSE)
      {
        stTransform.fDstY += fHeight;
      }
      else
      {
        stTransform.fDstX += fHeight;
      }
    }
  }

  /* Updates color */
  orxColor_Set(&stColor, &orxVECTOR_RED, orxRENDER_KF_PROFILER_BAR_ALPHA);
  orxColor_FromRGBToHSV(&stColor, &stColor);

  /* Gets hue delta */
  fHueDelta = orxRENDER_KF_PROFILER_HUE_UNSTACK_RANGE / ((s32MarkerCount != 0) ? orxS2F(s32MarkerCount) : 1);

  /* Updates vertical values & marker's height */
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth + fBorder;
    stTransform.fDstY = orxFLOAT_1;
    fHeight           = orxMath_Floor(fScreenHeight / ((s32MarkerCount > s32UniqueCount) ? orxS2F(s32MarkerCount - s32UniqueCount) : 1));
  }
  else
  {
    stTransform.fDstX = orxFLOAT_1;
    stTransform.fDstY = fScreenHeight - (orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight + fBorder);
    fHeight           = orxMath_Floor(fScreenWidth / ((s32MarkerCount > s32UniqueCount) ? orxS2F(s32MarkerCount - s32UniqueCount) : 1));
  }
  fHeight = orxCLAMP(fHeight, orxRENDER_KF_PROFILER_BAR_MIN_HEIGHT, orxRENDER_KF_PROFILER_BAR_MAX_HEIGHT);
  stTransform.fScaleY = fHeight - orx2F(2.0f);

  /* Reinits text scale */
  fTextScale = orxMIN(fTextScale, fHeight / pstMap->fCharacterHeight);
  fTextScale = orxCLAMP(fTextScale, orxRENDER_KF_PROFILER_TEXT_MIN_HEIGHT, orxRENDER_KF_PROFILER_TEXT_MAX_HEIGHT);

  /* For all markers */
  for(s32MarkerID = orxProfiler_GetNextMarkerID(orxPROFILER_KS32_MARKER_ID_NONE);
      s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE;
      s32MarkerID = orxProfiler_GetNextMarkerID(s32MarkerID))
  {
    /* Is non unique? */
    if(orxProfiler_IsUniqueMarker(s32MarkerID) == orxFALSE)
    {
      orxDOUBLE dTime;

      /* Gets its time */
      dTime = orxProfiler_GetMarkerTime(s32MarkerID);

      /* Has been pushed? */
      if(orxProfiler_GetMarkerPushCount(s32MarkerID) > 0)
      {
        orxCOLOR stBarColor;

        /* Updates its horizontal scale */
        stTransform.fScaleY = fHeight - orx2F(2.0f);
        stTransform.fScaleX = (orxFLOAT)(dTime * dRecTotalTime) * fWidth;

        /* Updates display color */
        stColor.vHSV.fH = orxMath_Mod(fHueDelta * orxS2F((s32MarkerID & 0x7FFFFFFF) % s32MarkerCount), orxFLOAT_1);

        /* Draws bar */
        orxDisplay_TransformBitmap(pstBitmap, &stTransform, orxColor_ToRGBA(orxColor_FromHSVToRGB(&stBarColor, &stColor)), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
      }

      /* Updates position */
      if(bLandscape != orxFALSE)
      {
        stTransform.fDstY += fHeight;
      }
      else
      {
        stTransform.fDstX += fHeight;
      }
    }
  }

  /* Updates vertical values & marker's height */
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth + fBorder;
    stTransform.fDstY = orxFLOAT_1 + orxMath_Floor(orx2F(0.2f) * fHeight);
  }
  else
  {
    stTransform.fDstX = orxFLOAT_1 + orxMath_Floor(orx2F(0.2f) * fHeight);
    stTransform.fDstY = fScreenHeight - (orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight + fBorder);
  }

  /* Reinits scale */
  stTransform.fScaleX = stTransform.fScaleY = fTextScale;

  /* For all markers */
  for(s32MarkerID = orxProfiler_GetNextMarkerID(orxPROFILER_KS32_MARKER_ID_NONE);
      s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE;
      s32MarkerID = orxProfiler_GetNextMarkerID(s32MarkerID))
  {
    /* Is non unique? */
    if(orxProfiler_IsUniqueMarker(s32MarkerID) == orxFALSE)
    {
      orxDOUBLE dTime;
      orxRGBA   stColor;

      /* Gets its time */
      dTime = orxProfiler_GetMarkerTime(s32MarkerID);

      /* Has been pushed? */
      if(orxProfiler_GetMarkerPushCount(s32MarkerID) > 0)
      {
        /* Updates display color */
        stColor = orx2RGBA(0xFF, 0xFF, 0xFF, 0xCC);
      }
      else
      {
        /* Updates display color */
        stColor = orx2RGBA(0x66, 0x66, 0x66, 0xCC);
      }

      /* Draws its label */
      orxString_NPrint(acLabel, sizeof(acLabel) - 1, "%s [%.2f|%.2fms][%dx]", orxProfiler_GetMarkerName(s32MarkerID), orx2D(1000.0) * dTime, orx2D(1000.0) * orxProfiler_GetMarkerMaxTime(s32MarkerID), orxProfiler_GetMarkerPushCount(s32MarkerID));
      orxDisplay_TransformText(acLabel, pstFontBitmap, orxFont_GetMap(pstFont), &stTransform, stColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

      /* Updates position */
      if(bLandscape != orxFALSE)
      {
        stTransform.fDstY += fHeight;
      }
      else
      {
        stTransform.fDstX += fHeight;
      }
    }
  }

#ifdef __orxPROFILER__

  /* Draws separator */
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX   = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth;
    stTransform.fScaleX = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth;
  }
  else
  {
    stTransform.fDstY   = fScreenHeight - orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight;
    stTransform.fScaleX = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight;
  }
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, orx2RGBA(0xFF, 0xFF, 0xFF, 0xCC), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
  stTransform.fScaleX = orxFLOAT_1;
  if(bLandscape != orxFALSE)
  {
    stTransform.fDstX = orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenWidth + fBorder;
  }
  else
  {
    stTransform.fDstY = fScreenHeight - (orxRENDER_KF_PROFILER_SEPARATOR_WIDTH * fScreenHeight + fBorder);
  }

  /* Draws memory stats */
  {
    static const orxFLOAT   sfSaturationThreshold = orxU2F(1.0f / (1024.0f * 1024.0f * 1024.0f));
    static const orxSTRING  azUnitList[] = {"B", "KB", "MB", "GB", "TB"};
    orxU64                  u64TotalCount = 0, u64TotalPeakCount = 0, u64TotalSize = 0, u64TotalPeakSize = 0, u64TotalOperationCount = 0;
    orxU32                  u32UnitIndex;

    /* For all memory types, including total */
    for(i = 0; i <= orxMEMORY_TYPE_NUMBER; i++)
    {
      const orxSTRING zType;
      orxU64          u64Count, u64PeakCount, u64Size, u64PeakSize, u64OperationCount;
      orxFLOAT        fSize, fPeakSize;

      /* Updates position */
      if(bLandscape != orxFALSE)
      {
        stTransform.fDstY += 20.0f;
      }
      else
      {
        stTransform.fDstX += 20.0f;
      }

      /* Total? */
      if(i == orxMEMORY_TYPE_NUMBER)
      {
        /* Gets values */
        u64Count             = u64TotalCount;
        u64PeakCount         = u64TotalPeakCount;
        u64Size              = u64TotalSize;
        u64PeakSize          = u64TotalPeakSize;
        u64OperationCount    = u64TotalOperationCount;
      }
      else
      {
        /* Gets its usage info */
        orxMemory_GetUsage((orxMEMORY_TYPE)i, &u64Count, &u64PeakCount, &u64Size, &u64PeakSize, &u64OperationCount);

        /* Updates totals */
        u64TotalCount            += u64Count;
        u64TotalPeakCount        += u64PeakCount;
        u64TotalSize             += u64Size;
        u64TotalPeakSize         += u64PeakSize;
        u64TotalOperationCount   += u64OperationCount;
      }

      /* Finds best unit */
      for(u32UnitIndex = 0, fSize = orxU2F(u64Size), fPeakSize = orxU2F(u64PeakSize);
          (u32UnitIndex < orxARRAY_GET_ITEM_COUNT(azUnitList) - 1) && (fPeakSize > orx2F(1024.0f));
          u32UnitIndex++, fSize *= orx2F(1.0f/1024.0f), fPeakSize *= orx2F(1.0f/1024.0f))
        ;

      /* Is used? */
      if(u64Count > 0)
      {
        /* Inits display color */
        orxColor_SetRGBA(&stColor, orx2RGBA(0xFF, 0xFF, 0xFF, 0xCC));
      }
      else
      {
        /* Inits display color */
        orxColor_SetRGBA(&stColor, orx2RGBA(0x66, 0x66, 0x66, 0xCC));
      }

      /* Updates color */
      orxColor_FromRGBToHSV(&stColor, &stColor);
      stColor.vHSV.fH = orxLERP(0.33f, orxFLOAT_0, orxU2F(u64Size) * sfSaturationThreshold);
      stColor.vHSV.fS = orx2F(0.8f);
      orxColor_FromHSVToRGB(&stColor, &stColor);

      /* Total? */
      if(i == orxMEMORY_TYPE_NUMBER)
      {
        /* Gets type string */
        zType = "MEM_TOTAL";
      }
      else
      {
        /* Gets type string */
        zType = orxMemory_GetTypeName((orxMEMORY_TYPE)i);
      }

      /* Draws it */
      orxString_NPrint(acLabel, sizeof(acLabel) - 1, "%-12s[%d|%dx] [%.2f|%.2f%s] [%d#]", zType, u64Count, u64PeakCount, fSize, fPeakSize, azUnitList[u32UnitIndex], u64OperationCount);
      orxDisplay_TransformText(acLabel, pstFontBitmap, orxFont_GetMap(pstFont), &stTransform, orxColor_ToRGBA(&stColor), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
    }
  }

#endif /* __orxPROFILER__ */

  /* Re-enables marker operations */
  orxProfiler_EnableMarkerOperations(orxTRUE);

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Renders console
 */
static orxINLINE void orxRender_Home_RenderConsole()
{
  orxDISPLAY_TRANSFORM    stTransform;
  orxVECTOR               vColor;
  orxTEXTURE             *pstTexture;
  orxBITMAP              *pstBitmap, *pstFontBitmap;
  orxFLOAT                fScreenWidth, fScreenHeight, fBackupY;
  orxU32                  u32CursorIndex, i, u32Count, u32MaxLength, u32Offset;
  orxCHAR                 acBackup[2];
  orxFLOAT                fCharacterHeight, fCharacterWidth;
  orxRGBA                 stBackgroundColor, stSeparatorColor, stLogColor, stInputColor, stCompletionColor;
  const orxFONT          *pstFont;
  const orxCHARACTER_MAP *pstMap;
  const orxSTRING         zText;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxRender_RenderConsole");

  /* Disables marker operations */
  orxProfiler_EnableMarkerOperations(orxFALSE);

  /* Gets console font */
  pstFont = orxConsole_GetFont();

  /* Gets its bitmap */
  pstFontBitmap = orxTexture_GetBitmap(orxFont_GetTexture(pstFont));

  /* Gets its map */
  pstMap = orxFont_GetMap(pstFont);

  /* Gets character size */
  fCharacterHeight  = sstRender.fConsoleFontScale * orxFont_GetCharacterHeight(pstFont);
  fCharacterWidth   = sstRender.fConsoleFontScale * orxFont_GetCharacterWidth(pstFont, orxString_GetFirstCharacterCodePoint(" ", orxNULL));

  /* Gets pixel texture */
  pstTexture = orxTexture_Get(orxTEXTURE_KZ_PIXEL);

  /* Gets its bitmap */
  pstBitmap = orxTexture_GetBitmap(pstTexture);

  /* Resets its clipping */
  orxDisplay_SetBitmapClipping(pstBitmap, 0, 0, 1, 1);

  /* Gets screen size */
  orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

  /* Inits transform */
  stTransform.fSrcX     = stTransform.fSrcY     = orxFLOAT_0;
  stTransform.fRepeatX  = stTransform.fRepeatY  = orxFLOAT_1;
  stTransform.fRotation = orxFLOAT_0;

  /* Pushes config section */
  orxConfig_PushSection(orxRENDER_KZ_CONFIG_SECTION);

  /* Gets colors */
#define orxRENDER_GET_COLOR(Name, NAME)                                                                                             \
  if(orxConfig_GetVector(orxRENDER_KZ_CONFIG_CONSOLE_##NAME##_COLOR, &vColor) != orxNULL)                                           \
  {                                                                                                                                 \
    st##Name##Color = orx2RGBA(orxF2U(vColor.fR), orxF2U(vColor.fG), orxF2U(vColor.fB), orxRENDER_KST_CONSOLE_##NAME##_COLOR.u8A);  \
  }                                                                                                                                 \
  else                                                                                                                              \
  {                                                                                                                                 \
    st##Name##Color = orxRENDER_KST_CONSOLE_##NAME##_COLOR;                                                                         \
  }                                                                                                                                 \
  if(orxConfig_HasValue(orxRENDER_KZ_CONFIG_CONSOLE_##NAME##_ALPHA) != orxFALSE)                                                    \
  {                                                                                                                                 \
    st##Name##Color.u8A = (orxU8)(orxCOLOR_DENORMALIZER * orxConfig_GetFloat(orxRENDER_KZ_CONFIG_CONSOLE_##NAME##_ALPHA));          \
  }

  /* Background color */
  orxRENDER_GET_COLOR(Background, BACKGROUND)
  /* separator color */
  orxRENDER_GET_COLOR(Separator, SEPARATOR)
  /* Log color */
  orxRENDER_GET_COLOR(Log, LOG)
  /* Input color */
  orxRENDER_GET_COLOR(Input, INPUT)
  /* Completion color */
  orxRENDER_GET_COLOR(Completion, COMPLETION)

#undef orxRENDER_GET_COLOR

  /* Uses color short names as background override */
  if(orxConfig_GetVector(orxRENDER_KZ_CONFIG_CONSOLE_COLOR, &vColor) != orxNULL)
  {
    stBackgroundColor.u8R = (orxU8)orxF2U(vColor.fR);
    stBackgroundColor.u8G = (orxU8)orxF2U(vColor.fG);
    stBackgroundColor.u8B = (orxU8)orxF2U(vColor.fB);
  }
  if(orxConfig_HasValue(orxRENDER_KZ_CONFIG_CONSOLE_ALPHA) != orxFALSE)
  {
    stBackgroundColor.u8A = (orxU8)orxF2U(orxCOLOR_DENORMALIZER * orxConfig_GetFloat(orxRENDER_KZ_CONFIG_CONSOLE_ALPHA));
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Draws background */
  stTransform.fDstX   = orxMath_Floor(fScreenWidth * orxRENDER_KF_CONSOLE_MARGIN_WIDTH) - orxFLOAT_1;
  stTransform.fDstY   = sstRender.fConsoleOffset;
  stTransform.fScaleX = orxMath_Floor(fScreenWidth * (orxFLOAT_1 - orx2F(2.0f) * orxRENDER_KF_CONSOLE_MARGIN_WIDTH)) + orx2F(2.0f);
  stTransform.fScaleY = orxMath_Floor(fScreenHeight * (orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_HEIGHT)) + orxFLOAT_1;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, stBackgroundColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

  /* Draws separators */
  stTransform.fDstY   = sstRender.fConsoleOffset + orxMath_Floor((orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_HEIGHT) * fScreenHeight - orx2F(1.5f) * fCharacterHeight);
  stTransform.fScaleY = orxFLOAT_1;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, stSeparatorColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
  stTransform.fDstY   = sstRender.fConsoleOffset + orxMath_Floor((orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_HEIGHT) * fScreenHeight) + orxFLOAT_1;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, stSeparatorColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
  stTransform.fDstY   = sstRender.fConsoleOffset;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, stSeparatorColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
  stTransform.fScaleY = orxMath_Floor(fScreenHeight * (orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_HEIGHT)) + orxFLOAT_1;
  stTransform.fScaleX = orxFLOAT_1;
  stTransform.fDstX   = orxMath_Floor(fScreenWidth * orxRENDER_KF_CONSOLE_MARGIN_WIDTH) - orxFLOAT_1;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, stSeparatorColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
  stTransform.fDstX   = orxMath_Floor(fScreenWidth * (orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_WIDTH));
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, stSeparatorColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);

  /* Clips screen bitmap */
  orxDisplay_SetBitmapClipping(orxNULL, 0, 0, orxF2U(orxMath_Floor(fScreenWidth * (orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_WIDTH) - orxFLOAT_1)), orxF2U(fScreenHeight));

  /* Displays input + cursor + autocompletion */
  stTransform.fDstX   = orxMath_Floor(orxRENDER_KF_CONSOLE_MARGIN_WIDTH * fScreenWidth);
  stTransform.fDstY   = sstRender.fConsoleOffset + orxMath_Floor((orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_HEIGHT) * fScreenHeight - fCharacterHeight);
  stTransform.fScaleY = stTransform.fScaleX = sstRender.fConsoleFontScale;
  zText               = orxConsole_GetInput(&u32CursorIndex);
  acBackup[0]         = zText[u32CursorIndex];
  acBackup[1]         = (u32CursorIndex < 255) ? zText[u32CursorIndex + 1] : orxCHAR_NULL;

  /* Has room for cursor & should display it? */
  if((u32CursorIndex < 255) && (orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_CONSOLE_BLINK)))
  {
    /* Is in insert mode? */
    if(orxConsole_IsInsertMode() != orxFALSE)
    {
      /* Displays full input, including auto-completion */
      orxDisplay_TransformText(zText, pstFontBitmap, pstMap, &stTransform, stCompletionColor, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);

      /* Overrides characters at cursor position */
      ((orxCHAR*)zText)[u32CursorIndex] = orxRENDER_KC_CONSOLE_INSERT_MARKER;
    }
    else
    {
      /* Overrides characters at cursor position */
      ((orxCHAR*)zText)[u32CursorIndex] = orxRENDER_KC_CONSOLE_OVERTYPE_MARKER;

      /* Displays full input, including auto-completion */
      orxDisplay_TransformText(zText, pstFontBitmap, pstMap, &stTransform, stCompletionColor, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);
    }

    /* Truncates to base input + cursor */
    ((orxCHAR*)zText)[u32CursorIndex + 1] = orxCHAR_NULL;
  }
  else
  {
    /* Displays full input, including auto-completion */
    orxDisplay_TransformText(zText, pstFontBitmap, pstMap, &stTransform, stCompletionColor, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);

    /* Truncates to base input */
    ((orxCHAR*)zText)[u32CursorIndex] = orxCHAR_NULL;
  }

  /* Displays base input (ie. validated part) */
  orxDisplay_TransformText(zText, pstFontBitmap, pstMap, &stTransform, stInputColor, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);
  ((orxCHAR*)zText)[u32CursorIndex] = acBackup[0];
  if(u32CursorIndex < 255)
  {
    ((orxCHAR*)zText)[u32CursorIndex + 1] = acBackup[1];
  }

  /* While there are log lines to display */
  for(i = 0, stTransform.fDstY -= orx2F(2.0f) * fCharacterHeight, fBackupY = stTransform.fDstY;
      (stTransform.fDstY >= sstRender.fConsoleOffset - fCharacterHeight) && ((zText = orxConsole_GetTrailLogLine(i)) != orxSTRING_EMPTY);
      i++, stTransform.fDstY -= fCharacterHeight)
  {
    /* Displays it */
    orxDisplay_TransformText(zText, pstFontBitmap, pstMap, &stTransform, stLogColor, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);
  }

  /* Gets log offset */
  u32Offset = orxConsole_GetTrailLogLineOffset();

  /* Has offset? */
  if(u32Offset != 0)
  {
    orxCHAR   acBuffer[32];
    orxFLOAT  fBackupX;

    /* Displays it */
    orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "+%u %s", u32Offset, (u32Offset == 1) ? "line" : "lines");
    fBackupX          = stTransform.fDstX;
    stTransform.fDstX = orxMath_Floor(fScreenWidth * (orxFLOAT_1 - orxRENDER_KF_CONSOLE_MARGIN_WIDTH)) - (orxString_GetLength(acBuffer) * fCharacterWidth);
    stTransform.fDstY = fBackupY;
    orxDisplay_TransformText(acBuffer, pstFontBitmap, pstMap, &stTransform, stInputColor, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);
    stTransform.fDstX = fBackupX;
  }

  /* Gets completion count */
  u32Count = orxConsole_GetCompletionCount(&u32MaxLength);

  /* Draws overlay */
  stBackgroundColor.u8A = (orxU8)orxF2U(orxCOLOR_DENORMALIZER * orx2F(0.9f));
  stTransform.fDstY     = fBackupY - (u32Count - 1) * fCharacterHeight;
  stTransform.fScaleX   = u32MaxLength * fCharacterWidth;
  stTransform.fScaleY   = u32Count * fCharacterHeight;
  orxDisplay_TransformBitmap(pstBitmap, &stTransform, stBackgroundColor, orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_ALPHA);
  stTransform.fScaleY   = stTransform.fScaleX = sstRender.fConsoleFontScale;

  /* For all current completions */
  for(i = 0;
      i < u32Count;
      i++)
  {
    orxBOOL bActive;

    /* Gets it */
    zText = orxConsole_GetCompletion(i, &bActive);

    /* Updates position */
    stTransform.fDstY = fBackupY - ((u32Count - i - 1) * fCharacterHeight);

    /* Displays it */
    orxDisplay_TransformText(zText, pstFontBitmap, pstMap, &stTransform, (bActive != orxFALSE) ? stInputColor : stCompletionColor, orxDISPLAY_SMOOTHING_OFF, orxDISPLAY_BLEND_MODE_ALPHA);
  }

  /* Restores screen bitmap clipping */
  orxDisplay_SetBitmapClipping(orxDisplay_GetScreenBitmap(), 0, 0, orxF2U(fScreenWidth), orxF2U(fScreenHeight));

  /* Re-enables marker operations */
  orxProfiler_EnableMarkerOperations(orxTRUE);

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Renders a viewport
 * @param[in]   _pstRenderNode    Render node
 * @param[in]   _pstTransform     Rendering transform
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxRender_Home_RenderObject(const orxRENDER_NODE *_pstRenderNode, orxDISPLAY_TRANSFORM *_pstTransform)
{
  orxRENDER_EVENT_PAYLOAD stPayload;
  orxOBJECT              *pstObject;
  orxEVENT                stEvent;
  orxSTATUS               eResult = orxSTATUS_FAILURE;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxRender_RenderObject");

  /* Gets object */
  pstObject = _pstRenderNode->pstObject;

  /* Cleans event payload */
  orxMemory_Zero(&stPayload, sizeof(orxRENDER_EVENT_PAYLOAD));

  /* Inits it */
  stPayload.stObject.pstTransform = _pstTransform;

  /* Inits event */
  orxEVENT_INIT(stEvent, orxEVENT_TYPE_RENDER, orxRENDER_EVENT_OBJECT_START, (orxHANDLE)pstObject, (orxHANDLE)pstObject, &stPayload);

  /* Should render? */
  if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
  {
    orxGRAPHIC *pstGraphic;
    orxTEXTURE *pstTexture;
    orxTEXT    *pstText;
    orxFONT    *pstFont;
    orxBITMAP  *pstBitmap = orxNULL;
    orxBOOL     bIs2D;

    /* Gets object's working graphic */
    pstGraphic = orxObject_GetWorkingGraphic(pstObject);

    /* Stores type */
    bIs2D = orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_2D);

    /* Is 2D? */
    if(bIs2D != orxFALSE)
    {
      orxVECTOR vClipTL, vClipBR, vSize;

      /* Profiles */
      orxPROFILER_PUSH_MARKER("RenderObject <2D>");

      /* Gets its texture */
      pstTexture = orxTEXTURE(orxGraphic_GetData(pstGraphic));

      /* Gets its bitmap */
      pstBitmap = orxTexture_GetBitmap(pstTexture);

      /* Gets its clipping corners */
      orxGraphic_GetOrigin(pstGraphic, &vClipTL);
      orxGraphic_GetSize(pstGraphic, &vSize);
      orxVector_Add(&vClipBR, &vClipTL, &vSize);

      /* Updates its clipping (before event start for updated texture coordinates in shader) */
      orxDisplay_SetBitmapClipping(pstBitmap, orxF2U(vClipTL.fX), orxF2U(vClipTL.fY), orxF2U(vClipBR.fX), orxF2U(vClipBR.fY));
    }
    else
    {
      /* Profiles */
      orxPROFILER_PUSH_MARKER("RenderObject <Text>");

      /* Gets text */
      pstText = orxTEXT(orxGraphic_GetData(pstGraphic));

      /* Valid? */
      if(pstText != orxNULL)
      {
        /* Gets its font */
        pstFont = orxText_GetFont(pstText);

        /* Valid? */
        if(pstFont != orxNULL)
        {
          /* Gets its texture */
          pstTexture = orxFont_GetTexture(pstFont);
          orxASSERT(pstTexture != orxNULL);

          /* Gets its bitmap */
          pstBitmap = orxTexture_GetBitmap(pstTexture);
        }
      }
    }

    /* Valid bitmap? */
    if(pstBitmap != orxNULL)
    {
      /* Valid scale? */
      if((stPayload.stObject.pstTransform->fScaleX != orxFLOAT_0) && (stPayload.stObject.pstTransform->fScaleY != orxFLOAT_0))
      {
        orxVECTOR         vPivot;
        orxSHADERPOINTER *pstShaderPointer;
        orxCOLOR          stColor;
        orxBOOL           bGraphicFlipX, bGraphicFlipY, bObjectFlipX, bObjectFlipY;

        /* Gets its shader pointer */
        pstShaderPointer = orxOBJECT_GET_STRUCTURE(orxOBJECT(pstObject), SHADERPOINTER);

        /* Valid? */
        if(pstShaderPointer != orxNULL)
        {
          /* Starts it */
          orxShaderPointer_Start(pstShaderPointer);
        }

        /* Gets graphic's pivot */
        orxGraphic_GetPivot(pstGraphic, &vPivot);

        /* Gets object & graphic flipping */
        orxObject_GetFlip(pstObject, &bObjectFlipX, &bObjectFlipY);
        orxGraphic_GetFlip(pstGraphic, &bGraphicFlipX, &bGraphicFlipY);

        /* Updates using combined flipping */
        if(bObjectFlipX ^ bGraphicFlipX)
        {
          stPayload.stObject.pstTransform->fScaleX *= -orxFLOAT_1;
        }
        if(bObjectFlipY ^ bGraphicFlipY)
        {
          stPayload.stObject.pstTransform->fScaleY *= -orxFLOAT_1;
        }

        /* Updates transform */
        stPayload.stObject.pstTransform->fSrcX += vPivot.fX;
        stPayload.stObject.pstTransform->fSrcY += vPivot.fY;

        /* Has graphic color? */
        if(orxGraphic_HasColor(pstGraphic) != orxFALSE)
        {
          /* Updates display color */
          orxGraphic_GetColor(pstGraphic, &stColor);
        }
        /* Has object color? */
        else if(orxObject_HasColor(pstObject) != orxFALSE)
        {
          /* Updates display color */
          orxObject_GetColor(pstObject, &stColor);
        }
        else
        {
          /* Applies white color */
          orxColor_SetRGBA(&stColor, orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF));
        }

        /* Is 2D? */
        if(bIs2D != orxFALSE)
        {
          /* Transforms bitmap */
          eResult = orxDisplay_TransformBitmap(pstBitmap, stPayload.stObject.pstTransform, orxColor_ToRGBA(&stColor), _pstRenderNode->eSmoothing, _pstRenderNode->eBlendMode);
        }
        else
        {
          /* Transforms text */
          eResult = orxDisplay_TransformText(orxText_GetString(pstText), pstBitmap, orxFont_GetMap(pstFont), stPayload.stObject.pstTransform, orxColor_ToRGBA(&stColor), _pstRenderNode->eSmoothing, _pstRenderNode->eBlendMode);
        }

        /* Has shader pointer? */
        if(pstShaderPointer != orxNULL)
        {
          /* Stops it */
          orxShaderPointer_Stop(pstShaderPointer);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Scaling component when rendering object [%s] should not be 0, got (%g, %g).", orxObject_GetName(_pstRenderNode->pstObject), stPayload.stObject.pstTransform->fScaleX, stPayload.stObject.pstTransform->fScaleY);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "No valid bitmap found when rendering object [%s].", orxObject_GetName(_pstRenderNode->pstObject));
    }

    /* Profiles */
    orxPROFILER_POP_MARKER();
  }
  else
  {
    /* Updates result, aborted by user request */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Sends stop event */
  orxEVENT_SEND(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_OBJECT_STOP, (orxHANDLE)pstObject, (orxHANDLE)pstObject, &stPayload);

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Renders a viewport
 * @param[in]   _pstViewport    Viewport to render
 */
static orxINLINE void orxRender_Home_RenderViewport(const orxVIEWPORT *_pstViewport)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxRender_RenderViewport");

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Is viewport enabled? */
  if(orxViewport_IsEnabled(_pstViewport) != orxFALSE)
  {
    orxU32      u32TextureCount, i;
    orxTEXTURE *apstTextureList[orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER];
    orxBITMAP  *apstBitmapList[orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER];
    orxBOOL     bSuccess = orxTRUE;

    /* Gets viewport's texture count */
    u32TextureCount = orxViewport_GetTextureCount(_pstViewport);

    /* Gets viewport textures */
    orxViewport_GetTextureList(_pstViewport, u32TextureCount, apstTextureList);

    /* For all of them */
    for(i = 0; i < u32TextureCount; i++)
    {
      /* Gets its bitmap */
      apstBitmapList[i] = orxTexture_GetBitmap(apstTextureList[i]);

      /* Invalid? */
      if(apstBitmapList[i] == orxNULL)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to get a textures for viewport.");

        /* Updates status */
        bSuccess = orxFALSE;

        break;
      }
    }

    /* Valid? */
    if(bSuccess != orxFALSE)
    {
      orxEVENT stEvent;

      /* Inits event */
      orxEVENT_INIT(stEvent, orxEVENT_TYPE_RENDER, orxRENDER_EVENT_VIEWPORT_START, (orxHANDLE)_pstViewport, (orxHANDLE)_pstViewport, orxNULL);

      /* Sends start event */
      if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
      {
        orxAABOX  stViewportBox, stTextureBox;
        orxFLOAT  fTextureWidth, fTextureHeight;
        orxVECTOR vViewportCenter;

        /* Gets texture size */
        orxTexture_GetSize(apstTextureList[0], &fTextureWidth, &fTextureHeight);

        /* Inits texture box */
        orxVector_SetAll(&(stTextureBox.vTL), orxFLOAT_0);
        orxVector_Set(&(stTextureBox.vBR), fTextureWidth, fTextureHeight, orxFLOAT_0);

        /* Gets viewport box */
        orxViewport_GetBox(_pstViewport, &stViewportBox);

        /* Gets its center */
        orxAABox_GetCenter(&stViewportBox, &vViewportCenter);

        /* Sets destination bitmap */
        orxDisplay_SetDestinationBitmaps(apstBitmapList, u32TextureCount);

        /* Does it intersect with texture? */
        if(orxAABox_Test2DIntersection(&stTextureBox, &stViewportBox) != orxFALSE)
        {
          orxFLOAT    fCorrectionRatio;
          orxCOLOR    stColor;
          orxBOOL     bHasColor = orxFALSE;
          orxCAMERA  *pstCamera;

          /* Gets current correction ratio */
          fCorrectionRatio = orxViewport_GetCorrectionRatio(_pstViewport);

          /* Has correction ratio? */
          if(fCorrectionRatio != orxFLOAT_1)
          {
            /* X axis? */
            if(fCorrectionRatio < orxFLOAT_1)
            {
              orxFLOAT fDelta;

              /* Gets rendering limit delta using correction ratio */
              fDelta = orx2F(0.5f) * (orxFLOAT_1 - fCorrectionRatio) * (stViewportBox.vBR.fX - stViewportBox.vTL.fX);

              /* Updates viewport */
              stViewportBox.vTL.fX += fDelta;
              stViewportBox.vBR.fX -= fDelta;
            }
            /* Y axis */
            else
            {
              orxFLOAT fDelta;

              /* Gets rendering limit delta using correction ratio */
              fDelta = orx2F(0.5f) * (orxFLOAT_1 - (orxFLOAT_1 / fCorrectionRatio)) * (stViewportBox.vBR.fY - stViewportBox.vTL.fY);

              /* Updates viewport */
              stViewportBox.vTL.fY += fDelta;
              stViewportBox.vBR.fY -= fDelta;
            }
          }

          /* Does viewport have a background color? */
          if((bHasColor = orxViewport_HasBackgroundColor(_pstViewport)) != orxFALSE)
          {
            /* Gets it */
            orxViewport_GetBackgroundColor(_pstViewport, &stColor);
          }

          /* For all bitmaps */
          for(i = 0; i < u32TextureCount; i++)
          {
            /* Sets its clipping */
            orxDisplay_SetBitmapClipping(apstBitmapList[i], orxF2U(orxMAX(stTextureBox.vTL.fX, stViewportBox.vTL.fX)), orxF2U(orxMAX(stTextureBox.vTL.fY, stViewportBox.vTL.fY)), orxF2U(orxCLAMP(stTextureBox.vBR.fX, orxFLOAT_0, stViewportBox.vBR.fX)), orxF2U(orxCLAMP(stTextureBox.vBR.fY, orxFLOAT_0, stViewportBox.vBR.fY)));
          }

          /* Does viewport have a background color? */
          if(bHasColor != orxFALSE)
          {
            /* Clears bitmap */
            orxDisplay_ClearBitmap(orxNULL, orxColor_ToRGBA(&stColor));
          }

          /* Gets camera */
          pstCamera = orxViewport_GetCamera(_pstViewport);

          /* Valid 2D camera? */
          if((pstCamera != orxNULL)
          && (orxStructure_TestFlags(pstCamera, orxCAMERA_KU32_FLAG_2D) != orxFALSE))
          {
            orxAABOX stFrustum;
            orxFLOAT fCameraWidth, fCameraHeight;

            /* Gets camera frustum */
            orxCamera_GetFrustum(pstCamera, &stFrustum);

            /* Gets camera size */
            fCameraWidth  = stFrustum.vBR.fX - stFrustum.vTL.fX;
            fCameraHeight = stFrustum.vBR.fY - stFrustum.vTL.fY;

            /* Valid? */
            if((fCameraWidth > orxFLOAT_0)
            && (fCameraHeight > orxFLOAT_0))
            {
              orxU32          u32Number;
              orxOBJECT      *pstObject;
              orxFRAME       *pstCameraFrame;
              orxRENDER_NODE *pstRenderNode;
              orxVECTOR       vCameraScale, vCameraCenter, vCameraPosition;
              orxFLOAT        fCameraDepth, fRenderScaleX, fRenderScaleY, fRecZoom, fRenderRotation, fCameraBoundingRadius;

              /* Gets camera frame */
              pstCameraFrame = orxCamera_GetFrame(pstCamera);

              /* Gets camera scale */
              orxFrame_GetScale(pstCameraFrame, orxFRAME_SPACE_GLOBAL, &vCameraScale);

              /* Gets camera reciprocal zoom */
              fRecZoom = (vCameraScale.fX != orxFLOAT_0) ? vCameraScale.fX : orxFLOAT_1;

              /* Gets camera position */
              orxFrame_GetPosition(pstCameraFrame, orxFRAME_SPACE_GLOBAL, &vCameraPosition);

              /* Gets camera world frustum */
              orxVector_Add(&(stFrustum.vTL), &(stFrustum.vTL), &vCameraPosition);
              orxVector_Add(&(stFrustum.vBR), &(stFrustum.vBR), &vCameraPosition);

              /* Gets camera center */
              orxVector_Add(&vCameraCenter, &(stFrustum.vTL), &(stFrustum.vBR));
              orxVector_Mulf(&vCameraCenter, &vCameraCenter, orx2F(0.5f));

              /* Gets camera depth */
              fCameraDepth = stFrustum.vBR.fZ - vCameraPosition.fZ;

              /* Gets camera square bounding radius */
              fCameraBoundingRadius = orx2F(0.5f) * orxMath_Sqrt((fCameraWidth * fCameraWidth) + (fCameraHeight * fCameraHeight)) * fRecZoom;

              /* Gets rendering scales */
              fRenderScaleX = (stViewportBox.vBR.fX - stViewportBox.vTL.fX) / (fRecZoom * fCameraWidth);
              fRenderScaleY = (stViewportBox.vBR.fY - stViewportBox.vTL.fY) / (fRecZoom * fCameraHeight);

              /* Gets camera rotation */
              fRenderRotation = orxFrame_GetRotation(pstCameraFrame, orxFRAME_SPACE_GLOBAL);

              /* For all camera group IDs */
              for(i = 0, u32Number = orxCamera_GetGroupIDCount(pstCamera); i < u32Number; i++)
              {
                orxSTRINGID stGroupID;

                /* Gets it */
                stGroupID = orxCamera_GetGroupID(pstCamera, i);

                /* For all objects in this group */
                for(pstObject = orxObject_GetNextEnabled(orxNULL, stGroupID);
                    pstObject != orxNULL;
                    pstObject = orxObject_GetNextEnabled(pstObject, stGroupID))
                {
                  /* Is object enabled? */
                  if(orxObject_IsEnabled(pstObject) != orxFALSE)
                  {
                    orxGRAPHIC *pstGraphic;

                    /* Gets object's graphic */
                    pstGraphic = orxObject_GetWorkingGraphic(pstObject);

                    /* Valid 2D graphic? */
                    if((pstGraphic != orxNULL)
                    && (orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_2D | orxGRAPHIC_KU32_FLAG_TEXT) != orxFALSE))
                    {
                      orxFRAME     *pstFrame;
                      orxSTRUCTURE *pstData;
                      orxTEXTURE   *pstTexture;

                      /* Gets object's frame */
                      pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

                      /* Gets graphic data */
                      pstData = orxGraphic_GetData(pstGraphic);

                      /* Valid and has text/texture data? */
                      if((pstFrame != orxNULL)
                      && (((pstTexture = orxTEXTURE(pstData)) != orxNULL)
                       || (orxTEXT(pstData) != orxNULL)))
                      {
                        orxVECTOR vObjectPos;

                        /* Not a texture? */
                        if(pstTexture == orxNULL)
                        {
                          /* Gets texture from text */
                          pstTexture = orxFont_GetTexture(orxText_GetFont(orxTEXT(pstData)));
                        }

                        /* Gets its position */
                        orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_GLOBAL, &vObjectPos);

                        /* Is object in Z frustum? */
                        if((vObjectPos.fZ >= vCameraPosition.fZ) && (vObjectPos.fZ >= stFrustum.vTL.fZ) && (vObjectPos.fZ <= stFrustum.vBR.fZ))
                        {
                          orxFLOAT  fObjectBoundingRadius, fSqrDist, fDepthCoef, fObjectRotation;
                          orxVECTOR vSize, vOffset, vObjectScale, vDist;

                          /* Gets its size */
                          orxGraphic_GetSize(pstGraphic, &vSize);

                          /* Gets object's scale & rotation */
                          orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vObjectScale);
                          fObjectRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL);

                          /* Updates its size with object scale */
                          vSize.fX  *= vObjectScale.fX;
                          vSize.fY  *= vObjectScale.fY;

                          /* Gets offset based on pivot */
                          orxGraphic_GetPivot(pstGraphic, &vOffset);
                          vOffset.fX = orx2F(0.5f) * vSize.fX - vObjectScale.fX * vOffset.fX;
                          vOffset.fY = orx2F(0.5f) * vSize.fY - vObjectScale.fY * vOffset.fY;
                          orxVector_2DRotate(&vOffset, &vOffset, fObjectRotation);

                          /* Gets real 2D distance vector */
                          orxVector_Sub(&vDist, &vObjectPos, &vCameraCenter);
                          vDist.fZ = orxFLOAT_0;

                          /* Uses differential scrolling or depth scaling? */
                          if((orxStructure_TestFlags(pstFrame, orxFRAME_KU32_MASK_SCROLL_BOTH) != orxFALSE)
                          || (orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_DEPTH_SCALE) != orxFALSE))
                          {
                            orxFLOAT fObjectRelativeDepth;

                            /* Gets objects relative depth */
                            fObjectRelativeDepth = vObjectPos.fZ - vCameraPosition.fZ;

                            /* On near plane? */
                            if(fObjectRelativeDepth == orxFLOAT_0)
                            {
                              /* Prints error message */
                              orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "[%s] is using AutoScroll/DepthScale and is on [%s]'s near plane: undefined results.", orxObject_GetName(pstObject), orxCamera_GetName(pstCamera));
                            }

                            /* Near space? */
                            if(fObjectRelativeDepth < (orx2F(0.5f) * fCameraDepth))
                            {
                              /* Gets depth scale coef */
                              fDepthCoef = (orx2F(0.5f) * fCameraDepth) / (fObjectRelativeDepth + orxMATH_KF_TINY_EPSILON);
                            }
                            /* Far space */
                            else
                            {
                              /* Gets depth scale coef */
                              fDepthCoef = (fCameraDepth - fObjectRelativeDepth) / (orx2F(0.5f) * fCameraDepth);
                            }

                            /* X-axis scroll? */
                            if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_X) != orxFALSE)
                            {
                              /* Updates base distance vector */
                              vDist.fX *= fDepthCoef;
                            }

                            /* Y-axis scroll? */
                            if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_Y) != orxFALSE)
                            {
                              /* Updates base distance vector */
                              vDist.fY *= fDepthCoef;
                            }

                            /* Depth scale? */
                            if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_DEPTH_SCALE) != orxFALSE)
                            {
                              /* Updates size & offset */
                              vSize.fX *= fDepthCoef;
                              vSize.fY *= fDepthCoef;
                              vOffset.fX *= fDepthCoef;
                              vOffset.fY *= fDepthCoef;
                            }
                          }
                          else
                          {
                            /* Clears depth coef */
                            fDepthCoef = orxFLOAT_1;
                          }

                          /* Gets object square bounding radius */
                          fObjectBoundingRadius = orx2F(0.5f) * orxMath_Sqrt((vSize.fX * vSize.fX) + (vSize.fY * vSize.fY));

                          /* Updates distance vector */
                          orxVector_Add(&vDist, &vDist, &vOffset);

                          /* Gets 2D square distance to camera */
                          fSqrDist = orxVector_GetSquareSize(&vDist);

                          /* Circle test between object & camera */
                          if(fSqrDist <= (fCameraBoundingRadius + fObjectBoundingRadius) * (fCameraBoundingRadius + fObjectBoundingRadius))
                          {
                            orxDISPLAY_BLEND_MODE eBlendMode;
                            orxDISPLAY_SMOOTHING  eSmoothing;
                            const orxSHADER      *pstShader;
                            orxSHADERPOINTER     *pstShaderPointer;

                            /* Gets shader pointer */
                            pstShaderPointer = orxOBJECT_GET_STRUCTURE(pstObject, SHADERPOINTER);

                            /* Valid? */
                            if(pstShaderPointer != orxNULL)
                            {
                              /* Gets first shader */
                              pstShader = orxShaderPointer_GetShader(pstShaderPointer, 0);
                            }
                            else
                            {
                              /* Clears shader */
                              pstShader = orxNULL;
                            }

                            /* Gets graphic smoothing */
                            eSmoothing = orxGraphic_GetSmoothing(pstGraphic);

                            /* Default? */
                            if(eSmoothing == orxDISPLAY_SMOOTHING_DEFAULT)
                            {
                              /* Gets object smoothing */
                              eSmoothing = orxObject_GetSmoothing(pstObject);
                            }

                            /* Has graphic blend mode? */
                            if(orxGraphic_HasBlendMode(pstGraphic) != orxFALSE)
                            {
                              /* Gets graphic blend mode */
                              eBlendMode = orxGraphic_GetBlendMode(pstGraphic);
                            }
                            /* Has object blend mode? */
                            else if(orxObject_HasBlendMode(pstObject) != orxFALSE)
                            {
                              /* Gets object blend mode */
                              eBlendMode = orxObject_GetBlendMode(pstObject);
                            }
                            else
                            {
                              /* Defaults to alpha blend mode */
                              eBlendMode = orxDISPLAY_BLEND_MODE_ALPHA;
                            }

                            /* Creates a render node */
                            pstRenderNode = (orxRENDER_NODE *)orxBank_Allocate(sstRender.pstRenderBank);

                            /* Cleans its internal node */
                            orxMemory_Zero(pstRenderNode, sizeof(orxLINKLIST_NODE));

                            /* Stores object */
                            pstRenderNode->pstObject  = pstObject;
                            pstRenderNode->pstTexture = pstTexture;
                            pstRenderNode->pstShader  = pstShader;
                            pstRenderNode->eSmoothing = eSmoothing;
                            pstRenderNode->eBlendMode = eBlendMode;

                            /* Stores its Z coordinate */
                            pstRenderNode->fZ = vObjectPos.fZ;

                            /* Stores its depth coef */
                            pstRenderNode->fDepthCoef = fDepthCoef;

                            /* Empty list? */
                            if(orxLinkList_GetCount(&(sstRender.stRenderList)) == 0)
                            {
                              /* Adds node at beginning */
                              orxLinkList_AddStart(&(sstRender.stRenderList), (orxLINKLIST_NODE *)pstRenderNode);
                            }
                            else
                            {
                              orxRENDER_NODE *pstNode;

                              /* Finds correct node position */
                              for(pstNode = (orxRENDER_NODE *)orxLinkList_GetFirst(&(sstRender.stRenderList));
                                  (pstNode != orxNULL)
                               && ((vObjectPos.fZ < pstNode->fZ)
                                || ((vObjectPos.fZ == pstNode->fZ)
                                 && ((pstTexture < pstNode->pstTexture)
                                  || ((pstTexture == pstNode->pstTexture)
                                   && ((pstShader < pstNode->pstShader)
                                    || ((pstShader == pstNode->pstShader)
                                     && (eBlendMode < pstNode->eBlendMode))
                                      || ((eBlendMode == pstNode->eBlendMode)
                                       && (eSmoothing < pstNode->eSmoothing)))))));
                                  pstNode = (orxRENDER_NODE *)orxLinkList_GetNext(&(pstNode->stNode)));

                              /* End of list reached? */
                              if(pstNode == orxNULL)
                              {
                                /* Adds it at end */
                                orxLinkList_AddEnd(&(sstRender.stRenderList), &(pstRenderNode->stNode));
                              }
                              else
                              {
                                /* Adds it before found node */
                                orxLinkList_AddBefore(&(pstNode->stNode), &(pstRenderNode->stNode));
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }

                /* For all render nodes */
                for(pstRenderNode = (orxRENDER_NODE *)orxLinkList_GetFirst(&(sstRender.stRenderList));
                    pstRenderNode != orxNULL;
                    pstRenderNode = (orxRENDER_NODE *)orxLinkList_GetNext((orxLINKLIST_NODE *)pstRenderNode))
                {
                  orxFRAME             *pstFrame;
                  orxVECTOR             vObjectPos, vRenderPos, vObjectScale;
                  orxFLOAT              fObjectRotation, fObjectScaleX, fObjectScaleY, fRepeatX, fRepeatY;
                  orxDISPLAY_TRANSFORM  stTransform;

                  /* Gets object */
                  pstObject = pstRenderNode->pstObject;

                  /* Gets object's position */
                  orxObject_GetWorldPosition(pstObject, &vObjectPos);

                  /* Gets object's frame */
                  pstFrame = orxOBJECT_GET_STRUCTURE(pstObject, FRAME);

                  /* Gets object's scales */
                  orxFrame_GetScale(pstFrame, orxFRAME_SPACE_GLOBAL, &vObjectScale);

                  /* Gets object's rotation */
                  fObjectRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_GLOBAL);

                  /* Gets object scale */
                  fObjectScaleX = fRenderScaleX;
                  fObjectScaleY = fRenderScaleY;

                  /* Gets position in camera space */
                  orxVector_Sub(&vRenderPos, &vObjectPos, &vCameraCenter);
                  vRenderPos.fX  *= fObjectScaleX;
                  vRenderPos.fY  *= fObjectScaleY;

                  /* Uses differential scrolling or depth scaling? */
                  if((orxStructure_TestFlags(pstFrame, orxFRAME_KU32_MASK_SCROLL_BOTH) != orxFALSE)
                  || (orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_DEPTH_SCALE) != orxFALSE))
                  {
                    /* X-axis scroll? */
                    if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_X) != orxFALSE)
                    {
                      /* Updates render position */
                      vRenderPos.fX *= pstRenderNode->fDepthCoef;
                    }

                    /* Y-axis scroll? */
                    if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_SCROLL_Y) != orxFALSE)
                    {
                      /* Updates render position */
                      vRenderPos.fY *= pstRenderNode->fDepthCoef;
                    }

                    /* Depth scale? */
                    if(orxStructure_TestFlags(pstFrame, orxFRAME_KU32_FLAG_DEPTH_SCALE) != orxFALSE)
                    {
                      /* Updates object scales */
                      vObjectScale.fX *= pstRenderNode->fDepthCoef;
                      vObjectScale.fY *= pstRenderNode->fDepthCoef;
                    }
                  }

                  /* Has camera rotation? */
                  if(fRenderRotation != orxFLOAT_0)
                  {
                    /* Rotates it */
                    orxVector_2DRotate(&vRenderPos, &vRenderPos, -fRenderRotation);
                  }

                  /* Gets position in screen space */
                  orxVector_Add(&vRenderPos, &vRenderPos, &vViewportCenter);
                  orxVector_Add(&vRenderPos, &vRenderPos, &(stTextureBox.vTL));

                  /* Gets object repeat values */
                  orxObject_GetRepeat(pstObject, &fRepeatX, &fRepeatY);

                  /* Updates if invalid */
                  if(fRepeatX == orxFLOAT_0)
                  {
                    fRepeatX = orxMATH_KF_EPSILON;
                  }
                  if(fRepeatY == orxFLOAT_0)
                  {
                    fRepeatY = orxMATH_KF_EPSILON;
                  }

                  /* Sets transformation values */
                  stTransform.fSrcX     = orxFLOAT_0;
                  stTransform.fSrcY     = orxFLOAT_0;
                  stTransform.fDstX     = vRenderPos.fX;
                  stTransform.fDstY     = vRenderPos.fY;
                  stTransform.fRepeatX  = fRepeatX;
                  stTransform.fRepeatY  = fRepeatY;
                  stTransform.fScaleX   = vObjectScale.fX * fObjectScaleX;
                  stTransform.fScaleY   = vObjectScale.fY * fObjectScaleY;
                  stTransform.fRotation = fObjectRotation - fRenderRotation;

                  /* Renders it */
                  if(orxRender_Home_RenderObject(pstRenderNode, &stTransform) == orxSTATUS_FAILURE)
                  {
                    /* Prints error message */
                    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "[orxOBJECT %p / %s] couldn't be rendered.", pstObject, orxObject_GetName(pstObject));
                  }
                }

                /* Cleans rendering bank */
                orxBank_Clear(sstRender.pstRenderBank);

                /* Cleans rendering list */
                orxMemory_Zero(&(sstRender.stRenderList), sizeof(orxLINKLIST));
              }
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Camera size out of bounds: %g, %g.", fCameraWidth, fCameraHeight);
            }
          }
          else
          {
            /* Doesn't the viewport have shaders? */
            if(orxViewport_GetShaderPointer(_pstViewport) == orxNULL)
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "No valid camera or no shader attached to viewport.");
            }
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Viewport does not intersect with texture.");
        }
      }

      /* Inits stop event */
      orxEVENT_INIT(stEvent, orxEVENT_TYPE_RENDER, orxRENDER_EVENT_VIEWPORT_STOP, (orxHANDLE)_pstViewport, (orxHANDLE)_pstViewport, orxNULL);

      /* Sends it */
      if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
      {
        const orxSHADERPOINTER *pstShaderPointer;

        /* Gets viewport's shader pointer */
        pstShaderPointer = orxViewport_GetShaderPointer(_pstViewport);

        /* Valid and enabled? */
        if((pstShaderPointer != orxNULL) && (orxShaderPointer_IsEnabled(pstShaderPointer) != orxFALSE))
        {
          /* Updates blend mode */
          orxDisplay_SetBlendMode(orxViewport_GetBlendMode(_pstViewport));

          /* Starts shader */
          orxShaderPointer_Start(pstShaderPointer);

          /* Draws render target's content */
          orxDisplay_TransformBitmap(orxNULL, orxNULL, orx2RGBA(0x00, 0x00, 0x00, 0x00), orxDISPLAY_SMOOTHING_NONE, orxDISPLAY_BLEND_MODE_NONE);

          /* Stops shader */
          orxShaderPointer_Stop(pstShaderPointer);
        }
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Not a valid bitmap.");
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Renders all (callback to register on a clock)
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pContext     Context sent when registering callback to the clock
 */
static void orxFASTCALL orxRender_Home_RenderAll(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxBOOL bRender;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* Sends render start event */
  bRender = (orxEvent_SendShort(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_START) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Should render? */
  if(bRender != orxFALSE)
  {
    orxVIEWPORT  *pstViewport;
    orxBITMAP    *pstScreen;
    orxFLOAT      fWidth, fHeight;

    /* Clears screen */
    orxDisplay_ClearBitmap(orxDisplay_GetScreenBitmap(), orx2RGBA(0x00, 0x00, 0x00, 0xFF));

    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxRender_RenderAll");

    /* For all viewports */
    for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
        pstViewport != orxNULL;
        pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
    {
      /* Renders it */
      orxRender_Home_RenderViewport(pstViewport);
    }

    /* Increases FPS count */
    orxFPS_IncreaseFrameCount();

    /* Gets screen bitmap */
    pstScreen = orxDisplay_GetScreenBitmap();

    /* Restores screen as destination bitmap */
    orxDisplay_SetDestinationBitmaps(&pstScreen, 1);

    /* Restores screen bitmap clipping */
    orxDisplay_GetScreenSize(&fWidth, &fHeight);
    orxDisplay_SetBitmapClipping(orxDisplay_GetScreenBitmap(), 0, 0, orxF2U(fWidth), orxF2U(fHeight));

    /* Pushes render config section */
    orxConfig_PushSection(orxRENDER_KZ_CONFIG_SECTION);

    /* Should render FPS? */
    if(orxConfig_GetBool(orxRENDER_KZ_CONFIG_SHOW_FPS) != orxFALSE)
    {
      /* Renders it */
      orxRender_Home_RenderFPS();
    }

    /* Should render profiler? */
    if((orxEvent_SendShort(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_PROFILER_START) != orxSTATUS_FAILURE)
    && (orxConfig_GetBool(orxRENDER_KZ_CONFIG_SHOW_PROFILER) != orxFALSE))
    {
      /* Renders it */
      orxRender_Home_RenderProfiler();

      /* Doesn't have the reset maxima callback yet? */
      if(!orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_RESET_MAXIMA))
      {
        /* Adds it */
        orxClock_AddGlobalTimer(orxRender_Home_ResetProfilerMaxima, orxFLOAT_1, -1, orxNULL);

        /* Updates status */
        orxFLAG_SET(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_RESET_MAXIMA, orxRENDER_KU32_STATIC_FLAG_NONE);

        /* Enables input set */
        orxInput_EnableSet(orxRENDER_KZ_INPUT_SET, orxTRUE);
      }

      /* Updates status */
      orxFLAG_SET(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PROFILER, orxRENDER_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Has the reset maxima callback? */
      if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_RESET_MAXIMA))
      {
        /* Removes it */
        orxClock_RemoveGlobalTimer(orxRender_Home_ResetProfilerMaxima, orx2F(-1.0f), orxNULL);

        /* Updates status */
        orxFLAG_SET(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_NONE, orxRENDER_KU32_STATIC_FLAG_RESET_MAXIMA);

        /* Disables input set */
        orxInput_EnableSet(orxRENDER_KZ_INPUT_SET, orxFALSE);
      }

      /* Updates status */
      orxFLAG_SET(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_NONE, orxRENDER_KU32_STATIC_FLAG_PROFILER);
    }

    /* Sends profiler render stop event */
    orxEvent_SendShort(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_PROFILER_STOP);

    /* Should render console? */
    if(orxEvent_SendShort(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_CONSOLE_START) != orxSTATUS_FAILURE)
    {
      /* Updates console */
      orxRender_Home_UpdateConsole(orxFLOAT_0, orxFLOAT_0);

      /* Is console enabled? */
      if(orxConsole_IsEnabled() != orxFALSE)
      {
        /* Updates its offset */
        sstRender.fConsoleOffset += orxMath_Floor(_pstClockInfo->fDT * orxRENDER_KF_CONSOLE_SPEED);
        sstRender.fConsoleOffset  = orxMIN(sstRender.fConsoleOffset, orxFLOAT_0);
      }
      else
      {
        /* Updates its offset */
        sstRender.fConsoleOffset -= orxMath_Floor(_pstClockInfo->fDT * orxRENDER_KF_CONSOLE_SPEED);
        sstRender.fConsoleOffset  = orxMAX(sstRender.fConsoleOffset, sstRender.fDefaultConsoleOffset);
      }

      /* Should render console? */
      if(sstRender.fConsoleOffset != sstRender.fDefaultConsoleOffset)
      {
        /* Renders it */
        orxRender_Home_RenderConsole();
      }
    }
    else
    {
      /* Resets console offset */
      sstRender.fConsoleOffset = sstRender.fDefaultConsoleOffset;
    }

    /* Sends console render stop event */
    orxEvent_SendShort(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_CONSOLE_STOP);

    /* Pops previous section */
    orxConfig_PopSection();

    /* Sends render stop event */
    orxEvent_SendShort(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_STOP);

    /* Updates status */
    orxFLAG_SET(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PRESENT_REQUEST, orxRENDER_KU32_STATIC_FLAG_NONE);

    /* Profiles */
    orxPROFILER_POP_MARKER();
  }
  else
  {
    /* Sends render stop event */
    orxEvent_SendShort(orxEVENT_TYPE_RENDER, orxRENDER_EVENT_STOP);
  }

  /* Done! */
  return;
}

/** Presents frame (callback to register on a clock)
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pContext     Context sent when registering callback to the clock
 */
static void orxFASTCALL orxRender_Home_Present(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* Should present? */
  if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PRESENT_REQUEST))
  {
    orxFLOAT fWidth, fHeight;

    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxDisplay_Swap");

    /* Swap buffers */
    orxDisplay_Swap();

    /* Profiles */
    orxPROFILER_POP_MARKER();

    /* Updates status */
    orxFLAG_SET(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_NONE, orxRENDER_KU32_STATIC_FLAG_PRESENT_REQUEST);

    /* Restores screen bitmap clipping */
    orxDisplay_GetScreenSize(&fWidth, &fHeight);
    orxDisplay_SetBitmapClipping(orxDisplay_GetScreenBitmap(), 0, 0, orxF2U(fWidth), orxF2U(fHeight));
  }

  /* Resets all profiler markers */
  orxProfiler_ResetAllMarkers();
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxRender_Home_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Depending on type */
  switch(_pstEvent->eType)
  {
    case orxEVENT_TYPE_DISPLAY:
    {
      /* New video mode? */
      if(_pstEvent->eID == orxDISPLAY_EVENT_SET_VIDEO_MODE)
      {
        orxDISPLAY_EVENT_PAYLOAD *pstPayload;

        /* Gets payload */
        pstPayload = (orxDISPLAY_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        /* Updates console */
        orxRender_Home_UpdateConsole(orxU2F(pstPayload->stVideoMode.u32Width), orxU2F(pstPayload->stVideoMode.u32Height));
      }

      break;
    }

    case orxEVENT_TYPE_INPUT:
    {
      /* If profiler rendered? */
      if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PROFILER))
      {
        /* Input on? */
        if(_pstEvent->eID == orxINPUT_EVENT_ON)
        {
          orxS32                  s32ThreadDelta = 0;
          orxINPUT_EVENT_PAYLOAD *pstPayload;

          /* Gets payload */
          pstPayload = (orxINPUT_EVENT_PAYLOAD *)_pstEvent->pstPayload;

          /* Toggle profiler history? */
          if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_TOGGLE_HISTORY))
          {
            /* Updates profiler history status */
            orxFLAG_SWAP(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PROFILER_HISTORY);

            /* Resets selected frame */
            sstRender.u32SelectedFrame = 0;
          }
          /* Toggle profiler pause? */
          else if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_PAUSE))
          {
            /* Updates profiler pause status */
            orxProfiler_Pause(!orxProfiler_IsPaused());
          }
          /* Previous profiler frame? */
          else if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_FRAME))
          {
            /* Not first frame? */
            if(sstRender.u32SelectedFrame < orxPROFILER_KU32_HISTORY_LENGTH - 2)
            {
              /* Updates it */
              sstRender.u32SelectedFrame++;
            }

            /* Adds input reset timer */
            orxClock_AddGlobalTimer(orxRender_ResetInput, orxRENDER_KF_INPUT_RESET_FIRST_DELAY, 1, (void *)orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_FRAME);
          }
          /* Next profiler frame? */
          else if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_NEXT_FRAME))
          {
            /* Not last frame? */
            if(sstRender.u32SelectedFrame > 0)
            {
              /* Updates it */
              sstRender.u32SelectedFrame--;
            }

            /* Adds input reset timer */
            orxClock_AddGlobalTimer(orxRender_ResetInput, orxRENDER_KF_INPUT_RESET_FIRST_DELAY, 1, (void *)orxRENDER_KZ_INPUT_PROFILER_NEXT_FRAME);
          }

          /* Is profiler history rendered? */
          if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_PROFILER_HISTORY))
          {
            /* Previous profiler depth? */
            if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_DEPTH))
            {
              /* Not first depth? */
              if(sstRender.u32SelectedMarkerDepth > 1)
              {
                /* Updates it */
                sstRender.u32SelectedMarkerDepth--;
              }
            }
            /* Next profiler depth? */
            else if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_NEXT_DEPTH))
            {
              /* Not last marker depth? */
              if(sstRender.u32SelectedMarkerDepth < sstRender.u32MaxMarkerDepth)
              {
                /* Updates depth */
                sstRender.u32SelectedMarkerDepth++;
              }
            }
          }

          /* Previous thread? */
          if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_THREAD))
          {
            /* Updates thread delta */
            s32ThreadDelta = -1;
          }
          /* Next profiler depth? */
          else if(!orxString_Compare(pstPayload->zInputName, orxRENDER_KZ_INPUT_PROFILER_NEXT_THREAD))
          {
            /* Updates thread delta */
            s32ThreadDelta = 1;
          }

          /* Should update selected thread? */
          if(s32ThreadDelta != 0)
          {
            do
            {
              /* Previous? */
              if(s32ThreadDelta < 0)
              {
                /* Updates selected thread */
                sstRender.u32SelectedThread = (sstRender.u32SelectedThread == 0) ? orxTHREAD_KU32_MAX_THREAD_NUMBER - 1 : sstRender.u32SelectedThread - 1;
              }
              /* Next */
              else
              {
                /* Updates selected thread */
                sstRender.u32SelectedThread = (sstRender.u32SelectedThread == orxTHREAD_KU32_MAX_THREAD_NUMBER - 1) ? 0 : sstRender.u32SelectedThread + 1;
              }
            }
            while(orxProfiler_SelectQueryFrame(sstRender.u32SelectedFrame, sstRender.u32SelectedThread) == orxSTATUS_FAILURE);
          }
        }
      }

      break;
    }

    case orxEVENT_TYPE_SYSTEM:
    {
      /* Close event? */
      if(_pstEvent->eID == orxSYSTEM_EVENT_CLOSE)
      {
        /* Unregisters render & present functions */
        orxClock_Unregister(sstRender.pstClock, orxRender_Home_RenderAll);
        orxClock_Unregister(sstRender.pstClock, orxRender_Home_Present);

        /* Updates flags */
        sstRender.u32Flags &= ~orxRENDER_KU32_STATIC_FLAG_REGISTERED;
      }

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Get a world position given a screen one (absolute picking)
 * @param[in]   _pvScreenPosition                     Concerned screen position
 * @param[in]   _pstViewport                          Concerned viewport, if orxNULL then either the last viewport that contains the position (if any), or the last viewport with a camera in the list if none contains the position
 * @param[out]  _pvWorldPosition                      Corresponding world position
 * @return      orxVECTOR if found *inside* the display surface, orxNULL otherwise
 */
orxVECTOR *orxFASTCALL orxRender_Home_GetWorldPosition(const orxVECTOR *_pvScreenPosition, const orxVIEWPORT *_pstViewport, orxVECTOR *_pvWorldPosition)
{
  orxVECTOR     vResult;
  orxVIEWPORT  *pstViewport;
  orxBOOL       bLastViewport, bFound;
  orxVECTOR    *pvResult = orxNULL;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvScreenPosition != orxNULL);
  orxASSERT(_pvWorldPosition != orxNULL);

  /* For all viewports */
  for(pstViewport = orxVIEWPORT(orxStructure_GetLast(orxSTRUCTURE_ID_VIEWPORT)), bLastViewport = orxTRUE, bFound = orxFALSE;
      pstViewport != orxNULL;
      pstViewport = orxVIEWPORT(orxStructure_GetPrevious(pstViewport)))
  {
    orxCAMERA *pstCamera;

    /* Is active or is selected, and has camera? */
    if((((_pstViewport == orxNULL)
      && (orxViewport_IsEnabled(pstViewport) != orxFALSE))
     || (_pstViewport == pstViewport))
    && ((pstCamera = orxViewport_GetCamera(pstViewport)) != orxNULL))
    {
      orxAABOX  stViewportBox;
      orxFLOAT  fCorrectionRatio;
      orxBOOL   bInViewportBox;

      /* Gets viewport box */
      orxViewport_GetBox(pstViewport, &stViewportBox);

      /* Gets viewport correction ratio */
      fCorrectionRatio = orxViewport_GetCorrectionRatio(pstViewport);

      /* Has one? */
      if(fCorrectionRatio != orxFLOAT_1)
      {
        orxFLOAT fDelta;

        /* Should correct horizontally? */
        if(fCorrectionRatio < orxFLOAT_1)
        {
          /* Gets rendering limit delta using correction ratio */
          fDelta = orx2F(0.5f) * (orxFLOAT_1 - fCorrectionRatio) * (stViewportBox.vBR.fX - stViewportBox.vTL.fX);

          /* Updates viewport */
          stViewportBox.vTL.fX += fDelta;
          stViewportBox.vBR.fX -= fDelta;
        }
        else
        {
          /* Gets rendering limit delta using correction ratio */
          fDelta = orx2F(0.5f) * (orxFLOAT_1 - (orxFLOAT_1 / fCorrectionRatio)) * (stViewportBox.vBR.fY - stViewportBox.vTL.fY);

          /* Updates viewport */
          stViewportBox.vTL.fY += fDelta;
          stViewportBox.vBR.fY -= fDelta;
        }
      }

      /* Updates position in box status? */
      bInViewportBox = ((_pvScreenPosition->fX >= stViewportBox.vTL.fX)
                     && (_pvScreenPosition->fX <= stViewportBox.vBR.fX)
                     && (_pvScreenPosition->fY >= stViewportBox.vTL.fY)
                     && (_pvScreenPosition->fY <= stViewportBox.vBR.fY)) ? orxTRUE : orxFALSE;

      /* Is position in box or last viewport? */
      if((bInViewportBox != orxFALSE) || (bLastViewport != orxFALSE))
      {
        orxVECTOR vLocalPosition, vCenter, vCameraCenter, vCameraPosition;
        orxAABOX  stCameraFrustum;
        orxFLOAT  fZoom, fRotation;

        /* Updates status */
        bFound = orxTRUE;

        /* Gets viewport center */
        orxVector_Mulf(&vCenter, orxVector_Add(&vCenter, &(stViewportBox.vBR), &(stViewportBox.vTL)), orx2F(0.5f));

        /* Gets camera position */
        orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

        /* Gets camera world frustum */
        orxCamera_GetFrustum(pstCamera, &stCameraFrustum);
        orxVector_Add(&(stCameraFrustum.vTL), &(stCameraFrustum.vTL), &vCameraPosition);
        orxVector_Add(&(stCameraFrustum.vBR), &(stCameraFrustum.vBR), &vCameraPosition);

        /* Gets camera center */
        orxVector_Mulf(&vCameraCenter, orxVector_Add(&vCameraCenter, &(stCameraFrustum.vBR), &(stCameraFrustum.vTL)), orx2F(0.5f));

        /* Gets viewport space normalized position */
        orxVector_Set(&vLocalPosition, (_pvScreenPosition->fX - vCenter.fX) / (stViewportBox.vBR.fX - stViewportBox.vTL.fX), (_pvScreenPosition->fY - vCenter.fY) / (stViewportBox.vBR.fY - stViewportBox.vTL.fY), orxFLOAT_0);

        /* Gets camera zoom */
        fZoom = orxCamera_GetZoom(pstCamera);

        /* Has rotation */
        if((fRotation = orxFrame_GetRotation(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL)) != orxFLOAT_0)
        {
          orxFLOAT fCos, fSin;

          /* Gets values in camera space */
          vLocalPosition.fX *= (stCameraFrustum.vBR.fX - stCameraFrustum.vTL.fX);
          vLocalPosition.fY *= (stCameraFrustum.vBR.fY - stCameraFrustum.vTL.fY);

          /* Gets cosine and sine of the camera angle */
          fCos = orxMath_Cos(-fRotation);
          fSin = orxMath_Sin(-fRotation);

          /* Gets its world coordinates */
          orxVector_Set(&vResult, (vCameraCenter.fX * fZoom) + (fCos * vLocalPosition.fX) + (fSin * vLocalPosition.fY), (vCameraCenter.fY * fZoom) + (-fSin * vLocalPosition.fX) + (fCos * vLocalPosition.fY), stCameraFrustum.vTL.fZ);
        }
        else
        {
          /* Gets its world coordinates */
          orxVector_Set(&vResult, vCameraCenter.fX * fZoom + vLocalPosition.fX * (stCameraFrustum.vBR.fX - stCameraFrustum.vTL.fX), vCameraCenter.fY * fZoom + vLocalPosition.fY * (stCameraFrustum.vBR.fY - stCameraFrustum.vTL.fY), stCameraFrustum.vTL.fZ);
        }

        /* Has zoom? */
        if((fZoom = orxCamera_GetZoom(pstCamera)) != orxFLOAT_1)
        {
          orxFLOAT fRecZoom;

          /* Gets reciprocal zoom */
          fRecZoom = orxFLOAT_1 / fZoom;

          /* Updates result */
          vResult.fX *= fRecZoom;
          vResult.fY *= fRecZoom;
        }

        /* Is position in viewport box? */
        if(bInViewportBox != orxFALSE)
        {
          /* Updates result */
          pvResult = _pvWorldPosition;

          break;
        }
      }

      /* Updates status */
      bLastViewport = orxFALSE;
    }
  }

  /* Has found result? */
  if(bFound != orxFALSE)
  {
    /* Copies it */
    orxVector_Copy(_pvWorldPosition, &vResult);
  }

  /* Done! */
  return pvResult;
}

/** Get a screen position given a world one and a viewport (rendering position)
 * @param[in]   _pvWorldPosition                      Concerned world position
 * @param[in]   _pstViewport                          Concerned viewport, if orxNULL then the last viewport with a camera will be used
 * @param[out]  _pvScreenPosition                     Corresponding screen position
 * @return      orxVECTOR if found (can be off-screen), orxNULL otherwise
 */
orxVECTOR *orxFASTCALL orxRender_Home_GetScreenPosition(const orxVECTOR *_pvWorldPosition, const orxVIEWPORT *_pstViewport, orxVECTOR *_pvScreenPosition)
{
  const orxVIEWPORT  *pstViewport;
  orxVECTOR          *pvResult = orxNULL;

  /* Checks */
  orxASSERT(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvScreenPosition != orxNULL);
  orxASSERT(_pvWorldPosition != orxNULL);

  /* Has a specified viewport? */
  if(_pstViewport != orxNULL)
  {
    /* Selects it */
    pstViewport = _pstViewport;
  }
  else
  {
    /* Uses last with camera viewport */
    for(pstViewport = orxVIEWPORT(orxStructure_GetLast(orxSTRUCTURE_ID_VIEWPORT));
        (pstViewport != orxNULL) && ((orxViewport_IsEnabled(pstViewport) == orxFALSE) || (orxViewport_GetCamera(pstViewport) == orxNULL));
        pstViewport = orxVIEWPORT(orxStructure_GetPrevious(pstViewport)));
  }

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    orxCAMERA *pstCamera;

    /* Is active and has camera? */
    if((orxViewport_IsEnabled(pstViewport) != orxFALSE)
    && ((pstCamera = orxViewport_GetCamera(pstViewport)) != orxNULL))
    {
      orxVECTOR vCameraCenter, vCameraPosition;
      orxAABOX  stCameraFrustum;

      /* Gets camera position */
      orxFrame_GetPosition(orxCamera_GetFrame(pstCamera), orxFRAME_SPACE_GLOBAL, &vCameraPosition);

      /* Gets camera world frustum */
      orxCamera_GetFrustum(pstCamera, &stCameraFrustum);
      orxVector_Add(&(stCameraFrustum.vTL), &(stCameraFrustum.vTL), &vCameraPosition);
      orxVector_Add(&(stCameraFrustum.vBR), &(stCameraFrustum.vBR), &vCameraPosition);

      /* Gets camera center */
      orxVector_Mulf(&vCameraCenter, orxVector_Add(&vCameraCenter, &(stCameraFrustum.vBR), &(stCameraFrustum.vTL)), orx2F(0.5f));

      /* No viewport specified or is position depth in camera frustum? */
      if((_pstViewport == orxNULL)
      || ((_pvWorldPosition->fZ >= stCameraFrustum.vTL.fZ)
       && (_pvWorldPosition->fZ <= stCameraFrustum.vBR.fZ)))
      {
        orxAABOX  stViewportBox;
        orxVECTOR vLocalPosition, vViewportCenter, vCoef;
        orxFLOAT  fCorrectionRatio;

        /* Links the conversion frame to the camera */
        orxFrame_SetParent(sstRender.pstFrame, orxCamera_GetFrame(pstCamera));

        /* Updates the conversion frame with world position */
        orxFrame_SetPosition(sstRender.pstFrame, orxFRAME_SPACE_GLOBAL, _pvWorldPosition);

        /* Updates local position */
        orxFrame_GetPosition(sstRender.pstFrame, orxFRAME_SPACE_LOCAL, &vLocalPosition);

        /* Unlinks frame from the camera */
        orxFrame_SetParent(sstRender.pstFrame, orxNULL);

        /* Makes it relative to the camera frustum size */
        vLocalPosition.fX = vLocalPosition.fX / (stCameraFrustum.vBR.fX - stCameraFrustum.vTL.fX);
        vLocalPosition.fY = vLocalPosition.fY / (stCameraFrustum.vBR.fY - stCameraFrustum.vTL.fY);

        /* Gets viewport box */
        orxViewport_GetBox(pstViewport, &stViewportBox);

        /* Gets viewport center */
        orxVector_Mulf(&vViewportCenter, orxVector_Add(&vViewportCenter, &(stViewportBox.vBR), &(stViewportBox.vTL)), orx2F(0.5f));

        /* Gets coef */
        orxVector_Sub(&vCoef, &(stViewportBox.vBR), &(stViewportBox.vTL));

        /* Gets its correction ratio */
        fCorrectionRatio = orxViewport_GetCorrectionRatio(pstViewport);

        /* Has correction ratio? */
        if(fCorrectionRatio != orxFLOAT_1)
        {
          /* X axis? */
          if(fCorrectionRatio < orxFLOAT_1)
          {
            /* Applies it */
            vCoef.fX *= fCorrectionRatio;
          }
          else
          {
            /* Applies it */
            vCoef.fY *= orxFLOAT_1 / fCorrectionRatio;
          }
        }

        /* Updates screen position */
        orxVector_Add(_pvScreenPosition, &vViewportCenter, orxVector_Mul(&vLocalPosition, &vLocalPosition, &vCoef));

        /* Updates result */
        pvResult = _pvScreenPosition;
      }
    }
  }

  /* Done! */
  return pvResult;
}

/** Inits the Render module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxRender_Home_Init()
{
  orxSTATUS eResult;

  /* Not already Initialized? */
  if(!(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstRender, sizeof(orxRENDER_STATIC));

    /* Creates rendering bank */
    sstRender.pstRenderBank = orxBank_Create(orxRENDER_KU32_ORDER_BANK_SIZE, sizeof(orxRENDER_NODE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstRender.pstRenderBank != orxNULL)
    {
      orxFLOAT fMinFrequency = orxFLOAT_0;

      /* Gets core clock */
      sstRender.pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

      /* Pushes render config section clock */
      orxConfig_PushSection(orxRENDER_KZ_CONFIG_SECTION);

      /* Min frequency is not inhibited? */
      if((orxConfig_HasValue(orxRENDER_KZ_CONFIG_MIN_FREQUENCY) == orxFALSE)
      || ((fMinFrequency = orxConfig_GetFloat(orxRENDER_KZ_CONFIG_MIN_FREQUENCY)) > orxFLOAT_0))
      {
        /* Sets clock modifier */
        orxClock_SetModifier(sstRender.pstClock, orxCLOCK_MODIFIER_MAXED, (fMinFrequency > orxFLOAT_0) ? (orxFLOAT_1 / fMinFrequency) : orxRENDER_KF_TICK_SIZE);
      }

      /* Pops config section */
      orxConfig_PopSection();

      /* Valid? */
      if(sstRender.pstClock != orxNULL)
      {
        /* Creates conversion frame */
        sstRender.pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

        /* Valid? */
        if(sstRender.pstFrame != orxNULL)
        {
          /* Sets it as its own owner */
          orxStructure_SetOwner(sstRender.pstFrame, sstRender.pstFrame);

          /* Registers render & present functions */
          eResult = orxClock_Register(sstRender.pstClock, orxRender_Home_RenderAll, orxNULL, orxMODULE_ID_RENDER, orxCLOCK_PRIORITY_HIGHEST);
          eResult = ((eResult != orxSTATUS_FAILURE) && (orxClock_Register(sstRender.pstClock, orxRender_Home_Present, orxNULL, orxMODULE_ID_RENDER, orxCLOCK_PRIORITY_LOWEST) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

          /* Success? */
          if(eResult != orxSTATUS_FAILURE)
          {
            const orxSTRING zPreviousSet;

            /* Backups previous input set */
            zPreviousSet = orxInput_GetCurrentSet();

            /* Selects render input set */
            orxInput_SelectSet(orxRENDER_KZ_INPUT_SET);

            /* Binds console inputs */
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_TOGGLE_HISTORY, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_TOGGLE_HISTORY, orxINPUT_MODE_FULL, -1);
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_PAUSE, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_PAUSE, orxINPUT_MODE_FULL, -1);
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_FRAME, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_PREVIOUS_FRAME, orxINPUT_MODE_FULL, -1);
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_NEXT_FRAME, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_NEXT_FRAME, orxINPUT_MODE_FULL, -1);
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_DEPTH, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_PREVIOUS_DEPTH, orxINPUT_MODE_FULL, -1);
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_NEXT_DEPTH, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_NEXT_DEPTH, orxINPUT_MODE_FULL, -1);
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_PREVIOUS_THREAD, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_PREVIOUS_THREAD, orxINPUT_MODE_FULL, -1);
            orxInput_Bind(orxRENDER_KZ_INPUT_PROFILER_NEXT_THREAD, orxINPUT_TYPE_KEYBOARD_KEY, orxRENDER_KE_KEY_PROFILER_NEXT_THREAD, orxINPUT_MODE_FULL, -1);

            /* Restores previous set */
            orxInput_SelectSet(zPreviousSet);

            /* Inits it */
            orxFrame_SetPosition(sstRender.pstFrame, orxFRAME_SPACE_LOCAL, &orxVECTOR_0);
            orxFrame_SetRotation(sstRender.pstFrame, orxFRAME_SPACE_LOCAL, orxFLOAT_0);
            orxFrame_SetScale(sstRender.pstFrame, orxFRAME_SPACE_LOCAL, &orxVECTOR_1);

            /* Adds blinking timer */
            orxClock_AddGlobalTimer(orxRender_Home_BlinkTimer, orxRENDER_KF_CONSOLE_BLINK_DELAY, -1, orxNULL);

            /* Adds event handlers */
            orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxRender_Home_EventHandler);
            orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxRender_Home_EventHandler);
            orxEvent_AddHandler(orxEVENT_TYPE_INPUT, orxRender_Home_EventHandler);
            orxEvent_SetHandlerIDFlags(orxRender_Home_EventHandler, orxEVENT_TYPE_DISPLAY, orxNULL, orxEVENT_GET_FLAG(orxDISPLAY_EVENT_SET_VIDEO_MODE), orxEVENT_KU32_MASK_ID_ALL);
            orxEvent_SetHandlerIDFlags(orxRender_Home_EventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_CLOSE), orxEVENT_KU32_MASK_ID_ALL);
            orxEvent_SetHandlerIDFlags(orxRender_Home_EventHandler, orxEVENT_TYPE_INPUT, orxNULL, orxEVENT_GET_FLAG(orxINPUT_EVENT_ON), orxEVENT_KU32_MASK_ID_ALL);

            /* Inits console */
            orxRender_Home_InitConsole();

            /* Inits selected marker depth */
            sstRender.u32SelectedMarkerDepth = 1;

            /* Inits selected frame */
            sstRender.u32SelectedFrame = 0;

            /* Inits selected thread */
            sstRender.u32SelectedThread = orxTHREAD_KU32_MAIN_THREAD_ID;

            /* Inits Flags */
            sstRender.u32Flags = orxRENDER_KU32_STATIC_FLAG_READY | orxRENDER_KU32_STATIC_FLAG_REGISTERED;
          }
          else
          {
            /* Unregisters render function */
            orxClock_Unregister(sstRender.pstClock, orxRender_Home_RenderAll);

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Can't register render clock callback.");

            /* Deletes frame */
            orxFrame_Delete(sstRender.pstFrame);

            /* Deletes bank */
            orxBank_Delete(sstRender.pstRenderBank);
          }
        }
        else
        {
          /* Deletes bank */
          orxBank_Delete(sstRender.pstRenderBank);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Deletes bank */
        orxBank_Delete(sstRender.pstRenderBank);

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
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Render module already loaded.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the Render module
 */
void orxFASTCALL orxRender_Home_Exit()
{
  /* Initialized? */
  if(sstRender.u32Flags & orxRENDER_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handlers */
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxRender_Home_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxRender_Home_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_INPUT, orxRender_Home_EventHandler);

    /* Removes blinking timer */
    orxClock_RemoveGlobalTimer(orxRender_Home_BlinkTimer, orxRENDER_KF_CONSOLE_BLINK_DELAY, orxNULL);

    /* Unregisters rendering function */
    if(orxFLAG_TEST(sstRender.u32Flags, orxRENDER_KU32_STATIC_FLAG_REGISTERED))
    {
      orxClock_Unregister(sstRender.pstClock, orxRender_Home_RenderAll);
      orxClock_Unregister(sstRender.pstClock, orxRender_Home_Present);
    }

    /* Deletes conversion frame */
    orxFrame_Delete(sstRender.pstFrame);

    /* Deletes rendering bank */
    orxBank_Delete(sstRender.pstRenderBank);

    /* Updates flags */
    sstRender.u32Flags &= ~(orxRENDER_KU32_STATIC_FLAG_READY | orxRENDER_KU32_STATIC_FLAG_REGISTERED);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Attempting to exit from Render module without initializing it.");
  }

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(RENDER);

orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_Init, RENDER, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_Exit, RENDER, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_GetWorldPosition, RENDER, GET_WORLD_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxRender_Home_GetScreenPosition, RENDER, GET_SCREEN_POSITION);

orxPLUGIN_USER_CORE_FUNCTION_END();

#ifdef __orxGCC__

  #pragma GCC diagnostic pop

#endif /* __orxGCC__ */
