//! Includes

#ifndef _orxNUKLEAR_H_
#define _orxNUKLEAR_H_

#include "orx.h"

#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#define NK_INCLUDE_FIXED_TYPES
#define NK_ASSERT(EXPR) orxASSERT(EXPR)

#define STBTT_malloc(SIZE, USER)  orxMemory_Allocate((orxU32)(SIZE), orxMEMORY_TYPE_TEMP)
#define STBTT_free(PTR, USER)     orxMemory_Free(PTR)

#include "nuklear.h"


//! Prototypes / Declarations

#define orxNUKLEAR_KU32_DEFAULT_FONT_SIZE     13.0f
#define orxNUKLEAR_KU32_DEFAULT_GLOBAL_ALPHA  1.0f
#define orxNUKLEAR_KU32_DEFAULT_SEGMENT_COUNT 22

#define orxNUKLEAR_KZ_FONT_RESOURCE_GROUP     "Font"

#define orxNUKLEAR_KZ_CONFIG_SECTION          "Nuklear"
#define orxNUKLEAR_KZ_CONFIG_FONT_LIST        "FontList"
#define orxNUKLEAR_KZ_CONFIG_SKIN_LIST        "SkinList"
#define orxNUKLEAR_KZ_CONFIG_FONT_SIZE        "FontSize"
#define orxNUKLEAR_KZ_CONFIG_GLOBAL_ALPHA     "GlobalAlpha"
#define orxNUKLEAR_KZ_CONFIG_SEGMENT_COUNT    "SegmentCount"

typedef struct __orxNUKLEAR_SKIN_t
{
  struct nk_image     stImage;
  orxTEXTURE         *pstTexture;
} orxNUKLEAR_SKIN;

typedef struct __orxNUKLEAR_t
{
  struct nk_context     stContext;
  struct nk_font_atlas  stAtlas;
  struct nk_font       *pstDefaultFont;
  struct nk_font      **apstFonts;
  orxNUKLEAR_SKIN      *astSkins;
  orxS32                s32FontCount;
  orxS32                s32SkinCount;

} orxNUKLEAR;

#ifndef orxNUKLEAR_IMPL
extern      orxNUKLEAR  sstNuklear;
#endif // !orxNUKLEAR_IMPL

orxSTATUS   orxFASTCALL orxNuklear_Init();
void        orxFASTCALL orxNuklear_Exit();


//! Code

#ifdef orxNUKLEAR_IMPL

static void *orxNuklear_Malloc(nk_handle _hHandle, void *_pPtr, nk_size _sSize)
{
  return orxMemory_Allocate((orxU32)_sSize, orxMEMORY_TYPE_TEMP);
}

static void orxNuklear_Free(nk_handle _hHandle, void *_pPtr)
{
  orxMemory_Free(_pPtr);
}

#define NK_IMPLEMENTATION
#include "nuklear.h"
#undef NK_IMPLEMENTATION

static orxBOOL              sbNuklearInFrame  = orxFALSE;
static orxBITMAP           *spstFontBitmap    = orxNULL;
struct nk_draw_null_texture sstNull;
struct nk_buffer            sstCommandBuffer;
struct nk_buffer            sstVertexBuffer;
struct nk_buffer            sstElementBuffer;
struct nk_allocator         sstAllocator;

orxNUKLEAR                  sstNuklear;

static void orxNuklear_Paste(nk_handle _hHandle, struct nk_text_edit *_pstTextEdit)
{
  const orxSTRING zClipboard = orxSystem_GetClipboard();
  nk_textedit_paste(_pstTextEdit, zClipboard, orxString_GetLength(zClipboard));
}

static void orxNuklear_Copy(nk_handle _hHandle, const char *_acText, int len)
{
#ifndef alloca
  #define alloca(x) __builtin_alloca((x))
#endif // alloca

  orxCHAR *acBuffer = (orxCHAR *)orxMemory_Copy(alloca(len + 1), _acText, len);
  acBuffer[len] = orxCHAR_NULL;
  orxSystem_SetClipboard(acBuffer);
}

static orxSTATUS orxFASTCALL orxNuklear_BeginFrame(const orxEVENT *_pstEvent)
{
  orxVECTOR       vMousePos;
  struct nk_vec2  vWheel = {0.0f, orxMouse_GetWheelDelta()};

  if(sbNuklearInFrame)
  {
    // Clear buffers
    nk_clear(&sstNuklear.stContext);
    nk_buffer_clear(&sstCommandBuffer);
    nk_buffer_clear(&sstVertexBuffer);
    nk_buffer_clear(&sstElementBuffer);
  }

  sbNuklearInFrame = orxTRUE;

  // Inputs
  nk_input_begin(&sstNuklear.stContext);

  // Mouse
  orxMouse_GetPosition(&vMousePos);
  nk_input_motion(&sstNuklear.stContext, orxF2S(vMousePos.fX), orxF2S(vMousePos.fY));

  nk_input_button(&sstNuklear.stContext, NK_BUTTON_LEFT, orxF2S(vMousePos.fX), orxF2S(vMousePos.fY), orxMouse_IsButtonPressed(orxMOUSE_BUTTON_LEFT) ? 1 : 0);
  nk_input_button(&sstNuklear.stContext, NK_BUTTON_RIGHT, orxF2S(vMousePos.fX), orxF2S(vMousePos.fY), orxMouse_IsButtonPressed(orxMOUSE_BUTTON_RIGHT) ? 1 : 0);
  nk_input_button(&sstNuklear.stContext, NK_BUTTON_MIDDLE, orxF2S(vMousePos.fX), orxF2S(vMousePos.fY), orxMouse_IsButtonPressed(orxMOUSE_BUTTON_MIDDLE) ? 1 : 0);

  nk_input_scroll(&sstNuklear.stContext, vWheel);

  // Keys
  nk_input_key(&sstNuklear.stContext, NK_KEY_DEL, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_DELETE) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_ENTER, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_ENTER) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_TAB, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_TAB) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_BACKSPACE, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_BACKSPACE) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_LEFT, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LEFT) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_RIGHT, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RIGHT) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_UP, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_UP) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_DOWN, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_DOWN) ? 1 : 0);

  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LCTRL)
  || orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RCTRL))
  {
    nk_input_key(&sstNuklear.stContext, NK_KEY_COPY, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_C) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_PASTE, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_V) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_CUT, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_X) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_LINE_START, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_LINE_END, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_START, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_HOME) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_END, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_END) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_UNDO, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_Z) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_REDO, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_Y) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_SELECT_ALL, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_A) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_WORD_LEFT, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LEFT) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_WORD_RIGHT, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RIGHT) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_CTRL, 1);
  }
  else
  {
    nk_input_key(&sstNuklear.stContext, NK_KEY_COPY, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_PASTE, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_CUT, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_LINE_START, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_HOME) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_LINE_END, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_END) ? 1 : 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_START, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_END, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_UNDO, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_REDO, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_SELECT_ALL, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_WORD_LEFT, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_TEXT_WORD_RIGHT, 0);
    nk_input_key(&sstNuklear.stContext, NK_KEY_CTRL, 0);
  }
  nk_input_key(&sstNuklear.stContext, NK_KEY_SHIFT, (orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LSHIFT) || orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RSHIFT)) ? 1 : 0);

  nk_input_key(&sstNuklear.stContext, NK_KEY_SCROLL_START, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_HOME) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_SCROLL_END, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_END) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_SCROLL_DOWN, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_PAGE_DOWN) ? 1 : 0);
  nk_input_key(&sstNuklear.stContext, NK_KEY_SCROLL_UP, orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_PAGE_UP) ? 1 : 0);

  // Text
  if(sstNuklear.stContext.text_edit.active)
  {
    for(const orxCHAR *pc = orxKeyboard_ReadString(); *pc != orxCHAR_NULL;)
    {
      nk_input_unicode(&sstNuklear.stContext, orxString_GetFirstCharacterCodePoint(pc, &pc));
    }
  }

  nk_input_end(&sstNuklear.stContext);

  return orxSTATUS_SUCCESS;
}

static orxSTATUS orxFASTCALL orxNuklear_EndFrame(const orxEVENT *_pstEvent)
{
  if(sbNuklearInFrame)
  {
    sbNuklearInFrame = orxFALSE;

    static const struct nk_draw_vertex_layout_element stVertexLayout[] =
    {
      {NK_VERTEX_POSITION,  NK_FORMAT_FLOAT,    NK_OFFSETOF(orxDISPLAY_VERTEX, fX)},
      {NK_VERTEX_TEXCOORD,  NK_FORMAT_FLOAT,    NK_OFFSETOF(orxDISPLAY_VERTEX, fU)},
      {NK_VERTEX_COLOR,     NK_FORMAT_R8G8B8A8, NK_OFFSETOF(orxDISPLAY_VERTEX, stRGBA)},
      {NK_VERTEX_LAYOUT_END}
    };
    struct nk_convert_config stConfig;

    // Config
    orxConfig_PushSection(orxNUKLEAR_KZ_CONFIG_SECTION);

    orxS32 s32SegmentCount = orxConfig_HasValue(orxNUKLEAR_KZ_CONFIG_SEGMENT_COUNT) ? orxConfig_GetS32(orxNUKLEAR_KZ_CONFIG_SEGMENT_COUNT) : orxNUKLEAR_KU32_DEFAULT_SEGMENT_COUNT;

    stConfig.vertex_layout        = stVertexLayout;
    stConfig.vertex_size          = sizeof(orxDISPLAY_VERTEX);
    stConfig.vertex_alignment     = NK_ALIGNOF(orxDISPLAY_VERTEX);
    stConfig.null                 = sstNull;
    stConfig.circle_segment_count = s32SegmentCount;
    stConfig.curve_segment_count  = s32SegmentCount;
    stConfig.arc_segment_count    = s32SegmentCount;
    stConfig.global_alpha         = orxConfig_HasValue(orxNUKLEAR_KZ_CONFIG_GLOBAL_ALPHA) ? orxConfig_GetFloat(orxNUKLEAR_KZ_CONFIG_GLOBAL_ALPHA) : orxNUKLEAR_KU32_DEFAULT_GLOBAL_ALPHA;
    stConfig.shape_AA             = NK_ANTI_ALIASING_ON;
    stConfig.line_AA              = NK_ANTI_ALIASING_ON;

    orxConfig_PopSection();

    // Convert commands to buffers
    nk_convert(&sstNuklear.stContext, &sstCommandBuffer, &sstVertexBuffer, &sstElementBuffer, &stConfig);

    orxVECTOR vScreenSize, vFramebufferSize, vScale;
    orxDisplay_GetScreenSize(&vScreenSize.fX, &vScreenSize.fY);
    vScreenSize.fZ = orxFLOAT_0;
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
    orxConfig_GetVector(orxDISPLAY_KZ_CONFIG_FRAMEBUFFER_SIZE, &vFramebufferSize);
    orxConfig_PopSection();
    orxVector_Div(&vScale, &vFramebufferSize, &vScreenSize);

    // Draw to screen
    orxBITMAP *pstScreen = orxDisplay_GetScreenBitmap();
    orxDisplay_SetDestinationBitmaps(&pstScreen, 1);

    orxDISPLAY_MESH stMesh;
    stMesh.ePrimitive       = orxDISPLAY_PRIMITIVE_TRIANGLES;
    stMesh.astVertexList    = (orxDISPLAY_VERTEX *)nk_buffer_memory_const(&sstVertexBuffer);
    stMesh.u32VertexNumber  = (orxU32)(sstVertexBuffer.needed / sizeof(orxDISPLAY_VERTEX));
    stMesh.au16IndexList    = (orxU16 *)nk_buffer_memory_const(&sstElementBuffer);

    const struct nk_draw_command *pstCommand;
    nk_draw_foreach(pstCommand, &sstNuklear.stContext, &sstCommandBuffer)
    {
      if(pstCommand->elem_count)
      {
        stMesh.u32IndexNumber = pstCommand->elem_count;
        orxDisplay_SetBitmapClipping(orxNULL, orxF2U(pstCommand->clip_rect.x * vScale.fX), orxF2U(pstCommand->clip_rect.y * vScale.fY), orxF2U((pstCommand->clip_rect.x + pstCommand->clip_rect.w) * vScale.fX), orxF2U((pstCommand->clip_rect.y + pstCommand->clip_rect.h) * vScale.fY));
        orxDisplay_DrawMesh(&stMesh, (orxBITMAP *)pstCommand->texture.ptr, orxDISPLAY_SMOOTHING_ON, orxDISPLAY_BLEND_MODE_ALPHA);
        stMesh.au16IndexList += pstCommand->elem_count;
      }
    }

    // Clear buffers
    nk_clear(&sstNuklear.stContext);
    nk_buffer_clear(&sstCommandBuffer);
    nk_buffer_clear(&sstVertexBuffer);
    nk_buffer_clear(&sstElementBuffer);
  }

  return orxSTATUS_SUCCESS;
}

orxSTATUS orxFASTCALL orxNuklear_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  orxMemory_Zero(&sstNuklear, sizeof(orxNUKLEAR));
  sstAllocator.userdata = nk_handle_ptr(0);
  sstAllocator.alloc    = orxNuklear_Malloc;
  sstAllocator.free     = orxNuklear_Free;

  orxConfig_PushSection(orxNUKLEAR_KZ_CONFIG_SECTION);

  // Custom Fonts
  nk_font_atlas_init(&sstNuklear.stAtlas, &sstAllocator);
  nk_font_atlas_begin(&sstNuklear.stAtlas);
  sstNuklear.pstDefaultFont = nk_font_atlas_add_default(&sstNuklear.stAtlas, orxConfig_HasValue(orxNUKLEAR_KZ_CONFIG_FONT_SIZE) ? orxConfig_GetFloat(orxNUKLEAR_KZ_CONFIG_FONT_SIZE) : orxNUKLEAR_KU32_DEFAULT_FONT_SIZE, NULL);

  orxS32 s32FontCount = orxConfig_GetListCount(orxNUKLEAR_KZ_CONFIG_FONT_LIST);

  if(s32FontCount)
  {
    sstNuklear.apstFonts = (struct nk_font **)orxMemory_Allocate(s32FontCount * sizeof(struct nk_font *), orxMEMORY_TYPE_TEMP);
    orxMemory_Zero(sstNuklear.apstFonts, s32FontCount * sizeof(struct nk_font*));

    for(sstNuklear.s32FontCount = 0; sstNuklear.s32FontCount < s32FontCount; sstNuklear.s32FontCount++)
    {
      const orxSTRING zFont = orxConfig_GetListString(orxNUKLEAR_KZ_CONFIG_FONT_LIST, sstNuklear.s32FontCount);
      const orxSTRING zLocation = orxResource_Locate(orxNUKLEAR_KZ_FONT_RESOURCE_GROUP, orxConfig_GetListString(zFont, 0));

      if(zLocation)
      {
        orxHANDLE hFont = orxResource_Open(zLocation, orxFALSE);

        if(hFont != orxHANDLE_UNDEFINED)
        {
          orxS32 s32Size = (orxS32)orxResource_GetSize(hFont);

          void *pBuffer = orxMemory_Allocate(s32Size, orxMEMORY_TYPE_TEMP);

          if(pBuffer)
          {
            if(orxResource_Read(hFont, s32Size, pBuffer, orxNULL, orxNULL) == s32Size)
            {
              orxFLOAT fFontSize = orxMAX(orxFLOAT_1, (orxConfig_GetListCount(zFont) > 1) ? orxConfig_GetListFloat(zFont, 1) : orxNUKLEAR_KU32_DEFAULT_FONT_SIZE);

              sstNuklear.apstFonts[sstNuklear.s32FontCount] = nk_font_atlas_add_from_memory(&sstNuklear.stAtlas, pBuffer, s32Size, fFontSize, NULL);
            }

            orxMemory_Free(pBuffer);
          }

          orxResource_Close(hFont);
        }
      }
    }
  }

  int iWidth, iHeight;
  const void *pcPixels = nk_font_atlas_bake(&sstNuklear.stAtlas, &iWidth, &iHeight, NK_FONT_ATLAS_RGBA32);
  spstFontBitmap = orxDisplay_CreateBitmap(iWidth, iHeight);
  orxDisplay_SetBitmapData(spstFontBitmap, (orxU8 *)pcPixels, iWidth * iHeight * 4);
  nk_font_atlas_end(&sstNuklear.stAtlas, nk_handle_ptr(spstFontBitmap), &sstNull);

  // Custom Skins (Images)
  orxS32 s32SkinCount = orxConfig_GetListCount(orxNUKLEAR_KZ_CONFIG_SKIN_LIST);

  if(s32SkinCount)
  {
    sstNuklear.astSkins = (orxNUKLEAR_SKIN *)orxMemory_Allocate(s32SkinCount * sizeof(orxNUKLEAR_SKIN), orxMEMORY_TYPE_TEMP);
    orxMemory_Zero(sstNuklear.astSkins, s32SkinCount * sizeof(orxNUKLEAR_SKIN));

    for(sstNuklear.s32SkinCount = 0; sstNuklear.s32SkinCount < s32SkinCount; sstNuklear.s32SkinCount++)
    {
      const orxSTRING zSkin = orxConfig_GetListString(orxNUKLEAR_KZ_CONFIG_SKIN_LIST, sstNuklear.s32SkinCount);

      sstNuklear.astSkins[sstNuklear.s32SkinCount].pstTexture = orxTexture_Load(orxConfig_GetListString(zSkin, 0), orxFALSE);

      if(sstNuklear.astSkins[sstNuklear.s32SkinCount].pstTexture)
      {
        orxBITMAP *pstBitmap = orxTexture_GetBitmap(sstNuklear.astSkins[sstNuklear.s32SkinCount].pstTexture);
        sstNuklear.astSkins[sstNuklear.s32SkinCount].stImage = nk_image_ptr(pstBitmap);
        if(orxConfig_GetListCount(zSkin) > 2)
        {
          orxVECTOR vCorner, vSize;
          if(orxConfig_GetListVector(zSkin, 1, &vCorner) && orxConfig_GetListVector(zSkin, 2, &vSize))
          {
            orxFLOAT fWidth, fHeight;
            orxDisplay_GetBitmapSize(pstBitmap, &fWidth, &fHeight);
            sstNuklear.astSkins[sstNuklear.s32SkinCount].stImage = nk_subimage_ptr(pstBitmap, orxF2U(fWidth), orxF2U(fHeight), nk_rect(vCorner.fX, vCorner.fY, vSize.fX, vSize.fY));
          }
        }
      }
    }
  }

  orxConfig_PopSection();

  // Inits
  if(nk_init(&sstNuklear.stContext, &sstAllocator, &sstNuklear.pstDefaultFont->handle))
  {
    sbNuklearInFrame = orxFALSE;

    // Copy/Paste
    sstNuklear.stContext.clip.copy = orxNuklear_Copy;
    sstNuklear.stContext.clip.paste = orxNuklear_Paste;
    sstNuklear.stContext.clip.userdata = nk_handle_ptr(0);

    // Cursors
    nk_style_load_all_cursors(&sstNuklear.stContext, sstNuklear.stAtlas.cursors);
    orxMouse_ShowCursor(orxFALSE);

    // Buffers
    nk_buffer_init(&sstCommandBuffer, &sstAllocator, NK_BUFFER_DEFAULT_INITIAL_SIZE);
    nk_buffer_init(&sstVertexBuffer, &sstAllocator, NK_BUFFER_DEFAULT_INITIAL_SIZE);
    nk_buffer_init(&sstElementBuffer, &sstAllocator, NK_BUFFER_DEFAULT_INITIAL_SIZE);

    // Events
    orxEvent_AddHandler(orxEVENT_TYPE_RENDER, &orxNuklear_BeginFrame);
    orxEvent_SetHandlerIDFlags(&orxNuklear_BeginFrame, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_STOP), orxEVENT_KU32_MASK_ID_ALL);
    orxEvent_AddHandler(orxEVENT_TYPE_RENDER, &orxNuklear_EndFrame);
    orxEvent_SetHandlerIDFlags(&orxNuklear_EndFrame, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_PROFILER_START), orxEVENT_KU32_MASK_ID_ALL);

    orxNuklear_BeginFrame(orxNULL);

    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    orxDisplay_DeleteBitmap(spstFontBitmap);
    nk_font_atlas_clear(&sstNuklear.stAtlas);
  }

  // Done!
  return eResult;
}

void orxFASTCALL orxNuklear_Exit()
{
  nk_buffer_free(&sstCommandBuffer);
  nk_buffer_free(&sstVertexBuffer);
  nk_buffer_free(&sstElementBuffer);
  nk_free(&sstNuklear.stContext);
  nk_font_atlas_clear(&sstNuklear.stAtlas);
  orxDisplay_DeleteBitmap(spstFontBitmap);
  orxMemory_Free(sstNuklear.apstFonts);
  for(orxS32 i = 0; i < sstNuklear.s32SkinCount; i++)
  {
    orxTexture_Delete(sstNuklear.astSkins[i].pstTexture);
  }
  orxMemory_Free(sstNuklear.astSkins);
}

#endif // orxNUKLEAR_IMPL

#undef STBTT_malloc
#undef STBTT_free

#undef NK_ASSERT
#undef NK_INCLUDE_FIXED_TYPES
#undef NK_KEYSTATE_BASED_INPUT
#undef NK_INCLUDE_DEFAULT_FONT
#undef NK_INCLUDE_FONT_BAKING
#undef NK_INCLUDE_VERTEX_BUFFER_OUTPUT

#endif // _orxNUKLEAR_H_
