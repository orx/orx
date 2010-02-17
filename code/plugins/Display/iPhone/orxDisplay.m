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
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

#define orxDISPLAY_KU32_SCREEN_WIDTH            320
#define orxDISPLAY_KU32_SCREEN_HEIGHT           480
#define orxDISPLAY_KU32_SCREEN_DEPTH            32

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        256


/**  Misc defines
 */
#ifdef __orxDEBUG__

#define glASSERT()                                                      \
do                                                                      \
{                                                                       \
  GLenum eError = glGetError();                                         \
  orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: %ld", eError); \
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal bitmap structure
 */
struct __orxBITMAP_t
{
  GLuint                uiTexture;
  orxBOOL               bSmoothing;
  orxFLOAT              fWidth, fHeight;
  orxU32                u32RealWidth, u32RealHeight;
  orxFLOAT              fRecRealWidth, fRecRealHeight;
  orxCOLOR              stColor;
  orxAABOX              stClip;
};

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxBANK    *pstBitmapBank;
  orxBOOL     bDefaultSmoothing;
  orxBITMAP  *pstScreen;
  orxView    *poView;
  orxU32      u32Flags;

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
- (BOOL) CreateFrameBuffer;
- (void) DeleteFrameBuffer;
- (void) InitRender;
- (void) Swap;

@end

@implementation orxView

@synthesize poMainContext;
@synthesize poThreadContext;

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

    /* Gets the layer */
    poLayer = (CAEAGLLayer *)self.layer;

    /* Inits it */
    poLayer.opaque = YES;
    poLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

    /* Creates main OpenGL context */
    poMainContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

    /* Success? */
    if((poMainContext != nil) && ([EAGLContext setCurrentContext:poMainContext] != 0))
    {
      /* Inits it */
      glEnable(GL_TEXTURE_2D);
      glASSERT();
      glDisable(GL_LIGHTING);
      glASSERT();
      glDisable(GL_FOG);
      glASSERT();
      glDisable(GL_DEPTH_TEST);
      glASSERT();
      glDisable(GL_STENCIL_TEST);
      glASSERT();
      glEnableClientState(GL_VERTEX_ARRAY);
      glASSERT();
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
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

- (void) layoutSubviews
{
  /* Sets current OpenGL context */
  [EAGLContext setCurrentContext:poMainContext];

  /* Recreates frame buffer */
  [self DeleteFrameBuffer];
  [self CreateFrameBuffer];
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
    /* Creates thread context */
    poThreadContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1 sharegroup:poGroup];

    /* Valid? */
    if((poThreadContext != nil) && ([EAGLContext setCurrentContext:poThreadContext] != NO))
    {
      /* Inits it */
      glEnable(GL_TEXTURE_2D);
      glASSERT();
      glDisable(GL_LIGHTING);
      glASSERT();
      glDisable(GL_FOG);
      glASSERT();
      glDisable(GL_DEPTH_TEST);
      glASSERT();
      glDisable(GL_STENCIL_TEST);
      glASSERT();
      glEnableClientState(GL_VERTEX_ARRAY);
      glASSERT();
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glASSERT();

      /* Updates result */
      bResult = YES;
    }
  }
}

- (BOOL) CreateFrameBuffer
{
  BOOL bResult;

  /* Generates buffer IDs */
  glGenRenderbuffersOES(1, &uiRenderBuffer);
  glASSERT();
  glGenFramebuffersOES(1, &uiFrameBuffer);
  glASSERT();

  /* Binds them */
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiFrameBuffer);
  glASSERT();
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);
  glASSERT();

  /* Links them */
  if([poMainContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer] != NO)
  {
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, uiRenderBuffer);
    glASSERT();
    glFlush();
    glASSERT();

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
  }
  else
  {
    /* Updates result */
    bResult = NO;
  }

  /* Done! */
  return bResult;
}

- (void) DeleteFrameBuffer
{
  /* Deletes both buffers */
  glDeleteRenderbuffersOES(1, &uiRenderBuffer);
  glASSERT();
  glDeleteFramebuffersOES(1, &uiFrameBuffer);
  glASSERT();

  /* Updates references */
  uiFrameBuffer   = 0;
  uiRenderBuffer  = 0;
}

- (void) InitRender
{
  /* Sets OpenGL context */
  [EAGLContext setCurrentContext:poThreadContext];

  glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiFrameBuffer);
  glASSERT();
  glViewport(0, 0, sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight);
  glASSERT();

  glMatrixMode(GL_PROJECTION);
  glASSERT();
  glLoadIdentity();
  glASSERT();
  glOrthof(0.0f, sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, 0.0f, -1.0f, 1.0f);
  glASSERT();
  glMatrixMode(GL_MODELVIEW);
  glASSERT();
  glLoadIdentity();
  glASSERT();
}

- (void) Swap
{
  /* Sets OpenGL context */
  [EAGLContext setCurrentContext:poThreadContext];

  /* Binds render buffer */
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);
  glASSERT();

  /* Presents it */
  [poThreadContext presentRenderbuffer:GL_RENDERBUFFER_OES];
}

@end

static orxSTATUS orxFASTCALL orxDisplay_iPhone_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Rendering start? */
  if(_pstEvent->eID == orxRENDER_EVENT_START)
  {
    /* Sets OpenGL context */
    [EAGLContext setCurrentContext:[sstDisplay.poView poThreadContext]];
  }
  /* Viewport rendering start? */
  else if(_pstEvent->eID == orxRENDER_EVENT_VIEWPORT_START)
  {
    /* Checks */
    orxASSERT(orxViewport_GetTexture(orxVIEWPORT(_pstEvent->hSender)) == orxTexture_GetScreenTexture());

    /* Inits rendering */
    [sstDisplay.poView InitRender];
  }

  /* Done! */
  return eResult;
}

static orxINLINE void orxDisplay_iPhone_DrawBitmap(orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL   bSmoothing;
  orxFLOAT  fWidth, fHeight;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* Binds source's texture */
  glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
  glASSERT();

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
    /* Smoothing? */
    if(bSmoothing != orxFALSE)
    {
      /* Updates texture */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glASSERT();

      /* Updates mode */
      _pstBitmap->bSmoothing = orxTRUE;
    }
    else
    {
      /* Updates texture */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glASSERT();

      /* Updates mode */
      _pstBitmap->bSmoothing = orxFALSE;
    }
  }

  /* Gets bitmap working size */
  fWidth  = _pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX;
  fHeight = _pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY;

  /* Defines the vertex list */
  GLfloat afVertexList[] =
  {
    0.0f, fHeight,
    fWidth, fHeight,
    0.0f, 0.0f,
    fWidth, 0.0f
  };

  /* Defines the texture coord list */
  GLfloat afTextureCoordList[] =
  {
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vBR.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vBR.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vTL.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vTL.fY)
  };

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

  /* Applies color */
  glColor4f(_pstBitmap->stColor.vRGB.fR, _pstBitmap->stColor.vRGB.fG, _pstBitmap->stColor.vRGB.fB, _pstBitmap->stColor.fAlpha);

  /* Renders it */
  glVertexPointer(2, GL_FLOAT, 0, afVertexList);
  glASSERT();
  glTexCoordPointer(2, GL_FLOAT, 0, afTextureCoordList);
  glASSERT();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glASSERT();

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

orxDISPLAY_TEXT *orxFASTCALL orxDisplay_iPhone_CreateText()
{
  orxDISPLAY_TEXT *pstResult = orxNULL;

//! TODO
  
  /* Done! */
  return pstResult;
}

void orxFASTCALL orxDisplay_iPhone_DeleteText(orxDISPLAY_TEXT *_pstText)
{
  //! TODO
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_TransformText(orxBITMAP *_pstDst, const orxDISPLAY_TEXT *_pstText, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetTextString(orxDISPLAY_TEXT *_pstText, const orxSTRING _zString)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetTextFont(orxDISPLAY_TEXT *_pstText, const orxSTRING _zFont)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

  /* Done! */
  return eResult;
}

orxSTRING orxFASTCALL orxDisplay_iPhone_GetTextString(const orxDISPLAY_TEXT *_pstText)
{
  orxSTRING zResult = orxNULL;

  //! TODO

  /* Done! */
  return zResult;
}

orxSTRING orxFASTCALL orxDisplay_iPhone_GetTextFont(const orxDISPLAY_TEXT *_pstText)
{
  orxSTRING zResult = orxNULL;

  //! TODO

  /* Done! */
  return zResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_GetTextSize(const orxDISPLAY_TEXT *_pstText, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_PrintString(const orxBITMAP *_pstBitmap, const orxSTRING _zString, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

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
    orxColor_Set(&(pstBitmap->stColor), &orxVECTOR_WHITE, orxFLOAT_1);
    orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

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
    orxRGBA *astBuffer, *pstPixel;

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

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates texture */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, astBuffer);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(astBuffer);
  }
  else
  {
    /* Clear the color buffer with given color */
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

  /* Swaps */
  [sstDisplay.poView Swap];

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxLOG("Not available on this platform!");
  
  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != (orxBITMAP *)sstDisplay.pstScreen));

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Stores it */
    orxColor_SetRGBA(&(_pstBitmap->stColor), _stColor);
  }

  /* Done! */
  return eResult;
}

orxRGBA orxFASTCALL orxDisplay_iPhone_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Updates result */
    stResult = orxColor_ToRGBA(&(_pstBitmap->stColor));
  }

  /* Done! */
  return stResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_BlitBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
  orxASSERT((_pstDst == sstDisplay.pstScreen) && "Can only draw on screen with this version!");

  /* Translates it */
  glTranslatef(_fPosX, _fPosY, 0.0f);
  glASSERT();

  /* Draws it */
  orxDisplay_iPhone_DrawBitmap((orxBITMAP *)_pstSrc, _eSmoothing, _eBlendMode);

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_TransformBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
  orxASSERT((_pstDst == sstDisplay.pstScreen) && "Can only draw on screen with this version!");
  orxASSERT(_pstTransform != orxNULL);

  /* Translates it */
  glTranslatef(_pstTransform->fDstX, _pstTransform->fDstY, 0.0f);
  glASSERT();

  /* Applies rotation */
  glRotatef(orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation, 0.0f, 0.0f, 1.0f);
  glASSERT();

  /* Applies scale */
  glScalef(_pstTransform->fScaleX, _pstTransform->fScaleY, 1.0f);
  glASSERT();

  /* Applies pivot translation */
  glTranslatef(-_pstTransform->fSrcX, -_pstTransform->fSrcY, 0.0f);
  glASSERT();

  /* Draws it */
  orxDisplay_iPhone_DrawBitmap((orxBITMAP *)_pstSrc, _eSmoothing, _eBlendMode);

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Not available */
  orxLOG("Not available on this platform!");
  
  /* Done! */
  return eResult;
}

orxBITMAP *orxFASTCALL orxDisplay_iPhone_LoadBitmap(const orxSTRING _zFilename)
{
  CGImageRef          oImage;
  NSString           *poName;
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
    GLubyte  *u8ImageBuffer;

    /* Gets its size */
    uiWidth   = CGImageGetWidth(oImage);
    uiHeight  = CGImageGetHeight(oImage);

    /* Gets its real size */
    uiRealWidth   = orxMath_GetNextPowerOfTwo(uiWidth);
    uiRealHeight  = orxMath_GetNextPowerOfTwo(uiHeight);

    /* Allocates image buffer */
    u8ImageBuffer = (GLubyte *)orxMemory_Allocate(uiRealWidth * uiRealHeight * sizeof(GLuint), orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(u8ImageBuffer != orxNULL)
    {
      /* Allocates bitmap */
      pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);
      
      /* Valid? */
      if(pstBitmap != orxNULL)
      {
        CGColorSpaceRef oColorSpace;
        CGContextRef    oContext;

        /* Creates a device color space */
        oColorSpace = CGColorSpaceCreateDeviceRGB();

        /* Creates graphic context */
        oContext = CGBitmapContextCreate(u8ImageBuffer, uiRealWidth, uiRealHeight, 8, 4 * uiRealWidth, oColorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

        /* Clears it */
        CGContextClearRect(oContext, CGRectMake(0, 0, uiRealWidth, uiRealHeight));

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
        orxColor_Set(&(pstBitmap->stColor), &orxVECTOR_WHITE, orxFLOAT_1);
        orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

        /* Creates new texture */
        glGenTextures(1, &pstBitmap->uiTexture);
        glASSERT();
        glBindTexture(GL_TEXTURE_2D, pstBitmap->uiTexture);
        glASSERT();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, u8ImageBuffer);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        
        /* Pops config section */
        orxConfig_PopSection();

        /* Deletes context */
        CGContextRelease(oContext);

        /* Deletes color space */
        CGColorSpaceRelease(oColorSpace);
      }

      /* Frees image buffer */
      orxMemory_Free(u8ImageBuffer);
    }
  }

  /* Releases name */
  [poName release];

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
    /* Enables clipping */
    glEnable(GL_SCISSOR_TEST);
    glASSERT();

    /* Stores screen clipping */
    glScissor(_u32TLX, orxF2U(sstDisplay.pstScreen->fHeight)
              - _u32BRY, _u32BRX - _u32TLX, _u32BRY - _u32TLY);
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
  orxLOG("Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iPhone_IsVSyncEnabled()
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetFullScreen(orxBOOL _bFullScreen)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iPhone_IsFullScreen()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return bResult;
}

orxU32 orxFASTCALL orxDisplay_iPhone_GetVideoModeCounter()
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_iPhone_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iPhone_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_Init()
{
  orxSTATUS eResult;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* Adds event handler */
    if(orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxDisplay_iPhone_EventHandler) != orxSTATUS_FAILURE)
    {
      /* Creates bitmap bank */
      sstDisplay.pstBitmapBank = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(sstDisplay.pstBitmapBank != orxNULL)
      {
        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Inits default values */
        sstDisplay.bDefaultSmoothing  = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
        sstDisplay.pstScreen          = orxBank_Allocate(sstDisplay.pstBitmapBank);
        orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));
        sstDisplay.pstScreen->fWidth  = orx2F(orxDISPLAY_KU32_SCREEN_WIDTH);
        sstDisplay.pstScreen->fHeight = orx2F(orxDISPLAY_KU32_SCREEN_HEIGHT);
        orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);

        /* Pops config section */
        orxConfig_PopSection();

        /* Stores view instance */
        sstDisplay.poView = [orxView GetInstance];

        /* Creates OpenGL thread context */
        [sstDisplay.poView CreateThreadContext];

        /* Inits flags */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_MASK_ALL);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_iPhone_EventHandler);

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
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_iPhone_EventHandler);
    
    /* Deletes bitmap bank */
    orxBank_Delete(sstDisplay.pstBitmapBank);
    sstDisplay.pstBitmapBank = orxNULL;

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  /* Done! */
  return;
}

orxHANDLE orxFASTCALL orxDisplay_iPhone_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return hResult;
}

void orxFASTCALL orxDisplay_iPhone_DeleteShader(orxHANDLE _hShader)
{
  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_RenderShader(orxHANDLE _hShader)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return eResult;
}

orxHANDLE orxFASTCALL orxDisplay_iPhone_GetApplicationInput()
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Not available */
  orxLOG("Not available on this platform!");

  /* Done! */
  return hResult;
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_CreateText, DISPLAY, CREATE_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_DeleteText, DISPLAY, DELETE_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetTextString, DISPLAY, SET_TEXT_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetTextFont, DISPLAY, SET_TEXT_FONT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetTextString, DISPLAY, GET_TEXT_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetTextFont, DISPLAY, GET_TEXT_FONT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetTextSize, DISPLAY, GET_TEXT_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_PrintString, DISPLAY, PRINT_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_RenderShader, DISPLAY, RENDER_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetApplicationInput, DISPLAY, GET_APPLICATION_INPUT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
