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
 * @file orxDisplay.m
 * @date 23/01/2010
 * @author iarwain@orx-project.org
 *
 * iPhone display plugin implementation
 *
 */


#include "orxPluginAPI.h"
#import <QuartzCore/QuartzCore.h>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000  /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001  /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHADER      0x00000002  /**< Shader support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_LOCATION    0x00000004  /**< Has location support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER	0x00000008  /**< Has depth buffer support flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        128
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        16

#define orxDISPLAY_KU32_VERTEX_BUFFER_SIZE      (4 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_INDEX_BUFFER_SIZE       (6 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      65536

#define orxDISPLAY_KF_BORDER_FIX                0.1f


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

#define glASSERT()
//do                                                                      \
//{                                                                       \
//  glGetError();                                                         \
//} while(orxFALSE)

#endif /* __orxDEBUG__ */


typedef enum __orxDISPLAY_ATTRIBUTE_LOCATION_t
{
  orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX = 0,
  orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD,
  orxDISPLAY_ATTRIBUTE_LOCATION_COLOR,

  orxDISPLAY_ATTRIBUTE_LOCATION_NUMBER,

  orxDISPLAY_ATTRIBUTE_LOCATION_NONE = orxENUM_NONE

} orxDISPLAY_ATTRIBUTE_LOCATION;


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal matrix structure
 */
typedef struct __orxDISPLAY_MATRIX_t
{
  orxVECTOR vX;
  orxVECTOR vY;

} orxDISPLAY_MATRIX;

/** Internal vertex structure
 */
typedef struct __orxDISPLAY_VERTEX_t
{
  orxFLOAT  fX, fY, fU, fV;
  orxRGBA   stRGBA;
} orxDISPLAY_VERTEX;

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
  GLuint                    uiTexture;
  orxBOOL                   bSmoothing;
  orxFLOAT                  fWidth, fHeight;
  orxAABOX                  stClip;
  orxU32                    u32RealWidth, u32RealHeight;
  orxFLOAT                  fRecRealWidth, fRecRealHeight;
  orxRGBA                   stColor;
};

/** Internal texture info structure
 */
typedef struct __orxDISPLAY_TEXTURE_INFO_t
{
  GLint                     iLocation;
  const orxBITMAP          *pstBitmap;

} orxDISPLAY_TEXTURE_INFO;

/** Internal shader structure
 */
typedef struct __orxDISPLAY_SHADER_t
{
  GLuint                    uiProgram;
  GLint                     iTextureCounter;
  orxBOOL                   bActive;
  orxBOOL                   bInitialized;
  orxSTRING                 zCode;
  orxDISPLAY_TEXTURE_INFO  *astTextureInfoList;

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
  GLuint                    uiColorLocation;
  GLuint                    uiTextureLocation;
  GLuint                    uiProjectionMatrixLocation;
  GLint                     iTextureUnitNumber;
  orxS32                    s32ActiveShaderCounter;
  orxS32                    s32BufferIndex;
  orxView                  *poView;
  orxU32                    u32Flags;
  orxDISPLAY_PROJ_MATRIX    mProjectionMatrix;
  orxDISPLAY_VERTEX         astVertexList[orxDISPLAY_KU32_VERTEX_BUFFER_SIZE];
  GLushort                  au16IndexList[orxDISPLAY_KU32_INDEX_BUFFER_SIZE];
  orxCHAR                   acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE];

} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDISPLAY_STATIC sstDisplay;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** orxView class
 */
static orxView *spoInstance;

@interface orxView ()

+ (orxView *) GetInstance;

- (BOOL) CreateThreadContext;
- (BOOL) CreateBuffers;
- (BOOL) CreateRenderTarget:(const orxBITMAP *)_pstBitmap;

@end

@implementation orxView

@synthesize poMainContext;
@synthesize poThreadContext;
@synthesize bShaderSupport;

+ (Class) layerClass
{
  return [CAEAGLLayer class];
}

+ (orxView *) GetInstance
{
  return spoInstance;
}

- (id) initWithFrame:(CGRect)_stFrame
{
  id oResult = nil;

  /* Inits parent */
  if((self = [super initWithFrame:_stFrame]) != nil)
  {
    CAEAGLLayer *poLayer;

    /* Enables multi-touch */
    self.multipleTouchEnabled = YES;

    /* Gets the layer */
    poLayer = (CAEAGLLayer *)self.layer;

    /* Inits it */
    poLayer.opaque = YES;
    poLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

    /* Creates main OpenGL ES 2.0 context */
    poMainContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

    /* Success? */
    if(poMainContext != nil)
    {
      /* Updates status */
      bShaderSupport = YES;
    }
    else
    {
      /* Creates main OpenGL ES 1.1 context */
      poMainContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

      /* Updates status */
      bShaderSupport = NO;
    }

    /* Success? */
    if((poMainContext != nil) && ([EAGLContext setCurrentContext:poMainContext] != 0))
    {
      /* Shader support? */
      if(bShaderSupport != NO)
      {
        /* Sets vextex attribute arrays */
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX);
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD);
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR);
        glASSERT();
      }
      else
      {
        /* Inits it */
        glEnable(GL_TEXTURE_2D);
        glASSERT();
        glDisable(GL_LIGHTING);
        glASSERT();
        glDisable(GL_FOG);
        glASSERT();
        glEnableClientState(GL_VERTEX_ARRAY);
        glASSERT();
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glASSERT();
        glEnableClientState(GL_COLOR_ARRAY);
        glASSERT();

        /* Selects arrays */
        glVertexPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
        glASSERT();
        glTexCoordPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
        glASSERT();
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
        glASSERT();
      }

      /* Common init */
      glDisable(GL_CULL_FACE);
      glASSERT();
      glDisable(GL_ALPHA_TEST);
      glASSERT();
      glDisable(GL_DEPTH_TEST);
      glASSERT();
      glDisable(GL_STENCIL_TEST);
      glASSERT();

      /* Stores self */
      spoInstance = self;

      /* Updates result */
      oResult = self;
    }
    else
    {
      /* Releases self */
      [self release];
    }
  }

  /* Done! */
  return oResult;
}

- (BOOL) CreateThreadContext
{
  EAGLSharegroup *poGroup;
  BOOL            bResult = NO;

  /* Gets share group */
  poGroup = poMainContext.sharegroup;

  /* Valid? */
  if(poGroup != nil)
  {
    /* Has shader support? */
    if(bShaderSupport != NO)
    {
      /* Creates thread context */
      poThreadContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:poGroup];
    }
    else
    {
      /* Creates thread context */
      poThreadContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1 sharegroup:poGroup];
    }

    /* Valid? */
    if((poThreadContext != nil) && ([EAGLContext setCurrentContext:poThreadContext] != NO))
    {
      /* Shader support? */
      if(bShaderSupport != NO)
      {
        /* Sets vextex attribute arrays */
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX);
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD);
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR);
        glASSERT();
      }
      else
      {
        /* Inits it */
        glEnable(GL_TEXTURE_2D);
        glASSERT();
        glDisable(GL_LIGHTING);
        glASSERT();
        glDisable(GL_FOG);
        glASSERT();
        glEnableClientState(GL_VERTEX_ARRAY);
        glASSERT();
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glASSERT();
        glEnableClientState(GL_COLOR_ARRAY);
        glASSERT();

        /* Selects arrays */
        glVertexPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
        glASSERT();
        glTexCoordPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
        glASSERT();
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
        glASSERT();
      }

      /* Common init */
      glDisable(GL_CULL_FACE);
      glASSERT();
      glDisable(GL_DEPTH_TEST);
      glASSERT();
      glDisable(GL_STENCIL_TEST);
      glASSERT();

      /* Creates frame & render buffers */
      [self CreateBuffers];

      /* Updates result */
      bResult = YES;
    }
  }

  /* Done! */
  return bResult;
}

- (BOOL) CreateBuffers
{
  BOOL bResult = YES;

  /* Generates frame buffer */
  glGenFramebuffersOES(1, &uiFrameBuffer);
  glASSERT();

  /* Binds it */
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiFrameBuffer);
  glASSERT();

  /* Generates render buffer */
  glGenRenderbuffersOES(1, &uiRenderBuffer);
  glASSERT();

  /* Binds it */
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);
  glASSERT();

  /* Links render buffer to layer */
  bResult = [poThreadContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer];

  /* Uses depth buffer? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER))
  {
    GLint iWidth, iHeight;

    /* Gets render buffer's size */
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &iWidth);
    glASSERT();
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &iHeight);
    glASSERT();

    /* Creates depth buffer */
    glGenRenderbuffersOES(1, &uiDepthBuffer);
    glASSERT();

    /* Binds it */
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiDepthBuffer);

    /* Sets its size */
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, iWidth, iHeight);
  }

  /* Done! */
  return bResult;
}

- (BOOL) CreateRenderTarget:(const orxBITMAP *)_pstBitmap
{
  BOOL bResult = NO;

  /* Screen? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    /* Unbinds texture from frame buffer */
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, 0, 0);
    glASSERT();

    /* Binds render buffer to frame buffer */
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, uiRenderBuffer);
    glASSERT();

    /* Uses depth buffer? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER))
    {
      /* Binds it to frame buffer */
      glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, uiDepthBuffer);
    }
    glFlush();
    glASSERT();

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
  }
  else
  {
    /* Unbinds render buffer from frame buffer */
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, 0);
    glASSERT();

    /* Uses depth buffer? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER))
    {
      /* Unbinds depth buffer from frame buffer */
      glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, 0);
      glASSERT();
    }

    /* Binds corresponding texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Links it to frame buffer */
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, _pstBitmap->uiTexture, 0);
    glASSERT();
    glFlush();
    glASSERT();

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
  }

  /* Done! */
  return bResult;
}

- (void) touchesBegan:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  orxIPHONE_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxIPHONE_EVENT_PAYLOAD));
  stPayload.poUIEvent   = _poEvent;
  stPayload.poTouchList = _poTouchList;

  /* Sends it */
  orxEVENT_SEND(orxEVENT_TYPE_IPHONE, orxIPHONE_EVENT_TOUCH_BEGIN, self, orxNULL, &stPayload);

  /* Done! */
  return;
}

- (void) touchesMoved:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  orxIPHONE_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxIPHONE_EVENT_PAYLOAD));
  stPayload.poUIEvent   = _poEvent;
  stPayload.poTouchList = _poTouchList;

  /* Sends it */
  orxEVENT_SEND(orxEVENT_TYPE_IPHONE, orxIPHONE_EVENT_TOUCH_MOVE, self, orxNULL, &stPayload);

  /* Done! */
  return;
}

- (void) touchesEnded:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  orxIPHONE_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxIPHONE_EVENT_PAYLOAD));
  stPayload.poUIEvent   = _poEvent;
  stPayload.poTouchList = _poTouchList;

  /* Sends it */
  orxEVENT_SEND(orxEVENT_TYPE_IPHONE, orxIPHONE_EVENT_TOUCH_END, self, orxNULL, &stPayload);

  /* Done! */
  return;
}

- (void) touchesCancelled:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  orxIPHONE_EVENT_PAYLOAD stPayload;

  /* Inits event's payload */
  orxMemory_Zero(&stPayload, sizeof(orxIPHONE_EVENT_PAYLOAD));
  stPayload.poUIEvent   = _poEvent;
  stPayload.poTouchList = _poTouchList;

  /* Sends it */
  orxEVENT_SEND(orxEVENT_TYPE_IPHONE, orxIPHONE_EVENT_TOUCH_CANCEL, self, orxNULL, &stPayload);

  /* Done! */
  return;
}

#ifdef __IPHONE_3_0

- (void) motionEnded:(UIEventSubtype)_eMotion withEvent:(UIEvent *)_poEvent
{
  /* Shake? */
  if(_eMotion == UIEventSubtypeMotionShake)
  {
    orxIPHONE_EVENT_PAYLOAD stPayload;

    /* Inits event's payload */
    orxMemory_Zero(&stPayload, sizeof(orxIPHONE_EVENT_PAYLOAD));
    stPayload.poUIEvent = _poEvent;
    stPayload.eMotion   = _eMotion;

    /* Sends it */
    orxEVENT_SEND(orxEVENT_TYPE_IPHONE, orxIPHONE_EVENT_MOTION_SHAKE, self, orxNULL, &stPayload);
  }

  /* Done! */
  return;
}

#endif

@end

static orxINLINE orxDISPLAY_MATRIX *orxDisplay_iPhone_InitMatrix(orxDISPLAY_MATRIX *_pmMatrix, orxFLOAT _fPosX, orxFLOAT _fPosY, orxFLOAT _fScaleX, orxFLOAT _fScaleY, orxFLOAT _fRotation, orxFLOAT _fPivotX, orxFLOAT _fPivotY)
{
  orxFLOAT fCos, fSin, fSCosX, fSCosY, fSSinX, fSSinY, fTX, fTY;

  /* Has rotation? */
  if(_fRotation != orxFLOAT_0)
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
  fSCosX  = _fScaleX * fCos;
  fSCosY  = _fScaleY * fCos;
  fSSinX  = _fScaleX * fSin;
  fSSinY  = _fScaleY * fSin;
  fTX     = _fPosX - (_fPivotX * fSCosX) + (_fPivotY * fSSinY);
  fTY     = _fPosY - (_fPivotX * fSSinX) - (_fPivotY * fSCosY);

  /* Updates matrix */
  orxVector_Set(&(_pmMatrix->vX), fSCosX, -fSSinY, fTX);
  orxVector_Set(&(_pmMatrix->vY), fSSinX, fSCosY, fTY);

  /* Done! */
  return _pmMatrix;
}

static orxDISPLAY_PROJ_MATRIX *orxDisplay_iPhone_OrthoProjMatrix(orxDISPLAY_PROJ_MATRIX *_pmResult, orxFLOAT _fLeft, orxFLOAT _fRight, orxFLOAT _fBottom, orxFLOAT _fTop, orxFLOAT _fNear, orxFLOAT _fFar)
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
  return _pmResult;
}

static orxSTATUS orxFASTCALL orxDisplay_iPhone_CompileShader(orxDISPLAY_SHADER *_pstShader)
{
  static const orxSTRING szVertexShaderSource =
  "attribute vec2 __vPosition__;"
  "uniform mat4 __mProjection__;"
  "attribute mediump vec2 __vTexCoord__;"
  "varying mediump vec2 ___TexCoord___;"
  "attribute mediump vec4 __vColor__;"
  "varying mediump vec4 ___Color___;"
  "void main()"
  "{"
  "  mediump float fCoef = 1.0 / 255.0;"
  "  gl_Position      = __mProjection__ * vec4(__vPosition__.xy, 0.0, 1.0);"
  "  ___TexCoord___   = __vTexCoord__;"
  "  ___Color___      = fCoef * __vColor__;"
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
  glShaderSource(uiVertexShader, 1, (const GLchar **)&szVertexShaderSource, NULL);
  glASSERT();
  glShaderSource(uiFragmentShader, 1, (const GLchar **)&(_pstShader->zCode), NULL);
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
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, "__vPosition__");
      glASSERT();
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, "__vTexCoord__");
      glASSERT();
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, "__vColor__");
      glASSERT();

      /* Links program */
      glLinkProgram(uiProgram);
      glASSERT();

      /* Doesn't have default location? */
      if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_LOCATION))
      {
        /* Gets texture location */
        sstDisplay.uiTextureLocation = glGetUniformLocation(uiProgram, "__Texture__");
        glASSERT();

        /* Gets projection matrix location */
        sstDisplay.uiProjectionMatrixLocation = glGetUniformLocation(uiProgram, "__mProjection__");
        glASSERT();

        /* Updates status */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_LOCATION, orxDISPLAY_KU32_STATIC_FLAG_NONE);
      }

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
        glGetProgramInfoLog(uiProgram, 4095 * sizeof(orxCHAR), NULL, (GLchar *)acBuffer);
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
      glGetShaderInfoLog(uiFragmentShader, 4095 * sizeof(orxCHAR), NULL, (GLchar *)acBuffer);
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
    glGetShaderInfoLog(uiVertexShader, 4095 * sizeof(orxCHAR), NULL, (GLchar *)acBuffer);
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

static void orxFASTCALL orxDisplay_iPhone_InitShader(orxDISPLAY_SHADER *_pstShader)
{
  /* Uses shader's program */
  glUseProgram(_pstShader->uiProgram);
  glASSERT();

  /* Has custom textures? */
  if(_pstShader->iTextureCounter > 0)
  {
    GLint i;

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

      /* Screen? */
      if(_pstShader->astTextureInfoList[i].pstBitmap == sstDisplay.pstScreen)
      {
        /* Copies screen content */
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, orxF2U(sstDisplay.pstScreen->fHeight) - sstDisplay.pstScreen->u32RealHeight, orxF2U(sstDisplay.pstScreen->fWidth), sstDisplay.pstScreen->u32RealHeight);
        glASSERT();
      }
    }
  }

  /* Updates its status */
  _pstShader->bInitialized = orxTRUE;

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_iPhone_DrawArrays()
{
  /* Has data? */
  if(sstDisplay.s32BufferIndex > 0)
  {
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
          orxDisplay_iPhone_InitShader(pstShader);

          /* Draws arrays */
          glDrawElements(GL_TRIANGLE_STRIP, sstDisplay.s32BufferIndex + (sstDisplay.s32BufferIndex >> 1), GL_UNSIGNED_SHORT, sstDisplay.au16IndexList);
          glASSERT();
        }
      }
    }
    else
    {
      /* Draws arrays */
      glDrawElements(GL_TRIANGLE_STRIP, sstDisplay.s32BufferIndex + (sstDisplay.s32BufferIndex >> 1), GL_UNSIGNED_SHORT, sstDisplay.au16IndexList);
      glASSERT();
    }

    /* Clears buffer index */
    sstDisplay.s32BufferIndex = 0;
  }

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_iPhone_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.pstLastBitmap)
  {
    /* Draws remaining items */
    orxDisplay_iPhone_DrawArrays();

    /* No active shader? */
    if(sstDisplay.s32ActiveShaderCounter == 0)
    {
      /* Binds source's texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

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
    orxDisplay_iPhone_DrawArrays();

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
    orxDisplay_iPhone_DrawArrays();

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

static orxINLINE void orxDisplay_iPhone_DrawBitmap(const orxBITMAP *_pstBitmap, const orxDISPLAY_MATRIX *_pmTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  GLfloat fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_iPhone_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = (GLfloat)(_pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat)(_pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY);

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
  {
    /* Draw arrays */
    orxDisplay_iPhone_DrawArrays();
  }

  /* Fills the vertex list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX      = (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY      = (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX  = _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY  = _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX  = (_pmTransform->vX.fX * fWidth) + (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY  = (_pmTransform->vY.fX * fWidth) + (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX  = (_pmTransform->vX.fX * fWidth) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY  = (_pmTransform->vY.fX * fWidth) + _pmTransform->vY.fZ;

  /* Fills the texture coord list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU  = (GLfloat)(_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU  = (GLfloat)(_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));

  /* Fills the color list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _pstBitmap->stColor;

  /* Updates index */
  sstDisplay.s32BufferIndex += 4;

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_iPhone_GetScreen()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstDisplay.pstScreen;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  const orxCHAR    *pc;
  orxU32            u32CharacterCodePoint;
  GLfloat           fX, fY, fWidth, fHeight;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_iPhone_InitMatrix(&mTransform, orxMath_Floor(_pstTransform->fDstX), orxMath_Floor(_pstTransform->fDstY), _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* Gets character's size */
  fWidth  = _pstMap->vCharacterSize.fX;
  fHeight = _pstMap->vCharacterSize.fY;

  /* Prepares font for drawing */
  orxDisplay_iPhone_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode);

  /* For all characters */
  for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_zString, &pc), fX = 0.0f, fY = 0.0f;
      u32CharacterCodePoint != orxCHAR_NULL;
      u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc))
  {
    /* Depending on character */
    switch(u32CharacterCodePoint)
    {
      case orxCHAR_CR:
      {
        /* Half EOL? */
        if(*pc == orxCHAR_LF)
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
        pstGlyph = (orxCHARACTER_GLYPH *)orxHashTable_Get(_pstMap->pstCharacterTable, u32CharacterCodePoint);

        /* Valid? */
        if(pstGlyph != orxNULL)
        {
          /* End of buffer? */
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
          {
            /* Draw arrays */
            orxDisplay_iPhone_DrawArrays();
          }

          /* Outputs vertices and texture coordinates */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX      = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY      = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX  = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY  = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU  = (GLfloat)(_pstFont->fRecRealWidth * (pstGlyph->fX + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU  = (GLfloat)(_pstFont->fRecRealWidth * (pstGlyph->fX + fWidth - orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(orxFLOAT_1 - _pstFont->fRecRealHeight * (pstGlyph->fY + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(orxFLOAT_1 - _pstFont->fRecRealHeight * (pstGlyph->fY + fHeight - orxDISPLAY_KF_BORDER_FIX));

          /* Fills the color list */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _pstFont->stColor;

          /* Updates counter */
          sstDisplay.s32BufferIndex += 4;
        }
      }

      /* Updates X position */
      fX += fWidth;
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_iPhone_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Deletes its texture */
    glDeleteTextures(1, &(_pstBitmap->uiTexture));
    glASSERT();

    /* Deletes it */
    orxBank_Free(sstDisplay.pstBitmapBank, _pstBitmap);
  }

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_iPhone_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  orxBITMAP *pstBitmap;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Allocates bitmap */
  pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

  /* Valid? */
  if(pstBitmap != orxNULL)
  {
    GLint iTexture;

    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Inits it */
    pstBitmap->bSmoothing     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
    pstBitmap->fWidth         = orxU2F(_u32Width);
    pstBitmap->fHeight        = orxU2F(_u32Height);
    pstBitmap->u32RealWidth   = orxMath_GetNextPowerOfTwo(_u32Width);
    pstBitmap->u32RealHeight  = orxMath_GetNextPowerOfTwo(_u32Height);
    pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
    pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
    pstBitmap->stColor        = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
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

orxSTATUS orxFASTCALL orxDisplay_iPhone_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Is not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    GLint     iTexture;
    orxRGBA  *astBuffer, *pstPixel;

    /* Allocates buffer */
    astBuffer = (orxRGBA *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_MAIN);

    /* Checks */
    orxASSERT(astBuffer != orxNULL);

    /* For all pixels */
    for(pstPixel = astBuffer; pstPixel < astBuffer + (_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight); pstPixel++)
    {
      /* Sets its value */
      *pstPixel = _stColor;
    }

    /* Backups current texture */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
    glASSERT();

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates texture */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, astBuffer);
    glASSERT();

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, iTexture);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(astBuffer);
  }
  else
  {
    /* Clears the color buffer with given color */
    glClearColor((1.0f / 255.f) * orxU2F(orxRGBA_R(_stColor)), (1.0f / 255.f) * orxU2F(orxRGBA_G(_stColor)), (1.0f / 255.f) * orxU2F(orxRGBA_B(_stColor)), (1.0f / 255.f) * orxU2F(orxRGBA_A(_stColor)));
    glASSERT();
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_iPhone_DrawArrays();

  /* Swaps */
  [sstDisplay.poView.poThreadContext presentRenderbuffer:GL_RENDERBUFFER_OES];

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxU32    u32Width, u32Height;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Gets bitmap's size */
  u32Width  = orxF2U(_pstBitmap->fWidth);
  u32Height = orxF2U(_pstBitmap->fHeight);

  /* Valid? */
  if((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * sizeof(orxRGBA)))
  {
    GLint   iTexture;
    orxU8  *pu8ImageBuffer;
    orxU32  i, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset;

    /* Allocates buffer */
    pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_VIDEO);

    /* Gets line sizes */
    u32LineSize     = orxF2U(_pstBitmap->fWidth) * sizeof(orxRGBA);
    u32RealLineSize = _pstBitmap->u32RealWidth * sizeof(orxRGBA);

    /* Clears padding */
    orxMemory_Zero(pu8ImageBuffer, u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)));

    /* For all lines */
    for(i = 0, u32SrcOffset = 0, u32DstOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - 1);
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
    if(_pstBitmap == sstDisplay.pstScreen)
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

orxSTATUS orxFASTCALL orxDisplay_iPhone_GetBitmapData(orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
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
    /* Updates result */
    eResult = ([sstDisplay.poView CreateRenderTarget:_pstBitmap] != NO) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
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

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Stores it */
    _pstBitmap->stColor = _stColor;
  }

  /* Done! */
  return eResult;
}

orxRGBA orxFASTCALL orxDisplay_iPhone_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Updates result */
    stResult = _pstBitmap->stColor;
  }

  /* Done! */
  return stResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetDestinationBitmap(orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Different destination bitmap? */
  if(_pstBitmap != sstDisplay.pstDestinationBitmap)
  {
    /* Draws remaining items */
    orxDisplay_iPhone_DrawArrays();

    /* Stores it */
    sstDisplay.pstDestinationBitmap = _pstBitmap;

    /* Sets OpenGL context */
    [EAGLContext setCurrentContext:sstDisplay.poView.poThreadContext];

    /* Recreates render target */
    [sstDisplay.poView CreateRenderTarget:_pstBitmap];

    /* Inits viewport */
    glViewport(0, 0, sstDisplay.pstDestinationBitmap->fWidth, sstDisplay.pstDestinationBitmap->fHeight);
    glASSERT();

    /* Shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Inits projection matrix */
      orxDisplay_iPhone_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.pstDestinationBitmap->fWidth, sstDisplay.pstDestinationBitmap->fHeight, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1);

      /* Passes it to shader */
      glUniformMatrix4fv(sstDisplay.uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
    }
    else
    {
      /* Inits matrices */
      glMatrixMode(GL_PROJECTION);
      glASSERT();
      glLoadIdentity();
      glASSERT();
      glOrthof(0.0f, sstDisplay.pstDestinationBitmap->fWidth, sstDisplay.pstDestinationBitmap->fHeight, 0.0f, -1.0f, 1.0f);
      glASSERT();
      glMatrixMode(GL_MODELVIEW);
      glASSERT();
      glLoadIdentity();
      glASSERT();
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_BlitBitmap(const orxBITMAP *_pstSrc, orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));

  /* Inits matrix */
  orxVector_Set(&(mTransform.vX), orxFLOAT_1, orxFLOAT_0, _fPosX);
  orxVector_Set(&(mTransform.vY), orxFLOAT_0, orxFLOAT_1, _fPosY);

  /* Draws it */
  orxDisplay_iPhone_DrawBitmap(_pstSrc, &mTransform, _eSmoothing, _eBlendMode);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_iPhone_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* No repeat? */
  if((_pstTransform->fRepeatX == orxFLOAT_1) && (_pstTransform->fRepeatY == orxFLOAT_1))
  {
    /* Draws it */
    orxDisplay_iPhone_DrawBitmap(_pstSrc, &mTransform, _eSmoothing, _eBlendMode);
  }
  else
  {
    orxFLOAT  i, j, fRecRepeatX;
    GLfloat   fX, fY, fWidth, fHeight, fTop, fBottom, fLeft, fRight;

    /* Prepares bitmap for drawing */
    orxDisplay_iPhone_PrepareBitmap(_pstSrc, _eSmoothing, _eBlendMode);

    /* Inits bitmap height */
    fHeight = (GLfloat)((_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY) / _pstTransform->fRepeatY);

    /* Inits texture coords */
    fLeft = _pstSrc->fRecRealWidth * _pstSrc->stClip.vTL.fX;
    fTop  = orxFLOAT_1 - (_pstSrc->fRecRealHeight * _pstSrc->stClip.vTL.fY);

    /* For all lines */
    for(fY = 0.0f, i = _pstTransform->fRepeatY, fRecRepeatX = orxFLOAT_1 / _pstTransform->fRepeatX; i > orxFLOAT_0; i -= orxFLOAT_1, fY += fHeight)
    {
      /* Partial line? */
      if(i < orxFLOAT_1)
      {
        /* Updates height */
        fHeight *= (GLfloat)i;

        /* Resets texture coords */
        fRight  = (GLfloat)(_pstSrc->fRecRealWidth * _pstSrc->stClip.vBR.fX);
        fBottom = (GLfloat)(orxFLOAT_1 - (_pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + (i * (_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY)))));
      }
      else
      {
        /* Resets texture coords */
        fRight  = (GLfloat)(_pstSrc->fRecRealWidth * _pstSrc->stClip.vBR.fX);
        fBottom = (GLfloat)(orxFLOAT_1 - (_pstSrc->fRecRealHeight * _pstSrc->stClip.vBR.fY));
      }

      /* Resets bitmap width */
      fWidth = (GLfloat)((_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX) * fRecRepeatX);

      /* For all columns */
      for(fX = 0.0f, j = _pstTransform->fRepeatX; j > orxFLOAT_0; j -= orxFLOAT_1, fX += fWidth)
      {
        /* Partial column? */
        if(j < orxFLOAT_1)
        {
          /* Updates width */
          fWidth *= (GLfloat)j;

          /* Updates texture right coord */
          fRight = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + (j * (_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX))));
        }

        /* End of buffer? */
        if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
        {
          /* Draws arrays */
          orxDisplay_iPhone_DrawArrays();
        }

        /* Outputs vertices and texture coordinates */
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX      = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY      = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX  = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY  = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU      =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU  = fLeft;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU  =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU  = fRight;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV  =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = fTop;
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = fBottom;

        /* Fills the color list */
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _pstSrc->stColor;

        /* Updates counter */
        sstDisplay.s32BufferIndex += 4;
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxBOOL         bPNG = orxFALSE;
  orxU32          u32Length;
  const orxCHAR  *zExtension;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Gets file name's length */
  u32Length = orxString_GetLength(_zFilename);

  /* Gets extension */
  zExtension = (u32Length > 3) ? _zFilename + u32Length - 3 : orxSTRING_EMPTY;

  /* DDS? */
  if(orxString_ICompare(zExtension, "png") == 0)
  {
    /* Updates status */
    bPNG = orxTRUE;
  }
  /* BMP? */
  else if(orxString_ICompare(zExtension, "jpg") == 0)
  {
    /* Updates status */
    bPNG = orxFALSE;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save bitmap to <%s>: only PNG and JPG formats are supported.", _zFilename);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    GLubyte  *au8Buffer;
    orxU8    *au8ImageBuffer;
    orxU32    u32BufferSize;

    /* Gets buffer size */
    u32BufferSize = _pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(GLubyte);

    /* Allocates both buffers */
    au8Buffer       = (GLubyte *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN);
    au8ImageBuffer  = (orxU8 *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if((au8Buffer != orxNULL) && (au8ImageBuffer != orxNULL))
    {
      /* Updates result */
      eResult = ([sstDisplay.poView CreateRenderTarget:_pstBitmap] != NO) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      glASSERT();

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        CGDataProviderRef oProvider;
        CGColorSpaceRef   oColorSpace;
        CGContextRef      oContext;
        CGImageRef        oImage;

        /* Reads OpenGL data */
        glReadPixels(0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, au8Buffer);
        glASSERT();

        /* Creates data provider */
        oProvider = CGDataProviderCreateWithData(NULL, au8Buffer, u32BufferSize, NULL);

        /* Creates a device color space */
        oColorSpace = CGColorSpaceCreateDeviceRGB();

        /* Gets image reference */
        oImage = CGImageCreate(_pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, 8, 32, 4 * _pstBitmap->u32RealWidth, oColorSpace, kCGBitmapByteOrderDefault, oProvider, nil, NO, kCGRenderingIntentDefault);

        /* Creates graphic context */
        oContext = CGBitmapContextCreate(au8ImageBuffer, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, 8, 4 * _pstBitmap->u32RealWidth, CGImageGetColorSpace(oImage), kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

        /* Valid? */
        if(oContext)
        {
          UIImage *poImage;

          /* Applies vertical flip for OpenGL/bitmap reordering */
          CGContextTranslateCTM(oContext, 0, _pstBitmap->u32RealHeight);
          CGContextScaleCTM(oContext, 1.0f, -1.0f);

          /* Draws image */
          CGContextDrawImage(oContext, CGRectMake(0.0f, 0.0f, _pstBitmap->fWidth, _pstBitmap->fHeight), oImage);

          /* Gets UIImage */
          poImage = [UIImage imageWithCGImage:CGBitmapContextCreateImage(oContext)];

          /* PNG? */
          if(bPNG != orxFALSE)
          {
            /* Updates result */
            eResult = [UIImagePNGRepresentation(poImage) writeToFile:[NSString stringWithCString:_zFilename encoding:NSASCIIStringEncoding] atomically:YES] != NO ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
          }
          else
          {
            /* Updates result */
            eResult = [UIImageJPEGRepresentation(poImage, 1.0f) writeToFile:[NSString stringWithCString:_zFilename encoding:NSASCIIStringEncoding] atomically:YES] != NO ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
          }

          /* Deletes context */
          CGContextRelease(oContext);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save screen to <%s>: couldn't grab screen data.", _zFilename);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }

        /* Deletes image */
        CGImageRelease(oImage);

        /* Deletes color space */
        CGColorSpaceRelease(oColorSpace);

        /* Deletes provider */
        CGDataProviderRelease(oProvider);
      }

      /* Clears destination bitmap for a rebind */
      sstDisplay.pstDestinationBitmap = orxNULL;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save screen to <%s>: couldn't allocate memory buffers.", _zFilename);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* Deletes buffers */
    if(au8Buffer != orxNULL)
    {
      orxMemory_Free(au8Buffer);
    }
    if(au8ImageBuffer != orxNULL)
    {
      orxMemory_Free(au8ImageBuffer);
    }
  }

  /* Done! */
  return eResult;
}

orxBITMAP *orxFASTCALL orxDisplay_iPhone_LoadBitmap(const orxSTRING _zFilename)
{
  CGImageRef  oImage;
  NSString   *poName;
  orxBITMAP  *pstBitmap = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets NSString */
  poName = [NSString stringWithCString:_zFilename encoding:NSASCIIStringEncoding];

  /* Gets image */
  oImage = [UIImage imageNamed:poName].CGImage;

  /* Valid? */
  if(oImage != nil)
  {
    GLuint    uiWidth, uiHeight, uiRealWidth, uiRealHeight;
    GLubyte  *au8ImageBuffer;

    /* Gets its size */
    uiWidth   = CGImageGetWidth(oImage);
    uiHeight  = CGImageGetHeight(oImage);

    /* Gets its real size */
    uiRealWidth   = orxMath_GetNextPowerOfTwo(uiWidth);
    uiRealHeight  = orxMath_GetNextPowerOfTwo(uiHeight);

    /* Allocates image buffer */
    au8ImageBuffer = (GLubyte *)orxMemory_Allocate(uiRealWidth * uiRealHeight * sizeof(GLuint), orxMEMORY_TYPE_VIDEO);

    /* Valid? */
    if(au8ImageBuffer != orxNULL)
    {
      /* Allocates bitmap */
      pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

      /* Valid? */
      if(pstBitmap != orxNULL)
      {
        CGColorSpaceRef oColorSpace;
        CGContextRef    oContext;
        GLint           iTexture;

        /* Creates a device color space */
        oColorSpace = CGColorSpaceCreateDeviceRGB();

        /* Creates graphic context */
        oContext = CGBitmapContextCreate(au8ImageBuffer, uiRealWidth, uiRealHeight, 8, 4 * uiRealWidth, oColorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

        /* Clears it */
        CGContextClearRect(oContext, CGRectMake(0, 0, uiRealWidth, uiRealHeight));

        /* Inits it */
        CGContextTranslateCTM(oContext, 0.0f, uiHeight);
        CGContextScaleCTM(oContext, 1.0f, -1.0f);

        /* Copies image data */
        CGContextDrawImage(oContext, CGRectMake(0, 0, uiWidth, uiHeight), oImage);

        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Inits it */
        pstBitmap->bSmoothing     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
        pstBitmap->fWidth         = orxU2F(uiWidth);
        pstBitmap->fHeight        = orxU2F(uiHeight);
        pstBitmap->u32RealWidth   = uiRealWidth;
        pstBitmap->u32RealHeight  = uiRealHeight;
        pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
        pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
        pstBitmap->stColor        = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, au8ImageBuffer);
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

        /* Deletes context */
        CGContextRelease(oContext);

        /* Deletes color space */
        CGColorSpaceRelease(oColorSpace);
      }

      /* Frees image buffer */
      orxMemory_Free(au8ImageBuffer);
    }
  }

  /* Done! */
  return pstBitmap;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth   = _pstBitmap->fWidth;
  *_pfHeight  = _pstBitmap->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth   = sstDisplay.pstScreen->fWidth;
  *_pfHeight  = sstDisplay.pstScreen->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Screen? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    /* Draws remaining items */
    orxDisplay_iPhone_DrawArrays();

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

orxSTATUS orxFASTCALL orxDisplay_iPhone_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iPhone_IsVSyncEnabled()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetFullScreen(orxBOOL _bFullScreen)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iPhone_IsFullScreen()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxU32 orxFASTCALL orxDisplay_iPhone_GetVideoModeCounter()
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_iPhone_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iPhone_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_Init()
{
  orxSTATUS eResult;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    orxU32 i;
    GLushort u16Index;

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* For all indices */
    for(i = 0, u16Index = 0; i < orxDISPLAY_KU32_INDEX_BUFFER_SIZE; i += 6, u16Index += 4)
    {
      /* Computes them */
      sstDisplay.au16IndexList[i]     = u16Index;
      sstDisplay.au16IndexList[i + 1] = u16Index;
      sstDisplay.au16IndexList[i + 2] = u16Index + 1;
      sstDisplay.au16IndexList[i + 3] = u16Index + 2;
      sstDisplay.au16IndexList[i + 4] = u16Index + 3;
      sstDisplay.au16IndexList[i + 5] = u16Index + 3;
    }

    /* Creates banks */
    sstDisplay.pstBitmapBank  = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    sstDisplay.pstShaderBank  = orxBank_Create(orxDISPLAY_KU32_SHADER_BANK_SIZE, sizeof(orxDISPLAY_SHADER), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if((sstDisplay.pstBitmapBank != orxNULL)
    && (sstDisplay.pstShaderBank != orxNULL))
    {
      orxDISPLAY_EVENT_PAYLOAD stPayload;

      /* Pushes display section */
      orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

      /* Stores view instance */
      sstDisplay.poView = [orxView GetInstance];

      /* Inits default values */
      sstDisplay.bDefaultSmoothing          = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
      sstDisplay.pstScreen                  = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);
      orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));
      sstDisplay.pstScreen->fWidth          = [sstDisplay.poView frame].size.width;
      sstDisplay.pstScreen->fHeight         = [sstDisplay.poView frame].size.height;
      sstDisplay.pstScreen->u32RealWidth    = orxMath_GetNextPowerOfTwo(orxF2U(sstDisplay.pstScreen->fWidth));
      sstDisplay.pstScreen->u32RealHeight   = orxMath_GetNextPowerOfTwo(orxF2U(sstDisplay.pstScreen->fHeight));
      sstDisplay.pstScreen->fRecRealWidth   = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
      sstDisplay.pstScreen->fRecRealHeight  = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
      orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
      orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);
      sstDisplay.eLastBlendMode             = orxDISPLAY_BLEND_MODE_NUMBER;

      /* Updates config info */
      orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_WIDTH, sstDisplay.pstScreen->fWidth);
      orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_HEIGHT, sstDisplay.pstScreen->fHeight);
      orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_DEPTH, 32);

      /* Depth buffer? */
      if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DEPTHBUFFER) != orxFALSE)
      {
         /* Inits flags */
         sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER;
      }
      else
      {
         /* Inits flags */
         sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_NONE;
      }

      /* Pops config section */
      orxConfig_PopSection();

      /* Creates OpenGL thread context */
      [sstDisplay.poView CreateThreadContext];

      /* Gets max texture unit number */
      glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &(sstDisplay.iTextureUnitNumber));
      glASSERT();

      /* Has shader support? */
      if([sstDisplay.poView bShaderSupport] != NO)
      {
        static const orxSTRING szFragmentShaderSource =
        "precision mediump float;"
        "varying vec2 ___TexCoord___;"
        "varying vec4 ___Color___;"
        "uniform sampler2D __Texture__;"
        "void main()"
        "{"
        "  gl_FragColor = ___Color___ * texture2D(__Texture__, ___TexCoord___);"
        "}";

        /* Inits flags */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER | orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);

        /* Creates texture for screen backup */
        glGenTextures(1, &(sstDisplay.pstScreen->uiTexture));
        glASSERT();
        glBindTexture(GL_TEXTURE_2D, sstDisplay.pstScreen->uiTexture);
        glASSERT();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();

        /* Creates default shader */
        sstDisplay.pstDefaultShader = orxDisplay_CreateShader(szFragmentShaderSource, orxNULL);

        /* Uses it */
        orxDisplay_StopShader(orxNULL);
      }
      else
      {
        /* Inits flags */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);
      }

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
      stPayload.u32Width    = orxF2U(sstDisplay.pstScreen->fWidth);
      stPayload.u32Height   = orxF2U(sstDisplay.pstScreen->fHeight);
      stPayload.u32Depth    = 32;
      stPayload.bFullScreen = orxTRUE;

      /* Sends it */
      orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
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

void orxFASTCALL orxDisplay_iPhone_Exit()
{
  /* Was initialized? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Deletes default shader */
      orxDisplay_DeleteShader(sstDisplay.pstDefaultShader);
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

orxHANDLE orxFASTCALL orxDisplay_iPhone_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
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
          s32Offset = orxString_NPrint(pc, s32Free, "precision mediump float;\nvarying vec2 ___TexCoord___;\n");
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
              {
                /* Adds its literal value */
                s32Offset = orxString_NPrint(pc, s32Free, "uniform float %s;\n", pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_TEXTURE:
              {
                /* Adds its literal value and automated coordinates */
                s32Offset = orxString_NPrint(pc, s32Free, "uniform sampler2D %s;\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT";\n", pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_VECTOR:
              {
                /* Adds its literal value */
                s32Offset = orxString_NPrint(pc, s32Free, "uniform vec3 %s;\n", pstParam->zName);
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
            orxMemory_Copy(pcReplace, "___TexCoord___", 14 * sizeof(orxCHAR));
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
        pstShader->bActive                = orxFALSE;
        pstShader->bInitialized           = orxFALSE;
        pstShader->zCode                  = orxString_Duplicate(sstDisplay.acShaderCodeBuffer);
        pstShader->astTextureInfoList     = (orxDISPLAY_TEXTURE_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO), orxMEMORY_TYPE_MAIN);
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

        /* Compiles it */
        if(orxDisplay_iPhone_CompileShader(pstShader) != orxSTATUS_FAILURE)
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

          /* Frees shader */
          orxBank_Free(sstDisplay.pstShaderBank, pstShader);
        }
      }
    }
  }

  /* Done! */
  return hResult;
}

void orxFASTCALL orxDisplay_iPhone_DeleteShader(orxHANDLE _hShader)
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

  /* Frees it */
  orxBank_Free(sstDisplay.pstShaderBank, pstShader);

  return;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_StartShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Draws remaining items */
  orxDisplay_iPhone_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Uses program */
  glUseProgram(pstShader->uiProgram);
  glASSERT();

  /* Updates projection matrix */
  glUniformMatrix4fv(sstDisplay.uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

  /* Updates its status */
  pstShader->bActive      = orxTRUE;
  pstShader->bInitialized = orxFALSE;

  /* Updates active shader counter */
  sstDisplay.s32ActiveShaderCounter++;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_StopShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_hShader != orxHANDLE_UNDEFINED);

  /* Draws remaining items */
  orxDisplay_iPhone_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has shader? */
  if(pstShader != orxNULL)
  {
    /* Wasn't initialized? */
    if(pstShader->bInitialized == orxFALSE)
    {
      /* Inits it */
      orxDisplay_iPhone_InitShader(pstShader);

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
      orxDisplay_iPhone_DrawArrays();
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
  glUniform1i(sstDisplay.uiTextureLocation, 0);
  glASSERT();

  /* Updates projection matrix */
  glUniformMatrix4fv(sstDisplay.uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

  /* Selects it */
  glActiveTexture(GL_TEXTURE0);
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue)
{
  orxDISPLAY_SHADER  *pstShader;
  orxCHAR             acBuffer[256];
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has free texture unit left? */
  if(pstShader->iTextureCounter < sstDisplay.iTextureUnitNumber)
  {
    GLint iLocation;

    /* Gets parameter location */
    iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)_zParam);
    glASSERT();

    /* Valid? */
    if(iLocation != -1)
    {
      /* No bitmap? */
      if(_pstValue == orxNULL)
      {
        /* Uses screen bitmap */
        _pstValue = sstDisplay.pstScreen;
      }

      /* Updates texture info */
      pstShader->astTextureInfoList[pstShader->iTextureCounter].iLocation = iLocation;
      pstShader->astTextureInfoList[pstShader->iTextureCounter].pstBitmap = _pstValue;

      /* Updates texture counter */
      pstShader->iTextureCounter++;

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Valid? */
      if(iLocation != -1)
      {
          /* Updates its value */
          glUniform1f(iLocation, (GLfloat)(orxFLOAT_1 - (_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY)));
          glASSERT();
      }

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Valid? */
      if(iLocation != -1)
      {
          /* Updates its value */
          glUniform1f(iLocation, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
          glASSERT();
      }

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Valid? */
      if(iLocation != -1)
      {
          /* Updates its value */
          glUniform1f(iLocation, (GLfloat)(orxFLOAT_1 - (_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY)));
          glASSERT();
      }

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 256, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Valid? */
      if(iLocation != -1)
      {
          /* Updates its value */
          glUniform1f(iLocation, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));
          glASSERT();
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't find texture parameter <%s> for fragment shader.", _zParam);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter <%s> for fragment shader: all the texture units are used.", _zParam);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue)
{
  orxDISPLAY_SHADER  *pstShader;
  GLint               iLocation;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Gets parameter location */
  iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)_zParam);
  glASSERT();

  /* Valid? */
  if(iLocation != -1)
  {
    /* Updates its value */
    glUniform1f(iLocation, (GLfloat)_fValue);
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

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue)
{
  orxDISPLAY_SHADER  *pstShader;
  GLint               iLocation;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);
  orxASSERT(_pvValue != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Gets parameter location */
  iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)_zParam);
  glASSERT();

  /* Valid? */
  if(iLocation != -1)
  {
    /* Updates its value */
    glUniform3f(iLocation, (GLfloat)_pvValue->fX, (GLfloat)_pvValue->fY, (GLfloat)_pvValue->fZ);
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetDestinationBitmap, DISPLAY, SET_DESTINATION_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
