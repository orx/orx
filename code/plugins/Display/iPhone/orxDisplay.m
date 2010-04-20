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

#define orxDISPLAY_KU32_BUFFER_SIZE             (12 * 1024)


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
  orxBANK              *pstBitmapBank;
  orxBOOL               bDefaultSmoothing;
  orxBITMAP            *pstScreen;
  orxBITMAP            *pstDestinationBitmap;
  const orxBITMAP      *pstLastBitmap;
  orxDISPLAY_BLEND_MODE eLastBlendMode;
  orxView              *poView;
  orxU32                u32Flags;
  GLfloat               afVertexList[orxDISPLAY_KU32_BUFFER_SIZE];
  GLfloat               afTextureCoordList[orxDISPLAY_KU32_BUFFER_SIZE];

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
- (BOOL) CreateRenderTarget:(orxBITMAP *)_pstBitmap;
- (void) InitRender:(orxBITMAP *)_pstBitmap;
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

    /* Enables multi-touch */
    self.multipleTouchEnabled = YES;

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
      glDisable(GL_CULL_FACE);
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

      /* Creates frame buffer */
      [self CreateFrameBuffer];

      /* Updates result */
      bResult = YES;
    }
  }

  /* Done! */
  return bResult;
}

- (BOOL) CreateFrameBuffer
{
  BOOL bResult = YES;

  /* Generates frame buffer */
  glGenFramebuffersOES(1, &uiFrameBuffer);
  glASSERT();

  /* Binds it */
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiFrameBuffer);
  glASSERT();

  /* Done! */
  return bResult;
}

- (BOOL) CreateRenderTarget:(orxBITMAP *)_pstBitmap
{
  BOOL bResult = NO;

  /* Checks */
  orxASSERT((_pstBitmap == sstDisplay.pstScreen) && "This plugin can only render to screen.");

  /* Had a render buffer? */
  if(uiRenderBuffer != 0)
  {
    /* Deletes it */
    glDeleteRenderbuffersOES(1, &uiRenderBuffer);
    glASSERT();

    /* Updates references */
    uiRenderBuffer = 0;
  }

  /* Screen? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    /* Generates render buffer */
    glGenRenderbuffersOES(1, &uiRenderBuffer);
    glASSERT();

    /* Binds it */
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);
    glASSERT();

    /* Links it to layer */
    if([poThreadContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer] != NO)
    {
      /* Binds it to frame buffer */
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
  }
  else
  {
    /* Binds corresponding texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Links it to frame buffer */
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, _pstBitmap->uiTexture, 0);
    glASSERT();

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
  }

  /* Done! */
  return bResult;
}

- (void) InitRender:(orxBITMAP *)_pstBitmap
{
  /* Different destination bitmap? */
  if(_pstBitmap != sstDisplay.pstDestinationBitmap)
  {
    /* Stores it */
    sstDisplay.pstDestinationBitmap = _pstBitmap;

    /* Sets OpenGL context */
    [EAGLContext setCurrentContext:poThreadContext];

    /* Recreates render target */
    [self CreateRenderTarget:_pstBitmap];

    /* Inits viewport */
    glViewport(0, 0, sstDisplay.pstDestinationBitmap->fWidth, sstDisplay.pstDestinationBitmap->fHeight);
    glASSERT();

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

  /* Done! */
  return;
}

- (void) Swap
{
  /* Presents render buffer */
  [poThreadContext presentRenderbuffer:GL_RENDERBUFFER_OES];

  /* Done! */
  return;
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

static orxINLINE void orxDisplay_iPhone_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.pstLastBitmap)
  {
    /* Binds source's texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Stores it */
    sstDisplay.pstLastBitmap = _pstBitmap;
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

  /* Applies color */
  glColor4f(_pstBitmap->stColor.vRGB.fR, _pstBitmap->stColor.vRGB.fG, _pstBitmap->stColor.vRGB.fB, _pstBitmap->stColor.fAlpha);

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_iPhone_DrawBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxFLOAT fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_iPhone_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode);
  
  /* Gets bitmap working size */
  fWidth  = _pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX;
  fHeight = _pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY;

  /* Defines the vertex list */
  GLfloat afVertexList[] =
  {
    0.0f, fHeight,
    0.0f, 0.0f,
    fWidth, fHeight,
    fWidth, 0.0f
  };

  /* Defines the texture coord list */
  GLfloat afTextureCoordList[] =
  {
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vBR.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vTL.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vBR.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vTL.fY)
  };

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

orxSTATUS orxFASTCALL orxDisplay_iPhone_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  const orxCHAR  *pc;
  orxU32          u32Counter;
  GLfloat         fX, fY, fWidth, fHeight;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
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

  /* Gets character's size */
  fWidth  = _pstMap->vCharacterSize.fX;
  fHeight = _pstMap->vCharacterSize.fY;

  /* Prepares font for drawing */
  orxDisplay_iPhone_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode);

  /* For all characters */
  for(pc = _zString, u32Counter = 0, fX = fY = 0.0f; *pc != orxCHAR_NULL; pc++)
  {
    /* Depending on character */
    switch(*pc)
    {
      case orxCHAR_CR:
      {
        /* Half EOL? */
        if(*(pc + 1) == orxCHAR_LF)
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
        /* Is defined? */
        if(_pstMap->astCharacterList[*pc].fX >= orxFLOAT_0)
        {
          /* Outputs vertices and texture coordinates */
          sstDisplay.afVertexList[u32Counter]       =
          sstDisplay.afVertexList[u32Counter + 2]   =
          sstDisplay.afVertexList[u32Counter + 4]   = fX;
          sstDisplay.afVertexList[u32Counter + 6]   =
          sstDisplay.afVertexList[u32Counter + 8]   =
          sstDisplay.afVertexList[u32Counter + 10]  = fX + fWidth;
          sstDisplay.afVertexList[u32Counter + 5]   =
          sstDisplay.afVertexList[u32Counter + 9]   =
          sstDisplay.afVertexList[u32Counter + 11]  = fY;
          sstDisplay.afVertexList[u32Counter + 1]   =
          sstDisplay.afVertexList[u32Counter + 3]   =
          sstDisplay.afVertexList[u32Counter + 7]   = fY + fHeight;
          
          sstDisplay.afTextureCoordList[u32Counter]       =
          sstDisplay.afTextureCoordList[u32Counter + 2]   =
          sstDisplay.afTextureCoordList[u32Counter + 4]   = _pstFont->fRecRealWidth * _pstMap->astCharacterList[*pc].fX;
          sstDisplay.afTextureCoordList[u32Counter + 6]   =
          sstDisplay.afTextureCoordList[u32Counter + 8]   =
          sstDisplay.afTextureCoordList[u32Counter + 10]  = _pstFont->fRecRealWidth * (_pstMap->astCharacterList[*pc].fX + fWidth);
          sstDisplay.afTextureCoordList[u32Counter + 5]   =
          sstDisplay.afTextureCoordList[u32Counter + 9]   =
          sstDisplay.afTextureCoordList[u32Counter + 11]  = orxFLOAT_1 - _pstFont->fRecRealHeight * (_pstFont->fHeight - _pstMap->astCharacterList[*pc].fY);
          sstDisplay.afTextureCoordList[u32Counter + 1]   =
          sstDisplay.afTextureCoordList[u32Counter + 3]   =
          sstDisplay.afTextureCoordList[u32Counter + 7]   = orxFLOAT_1 - _pstFont->fRecRealHeight * (_pstFont->fHeight - _pstMap->astCharacterList[*pc].fY - fHeight);
          
          /* Updates counter */
          u32Counter += 12;

          /* End of buffer? */
          if(u32Counter > orxDISPLAY_KU32_BUFFER_SIZE - 12)
          {
            /* Renders them */
            glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
            glASSERT();
            glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
            glASSERT();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
            glASSERT();
          }
        }
      }

      /* Updates X position */
      fX += fWidth;
    }
  }

  /* Renders last data */
  glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
  glASSERT();
  glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
  glASSERT();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
  glASSERT();

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

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

  /* Swaps */
  [sstDisplay.poView Swap];

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
  if((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * 4))
  {
    orxU8        *au8Buffer, *pu8Dst;
    const orxU8  *pu8Src;
    orxU32        i, u32Offset;

    /* Allocates buffer */
    au8Buffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_MAIN);

    /* Checks */
    orxASSERT(au8Buffer != orxNULL);

    /* For all visible rows */
    for(i = 0, u32Offset = (_pstBitmap->u32RealWidth - u32Width) * 4, pu8Src = _au8Data, pu8Dst = au8Buffer + ((_pstBitmap->u32RealHeight - u32Height) * _pstBitmap->u32RealWidth * 4); i < u32Height; i++)
    {
      orxU32 j;

      /* For all visible columns */
      for(j = 0; j < u32Width; j++, pu8Src += 4, pu8Dst += 4)
      {
        /* Copies pixel data */
        pu8Dst[0] = pu8Src[0];
        pu8Dst[1] = pu8Src[1];
        pu8Dst[2] = pu8Src[2];
        pu8Dst[3] = pu8Src[3];
      }

      /* Skips unvisible pixels */
      pu8Dst += u32Offset;
    }

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, au8Buffer);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(au8Buffer);

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
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Updates result */
    stResult = orxColor_ToRGBA(&(_pstBitmap->stColor));
  }

  /* Done! */
  return stResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetDestinationBitmap(orxBITMAP *_pstDst)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstDst == sstDisplay.pstScreen) && "Can only draw on screen with this version!");

  /* Inits rendering */
  [sstDisplay.poView InitRender:_pstDst];

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_BlitBitmap(const orxBITMAP *_pstSrc, const orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));

  /* Translates it */
  glTranslatef(_fPosX, _fPosY, 0.0f);
  glASSERT();

  /* Draws it */
  orxDisplay_iPhone_DrawBitmap(_pstSrc, _eSmoothing, _eBlendMode);

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
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
  orxDisplay_iPhone_DrawBitmap(_pstSrc, _eSmoothing, _eBlendMode);

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxBOOL   bPNG = orxFALSE;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Screen capture? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    /* PNG? */
    if(orxString_SearchString(_zFilename, ".png") != orxNULL)
    {
      /* Updates status */
      bPNG = orxTRUE;
    }
    /* JPG? */
    else if(orxString_SearchString(_zFilename, ".jpg") != orxNULL)
    {
      /* Updates status */
      bPNG = orxFALSE;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save bitmap to <%s>: only PNG and JPG supported.", _zFilename);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save bitmap tp <%s>: only screen can be saved to file.", _zFilename);

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
    u32BufferSize = sstDisplay.pstScreen->u32RealWidth * sstDisplay.pstScreen->u32RealHeight * 4 * sizeof(GLubyte);

    /* Allocates both buffers */
    au8Buffer       = (GLubyte *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN);
    au8ImageBuffer  = (orxU8 *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if((au8Buffer != orxNULL) && (au8ImageBuffer != orxNULL))
    {
      CGDataProviderRef oProvider;
      CGColorSpaceRef   oColorSpace;
      CGContextRef      oContext;
      CGImageRef        oImage;

      /* Reads OpenGL data */
      glReadPixels(0, 0, sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, au8Buffer);
      glASSERT();

      /* Creates data provider */
      oProvider = CGDataProviderCreateWithData(NULL, au8Buffer, u32BufferSize, NULL);

      /* Creates a device color space */
      oColorSpace = CGColorSpaceCreateDeviceRGB();

      /* Gets image reference */
      oImage = CGImageCreate(sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight, 8, 32, 4 * sstDisplay.pstScreen->u32RealWidth, oColorSpace, kCGBitmapByteOrderDefault, oProvider, nil, NO, kCGRenderingIntentDefault);

      /* Creates graphic context */
      oContext = CGBitmapContextCreate(au8ImageBuffer, sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight, 8, 4 * sstDisplay.pstScreen->u32RealWidth, CGImageGetColorSpace(oImage), kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

      /* Valid? */
      if(oContext)
      {
        UIImage *poImage;

        /* Applies vertical flip for OpenGL/bitmap reordering */
        CGContextTranslateCTM(oContext, 0, sstDisplay.pstScreen->u32RealHeight);
        CGContextScaleCTM(oContext, 1.0f, -1.0f);

        /* Draws image */
        CGContextDrawImage(oContext, CGRectMake(0.0f, 0.0f, sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight), oImage);

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
    au8ImageBuffer = (GLubyte *)orxMemory_Allocate(uiRealWidth * uiRealHeight * sizeof(GLuint), orxMEMORY_TYPE_MAIN);

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

        /* Creates a device color space */
        oColorSpace = CGColorSpaceCreateDeviceRGB();

        /* Creates graphic context */
        oContext = CGBitmapContextCreate(au8ImageBuffer, uiRealWidth, uiRealHeight, 8, 4 * uiRealWidth, oColorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

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
    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* Creates bitmap bank */
    sstDisplay.pstBitmapBank = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstDisplay.pstBitmapBank != orxNULL)
    {
      /* Pushes display section */
      orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

      /* Inits default values */
      sstDisplay.bDefaultSmoothing        = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
      sstDisplay.pstScreen                = orxBank_Allocate(sstDisplay.pstBitmapBank);
      orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));
      sstDisplay.pstScreen->fWidth        = orx2F(orxDISPLAY_KU32_SCREEN_WIDTH);
      sstDisplay.pstScreen->fHeight       = orx2F(orxDISPLAY_KU32_SCREEN_HEIGHT);
      sstDisplay.pstScreen->u32RealWidth  = orxDISPLAY_KU32_SCREEN_WIDTH;
      sstDisplay.pstScreen->u32RealHeight = orxDISPLAY_KU32_SCREEN_HEIGHT;
      orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
      orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);
      sstDisplay.eLastBlendMode           = orxDISPLAY_BLEND_MODE_NUMBER;

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
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return hResult;
}

void orxFASTCALL orxDisplay_iPhone_DeleteShader(orxHANDLE _hShader)
{
  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_RenderShader(orxHANDLE _hShader)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iPhone_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iPhone_RenderShader, DISPLAY, RENDER_SHADER);
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
