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
#define orxDISPLAY_KU32_STATIC_FLAG_RENDERING   0x00000002 /**< Rendering flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

#define orxDISPLAY_KU32_SCREEN_WIDTH            320
#define orxDISPLAY_KU32_SCREEN_HEIGHT           480
#define orxDISPLAY_KU32_SCREEN_DEPTH            32

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        256


/**  Misc defines
 */
#ifdef __orxDEBUG__

#define glASSERT()                              \
do                                              \
{                                               \
  GLenum eError = glGetError();                 \
  orxASSERT(eError == GL_NO_ERROR);             \
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal bitmap structure
 */
typedef struct __orxDISPLAY_BITMAP_t
{
  GLuint                uiTexture;
  orxDISPLAY_SMOOTHING  eSmoothing;
  orxFLOAT              fWidth, fHeight;
  orxU32                u32RealWidth, u32RealHeight;
  orxRGBA               stColor;
  orxAABOX              stClip;

} orxDISPLAY_BITMAP;

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxBANK          *pstBitmapBank;
  orxBITMAP        *pstScreen;
  orxView          *pstView;
  orxU32            u32ScreenWidth, u32ScreenHeight;
  orxU32            u32Flags;

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

- (BOOL) CreateFrameBuffer;
- (void) DeleteFrameBuffer;
- (void) Swap;

@end

@implementation orxView

@synthesize poContext;

+ (Class) layerClass
{
  return [CAEAGLLayer class];
}

+ (orxView *) GetInstance
{
  return spoInstance;
}

- (id) initWithCoder:(NSCoder *)coder
{
  id oResult = nil;

  /* Inits parent */
  if((self = [super initWithCoder:coder]) != nil)
  {
    CAEAGLLayer *poLayer;

    /* Gets the layer */
    poLayer = (CAEAGLLayer *)self.layer;

    /* Inits it */
    poLayer.opaque = YES;
    poLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

    /* Creates OpenGL context */
    poContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

    /* Success? */
    if((poContext != nil) && ([EAGLContext setCurrentContext:poContext] != 0))
    {
      /* Inits it */
      glEnable(GL_TEXTURE_2D);
      glASSERT();

      /* Inits view */
      [self layoutSubviews];

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
  [EAGLContext setCurrentContext:poContext];

  /* Recreates frame buffer */
  [self DeleteFrameBuffer];
  [self CreateFrameBuffer];
}

- (BOOL) CreateFrameBuffer
{
  GLint iWidth, iHeight;
  BOOL  bResult;

  /* Generates buffer IDs */
  glGenRenderbuffersOES(1, &uiRenderBuffer);
  glGenFramebuffersOES(1, &uiFrameBuffer);

  /* Binds them */
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiFrameBuffer);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);

  /* Links it to the OpenGL context */
  [poContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer];
  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, uiRenderBuffer);

  /* Stores new screen size */
  glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &iWidth);
  glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &iHeight);
  sstDisplay.u32ScreenWidth   = iWidth;
  sstDisplay.u32ScreenHeight  = iHeight;

  /* Inits viewport */
  glViewport(0, 0, iWidth, iHeight);

  /* Resets projection matrix */
  glMatrixMode(GL_PROJECTION);
  glASSERT();
  glLoadIdentity();
  glASSERT();
  glOrthof(0.0f, iWidth, iHeight, 0.0f, -1.0f, 1.0f);
  glASSERT();

  /* Updates result */
  bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
  glASSERT();

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

- (void) Swap
{
  /* Binds render buffer */
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);

//  orxLOG("SWAP BEGIN");
  /* Presents it */
//  BOOL b = [poContext presentRenderbuffer:GL_RENDERBUFFER_OES];
  
//  orxLOG("SWAP END %s", b == YES ? "YES": "NO");
}

@end

static orxINLINE orxU32 orxDisplay_GetNPOT(orxU32 _u32Value)
{
  orxU32 u32Result;

  /* Non-zero? */
  if(_u32Value != 0)
  {
    /* Updates result */
    u32Result = _u32Value - 1;
    u32Result = u32Result | (u32Result >> 1);
    u32Result = u32Result | (u32Result >> 2);
    u32Result = u32Result | (u32Result >> 4);
    u32Result = u32Result | (u32Result >> 8);
    u32Result = u32Result | (u32Result >> 16);
    u32Result++;
  }
  else
  {
    /* Updates result */
    u32Result = 1;
  }

  /* Done! */
  return u32Result;
}

static void orxFASTCALL orxDisplay_iPhone_EventUpdate(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
//! TODO
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
  orxDISPLAY_BITMAP *pstBitmap;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Gets bitmap */
    pstBitmap = (orxDISPLAY_BITMAP *)_pstBitmap;

    /* Deletes its texture */
    glDeleteTextures(1, &(pstBitmap->uiTexture));
    glASSERT();

    /* Deletes it */
    orxBank_Free(sstDisplay.pstBitmapBank, pstBitmap);
  }

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_iPhone_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  orxDISPLAY_BITMAP *pstBitmap;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Allocates bitmap */
  pstBitmap = (orxDISPLAY_BITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

  /* Valid? */
  if(pstBitmap != orxNULL)
  {
    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Inits it */
    pstBitmap->eSmoothing     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH) ? orxDISPLAY_SMOOTHING_ON : orxDISPLAY_SMOOTHING_OFF;
    pstBitmap->fWidth         = orxU2F(_u32Width);
    pstBitmap->fHeight        = orxU2F(_u32Height);
    pstBitmap->u32RealWidth   = orxDisplay_GetNPOT(_u32Width);
    pstBitmap->u32RealHeight  = orxDisplay_GetNPOT(_u32Height);
    pstBitmap->stColor        = orx2RGBA(255, 255, 255, 255);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->eSmoothing == orxDISPLAY_SMOOTHING_ON) ? GL_LINEAR : GL_NEAREST);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->eSmoothing == orxDISPLAY_SMOOTHING_ON) ? GL_LINEAR : GL_NEAREST);
    glASSERT();

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return (orxBITMAP *)pstBitmap;
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
    orxRGBA            *astBuffer, *pstPixel;
    orxDISPLAY_BITMAP  *pstBitmap;

    /* Gets bitmap */
    pstBitmap = (orxDISPLAY_BITMAP *)_pstBitmap;
    
    /* Allocates buffer */
    astBuffer = (orxRGBA *)orxMemory_Allocate(pstBitmap->u32RealWidth * pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_MAIN);

    /* Checks */
    orxASSERT(astBuffer != orxNULL);

    /* For all pixels */
    for(pstPixel = astBuffer; pstPixel < astBuffer + (pstBitmap->u32RealWidth * pstBitmap->u32RealHeight); pstPixel++)
    {
      /* Sets its value */
      *pstPixel = _stColor;
    }

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, pstBitmap->uiTexture);
    glASSERT();

    /* Updates texture */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, astBuffer);
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
  [sstDisplay.pstView Swap];

  /* Updates status */
  orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_RENDERING);

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
    ((orxDISPLAY_BITMAP *)_pstBitmap)->stColor = _stColor;
  }

  /* Done! */
  return eResult;
}

orxRGBA orxFASTCALL orxDisplay_iPhone_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != (orxBITMAP *)sstDisplay.pstScreen));

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Updates result */
    stResult = ((orxDISPLAY_BITMAP *)_pstBitmap)->stColor;
  }

  /* Done! */
  return stResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_BlitBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstDst != orxNULL);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != (orxBITMAP *)sstDisplay.pstScreen));

  //! TODO

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_TransformBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstDst != orxNULL);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != (orxBITMAP *)sstDisplay.pstScreen));
  orxASSERT(_pstTransform != orxNULL);

  //! TODO

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
  orxBITMAP *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  //! TODO

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Gets size */
    *_pfWidth   = ((orxDISPLAY_BITMAP *)_pstBitmap)->fWidth;
    *_pfHeight  = ((orxDISPLAY_BITMAP *)_pstBitmap)->fHeight;
  }
  else
  {
    /* Gets size */
    *_pfWidth   = orxU2F(sstDisplay.u32ScreenWidth);
    *_pfHeight  = orxU2F(sstDisplay.u32ScreenHeight);
  }

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
  *_pfWidth   = orxU2F(sstDisplay.u32ScreenWidth);
  *_pfHeight  = orxU2F(sstDisplay.u32ScreenHeight);

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
    /* New frame? */
    if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_RENDERING))
    {
      /* Updates status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_RENDERING, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }

    /* Enables clipping */
    glEnable(GL_SCISSOR_TEST);
    glASSERT();

    /* Stores screen clipping */
    glScissor(_u32TLX, sstDisplay.u32ScreenHeight - _u32BRY, _u32BRX - _u32TLX, _u32BRY - _u32TLY);
    glASSERT();
  }
  else
  {
    orxDISPLAY_BITMAP *pstBitmap;

    /* Gets bitmap */
    pstBitmap = (orxDISPLAY_BITMAP *)_pstBitmap;

    /* Stores clip coords */
    orxVector_Set(&(pstBitmap->stClip.vTL), orxU2F(_u32TLX), orxU2F(_u32TLY), orxFLOAT_0);
    orxVector_Set(&(pstBitmap->stClip.vBR), orxU2F(_u32BRX), orxU2F(_u32BRY), orxFLOAT_0);
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

    /* Creates bitmap bank */
    sstDisplay.pstBitmapBank = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxDISPLAY_BITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstDisplay.pstBitmapBank != orxNULL)
    {
      /* Inits default values */
      sstDisplay.pstScreen        = (orxBITMAP *)-1;
      sstDisplay.u32ScreenWidth   = orxDISPLAY_KU32_SCREEN_WIDTH;
      sstDisplay.u32ScreenHeight  = orxDISPLAY_KU32_SCREEN_HEIGHT;

      /* Stores view instance */
      sstDisplay.pstView = [orxView GetInstance];

      /* Sets current OpenGL context */
      [EAGLContext setCurrentContext:[sstDisplay.pstView poContext]];

      /* Inits flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_MASK_ALL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
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
