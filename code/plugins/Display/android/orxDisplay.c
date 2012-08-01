/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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
 * @file orxDisplay.c
 * @date 13/01/2011
 * @author simons.philippe@gmail.com
 *
 * Android display plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"

#include "SOIL.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <nv_file/nv_file.h>
#include <nv_event/nv_event.h>

/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE          0x00000000  /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY         0x00000001  /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHADER        0x00000002  /**< Shader support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER   0x00000004  /**< Has depth buffer support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DISPLAY_READY 0x00000008 /** egl window inited */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        128
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        16

#define orxDISPLAY_KU32_VERTEX_BUFFER_SIZE      (4 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_INDEX_BUFFER_SIZE       (6 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      65536

#define orxDISPLAY_KF_BORDER_FIX                0.1f

#define orxDISPLAY_KU32_CIRCLE_LINE_NUMBER      32

/**  Misc defines
 */
#ifdef __orxDEBUG__

#define glASSERT()                                                      \
do                                                                      \
{                                                                       \
  GLenum eError = glGetError();                                         \
  orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: 0x%X", eError);\
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()                                                      \
do                                                                      \
{                                                                       \
  glGetError();                                                         \
} while(orxFALSE)

#endif /* __orxDEBUG__ */

/* defined in orxAndroidSupport.cpp */
extern int32_t s_winWidth;
extern int32_t s_winHeight;

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef enum __orxDISPLAY_ATTRIBUTE_LOCATION_t
{
  orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX = 0,
  orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD,
  orxDISPLAY_ATTRIBUTE_LOCATION_COLOR,

  orxDISPLAY_ATTRIBUTE_LOCATION_NUMBER,

  orxDISPLAY_ATTRIBUTE_LOCATION_NONE = orxENUM_NONE

} orxDISPLAY_ATTRIBUTE_LOCATION;

/** Internal matrix structure
 */
typedef struct __orxDISPLAY_MATRIX_t
{
  orxVECTOR vX;
  orxVECTOR vY;

} orxDISPLAY_MATRIX;

/** Internal projection matrix structure
 */
typedef struct __orxDISPLAY_PROJ_MATRIX_t
{
  orxFLOAT aafValueList[4][4];

} orxDISPLAY_PROJ_MATRIX;

/** Internal bitmap structure
 */
struct __orxBITMAP_t
{
  GLuint uiTexture;
  orxBOOL bSmoothing;
  orxFLOAT fWidth, fHeight;
  orxU32 u32RealWidth, u32RealHeight;
  orxFLOAT fRecRealWidth, fRecRealHeight;
  orxRGBA stColor;
  orxAABOX stClip;
  orxBOOL bCompressed;
};

/** Internal param info structure
 */
typedef struct __orxDISPLAY_PARAM_INFO_t
{
  GLint                     iLocation, iLocationTop, iLocationLeft, iLocationBottom, iLocationRight;

} orxDISPLAY_PARAM_INFO;

/** Internal texture info structure
 */
typedef struct __orxDISPLAY_TEXTURE_INFO_t
{
  GLint iLocation;
  const orxBITMAP *pstBitmap;

} orxDISPLAY_TEXTURE_INFO;

/** Internal shader structure
 */
typedef struct __orxDISPLAY_SHADER_t
{
  GLuint                    uiProgram;
  GLuint                    uiTextureLocation;
  GLuint                    uiProjectionMatrixLocation;
  GLint                     iTextureCounter;
  orxS32                    s32ParamCounter;
  orxBOOL                   bActive;
  orxBOOL                   bInitialized;
  orxSTRING                 zCode;
  orxDISPLAY_TEXTURE_INFO  *astTextureInfoList;
  orxDISPLAY_PARAM_INFO    *astParamInfoList;

} orxDISPLAY_SHADER;

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxBANK                  *pstBitmapBank;
  orxBANK                  *pstShaderBank;
  orxBOOL                   bDefaultSmoothing;
  orxBITMAP                *pstScreen;
  orxBITMAP                *pstDestinationBitmap;
  const orxBITMAP          *pstLastBitmap;
  orxCOLOR                  stLastColor;
  orxDISPLAY_BLEND_MODE     eLastBlendMode;
  orxDISPLAY_SHADER        *pstDefaultShader;
  orxDISPLAY_SHADER        *pstNoTextureShader;
  GLuint                    uiIndexBuffer;
  GLuint                    uiFrameBuffer;
  orxU8                   **aau8BufferArray;
  orxS32                    s32BitmapCounter;
  orxS32                    s32ShaderCounter;
  GLint                     iTextureUnitNumber;
  orxS32                    s32ActiveShaderCounter;
  orxS32                    s32BufferIndex;
  orxU32                    u32Flags;
  orxU32                    u32Depth;
  orxDISPLAY_PROJ_MATRIX    mProjectionMatrix;
  orxDISPLAY_VERTEX         astVertexList[orxDISPLAY_KU32_VERTEX_BUFFER_SIZE];
  GLushort                  au16IndexList[orxDISPLAY_KU32_INDEX_BUFFER_SIZE];
  orxCHAR                   acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE];

  int32_t                   width;
  int32_t                   height;
} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDISPLAY_STATIC sstDisplay;

/** ETC1 compressed textures utility
 */

#define ETC_PKM_HEADER_SIZE 16

typedef unsigned char etc1_byte;
typedef int etc1_bool;
typedef unsigned int etc1_uint32;

static const char kMagic[] = { 'P', 'K', 'M', ' ', '1', '0' };
static const etc1_uint32 ETC1_PKM_FORMAT_OFFSET = 6;
static const etc1_uint32 ETC1_PKM_ENCODED_WIDTH_OFFSET = 8;
static const etc1_uint32 ETC1_PKM_ENCODED_HEIGHT_OFFSET = 10;
static const etc1_uint32 ETC1_PKM_WIDTH_OFFSET = 12;
static const etc1_uint32 ETC1_PKM_HEIGHT_OFFSET = 14;

static const etc1_uint32 ETC1_RGB_NO_MIPMAPS = 0;

static etc1_uint32 readBEUint16(const etc1_byte* pIn) {
    return (pIn[0] << 8) | pIn[1];
}

// Check if a PKM header is correctly formatted.

static etc1_bool etc1_pkm_is_valid(const etc1_byte* pHeader) {
    if (memcmp(pHeader, kMagic, sizeof(kMagic))) {
        return false;
    }
    etc1_uint32 format = readBEUint16(pHeader + ETC1_PKM_FORMAT_OFFSET);
    etc1_uint32 encodedWidth = readBEUint16(pHeader + ETC1_PKM_ENCODED_WIDTH_OFFSET);
    etc1_uint32 encodedHeight = readBEUint16(pHeader + ETC1_PKM_ENCODED_HEIGHT_OFFSET);
    etc1_uint32 width = readBEUint16(pHeader + ETC1_PKM_WIDTH_OFFSET);
    etc1_uint32 height = readBEUint16(pHeader + ETC1_PKM_HEIGHT_OFFSET);
    return format == ETC1_RGB_NO_MIPMAPS &&
            encodedWidth >= width && encodedWidth - width < 4 &&
            encodedHeight >= height && encodedHeight - height < 4;
}

// Return the size of the encoded image data (does not include size of PKM header).

etc1_uint32 etc1_get_encoded_data_size(etc1_uint32 width, etc1_uint32 height) {
    return (((width + 3) & ~3) * ((height + 3) & ~3)) >> 1;
}

// Read the image width from a PKM header

etc1_uint32 etc1_pkm_get_width(const etc1_byte* pHeader) {
    return readBEUint16(pHeader + ETC1_PKM_WIDTH_OFFSET);
}

// Read the image height from a PKM header

etc1_uint32 etc1_pkm_get_height(const etc1_byte* pHeader){
    return readBEUint16(pHeader + ETC1_PKM_HEIGHT_OFFSET);
}

static orxINLINE orxBOOL initGLESConfig()
{
  /* Inits it */
  /* Enables vextex attribute arrays */
  glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX);
  glASSERT();
  glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD);
  glASSERT();
  glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR);
  glASSERT();

  /* Common init */
  glDisable(GL_CULL_FACE);
  glASSERT();
  glDisable(GL_DEPTH_TEST);
  glASSERT();
  glDisable(GL_STENCIL_TEST);
  glASSERT();
  return orxTRUE;
}

static orxSTATUS orxFASTCALL orxDisplay_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Is a display move? */
  if(_pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + NV_EVENT_FOCUS_LOST ||
     _pstEvent->eType == orxEVENT_TYPE_FIRST_RESERVED + NV_EVENT_FOCUS_GAINED)
  {
    switch (_pstEvent->eID)
    {
      /* Need to save context? */
      case NV_EVENT_FOCUS_LOST:
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "SAVE_CONTEXT");

        break;
      }
      /* Need to restore context? */
      case NV_EVENT_FOCUS_GAINED:
      {
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "RESTORE_CONTEXT");
        break;
      }
      default:
      {
        eResult = orxSTATUS_FAILURE;
        break;
      }
    }
  }

  return eResult;
}

static orxDISPLAY_PROJ_MATRIX *orxDisplay_Android_OrthoProjMatrix(orxDISPLAY_PROJ_MATRIX *_pmResult, orxFLOAT _fLeft, orxFLOAT _fRight, orxFLOAT _fBottom, orxFLOAT _fTop, orxFLOAT _fNear, orxFLOAT _fFar)
{
  orxFLOAT                fDeltaX, fDeltaY, fDeltaZ;
  orxDISPLAY_PROJ_MATRIX *pmResult;

  /* Checks */
  orxASSERT(_pmResult != orxNULL);

  /* Gets deltas */
  fDeltaX = _fRight - _fLeft;
  fDeltaY = _fTop - _fBottom;
  fDeltaZ = _fFar - _fNear;

  /* Valid? */
  if((fDeltaX != orxFLOAT_0) && (fDeltaY != orxFLOAT_0) && (fDeltaZ != orxFLOAT_0))
  {
    /* Clears matrix */
    orxMemory_Zero(_pmResult, sizeof(orxDISPLAY_PROJ_MATRIX));

    /* Updates result */
    _pmResult->aafValueList[0][0] = orx2F(2.0f) / fDeltaX;
    _pmResult->aafValueList[3][0] = -(_fRight + _fLeft) / fDeltaX;
    _pmResult->aafValueList[1][1] = orx2F(2.0f) / fDeltaY;
    _pmResult->aafValueList[3][1] = -(_fTop + _fBottom) / fDeltaY;
    _pmResult->aafValueList[2][2] = orx2F(-2.0f) / fDeltaZ;
    _pmResult->aafValueList[3][2] = -(_fNear + _fFar) / fDeltaZ;
    _pmResult->aafValueList[3][3] = orxFLOAT_1;
    pmResult = _pmResult;
  }
  else
  {
    /* Updates result */
    pmResult = orxNULL;
  }

  /* Done! */
  return pmResult;
}

static orxINLINE orxDISPLAY_MATRIX *orxDisplay_Android_InitMatrix(orxDISPLAY_MATRIX *_pmMatrix, orxFLOAT _fPosX, orxFLOAT _fPosY, orxFLOAT _fScaleX, orxFLOAT _fScaleY, orxFLOAT _fRotation, orxFLOAT _fPivotX, orxFLOAT _fPivotY)
{
  orxFLOAT fCos, fSin, fSCosX, fSCosY, fSSinX, fSSinY, fTX, fTY;

  /* Has rotation? */
  if (_fRotation != orxFLOAT_0)
  {
    /* Gets its cos/sin */
    fCos = orxMath_Cos(_fRotation);
    fSin = orxMath_Sin(_fRotation);
  }
  else
  {
    /* Inits cos/sin */
    fCos = orxFLOAT_1;
    fSin = orxFLOAT_0;
  }

  /* Computes values */
  fSCosX = _fScaleX * fCos;
  fSCosY = _fScaleY * fCos;
  fSSinX = _fScaleX * fSin;
  fSSinY = _fScaleY * fSin;
  fTX = _fPosX - (_fPivotX * fSCosX) + (_fPivotY * fSSinY);
  fTY = _fPosY - (_fPivotX * fSSinX) - (_fPivotY * fSCosY);

  /* Updates matrix */
  orxVector_Set(&(_pmMatrix->vX), fSCosX, -fSSinY, fTX);
  orxVector_Set(&(_pmMatrix->vY), fSSinX, fSCosY, fTY);

  /* Done! */
  return _pmMatrix;
}

static orxSTATUS orxFASTCALL orxDisplay_Android_CompileShader(orxDISPLAY_SHADER *_pstShader)
{
  static const orxSTRING szVertexShaderSource =
  "attribute vec2 _vPosition_;"
  "uniform mat4 _mProjection_;"
  "attribute mediump vec2 _vTexCoord_;"
  "varying mediump vec2 _gl_TexCoord0_;"
  "attribute mediump vec4 _vColor_;"
  "varying mediump vec4 _Color_;"
  "void main()"
  "{"
  "  mediump float fCoef = 1.0 / 255.0;"
  "  gl_Position      = _mProjection_ * vec4(_vPosition_.xy, 0.0, 1.0);"
  "  _gl_TexCoord0_   = _vTexCoord_;"
  "  _Color_      = fCoef * _vColor_;"
  "}";

  GLuint    uiProgram, uiVertexShader, uiFragmentShader;
  GLint     iSuccess;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Creates program */
  uiProgram = glCreateProgram();
  glASSERT();

  /* Creates vertex and fragment shaders */
  uiVertexShader   = glCreateShader(GL_VERTEX_SHADER);
  glASSERT();
  uiFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glASSERT();

  /* Compiles shader objects */
  glShaderSource(uiVertexShader, 1, (const char **)&szVertexShaderSource, NULL);
  glASSERT();
  glShaderSource(uiFragmentShader, 1, (const char **)&(_pstShader->zCode), NULL);
  glASSERT();
  glCompileShader(uiVertexShader);
  glASSERT();
  glCompileShader(uiFragmentShader);
  glASSERT();

  /* Gets vertex shader compiling status */
  glGetShaderiv(uiVertexShader, GL_COMPILE_STATUS, &iSuccess);
  glASSERT();

  /* Success? */
  if(iSuccess != GL_FALSE)
  {
    /* Gets fragment shader compiling status */
    glGetShaderiv(uiFragmentShader, GL_COMPILE_STATUS, &iSuccess);
    glASSERT();

    /* Success? */
    if(iSuccess != GL_FALSE)
    {
      /* Attaches shader objects to program */
      glAttachShader(uiProgram, uiVertexShader);
      glASSERT();
      glAttachShader(uiProgram, uiFragmentShader);
      glASSERT();

      /* Deletes shader objects */
      glDeleteShader(uiVertexShader);
      glASSERT();
      glDeleteShader(uiFragmentShader);
      glASSERT();

      /* Binds attributes */
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, "_vPosition_");
      glASSERT();
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, "_vTexCoord_");
      glASSERT();
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, "_vColor_");
      glASSERT();

      /* Links program */
      glLinkProgram(uiProgram);
      glASSERT();

      /* Gets texture location */
      _pstShader->uiTextureLocation = glGetUniformLocation(uiProgram, "_Texture_");
      glASSERT();

      /* Gets projection matrix location */
      _pstShader->uiProjectionMatrixLocation = glGetUniformLocation(uiProgram, "_mProjection_");
      glASSERT();

      /* Gets linking status */
      glGetProgramiv(uiProgram, GL_LINK_STATUS, &iSuccess);
      glASSERT();

      /* Success? */
      if(iSuccess != GL_FALSE)
      {
        /* Updates shader */
        _pstShader->uiProgram       = uiProgram;
        _pstShader->iTextureCounter = 0;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxCHAR acBuffer[4096];

        /* Gets log */
        glGetProgramInfoLog(uiProgram, 4095 * sizeof(orxCHAR), NULL, (char *)acBuffer);
        glASSERT();
        acBuffer[4095] = orxCHAR_NULL;

        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link shader program:\n%s\n", acBuffer);

        /* Deletes program */
        glDeleteProgram(uiProgram);
        glASSERT();
      }
    }
    else
    {
      orxCHAR acBuffer[4096];

      /* Gets log */
      glGetShaderInfoLog(uiFragmentShader, 4095 * sizeof(orxCHAR), NULL, (char *)acBuffer);
      glASSERT();
      acBuffer[4095] = orxCHAR_NULL;

      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile fragment shader:\n%s\n", acBuffer);

      /* Deletes shader objects & program */
      glDeleteShader(uiVertexShader);
      glASSERT();
      glDeleteShader(uiFragmentShader);
      glASSERT();
      glDeleteProgram(uiProgram);
      glASSERT();
    }
  }
  else
  {
    orxCHAR acBuffer[4096];

    /* Gets log */
    glGetShaderInfoLog(uiVertexShader, 4095 * sizeof(orxCHAR), NULL, (char *)acBuffer);
    glASSERT();
    acBuffer[4095] = orxCHAR_NULL;

    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile vertex shader:\n%s\n", acBuffer);

    /* Deletes shader objects & program */
    glDeleteShader(uiVertexShader);
    glASSERT();
    glDeleteShader(uiFragmentShader);
    glASSERT();
    glDeleteProgram(uiProgram);
    glASSERT();
  }

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_Android_InitShader(orxDISPLAY_SHADER *_pstShader)
{
  /* Uses shader's program */
  glUseProgram(_pstShader->uiProgram);
  glASSERT();

  /* Has custom textures? */
  if(_pstShader->iTextureCounter > 0)
  {
    GLint i;
    orxBOOL bCaptured = orxFALSE;

    /* For all defined textures */
    for(i = 0; i < _pstShader->iTextureCounter; i++)
    {
      /* Updates corresponding texture unit */
      glUniform1i(_pstShader->astTextureInfoList[i].iLocation, i);
      glASSERT();
      glActiveTexture(GL_TEXTURE0 + i);
      glASSERT();
      glBindTexture(GL_TEXTURE_2D, _pstShader->astTextureInfoList[i].pstBitmap->uiTexture);
      glASSERT();

      /* Screen and not already captured? */
      if((_pstShader->astTextureInfoList[i].pstBitmap == sstDisplay.pstScreen) && (bCaptured == orxFALSE))
      {
        /* Copies screen content */
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, orxF2U(sstDisplay.pstScreen->fHeight) - sstDisplay.pstScreen->u32RealHeight, orxF2U(sstDisplay.pstScreen->fWidth), sstDisplay.pstScreen->u32RealHeight);
        glASSERT();

        /* Updates captured status */
        bCaptured = orxTRUE;
      }
    }
  }

  /* Updates its status */
  _pstShader->bInitialized = orxTRUE;

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_Android_DrawArrays()
{
  /* Has data? */
  if(sstDisplay.s32BufferIndex > 0)
  {
    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxDisplay_DrawArrays");

    /* Sets vextex attribute arrays */
    glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
    glASSERT();
    glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
    glASSERT();
    glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
    glASSERT();

    /* Has active shaders? */
    if(sstDisplay.s32ActiveShaderCounter > 0)
    {
      orxDISPLAY_SHADER *pstShader;

      /* For all shaders */
      for(pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, orxNULL);
          pstShader != orxNULL;
          pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, pstShader))
      {
        /* Is active? */
        if(pstShader->bActive != orxFALSE)
        {
          /* Inits shader */
          orxDisplay_Android_InitShader(pstShader);

          /* Draws arrays */
          glDrawElements(GL_TRIANGLE_STRIP, sstDisplay.s32BufferIndex + (sstDisplay.s32BufferIndex >> 1), GL_UNSIGNED_SHORT, 0);
          glASSERT();
        }
      }
    }
    else
    {
      /* Draws arrays */
      glDrawElements(GL_TRIANGLE_STRIP, sstDisplay.s32BufferIndex + (sstDisplay.s32BufferIndex >> 1), GL_UNSIGNED_SHORT, 0);
      glASSERT();
    }

    /* Clears buffer index */
    sstDisplay.s32BufferIndex = 0;

	/* Profiles */
    orxPROFILER_POP_MARKER();
  }

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_Android_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.pstLastBitmap)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Binds source's texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* No active shader? */
    if(sstDisplay.s32ActiveShaderCounter == 0)
    {
      /* Stores it */
      sstDisplay.pstLastBitmap = _pstBitmap;
    }
    else
    {
      /* Clears last bitmap */
      sstDisplay.pstLastBitmap = orxNULL;
    }
  }

  /* Depending on smoothing type */
  switch(_eSmoothing)
  {
    case orxDISPLAY_SMOOTHING_ON:
    {
      /* Applies smoothing */
      bSmoothing = orxTRUE;

      break;
    }

    case orxDISPLAY_SMOOTHING_OFF:
    {
      /* Applies no smoothing */
      bSmoothing = orxFALSE;

      break;
    }

    default:
    case orxDISPLAY_SMOOTHING_DEFAULT:
    {
      /* Applies default smoothing */
      bSmoothing = sstDisplay.bDefaultSmoothing;

      break;
    }
  }

  /* Should update smoothing? */
  if(bSmoothing != _pstBitmap->bSmoothing)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Smoothing? */
    if(bSmoothing != orxFALSE)
    {
      /* Updates texture */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glASSERT();

      /* Updates mode */
      ((orxBITMAP *)_pstBitmap)->bSmoothing = orxTRUE;
    }
    else
    {
      /* Updates texture */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glASSERT();

      /* Updates mode */
      ((orxBITMAP *)_pstBitmap)->bSmoothing = orxFALSE;
    }
  }

  /* New blend mode? */
  if(_eBlendMode != sstDisplay.eLastBlendMode)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Stores it */
    sstDisplay.eLastBlendMode = _eBlendMode;

    /* Depending on blend mode */
    switch(_eBlendMode)
    {
      case orxDISPLAY_BLEND_MODE_ALPHA:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glASSERT();
        break;
      }
      case orxDISPLAY_BLEND_MODE_MULTIPLY:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        glASSERT();
        break;
      }
      case orxDISPLAY_BLEND_MODE_ADD:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glASSERT();
        break;
      }
      default:
      {
        glDisable(GL_BLEND);
        glASSERT();
        break;
      }
    }
  }

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_Android_DrawBitmap(const orxBITMAP *_pstBitmap, const orxDISPLAY_MATRIX *_pmTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  GLfloat fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_Android_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth = (GLfloat) (_pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat) (_pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY);

  /* End of buffer? */
  if (sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
  {
    /* Draw arrays */
    orxDisplay_Android_DrawArrays();
  }

  /* Fills the vertex list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX     = (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY     = (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX = _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY = _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX = (_pmTransform->vX.fX * fWidth) + (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY = (_pmTransform->vY.fX * fWidth) + (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX = (_pmTransform->vX.fX * fWidth) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY = (_pmTransform->vY.fX * fWidth) + _pmTransform->vY.fZ;

  /* Fills the texture coord list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU     =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU = (GLfloat) (_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU = (GLfloat) (_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV = (GLfloat) (orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV     =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV = (GLfloat) (orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));

  /* Fills the color list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA     =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA = _pstBitmap->stColor;

  /* Updates index */
  sstDisplay.s32BufferIndex += 4;

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_Android_DrawPrimitive(orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill, orxBOOL _bOpen)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DrawPrimitive");

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Starts no texture shader */
    orxDisplay_StartShader((orxHANDLE)sstDisplay.pstNoTextureShader);

    /* Inits it */
    orxDisplay_Android_InitShader(sstDisplay.pstNoTextureShader);

    /* Selects arrays */
    glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
    glASSERT();
    glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
    glASSERT();
  }

  /* Has alpha? */
  if(orxRGBA_A(_stColor) != 0xFF)
  {
    /* Enables alpha blending */
    glEnable(GL_BLEND);
    glASSERT();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glASSERT();
  }
  else
  {
    /* Disables alpha blending */
    glDisable(GL_BLEND);
    glASSERT();
  }

  /* Only 2 vertices? */
  if(_u32VertexNumber == 2)
  {
    /* Draws it */
    glDrawArrays(GL_LINES, 0, 2);
    glASSERT();
  }
  else
  {
    /* Should fill? */
    if(_bFill != orxFALSE)
    {
      /* Draws it */
      glDrawArrays(GL_TRIANGLE_FAN, 0, _u32VertexNumber);
      glASSERT();
    }
    else
    {
      /* Is open? */
      if(_bOpen != orxFALSE)
      {
        /* Draws it */
        glDrawArrays(GL_LINE_STRIP, 0, _u32VertexNumber);
        glASSERT();
      }
      else
      {
        /* Draws it */
        glDrawArrays(GL_LINE_LOOP, 0, _u32VertexNumber);
        glASSERT();
      }
    }
  }

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Stops current shader */
    orxDisplay_StopShader((orxHANDLE)sstDisplay.pstNoTextureShader);
  }
  else
  {
    /* Reenables texturing */
    glEnable(GL_TEXTURE_2D);
    glASSERT();
  }

  /* Clears last blend mode */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_Android_GetScreenBitmap()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstDisplay.pstScreen;
}

orxSTATUS orxFASTCALL orxDisplay_Android_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  const orxCHAR *pc;
  orxU32 u32CharacterCodePoint;
  GLfloat fX, fY, fHeight;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_Android_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* Gets character's height */
  fHeight = _pstMap->fCharacterHeight;

  /* Prepares font for drawing */
  orxDisplay_Android_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode);

  /* For all characters */
  for (u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_zString, &pc), fX = 0.0f, fY = 0.0f;
       u32CharacterCodePoint != orxCHAR_NULL;
       u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc))
  {
    /* Depending on character */
    switch (u32CharacterCodePoint)
    {
      case orxCHAR_CR:
      {
        /* Half EOL? */
        if (*pc == orxCHAR_LF)
        {
          /* Updates pointer */
          pc++;
        }

        /* Fall through */
      }

      case orxCHAR_LF:
      {
        /* Updates Y position */
        fY += fHeight;

        /* Resets X position */
        fX = 0.0f;

        break;
      }

      default:
      {
        const orxCHARACTER_GLYPH *pstGlyph;

        /* Gets glyph from table */
        pstGlyph = (orxCHARACTER_GLYPH *) orxHashTable_Get(_pstMap->pstCharacterTable, u32CharacterCodePoint);
        orxFLOAT                  fWidth;

        /* Valid? */
        if (pstGlyph != orxNULL)
        {
          /* Gets character width */
          fWidth = pstGlyph->fWidth;

          /* End of buffer? */
          if (sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
          {
            /* Draw arrays */
            orxDisplay_Android_DrawArrays();
          }

          /* Outputs vertices and texture coordinates */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX     = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY     = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU = (GLfloat) (_pstFont->fRecRealWidth * (pstGlyph->fX + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU = (GLfloat) (_pstFont->fRecRealWidth * (pstGlyph->fX + fWidth - orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV = (GLfloat) (orxFLOAT_1 - _pstFont->fRecRealHeight * (pstGlyph->fY + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV = (GLfloat) (orxFLOAT_1 - _pstFont->fRecRealHeight * (pstGlyph->fY + fHeight - orxDISPLAY_KF_BORDER_FIX));

          /* Fills the color list */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA = _pstFont->stColor;

          /* Updates counter */
          sstDisplay.s32BufferIndex += 4;
        }
        else
        {
          /* Gets default width */
          fWidth = fHeight;
        }

        /* Updates X position */
        fX += fWidth;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawLine(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvStart != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* Copies vertices */
  sstDisplay.astVertexList[0].fX = (GLfloat)(_pvStart->fX);
  sstDisplay.astVertexList[0].fY = (GLfloat)(_pvStart->fY);
  sstDisplay.astVertexList[1].fX = (GLfloat)(_pvEnd->fX);
  sstDisplay.astVertexList[1].fY = (GLfloat)(_pvEnd->fY);

  /* Copies color */
  sstDisplay.astVertexList[0].stRGBA =
  sstDisplay.astVertexList[1].stRGBA = _stColor;

  /* Draws it */
  orxDisplay_Android_DrawPrimitive(2, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawPolyline(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* For all vertices */
  for(i = 0; i < _u32VertexNumber; i++)
  {
    /* Copies its coords */
    sstDisplay.astVertexList[i].fX = (GLfloat)(_avVertexList[i].fX);
    sstDisplay.astVertexList[i].fY = (GLfloat)(_avVertexList[i].fY);

    /* Copies color */
    sstDisplay.astVertexList[i].stRGBA = _stColor;
  }

  /* Draws it */
  orxDisplay_Android_DrawPrimitive(_u32VertexNumber, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawPolygon(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* For all vertices */
  for(i = 0; i < _u32VertexNumber; i++)
  {
    /* Copies its coords */
    sstDisplay.astVertexList[i].fX = (GLfloat)(_avVertexList[i].fX);
    sstDisplay.astVertexList[i].fY = (GLfloat)(_avVertexList[i].fY);

    /* Copies color */
    sstDisplay.astVertexList[i].stRGBA = _stColor;
  }

  /* Draws it */
  orxDisplay_Android_DrawPrimitive(_u32VertexNumber, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawCircle(const orxVECTOR *_pvCenter, orxFLOAT _fRadius, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxFLOAT  fAngle;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvCenter != orxNULL);
  orxASSERT(_fRadius >= orxFLOAT_0);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* For all vertices */
  for(i = 0, fAngle = orxFLOAT_0; i < orxDISPLAY_KU32_CIRCLE_LINE_NUMBER; i++, fAngle += orxMATH_KF_2_PI / orxDISPLAY_KU32_CIRCLE_LINE_NUMBER)
  {
    /* Copies its coords */
    sstDisplay.astVertexList[i].fX = (GLfloat)(_fRadius * orxMath_Cos(fAngle) + _pvCenter->fX);
    sstDisplay.astVertexList[i].fY = (GLfloat)(_fRadius * orxMath_Sin(fAngle) + _pvCenter->fY);

    /* Copies color */
    sstDisplay.astVertexList[i].stRGBA = _stColor;
  }

  /* Draws it */
  orxDisplay_Android_DrawPrimitive(orxDISPLAY_KU32_CIRCLE_LINE_NUMBER, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawOBox(const orxOBOX *_pstBox, orxRGBA _stColor, orxBOOL _bFill)
{
  orxVECTOR vOrigin;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBox != orxNULL);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* Gets origin */
  orxVector_Sub(&vOrigin, &(_pstBox->vPosition), &(_pstBox->vPivot));

  /* Sets vertices */
  sstDisplay.astVertexList[0].fX = (GLfloat)(vOrigin.fX);
  sstDisplay.astVertexList[0].fY = (GLfloat)(vOrigin.fY);
  sstDisplay.astVertexList[1].fX = (GLfloat)(vOrigin.fX + _pstBox->vX.fX);
  sstDisplay.astVertexList[1].fY = (GLfloat)(vOrigin.fY + _pstBox->vX.fY);
  sstDisplay.astVertexList[2].fX = (GLfloat)(vOrigin.fX + _pstBox->vX.fX + _pstBox->vY.fX);
  sstDisplay.astVertexList[2].fY = (GLfloat)(vOrigin.fY + _pstBox->vX.fY + _pstBox->vY.fY);
  sstDisplay.astVertexList[3].fX = (GLfloat)(vOrigin.fX + _pstBox->vY.fX);
  sstDisplay.astVertexList[3].fY = (GLfloat)(vOrigin.fY + _pstBox->vY.fY);

  /* Copies color */
  sstDisplay.astVertexList[0].stRGBA =
  sstDisplay.astVertexList[1].stRGBA =
  sstDisplay.astVertexList[2].stRGBA =
  sstDisplay.astVertexList[3].stRGBA = _stColor;

  /* Draws it */
  orxDisplay_Android_DrawPrimitive(4, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawMesh(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode, orxU32 _u32VertexNumber, const orxDISPLAY_VERTEX *_astVertexList)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32VertexNumber > 2);
  orxASSERT(_astVertexList != orxNULL);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_Android_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if (_pstBitmap != sstDisplay.pstScreen)
  {
    /* Is last used bitmap? */
    if(sstDisplay.pstLastBitmap == _pstBitmap)
    {
      /* Resets it */
      sstDisplay.pstLastBitmap = orxNULL;
    }

    /* Deletes its texture */
    glDeleteTextures(1, &(_pstBitmap->uiTexture));
    glASSERT();

    /* Deletes it */
    orxBank_Free(sstDisplay.pstBitmapBank, _pstBitmap);
  }

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_Android_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  orxBITMAP *pstBitmap;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Allocates bitmap */
  pstBitmap = (orxBITMAP *) orxBank_Allocate(sstDisplay.pstBitmapBank);

  /* Valid? */
  if (pstBitmap != orxNULL)
  {
    GLint iTexture;

    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Inits it */
    pstBitmap->bSmoothing = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
    pstBitmap->fWidth = orxU2F(_u32Width);
    pstBitmap->fHeight = orxU2F(_u32Height);
    pstBitmap->u32RealWidth = _u32Width;
    pstBitmap->u32RealHeight = _u32Height;
    pstBitmap->fRecRealWidth = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
    pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
    pstBitmap->stColor = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
    orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

    /* Backups current texture */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
    glASSERT();

    /* Creates new texture */
    glGenTextures(1, &pstBitmap->uiTexture);
    glASSERT();
    glBindTexture(GL_TEXTURE_2D, pstBitmap->uiTexture);
    glASSERT();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
    glASSERT();

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, iTexture);
    glASSERT();

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return pstBitmap;
}

orxSTATUS orxFASTCALL orxDisplay_Android_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Is not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    orxBITMAP *pstBackupBitmap;

    /* Backups current destination */
    pstBackupBitmap = sstDisplay.pstDestinationBitmap;

    /* Sets new destination bitmap */
    orxDisplay_SetDestinationBitmap(_pstBitmap);

    /* Clears the color buffer with given color */
    glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
    glASSERT();
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();

    /* Restores previous destination */
    orxDisplay_SetDestinationBitmap(pstBackupBitmap);
  }
  else
  {
    /* Makes sure we're working on screen */
    orxDisplay_SetDestinationBitmap(sstDisplay.pstScreen);

    /* Clears the color buffer with given color */
    glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
    glASSERT();
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* swap EGL */
  NVEventSwapBuffersEGL();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxU32 u32Width, u32Height;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Gets bitmap's size */
  u32Width = orxF2U(_pstBitmap->fWidth);
  u32Height = orxF2U(_pstBitmap->fHeight);

  /* Valid? */
  if ((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * sizeof(orxRGBA)))
  {
    GLint iTexture;
    orxU8 *pu8ImageBuffer;
    orxU32 i, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset;

    /* Allocates buffer */
    pu8ImageBuffer = (orxU8 *) orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_VIDEO);

    /* Gets line sizes */
    u32LineSize = orxF2U(_pstBitmap->fWidth) * sizeof(orxRGBA);
    u32RealLineSize = _pstBitmap->u32RealWidth * sizeof(orxRGBA);

    /* Clears padding */
    orxMemory_Zero(pu8ImageBuffer, u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)));

    /* For all lines */
    for (i = 0, u32SrcOffset = 0, u32DstOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - 1);
         i < u32Height;
         i++, u32SrcOffset += u32LineSize, u32DstOffset -= u32RealLineSize)
    {
      /* Copies data */
      orxMemory_Copy(pu8ImageBuffer + u32DstOffset, _au8Data + u32SrcOffset, u32LineSize);

      /* Adds padding */
      orxMemory_Zero(pu8ImageBuffer + u32DstOffset + u32LineSize, u32RealLineSize - u32LineSize);
    }

    /* Backups current texture */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
    glASSERT();

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
    glASSERT();

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, iTexture);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(pu8ImageBuffer);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Screen? */
    if (_pstBitmap == sstDisplay.pstScreen)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data: can't use screen as destination bitmap.");
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data: format needs to be RGBA.");
    }

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_GetBitmapData(orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxU32    u32BufferSize;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Gets buffer size */
  u32BufferSize = orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8);

  /* Is size matching? */
  if(_u32ByteNumber == u32BufferSize)
  {
    GLuint uiFrameBuffer;

    /* Generates frame buffer */
    glGenFramebuffers(1, &uiFrameBuffer);
    glASSERT();

    /* Binds frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, uiFrameBuffer);
    glASSERT();

    /* Links it to frame buffer */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pstBitmap->uiTexture, 0);
    glASSERT();

    /* Updates result */
    eResult = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    glASSERT();

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      orxU32  u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset, i;
      orxU8  *pu8ImageData;

      /* Allocates buffer */
      pu8ImageData = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

      /* Checks */
      orxASSERT(pu8ImageData != orxNULL);

   		/* Inits viewport */
 	  	glViewport(0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight);
 	  	glASSERT();

      /* Reads OpenGL data */
      glReadPixels(0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageData);
      glASSERT();

      /* Gets line sizes */
      u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
      u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

      /* Clears padding */
      orxMemory_Zero(_au8Data, u32LineSize * orxF2U(_pstBitmap->fHeight));

      /* For all lines */
      for(i = 0, u32SrcOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)), u32DstOffset = u32LineSize * (orxF2U(_pstBitmap->fHeight) - 1);
          i < orxF2U(_pstBitmap->fHeight);
          i++, u32SrcOffset += u32RealLineSize, u32DstOffset -= u32LineSize)
      {
        /* Copies data */
        orxMemory_Copy(_au8Data + u32DstOffset, pu8ImageData + u32SrcOffset, u32LineSize);
      }

      /* Frees buffers */
      orxMemory_Free(pu8ImageData);
    }

    /* unBinds frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glASSERT();

    /* Deletes it */
    glDeleteFramebuffers(1, &uiFrameBuffer);
    glASSERT();

    /* Clears destination bitmap for a rebind */
    sstDisplay.pstDestinationBitmap = orxNULL;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't get bitmap's data <0x%X> as the buffer size is %ld when it should be %ls.", _pstBitmap, _u32ByteNumber, u32BufferSize);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if (_pstBitmap != sstDisplay.pstScreen)
  {
    /* Stores it */
    _pstBitmap->stColor = _stColor;
  }

  /* Done! */
  return eResult;
}

orxRGBA orxFASTCALL orxDisplay_Android_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = orx2RGBA(0, 0, 0, 0);

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if (_pstBitmap != sstDisplay.pstScreen)
  {
    /* Updates result */
    stResult = _pstBitmap->stColor;
  }

  /* Done! */
  return stResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetDestinationBitmap(orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

	/* Checks */
	orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

	/* Different destination bitmap? */
	if (_pstBitmap != sstDisplay.pstDestinationBitmap)
    {
      /* Draws remaining items */
      orxDisplay_Android_DrawArrays();

      /* Stores it */
      sstDisplay.pstDestinationBitmap = _pstBitmap;

	  /* Is valid? */
      if(_pstBitmap != orxNULL)
	  {
        /* Screen? */
        if(_pstBitmap == sstDisplay.pstScreen)
        {
          /* Unbinds frame buffer */
          orxPROFILER_PUSH_MARKER("glBindFramebuffer screen");
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          orxPROFILER_POP_MARKER();
          glASSERT();

          /* Updates result */
          eResult = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
          glASSERT();
        }
        else
        {
          /* Binds frame buffer */
          orxPROFILER_PUSH_MARKER("glBindFramebuffer viewport");
          glBindFramebuffer(GL_FRAMEBUFFER, sstDisplay.uiFrameBuffer);
          orxPROFILER_POP_MARKER();
          glASSERT();

          /* Links it to frame buffer */
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pstBitmap->uiTexture, 0);
          glASSERT();

          /* Updates result */
          eResult = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
          glASSERT();
        }

        /* Is screen? */
        if(sstDisplay.pstDestinationBitmap == sstDisplay.pstScreen)
        {
          /* Inits viewport */
          glViewport(0, 0, (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fWidth), (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fHeight));
          glASSERT();
        }
        else
        {
          /* Inits viewport */
          glViewport(0, (orxS32)sstDisplay.pstDestinationBitmap->u32RealHeight - orxF2S(sstDisplay.pstDestinationBitmap->fHeight), (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fWidth), (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fHeight));
          glASSERT();
        }

        /* Inits projection matrix */
        orxDisplay_Android_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.pstDestinationBitmap->fWidth, sstDisplay.pstDestinationBitmap->fHeight, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1);

        /* Passes it to shader */
        glUniformMatrix4fv(sstDisplay.pstDefaultShader->uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
        glASSERT();
	  }
	  else
      {
        eResult = orxSTATUS_FAILURE;
      }
	}

	/* Done! */
	return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_BlitBitmap(const orxBITMAP *_pstSrc, const orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));

  /* Inits matrix */
  orxVector_Set(&(mTransform.vX), orxFLOAT_1, orxFLOAT_0, _fPosX);
  orxVector_Set(&(mTransform.vY), orxFLOAT_0, orxFLOAT_1, _fPosY);

  /* Draws it */
  orxDisplay_Android_DrawBitmap(_pstSrc, &mTransform, _eSmoothing, _eBlendMode);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_Android_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* No repeat? */
  if ((_pstTransform->fRepeatX == orxFLOAT_1) && (_pstTransform->fRepeatY == orxFLOAT_1))
  {
    /* Draws it */
    orxDisplay_Android_DrawBitmap(_pstSrc, &mTransform, _eSmoothing, _eBlendMode);
  }
  else
  {
    orxFLOAT i, j, fRecRepeatX;
    GLfloat fX, fY, fWidth, fHeight, fTop, fBottom, fLeft, fRight;

    /* Prepares bitmap for drawing */
    orxDisplay_Android_PrepareBitmap(_pstSrc, _eSmoothing, _eBlendMode);

    /* Inits bitmap height */
    fHeight = (GLfloat) ((_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY) / _pstTransform->fRepeatY);

    /* Inits texture coords */
    fLeft = _pstSrc->fRecRealWidth * _pstSrc->stClip.vTL.fX;
    fTop = orxFLOAT_1 - (_pstSrc->fRecRealHeight * _pstSrc->stClip.vTL.fY);

    /* For all lines */
    for (fY = 0.0f, i = _pstTransform->fRepeatY, fRecRepeatX = orxFLOAT_1 / _pstTransform->fRepeatX; i > orxFLOAT_0; i -= orxFLOAT_1, fY += fHeight)
    {
      /* Partial line? */
      if (i < orxFLOAT_1)
      {
        /* Updates height */
        fHeight *= (GLfloat) i;

        /* Resets texture coords */
        fRight = (GLfloat) (_pstSrc->fRecRealWidth * _pstSrc->stClip.vBR.fX);
        fBottom = (GLfloat) (orxFLOAT_1 - (_pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + (i * (_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY)))));
      }
      else
      {
        /* Resets texture coords */
        fRight = (GLfloat) (_pstSrc->fRecRealWidth * _pstSrc->stClip.vBR.fX);
        fBottom = (GLfloat) (orxFLOAT_1 - (_pstSrc->fRecRealHeight * _pstSrc->stClip.vBR.fY));
      }

      /* Resets bitmap width */
      fWidth = (GLfloat) ((_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX) * fRecRepeatX);

      /* For all columns */
      for (fX = 0.0f, j = _pstTransform->fRepeatX; j > orxFLOAT_0; j -= orxFLOAT_1, fX += fWidth)
      {
        /* Partial column? */
        if (j < orxFLOAT_1)
        {
          /* Updates width */
          fWidth *= (GLfloat) j;

          /* Updates texture right coord */
          fRight = (GLfloat) (_pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + (j * (_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX))));
        }

        /* End of buffer? */
        if (sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
        {
          /* Draws arrays */
          orxDisplay_Android_DrawArrays();
        }

        /* Outputs vertices and texture coordinates */
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX     = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY     = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU     =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU = fLeft;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU = fRight;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV = fTop;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV     =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV = fBottom;

        /* Fills the color list */
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA     =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA = _pstSrc->stColor;

        /* Updates counter */
        sstDisplay.s32BufferIndex += 4;
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not supported on this platform.");

  /* Done! */
  return eResult;
}

static orxBITMAP *orxDisplay_Android_LoadETC1Bitmap(const orxSTRING _zFilename)
{
	/* Checks */
	orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  orxBITMAP *pstResult = orxNULL;
  NvFile* apkFile;
  etc1_byte* fileData;
  orxCHAR zPKMFileName[255];

  orxString_Copy(zPKMFileName, _zFilename);
  orxString_Copy(zPKMFileName + orxString_GetLength(_zFilename), ".pkm");

  /* open the asset file and save them into memory */
  apkFile = NvFOpen(zPKMFileName);
  if(apkFile != orxNULL)
  {
    size_t apkFileSize;

    apkFileSize = NvFSize(apkFile);
    fileData = (etc1_byte *)orxMemory_Allocate(sizeof(unsigned char)*apkFileSize, orxMEMORY_TYPE_MAIN);
    /* read file */
    NvFRead(fileData, apkFileSize, sizeof(unsigned char), apkFile);

    /* close it */
    NvFClose(apkFile);

    /* check if file is a valid pkm */
    if(etc1_pkm_is_valid(fileData))
    {
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "valid pkm file found");
      /* Allocates bitmap */
      pstResult = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

      /* Valid? */
      if(pstResult != orxNULL)
      {

        GLuint uiRealWidth, uiRealHeight;
        GLint iTexture;

        uiRealWidth = etc1_pkm_get_width(fileData);
        uiRealHeight = etc1_pkm_get_height(fileData);


        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Inits bitmap */
        pstResult->bSmoothing = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
        pstResult->fWidth = orxU2F(uiRealWidth);
        pstResult->fHeight = orxU2F(uiRealHeight);
        pstResult->u32RealWidth = uiRealWidth;
        pstResult->u32RealHeight = uiRealHeight;
        pstResult->fRecRealWidth = orxFLOAT_1 / orxU2F(pstResult->u32RealWidth);
        pstResult->fRecRealHeight = orxFLOAT_1 / orxU2F(pstResult->u32RealHeight);
        pstResult->stColor = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
        pstResult->bCompressed = orxTRUE;
        orxVector_Copy(&(pstResult->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(pstResult->stClip.vBR), pstResult->fWidth, pstResult->fHeight, orxFLOAT_0);

        /* Backups current texture */
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
        glASSERT();

        /* Creates new texture */
        glGenTextures(1, &pstResult->uiTexture);
        glASSERT();
        glBindTexture(GL_TEXTURE_2D, pstResult->uiTexture);
        glASSERT();
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, pstResult->u32RealWidth, pstResult->u32RealHeight, 0, etc1_get_encoded_data_size(uiRealWidth, uiRealHeight), fileData + ETC_PKM_HEADER_SIZE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();

        /* Restores previous texture */
        glBindTexture(GL_TEXTURE_2D, iTexture);
        glASSERT();

        /* Pops config section */
        orxConfig_PopSection();
      }
    }

    /* free file data */
    orxMemory_Free(fileData);
  }

  return pstResult;
}

orxBITMAP *orxFASTCALL orxDisplay_Android_LoadBitmap(const orxSTRING _zFilename)
{
  unsigned char *pu8ImageData;
  GLuint uiWidth, uiHeight, uiBytesPerPixel;
  orxBITMAP *pstResult = orxNULL;
  NvFile* apkFile;
  unsigned char* fileData;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* try to open the ETC1 compressed file */
  pstResult = orxDisplay_Android_LoadETC1Bitmap(_zFilename);

  /* Not already loaded? */
  if(pstResult == orxNULL)
  {
    size_t apkFileSize;

    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "valid pkm file not found loading uncompressed file");
    /* open the asset file and save them into memory */
    apkFile = NvFOpen(_zFilename);
    apkFileSize = NvFSize(apkFile);
    fileData = (orxU8 *)orxMemory_Allocate(sizeof(unsigned char)*apkFileSize, orxMEMORY_TYPE_MAIN);

    /* read file */
    NvFRead(fileData, apkFileSize, sizeof(unsigned char), apkFile);

    /* close it */
    NvFClose(apkFile);

    /* Loads image */
    pu8ImageData = SOIL_load_image_from_memory(fileData, apkFileSize,(int *)&uiWidth, (int *)&uiHeight, (int *)&uiBytesPerPixel, SOIL_LOAD_RGBA);

    /* free file data */
    orxMemory_Free(fileData);

    /* Valid? */
    if(pu8ImageData != NULL)
    {
      /* Allocates bitmap */
      pstResult = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        GLuint i, uiSrcOffset, uiDstOffset, uiLineSize, uiRealLineSize, uiRealWidth, uiRealHeight;
        GLint iTexture;
        orxU8 *pu8ImageBuffer;

        /* Gets its real size */
        uiRealWidth = uiWidth;
        uiRealHeight = uiHeight;

        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Inits bitmap */
        pstResult->bSmoothing = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
        pstResult->fWidth = orxU2F(uiWidth);
        pstResult->fHeight = orxU2F(uiHeight);
        pstResult->u32RealWidth = uiRealWidth;
        pstResult->u32RealHeight = uiRealHeight;
        pstResult->fRecRealWidth = orxFLOAT_1 / orxU2F(pstResult->u32RealWidth);
        pstResult->fRecRealHeight = orxFLOAT_1 / orxU2F(pstResult->u32RealHeight);
        pstResult->stColor = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
        pstResult->bCompressed = orxFALSE;
        orxVector_Copy(&(pstResult->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(pstResult->stClip.vBR), pstResult->fWidth, pstResult->fHeight, orxFLOAT_0);

        /* Allocates buffer */
        pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(uiRealWidth * uiRealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

        /* Checks */
        orxASSERT(pu8ImageBuffer != orxNULL);

        /* Gets line sizes */
        uiLineSize = uiWidth * 4 * sizeof(orxU8);
        uiRealLineSize = uiRealWidth * 4 * sizeof(orxU8);

        /* Clears padding */
        orxMemory_Zero(pu8ImageBuffer, uiRealLineSize * (uiRealHeight - uiHeight));

        /* For all lines */
        for(i = 0, uiSrcOffset = 0, uiDstOffset = uiRealLineSize * (uiRealHeight - 1);
          i < uiHeight;
          i++, uiSrcOffset += uiLineSize, uiDstOffset -= uiRealLineSize)
        {
          /* Copies data */
          orxMemory_Copy(pu8ImageBuffer + uiDstOffset, pu8ImageData + uiSrcOffset, uiLineSize);

          /* Adds padding */
          orxMemory_Zero(pu8ImageBuffer + uiDstOffset + uiLineSize, uiRealLineSize - uiLineSize);
        }

        /* Backups current texture */
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
        glASSERT();

        /* Creates new texture */
        glGenTextures(1, &pstResult->uiTexture);
        glASSERT();
        glBindTexture(GL_TEXTURE_2D, pstResult->uiTexture);
        glASSERT();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstResult->u32RealWidth, pstResult->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();

        /* Restores previous texture */
        glBindTexture(GL_TEXTURE_2D, iTexture);
        glASSERT();

        /* Frees image buffer */
        orxMemory_Free(pu8ImageBuffer);

        /* Pops config section */
        orxConfig_PopSection();
      }

      /* Deletes surface */
      SOIL_free_image_data(pu8ImageData);
    }
  }

	/* Done! */
	return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth = _pstBitmap->fWidth;
  *_pfHeight = _pstBitmap->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth = sstDisplay.pstScreen->fWidth;
  *_pfHeight = sstDisplay.pstScreen->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Screen? */
  if (_pstBitmap == sstDisplay.pstScreen)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Enables clipping */
    glEnable(GL_SCISSOR_TEST);
    glASSERT();

    /* Stores screen clipping */
    glScissor(_u32TLX, orxF2U(sstDisplay.pstScreen->fHeight) - _u32BRY, _u32BRX - _u32TLX, _u32BRY - _u32TLY);
    glASSERT();

  }
  else
  {
    /* Stores clip coords */
    orxVector_Set(&(_pstBitmap->stClip.vTL), orxU2F(_u32TLX), orxU2F(_u32TLY), orxFLOAT_0);
    orxVector_Set(&(_pstBitmap->stClip.vBR), orxU2F(_u32BRX), orxU2F(_u32BRY), orxFLOAT_0);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_Android_IsVSyncEnabled()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetFullScreen(orxBOOL _bFullScreen)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_Android_IsFullScreen()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxU32 orxFASTCALL orxDisplay_Android_GetVideoModeCounter()
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_Android_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Clears last blend mode & last bitmap */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;
  sstDisplay.pstLastBitmap  = orxNULL;

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_Android_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

/*
 * init android display
 */
orxSTATUS orxFASTCALL orxDisplay_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if (!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "orxDisplay_Android_Init()");

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    orxU32 i;
    GLushort u16Index;

    /* For all indices */
    for (i = 0, u16Index = 0; i < orxDISPLAY_KU32_INDEX_BUFFER_SIZE; i += 6, u16Index += 4)
    {
      /* Computes them */
      sstDisplay.au16IndexList[i] = u16Index;
      sstDisplay.au16IndexList[i + 1] = u16Index;
      sstDisplay.au16IndexList[i + 2] = u16Index + 1;
      sstDisplay.au16IndexList[i + 3] = u16Index + 2;
      sstDisplay.au16IndexList[i + 4] = u16Index + 3;
      sstDisplay.au16IndexList[i + 5] = u16Index + 3;
    }

    /* Creates banks */
    sstDisplay.pstBitmapBank = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    sstDisplay.pstShaderBank  = orxBank_Create(orxDISPLAY_KU32_SHADER_BANK_SIZE, sizeof(orxDISPLAY_SHADER), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if (sstDisplay.pstBitmapBank != orxNULL
    && (sstDisplay.pstShaderBank != orxNULL))
    {
        orxDISPLAY_EVENT_PAYLOAD stPayload;

        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Depth buffer? */
        if (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DEPTHBUFFER) != orxFALSE)
        {
          /* Updates flags */
           sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER;
        }
        else
        {
          sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_NONE;
        }

        sstDisplay.u32Depth = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DEPTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH) : 24;

        // Init OpenGL ES 2.0
        initGLESConfig();

        /* Inits default values */
        sstDisplay.bDefaultSmoothing = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
        sstDisplay.pstScreen = (orxBITMAP *) orxBank_Allocate(sstDisplay.pstBitmapBank);
        orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));
        sstDisplay.pstScreen->fWidth = orxS2F(s_winWidth);
        sstDisplay.pstScreen->fHeight = orxS2F(s_winHeight);
        sstDisplay.pstScreen->u32RealWidth = orxF2U(sstDisplay.pstScreen->fWidth);
        sstDisplay.pstScreen->u32RealHeight = orxF2U(sstDisplay.pstScreen->fHeight);
        sstDisplay.pstScreen->fRecRealWidth = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
        sstDisplay.pstScreen->fRecRealHeight = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
        orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);
        sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;
        glGenFramebuffers(1, &sstDisplay.uiFrameBuffer);
        glASSERT();

        /* Updates config info */
        orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_WIDTH, sstDisplay.pstScreen->fWidth);
        orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_HEIGHT, sstDisplay.pstScreen->fHeight);
        orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_DEPTH, sstDisplay.u32Depth);

        /* Pops config section */
        orxConfig_PopSection();

        /* Gets max texture unit number */
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &(sstDisplay.iTextureUnitNumber));
        glASSERT();

        static const orxSTRING szFragmentShaderSource =
        "precision mediump float;"
        "varying vec2 _gl_TexCoord0_;"
        "varying vec4 _Color_;"
        "uniform sampler2D _Texture_;"
        "void main()"
        "{"
        "  gl_FragColor = _Color_ * texture2D(_Texture_, _gl_TexCoord0_);"
        "}";
        static const orxSTRING szNoTextureFragmentShaderSource =
        "precision mediump float;"
        "varying vec2 _gl_TexCoord0_;"
        "varying vec4 _Color_;"
        "uniform sampler2D _Texture_;"
        "void main()"
        "{"
        "  gl_FragColor = _Color_;"
        "}";

        /* Inits flags */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER | orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);

        /* Creates texture for screen backup */
        glGenTextures(1, &(sstDisplay.pstScreen->uiTexture));
        glASSERT();
        glBindTexture(GL_TEXTURE_2D, sstDisplay.pstScreen->uiTexture);
        glASSERT();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();

        /* Creates default shaders */
        sstDisplay.pstDefaultShader   = (orxDISPLAY_SHADER*) orxDisplay_CreateShader(szFragmentShaderSource, orxNULL);
        sstDisplay.pstNoTextureShader = (orxDISPLAY_SHADER*) orxDisplay_CreateShader(szNoTextureFragmentShaderSource, orxNULL);

        /* Uses it */
        orxDisplay_StopShader(orxNULL);

		/* Generates index buffer object (IBO) */
        glGenBuffers(1, &(sstDisplay.uiIndexBuffer));
        glASSERT();

        /* Binds it */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sstDisplay.uiIndexBuffer);
        glASSERT();

        /* Fills it */
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, orxDISPLAY_KU32_INDEX_BUFFER_SIZE * sizeof(GLushort), &(sstDisplay.au16IndexList), GL_STATIC_DRAW);
        glASSERT();

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
        stPayload.u32Width    = orxF2U(sstDisplay.pstScreen->fWidth);
        stPayload.u32Height   = orxF2U(sstDisplay.pstScreen->fHeight);
        stPayload.u32Depth    = 16;
        stPayload.bFullScreen = orxTRUE;

        /* Sends it */
        orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);

        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DISPLAY_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Deletes banks */
      orxBank_Delete(sstDisplay.pstBitmapBank);
      sstDisplay.pstBitmapBank = orxNULL;
      orxBank_Delete(sstDisplay.pstShaderBank);
      sstDisplay.pstShaderBank = orxNULL;

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
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

void orxFASTCALL orxDisplay_Android_Exit()
{
  if (sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Deletes default shaders */
      orxDisplay_DeleteShader(sstDisplay.pstDefaultShader);
      orxDisplay_DeleteShader(sstDisplay.pstNoTextureShader);
    }

    sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_NONE;

    glDeleteFramebuffers(1, &sstDisplay.uiFrameBuffer);
    glASSERT();

    /* Has index buffer? */
    if(sstDisplay.uiIndexBuffer != 0)
    {
      /* Deletes it */
      glDeleteBuffers(1, &(sstDisplay.uiIndexBuffer));
      glASSERT();
      sstDisplay.uiIndexBuffer = 0;
    }

    /* Deletes banks */
    orxBank_Delete(sstDisplay.pstBitmapBank);
    orxBank_Delete(sstDisplay.pstShaderBank);

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  /* Done! */
  return;
}

orxBOOL orxFASTCALL orxDisplay_Android_HasShaderSupport()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER)) ? orxTRUE : orxFALSE;
}

orxHANDLE orxFASTCALL orxDisplay_Android_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Valid? */
    if((_zCode != orxNULL) && (_zCode != orxSTRING_EMPTY))
    {
      orxDISPLAY_SHADER *pstShader;

      /* Creates a new shader */
      pstShader = (orxDISPLAY_SHADER *)orxBank_Allocate(sstDisplay.pstShaderBank);

      /* Successful? */
      if(pstShader != orxNULL)
      {
        orxSHADER_PARAM  *pstParam;
        orxCHAR          *pc;
        orxS32            s32Free, s32Offset;

        /* Inits shader code buffer */
        sstDisplay.acShaderCodeBuffer[0]  = orxCHAR_NULL;
        pc                                = sstDisplay.acShaderCodeBuffer;
        s32Free                           = orxDISPLAY_KU32_SHADER_BUFFER_SIZE;

        /* Has parameters? */
        if(_pstParamList != orxNULL)
        {
          orxCHAR *pcReplace;

          /* Adds wrapping code */
          s32Offset = orxString_NPrint(pc, s32Free, "precision mediump float;\nvarying vec2 _gl_TexCoord0_;\n");
          pc       += s32Offset;
          s32Free  -= s32Offset;

          /* For all parameters */
          for(pstParam = (orxSHADER_PARAM *)orxLinkList_GetFirst(_pstParamList);
              pstParam != orxNULL;
              pstParam = (orxSHADER_PARAM *)orxLinkList_GetNext(&(pstParam->stNode)))
          {
            /* Depending on type */
            switch(pstParam->eType)
            {
              case orxSHADER_PARAM_TYPE_FLOAT:
              case orxSHADER_PARAM_TYPE_TIME:
              {
                /* Adds its literal value */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform float %s[%ld];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform float %s;\n", pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_TEXTURE:
              {
                /* Adds its literal value and automated coordinates */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform sampler2D %s[%ld];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP"[%ld];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT"[%ld];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM"[%ld];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT"[%ld];\n", pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform sampler2D %s;\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT";\n", pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_VECTOR:
              {
                /* Adds its literal value */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform vec3 %s[%ld];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform vec3 %s;\n", pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              default:
              {
                break;
              }
            }
          }

          /* Adds code */
          s32Offset = orxString_NPrint(pc, s32Free, "%s\n", _zCode);
          pc       += s32Offset;
          s32Free  -= s32Offset;

          /* For all gl_TexCoord[0] */
          for(pcReplace = (orxCHAR *)orxString_SearchString(sstDisplay.acShaderCodeBuffer, "gl_TexCoord[0]");
              pcReplace != orxNULL;
              pcReplace = (orxCHAR *)orxString_SearchString(pcReplace + 14 * sizeof(orxCHAR), "gl_TexCoord[0]"))
          {
            /* Replaces it */
            orxMemory_Copy(pcReplace, "_gl_TexCoord0_", 14 * sizeof(orxCHAR));
          }
        }
        else
        {
          /* Adds code */
          orxString_NPrint(pc, s32Free, "%s\n", _zCode);
        }

        /* Inits shader */
        pstShader->uiProgram              = (GLuint)orxHANDLE_UNDEFINED;
        pstShader->iTextureCounter        = 0;
        pstShader->s32ParamCounter        = 0;
        pstShader->bActive                = orxFALSE;
        pstShader->bInitialized           = orxFALSE;
        pstShader->zCode                  = orxString_Duplicate(sstDisplay.acShaderCodeBuffer);
        pstShader->astTextureInfoList     = (orxDISPLAY_TEXTURE_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO), orxMEMORY_TYPE_MAIN);
        pstShader->astParamInfoList       = (orxDISPLAY_PARAM_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_PARAM_INFO), orxMEMORY_TYPE_MAIN);
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));
        orxMemory_Zero(pstShader->astParamInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_PARAM_INFO));

        /* Compiles it */
        if(orxDisplay_Android_CompileShader(pstShader) != orxSTATUS_FAILURE)
        {
          /* Updates result */
          hResult = (orxHANDLE)pstShader;
        }
        else
        {
          /* Deletes code */
          orxString_Delete(pstShader->zCode);

          /* Deletes texture info list */
          orxMemory_Free(pstShader->astTextureInfoList);

          /* Deletes param info list */
          orxMemory_Free(pstShader->astParamInfoList);

          /* Frees shader */
          orxBank_Free(sstDisplay.pstShaderBank, pstShader);
        }
      }
    }
  }

  /* Done! */
  return hResult;
}

void orxFASTCALL orxDisplay_Android_DeleteShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER *pstShader;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Deletes its program */
  glDeleteProgram(pstShader->uiProgram);
  glASSERT();

  /* Deletes its code */
  orxString_Delete(pstShader->zCode);

  /* Deletes its texture info list */
  orxMemory_Free(pstShader->astTextureInfoList);

  /* Deletes its param info list */
  orxMemory_Free(pstShader->astParamInfoList);

  /* Frees it */
  orxBank_Free(sstDisplay.pstShaderBank, pstShader);

  return;
}

orxSTATUS orxFASTCALL orxDisplay_Android_StartShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Uses program */
  glUseProgram(pstShader->uiProgram);
  glASSERT();

  /* Updates projection matrix */
  glUniformMatrix4fv(pstShader->uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
  glASSERT();

  /* Updates its status */
  pstShader->bActive      = orxTRUE;
  pstShader->bInitialized = orxFALSE;

  /* Updates active shader counter */
  sstDisplay.s32ActiveShaderCounter++;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_StopShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_hShader != orxHANDLE_UNDEFINED);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has shader? */
  if(pstShader != orxNULL)
  {
    /* Wasn't initialized? */
    if(pstShader->bInitialized == orxFALSE)
    {
      /* Inits it */
      orxDisplay_Android_InitShader(pstShader);

      /* Defines the vertex list */
      sstDisplay.astVertexList[0].fX  =
      sstDisplay.astVertexList[1].fX  = sstDisplay.pstScreen->stClip.vTL.fX;
      sstDisplay.astVertexList[2].fX  =
      sstDisplay.astVertexList[3].fX  = sstDisplay.pstScreen->stClip.vBR.fX;
      sstDisplay.astVertexList[1].fY  =
      sstDisplay.astVertexList[3].fY  = sstDisplay.pstScreen->stClip.vTL.fY;
      sstDisplay.astVertexList[0].fY  =
      sstDisplay.astVertexList[2].fY  = sstDisplay.pstScreen->stClip.vBR.fY;

      /* Defines the texture coord list */
      sstDisplay.astVertexList[0].fU  =
      sstDisplay.astVertexList[1].fU  = (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vTL.fX);
      sstDisplay.astVertexList[2].fU  =
      sstDisplay.astVertexList[3].fU  = (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vBR.fX);
      sstDisplay.astVertexList[1].fV  =
      sstDisplay.astVertexList[3].fV  = (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vTL.fY);
      sstDisplay.astVertexList[0].fV  =
      sstDisplay.astVertexList[2].fV  = (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vBR.fY);

      /* Fills the color list */
      sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
      sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
      sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
      sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = sstDisplay.pstScreen->stColor;

      /* Updates counter */
      sstDisplay.s32BufferIndex = 4;

      /* Draws arrays */
      orxDisplay_Android_DrawArrays();
    }

    /* Clears texture counter */
    pstShader->iTextureCounter = 0;

    /* Clears texture info list */
    orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

    /* Updates its status */
    pstShader->bActive = orxFALSE;

    /* Updates active shader counter */
    sstDisplay.s32ActiveShaderCounter--;
  }

  /* Uses default program */
  glUseProgram(sstDisplay.pstDefaultShader->uiProgram);
  glASSERT();

  /* Updates first texture unit */
  glUniform1i(sstDisplay.pstDefaultShader->uiTextureLocation, 0);
  glASSERT();

  /* Updates projection matrix */
  glUniformMatrix4fv(sstDisplay.pstDefaultShader->uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
  glASSERT();

  /* Selects it */
  glActiveTexture(GL_TEXTURE0);
  glASSERT();

  /* Done! */
  return eResult;
}


orxS32 orxFASTCALL orxDisplay_Android_GetParameterID(const orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture)
{
  orxDISPLAY_SHADER  *pstShader;
  orxS32              s32Result;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Is a texture? */
  if(_bIsTexture != orxFALSE)
  {
    orxDISPLAY_PARAM_INFO  *pstInfo;
    orxCHAR                 acBuffer[256];

    /* Checks */
    orxASSERT(pstShader->s32ParamCounter < sstDisplay.iTextureUnitNumber);

    /* Gets corresponding param info */
    pstInfo = &pstShader->astParamInfoList[pstShader->s32ParamCounter];

    /* Updates result */
    s32Result = pstShader->s32ParamCounter++;

    /* Array? */
    if(_s32Index >= 0)
    {
      /* Prints its name */
      orxString_NPrint(acBuffer, 256, "%s[%ld]", _zParam, _s32Index);

      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, acBuffer);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, (const char *)_zParam);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();
    }
  }
  else
  {
    /* Array? */
    if(_s32Index >= 0)
    {
      orxCHAR acBuffer[256];

      /* Prints its name */
      orxString_NPrint(acBuffer, 256, "%s[%ld]", _zParam, _s32Index);

      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocation(pstShader->uiProgram, acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocation(pstShader->uiProgram, (const char *)_zParam);
      glASSERT();
    }
  }

  /* Done! */
  return s32Result;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has free texture unit left? */
  if(pstShader->iTextureCounter < sstDisplay.iTextureUnitNumber)
  {
    /* Valid? */
    if(_s32ID >= 0)
    {
      /* No bitmap? */
      if(_pstValue == orxNULL)
      {
        /* Uses screen bitmap */
        _pstValue = sstDisplay.pstScreen;
      }

      /* Updates texture info */
      pstShader->astTextureInfoList[pstShader->iTextureCounter].iLocation = pstShader->astParamInfoList[_s32ID].iLocation;
      pstShader->astTextureInfoList[pstShader->iTextureCounter].pstBitmap = _pstValue;

      /* Updates corner values */
      glUniform1f(pstShader->astParamInfoList[_s32ID].iLocationTop, (GLfloat)(orxFLOAT_1 - (_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY)));
      glASSERT();
      glUniform1f(pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
      glASSERT();
      glUniform1f(pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)(orxFLOAT_1 - (_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY)));
      glASSERT();
      glUniform1f(pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));
      glASSERT();

			/* Updates texture counter */
			pstShader->iTextureCounter++;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't find texture parameter (ID <%ld>) for fragment shader.", _s32ID);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter (ID <%ld>) for fragment shader: all the texture units are used.", _s32ID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Valid? */
  if(_s32ID >= 0)
  {
    /* Updates its value */
    glUniform1f((GLint)_s32ID, (GLfloat)_fValue);
    glASSERT();

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

orxSTATUS orxFASTCALL orxDisplay_Android_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_pvValue != orxNULL);

  /* Valid? */
  if(_s32ID >= 0)
  {
    /* Updates its value */
    glUniform3f((GLint)_s32ID, (GLfloat)_pvValue->fX, (GLfloat)_pvValue->fY, (GLfloat)_pvValue->fZ);
    glASSERT();

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

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/
orxPLUGIN_USER_CORE_FUNCTION_START(DISPLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetDestinationBitmap, DISPLAY, SET_DESTINATION_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetScreenBitmap, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawLine, DISPLAY, DRAW_LINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawPolyline, DISPLAY, DRAW_POLYLINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawPolygon, DISPLAY, DRAW_POLYGON);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawCircle, DISPLAY, DRAW_CIRCLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawOBox, DISPLAY, DRAW_OBOX);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawMesh, DISPLAY, DRAW_MESH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_HasShaderSupport, DISPLAY, HAS_SHADER_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetParameterID, DISPLAY, GET_PARAMETER_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
