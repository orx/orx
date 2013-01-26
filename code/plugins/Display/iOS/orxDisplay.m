/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
 * iOS display plugin implementation
 *
 */


#include "orxPluginAPI.h"
#import <QuartzCore/QuartzCore.h>
#include <CoreFoundation/CFByteOrder.h>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000  /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001  /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHADER      0x00000002  /**< Shader support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER 0x00000004  /**< Has depth buffer support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_NPOT        0x00000008  /**< NPOT texture support flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        128
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        16

#define orxDISPLAY_KU32_VERTEX_BUFFER_SIZE      (4 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_INDEX_BUFFER_SIZE       (6 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      65536

#define orxDISPLAY_KF_BORDER_FIX                0.1f

#define orxDISPLAY_KU32_TOUCH_NUMBER            16

#define orxDISPLAY_KU32_EVENT_INFO_NUMBER       32

#define orxDISPLAY_KU32_CIRCLE_LINE_NUMBER      32


/**  Misc defines
 */

#define PVR_TEXTURE_FLAG_TYPE_MASK              0xFF


#ifdef __orxDEBUG__

#define glASSERT()                                                      \
do                                                                      \
{                                                                       \
  GLenum eError = glGetError();                                         \
  orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: 0x%X", eError);\
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()

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

/** PVR texture file header
 */
typedef struct __PVRTexHeader_t
{
  uint32_t headerLength;
  uint32_t height;
  uint32_t width;
  uint32_t numMipmaps;
  uint32_t flags;
  uint32_t dataLength;
  uint32_t bpp;
  uint32_t bitmaskRed;
  uint32_t bitmaskGreen;
  uint32_t bitmaskBlue;
  uint32_t bitmaskAlpha;
  uint32_t pvrTag;
  uint32_t numSurfs;
} PVRTexHeader;

/** PVR texture types
 */
enum
{
  kPVRTextureFlagTypeOGLARGB4444 = 16,
  kPVRTextureFlagTypeOGLARGB1555,
  kPVRTextureFlagTypeOGLARGB8888,
  kPVRTextureFlagTypeOGLRGB565,
  kPVRTextureFlagTypeOGLRGB555,
  kPVRTextureFlagTypeOGLRGB888,
  kPVRTextureFlagTypePVRTC_2 = 24,
  kPVRTextureFlagTypePVRTC_4
};

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

/** Internal param info structure
 */
typedef struct __orxDISPLAY_PARAM_INFO_t
{
  GLint iLocation, iLocationTop, iLocationLeft, iLocationBottom, iLocationRight;

} orxDISPLAY_PARAM_INFO;

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

/** Internal touch info
 */
typedef struct __orxDISPLAY_TOUCH_INFO_t
{
  const UITouch  *poTouch;
} orxDISPLAY_TOUCH_INFO;

/** Internal event info
 */
typedef struct __orxDISPLAY_EVENT_INFO_t
{
   orxSYSTEM_EVENT_PAYLOAD  stPayload;
   orxENUM                  eID;

} orxDISPLAY_EVENT_INFO;

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
  GLint                     iTextureUnitNumber;
  orxS32                    s32ActiveShaderCounter;
  orxS32                    s32BufferIndex;
  orxDOUBLE                 dTouchTimeCorrection;
  orxU32                    u32EventInfoNumber;
  orxView                  *poView;
  orxU32                    u32Flags;
  orxDISPLAY_PROJ_MATRIX    mProjectionMatrix;
  orxDISPLAY_VERTEX         astVertexList[orxDISPLAY_KU32_VERTEX_BUFFER_SIZE];
  GLushort                  au16IndexList[orxDISPLAY_KU32_INDEX_BUFFER_SIZE];
  orxCHAR                   acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE];
  orxDISPLAY_TOUCH_INFO     astTouchInfoList[orxDISPLAY_KU32_TOUCH_NUMBER];
  orxDISPLAY_EVENT_INFO     astEventInfoList[orxDISPLAY_KU32_EVENT_INFO_NUMBER];

} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDISPLAY_STATIC sstDisplay;

static unsigned char gPVRTexIdentifier[4] = "PVR!";


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** orxView controller class
 */
@implementation orxViewController

@synthesize eOrientation;

- (void) loadView
{
  CGRect    stFrame;
  orxView  *poView;
  NSString *zOrientation;

  /* Gets application's size */
  stFrame = [[UIScreen mainScreen] applicationFrame];
  stFrame.origin.y = 0.0;

  /* Gets literal initial orientation from Info.pList file */
  zOrientation = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UIInterfaceOrientation"];
  if(zOrientation == nil)
  {
    zOrientation = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"] objectAtIndex:0];
  }

  /* Stores orientation */
  eOrientation = ([zOrientation isEqualToString:@"UIInterfaceOrientationPortrait"])
                 ? UIInterfaceOrientationPortrait
                 : ([zOrientation isEqualToString:@"UIInterfaceOrientationLandscapeLeft"])
                   ? UIInterfaceOrientationLandscapeLeft
                   : ([zOrientation isEqualToString:@"UIInterfaceOrientationLandscapeRight"])
                     ? UIInterfaceOrientationLandscapeRight
                     : UIInterfaceOrientationPortraitUpsideDown;

  /* Is in landscape mode? */
  if(UIInterfaceOrientationIsLandscape(eOrientation))
  {
    CGFloat fTemp;

    /* Swaps width and height */
    fTemp = stFrame.size.width;
    stFrame.size.width = stFrame.size.height;
    stFrame.size.height = fTemp;
  }

  /* Creates and inits orx view */
  poView = [[orxView alloc] initWithFrame:stFrame];
  self.view = poView;
  [poView release];
}

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)_oInterfaceOrientation
{
  NSString *zOrientation;

  /* Depending on orientation */
  switch(_oInterfaceOrientation)
  {
    case UIInterfaceOrientationPortrait:            zOrientation = @"UIInterfaceOrientationPortrait"; break;
    case UIInterfaceOrientationPortraitUpsideDown:  zOrientation = @"UIInterfaceOrientationPortraitUpsideDown"; break;
    case UIInterfaceOrientationLandscapeLeft:       zOrientation = @"UIInterfaceOrientationLandscapeLeft"; break;
    case UIInterfaceOrientationLandscapeRight:      zOrientation = @"UIInterfaceOrientationLandscapeRight"; break;
    default:                                        zOrientation = @"Undefined"; break;
  }

  /* Done! */
  return [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"] containsObject:zOrientation];
}

@end

/** orxView class
 */
static orxView *spoInstance;

@interface orxView ()

- (BOOL) CreateThreadContext;
- (BOOL) CreateBuffers;
- (BOOL) CreateRenderTarget:(const orxBITMAP *)_pstBitmap;
- (BOOL) IsExtensionSupported:(NSString *)_zExtension;
- (void) ProcessEvents;
- (void) Swap;

@end

@implementation orxView

@synthesize poMainContext;
@synthesize poThreadContext;
@synthesize bShaderSupport;
@synthesize bCompressedTextureSupport;

+ (Class) layerClass
{
  return [CAEAGLLayer class];
}

+ (orxView *) GetInstance
{
  return spoInstance;
}

- (void) NotifyAcceleration:(UIAcceleration *)_poAcceleration
{
  @synchronized(self)
  {
    /* Is initialized? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
    {
      /* Not overflowing? */
      if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
      {
        orxSYSTEM_EVENT_PAYLOAD *pstPayload;

        /* Gets payload */
        pstPayload = &(sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber].stPayload);

        /* Inits it */
        orxMemory_Zero(pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        pstPayload->stTouch.fPressure = orxFLOAT_1;

        /* Updates it */
        pstPayload->stAccelerometer.dTime = _poAcceleration.timestamp + sstDisplay.dTouchTimeCorrection;
        orxVector_Set(&(pstPayload->stAccelerometer.vAcceleration), orx2F(_poAcceleration.x), orx2F(-_poAcceleration.y), orx2F(-_poAcceleration.z));

        /* Stores event info */
        sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber++].eID = orxSYSTEM_EVENT_ACCELERATE;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping accelerometer event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
      }
    }
  }
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

    /* Sets scale factor */
    self.contentScaleFactor = ([[UIScreen mainScreen] respondsToSelector:@selector(scale)] != NO)
                              ? [UIScreen mainScreen].scale
                              : 1.0f;

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
      /* Support for compressed textures */
      bCompressedTextureSupport = [self IsExtensionSupported:@"GL_IMG_texture_compression_pvrtc"];

      /* Shader support? */
      if(bShaderSupport != NO)
      {
        /* Enables vextex attribute arrays */
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX);
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD);
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR);
        glASSERT();

        /* Sets vextex attribute arrays */
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
        glASSERT();
      }
      else
      {
        /* Inits it */
        glEnable(GL_TEXTURE_2D);
        glASSERT();
        glDisable(GL_DITHER);
        glASSERT();
        glDisable(GL_LIGHTING);
        glASSERT();
        glDisable(GL_FOG);
        glASSERT();
        glDisable(GL_ALPHA_TEST);
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
      glEnable(GL_SCISSOR_TEST);
      glASSERT();
      glDisable(GL_CULL_FACE);
      glASSERT();
      glDisable(GL_DEPTH_TEST);
      glASSERT();
      glDisable(GL_STENCIL_TEST);
      glASSERT();

      /* Stores self */
      spoInstance = self;

      /* Creates frame & render buffers */
      uiScreenFrameBuffer = uiTextureFrameBuffer = uiRenderBuffer = 0;
      [self CreateBuffers];

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
        /* Enables vertex attribute arrays */
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX);
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD);
        glASSERT();
        glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR);
        glASSERT();

        /* Sets vextex attribute arrays */
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
        glASSERT();
        glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
        glASSERT();
      }
      else
      {
        /* Inits it */
        glEnable(GL_TEXTURE_2D);
        glASSERT();
        glDisable(GL_DITHER);
        glASSERT();
        glDisable(GL_LIGHTING);
        glASSERT();
        glDisable(GL_FOG);
        glASSERT();
        glDisable(GL_ALPHA_TEST);
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
      glEnable(GL_SCISSOR_TEST);
      glASSERT();
      glDisable(GL_CULL_FACE);
      glASSERT();
      glDisable(GL_DEPTH_TEST);
      glASSERT();
      glDisable(GL_STENCIL_TEST);
      glASSERT();

      /* Binds default frame and render buffers */
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiScreenFrameBuffer);
      glASSERT();
      glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);
      glASSERT();
      glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, uiRenderBuffer);
      glASSERT();

      /* Updates result */
      bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
      glASSERT();
    }
  }

  /* Done! */
  return bResult;
}

- (BOOL) CreateBuffers
{
  BOOL bResult = YES;

  /* Has screen frame buffer? */
  if(uiScreenFrameBuffer != 0)
  {
    /* Deletes it */
    glDeleteFramebuffers(1, &uiScreenFrameBuffer);
    glASSERT();
  }

  /* Has texture frame buffer? */
  if(uiTextureFrameBuffer != 0)
  {
    /* Deletes it */
    glDeleteFramebuffers(1, &uiTextureFrameBuffer);
    glASSERT();
  }

  /* Has render buffer? */
  if(uiRenderBuffer != 0)
  {
    /* Deletes it */
    glDeleteRenderbuffers(1, &uiRenderBuffer);
    glASSERT();
  }

  /* Generates frame buffers */
  glGenFramebuffersOES(1, &uiScreenFrameBuffer);
  glASSERT();
  glGenFramebuffersOES(1, &uiTextureFrameBuffer);
  glASSERT();

  /* Binds screen one */
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiScreenFrameBuffer);
  glASSERT();

  /* Generates render buffer */
  glGenRenderbuffersOES(1, &uiRenderBuffer);
  glASSERT();

  /* Binds it */
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);
  glASSERT();

  /* Links it to layer */
  bResult = [[EAGLContext currentContext] renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer];

  /* Success? */
  if(bResult != NO)
  {
    /* Links it to frame buffer */
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, uiRenderBuffer);
    glASSERT();

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

      /* Links it to frame buffer */
      glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, uiDepthBuffer);
      glASSERT();

      /* Binds render buffer back */
      glBindRenderbufferOES(GL_RENDERBUFFER_OES, uiRenderBuffer);
      glASSERT();

      /* Enables depth test */
      glEnable(GL_DEPTH_TEST);
      glASSERT();

      /* Sets depth function */
      glDepthFunc(GL_LEQUAL);
      glASSERT();

      /* Clears depth buffer */
      glClearDepthf(1.0f);
      glASSERT();
      glClear(GL_DEPTH_BUFFER_BIT);
      glASSERT();
    }

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
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
    /* Binds screen frame buffer */
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiScreenFrameBuffer);
    glASSERT();

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
  }
  else
  {
    /* Binds texture frame buffer */
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiTextureFrameBuffer);
    glASSERT();

    /* Links texture to it */
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, _pstBitmap->uiTexture, 0);
    glASSERT();

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
  }

  /* Done! */
  return bResult;
}

- (void) ProcessEvents
{
  @synchronized(self)
  {
    orxU32 i;

    /* For all events */
    for(i = 0; i < sstDisplay.u32EventInfoNumber; i++)
    {
      /* Sends it */
      orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, sstDisplay.astEventInfoList[i].eID, orxNULL, orxNULL, &(sstDisplay.astEventInfoList[i].stPayload));
    }

    /* Clears event info number */
    sstDisplay.u32EventInfoNumber = 0;
  }
}

- (void) Swap
{
  /* Swaps */
  [[EAGLContext currentContext] presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (BOOL) IsExtensionSupported:(NSString *)_zExtension
{
  NSString *zExtensionString  = [NSString stringWithCString:(char *)glGetString(GL_EXTENSIONS) encoding:NSUTF8StringEncoding];
  NSArray *zExtensionList     = [zExtensionString componentsSeparatedByString:@" "];

  /* Done! */
  return [zExtensionList containsObject:_zExtension];
}

- (void) touchesBegan:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  @synchronized(self)
  {
    /* Is initialized? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
    {
      /* For all new touches */
      for(UITouch *poTouch in _poTouchList)
      {
        /* Not overflowing? */
        if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
        {
          CGPoint                   vViewPosition;
          orxSYSTEM_EVENT_PAYLOAD  *pstPayload;
          orxU32                    u32ID;

          /* Finds first empty slot */
          for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != nil); u32ID++);

          /* Checks */
          orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

          /* Stores touch */
          sstDisplay.astTouchInfoList[u32ID].poTouch = poTouch;

          /* Gets its position inside view */
          vViewPosition = [poTouch locationInView:self];

          /* Gets payload */
          pstPayload = &(sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber].stPayload);

          /* Inits it */
          orxMemory_Zero(pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
          pstPayload->stTouch.fPressure = orxFLOAT_1;

          /* Updates it */
          pstPayload->stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
          pstPayload->stTouch.u32ID = u32ID;
          pstPayload->stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
          pstPayload->stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

          /* Stores event info */
          sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber++].eID = orxSYSTEM_EVENT_TOUCH_BEGIN;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch begin event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
        }
      }
    }
  }

  /* Done! */
  return;
}

- (void) touchesMoved:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  @synchronized(self)
  {
    /* Is initialized? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
    {
      /* For all moved touches */
      for(UITouch *poTouch in _poTouchList)
      {
        /* Not overflowing? */
        if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
        {
          CGPoint                   vViewPosition;
          orxSYSTEM_EVENT_PAYLOAD  *pstPayload;
          orxU32                    u32ID;

          /* Finds corresponding slot */
          for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != poTouch); u32ID++);

          /* Checks */
          orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

          /* Gets its position inside view */
          vViewPosition = [poTouch locationInView:self];

          /* Gets payload */
          pstPayload = &(sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber].stPayload);

          /* Inits it */
          orxMemory_Zero(pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
          pstPayload->stTouch.fPressure = orxFLOAT_1;

          /* Updates it */
          pstPayload->stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
          pstPayload->stTouch.u32ID = u32ID;
          pstPayload->stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
          pstPayload->stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

          /* Stores event info */
          sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber++].eID = orxSYSTEM_EVENT_TOUCH_MOVE;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch move event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
        }
      }
    }
  }

  /* Done! */
  return;
}

- (void) touchesEnded:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  @synchronized(self)
  {
    /* Is initialized? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
    {
      /* For all ended touches */
      for(UITouch *poTouch in _poTouchList)
      {
        /* Not overflowing? */
        if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
        {
          CGPoint                   vViewPosition;
          orxSYSTEM_EVENT_PAYLOAD  *pstPayload;
          orxU32                    u32ID;

          /* Finds corresponding slot */
          for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != poTouch); u32ID++);

          /* Checks */
          orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

          /* Removes touch */
          sstDisplay.astTouchInfoList[u32ID].poTouch = nil;

          /* Gets its position inside view */
          vViewPosition = [poTouch locationInView:self];

          /* Gets payload */
          pstPayload = &(sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber].stPayload);

          /* Inits it */
          orxMemory_Zero(pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
          pstPayload->stTouch.fPressure = orxFLOAT_0;

          /* Updates it */
          pstPayload->stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
          pstPayload->stTouch.u32ID = u32ID;
          pstPayload->stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
          pstPayload->stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

          /* Stores event info */
          sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber++].eID = orxSYSTEM_EVENT_TOUCH_END;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch end event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
        }
      }
    }
  }

  /* Done! */
  return;
}

- (void) touchesCancelled:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
{
  @synchronized(self)
  {
    /* Is initialized? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
    {
      /* For all cancelled touches */
      for(UITouch *poTouch in _poTouchList)
      {
        /* Not overflowing? */
        if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
        {
          CGPoint                   vViewPosition;
          orxSYSTEM_EVENT_PAYLOAD  *pstPayload;
          orxU32                    u32ID;

          /* Finds corresponding slot */
          for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != poTouch); u32ID++);

          /* Checks */
          orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

          /* Removes touch */
          sstDisplay.astTouchInfoList[u32ID].poTouch = nil;

          /* Gets its position inside view */
          vViewPosition = [poTouch locationInView:self];

          /* Gets payload */
          pstPayload = &(sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber].stPayload);

          /* Inits it */
          orxMemory_Zero(pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
          pstPayload->stTouch.fPressure = orxFLOAT_0;

          /* Updates it */
          pstPayload->stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
          pstPayload->stTouch.u32ID = u32ID;
          pstPayload->stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
          pstPayload->stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

          /* Stores event info */
          sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber++].eID = orxSYSTEM_EVENT_TOUCH_END;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch cancel event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
        }
      }
    }
  }

  /* Done! */
  return;
}

#ifdef __IPHONE_3_0

- (void) motionEnded:(UIEventSubtype)_eMotion withEvent:(UIEvent *)_poEvent
{
  /* Shake? */
  if(_eMotion == UIEventSubtypeMotionShake)
  {
    @synchronized(self)
    {
      /* Is initialized? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
      {
        /* Not overflowing? */
        if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
        {
          orxSYSTEM_EVENT_PAYLOAD *pstPayload;

          /* Gets payload */
          pstPayload = &(sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber].stPayload);

          /* Inits it */
          orxMemory_Zero(pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));

          /* Stores event info */
          sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber++].eID = orxSYSTEM_EVENT_MOTION_SHAKE;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping motion event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
        }
      }
    }
  }

  /* Done! */
  return;
}

#endif

@end

static void orxFASTCALL orxDisplay_iOS_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_Update");

  /* Processes accumulated events */
  [sstDisplay.poView ProcessEvents];

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

static orxINLINE orxDISPLAY_MATRIX *orxDisplay_iOS_InitMatrix(orxDISPLAY_MATRIX *_pmMatrix, orxFLOAT _fPosX, orxFLOAT _fPosY, orxFLOAT _fScaleX, orxFLOAT _fScaleY, orxFLOAT _fRotation, orxFLOAT _fPivotX, orxFLOAT _fPivotY)
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

static orxDISPLAY_PROJ_MATRIX *orxDisplay_iOS_OrthoProjMatrix(orxDISPLAY_PROJ_MATRIX *_pmResult, orxFLOAT _fLeft, orxFLOAT _fRight, orxFLOAT _fBottom, orxFLOAT _fTop, orxFLOAT _fNear, orxFLOAT _fFar)
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

static orxSTATUS orxFASTCALL orxDisplay_iOS_CompileShader(orxDISPLAY_SHADER *_pstShader)
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

      /* Gets texture location */
      _pstShader->uiTextureLocation = glGetUniformLocation(uiProgram, "__Texture__");
      glASSERT();

      /* Gets projection matrix location */
      _pstShader->uiProjectionMatrixLocation = glGetUniformLocation(uiProgram, "__mProjection__");
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

static void orxFASTCALL orxDisplay_iOS_InitShader(orxDISPLAY_SHADER *_pstShader)
{
  /* Uses shader's program */
  glUseProgram(_pstShader->uiProgram);
  glASSERT();

  /* Has custom textures? */
  if(_pstShader->iTextureCounter > 0)
  {
    GLint   i;
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

static void orxFASTCALL orxDisplay_iOS_DrawArrays()
{
  /* Has data? */
  if(sstDisplay.s32BufferIndex > 0)
  {
    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxDisplay_DrawArrays");

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
          orxDisplay_iOS_InitShader(pstShader);

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

static orxINLINE void orxDisplay_iOS_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.pstLastBitmap)
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

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
    orxDisplay_iOS_DrawArrays();

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
    orxDisplay_iOS_DrawArrays();

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

static orxINLINE void orxDisplay_iOS_DrawBitmap(const orxBITMAP *_pstBitmap, const orxDISPLAY_MATRIX *_pmTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  GLfloat fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_iOS_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = (GLfloat)(_pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat)(_pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY);

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
  {
    /* Draw arrays */
    orxDisplay_iOS_DrawArrays();
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

static orxBITMAP *orxDisplay_iOS_LoadPVRBitmap(const orxSTRING _zFilename)
{
  orxFILE    *pstFile;
  orxBITMAP  *pstBitmap = orxNULL;

  /* Opens file */
  pstFile = orxFile_Open(_zFilename, orxFILE_KU32_FLAG_OPEN_READ);

  /* Success? */
  if(pstFile != orxNULL)
  {
    PVRTexHeader  stHeader;
    orxS32        s32FileSize;

    /* Gets file size */
    s32FileSize = orxFile_GetSize(pstFile);

    /* Loads PVR header from file */
    if((s32FileSize >= (orxS32)sizeof(PVRTexHeader))
    && (orxFile_Read(&stHeader, sizeof(PVRTexHeader), 1, pstFile) > 0))
    {
      /* Swaps the header's bytes to host format */
      stHeader.headerLength = CFSwapInt32LittleToHost(stHeader.headerLength);
      stHeader.height       = CFSwapInt32LittleToHost(stHeader.height);
      stHeader.width        = CFSwapInt32LittleToHost(stHeader.width);
      stHeader.numMipmaps   = CFSwapInt32LittleToHost(stHeader.numMipmaps);
      stHeader.flags        = CFSwapInt32LittleToHost(stHeader.flags);
      stHeader.dataLength   = CFSwapInt32LittleToHost(stHeader.dataLength);
      stHeader.bpp          = CFSwapInt32LittleToHost(stHeader.bpp);
      stHeader.bitmaskRed   = CFSwapInt32LittleToHost(stHeader.bitmaskRed);
      stHeader.bitmaskGreen = CFSwapInt32LittleToHost(stHeader.bitmaskGreen);
      stHeader.bitmaskBlue  = CFSwapInt32LittleToHost(stHeader.bitmaskBlue);
      stHeader.bitmaskAlpha = CFSwapInt32LittleToHost(stHeader.bitmaskAlpha);
      stHeader.pvrTag       = CFSwapInt32LittleToHost(stHeader.pvrTag);
      stHeader.numSurfs     = CFSwapInt32LittleToHost(stHeader.numSurfs);

      /* Is a valid PVR header? */
      if((gPVRTexIdentifier[0] == ((stHeader.pvrTag >>  0) & 0xFF))
      && (gPVRTexIdentifier[1] == ((stHeader.pvrTag >>  8) & 0xFF))
      && (gPVRTexIdentifier[2] == ((stHeader.pvrTag >> 16) & 0xFF))
      && (gPVRTexIdentifier[3] == ((stHeader.pvrTag >> 24) & 0xFF)))
      {
        orxS32  u32FormatFlags;
        orxU32  u32BPP;
        orxBOOL bHasAlpha, bCompressed, bValidInfo = orxTRUE;
        GLenum  eInternalFormat, eTextureType = 0;

        /* Gets format flags */
        u32FormatFlags = stHeader.flags & PVR_TEXTURE_FLAG_TYPE_MASK;

        /* Updates alpha info */
        bHasAlpha = (stHeader.bitmaskAlpha != 0) ? orxTRUE : orxFALSE;

        /* Depending on format */
        switch(u32FormatFlags)
        {
          case kPVRTextureFlagTypeOGLARGB4444:
          {
            /* Updates info */
            eInternalFormat = GL_UNSIGNED_SHORT_4_4_4_4;
            eTextureType    = GL_RGBA;
            u32BPP          = 16;
            bCompressed     = orxFALSE;

            break;
          }

          case kPVRTextureFlagTypeOGLARGB1555:
          {
            /* Updates info */
            eInternalFormat = GL_UNSIGNED_SHORT_5_5_5_1;
            eTextureType    = GL_RGBA;
            u32BPP          = 16;
            bCompressed     = orxFALSE;

            break;
          }

          case kPVRTextureFlagTypeOGLARGB8888:
          {
            /* Updates info */
            eInternalFormat = GL_UNSIGNED_BYTE;
            eTextureType    = GL_RGBA;
            u32BPP          = 32;
            bCompressed     = orxFALSE;

            break;
          }

          case kPVRTextureFlagTypeOGLRGB565:
          {
            /* Updates info */
            eInternalFormat = GL_UNSIGNED_SHORT_5_6_5;
            eTextureType    = GL_RGB;
            u32BPP          = 16;
            bCompressed     = orxFALSE;

            break;
          }

          case kPVRTextureFlagTypeOGLRGB888:
          {
            /* Updates info */
            eInternalFormat = GL_UNSIGNED_BYTE;
            eTextureType    = GL_RGB;
            u32BPP          = 24;
            bCompressed     = orxFALSE;

            break;
          }

          case kPVRTextureFlagTypePVRTC_2:
          {
            /* Updates info */
            eInternalFormat = (bHasAlpha != orxFALSE) ? GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            bCompressed     = orxTRUE;
            u32BPP          = 2;

            break;
          }

          case kPVRTextureFlagTypePVRTC_4:
          {
            /* Updates info */
            eInternalFormat = (bHasAlpha != orxFALSE) ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            bCompressed     = orxTRUE;
            u32BPP          = 4;

            break;
          }

          default:
          case kPVRTextureFlagTypeOGLRGB555:
          {
            /* Not supported */
            bValidInfo = orxFALSE;

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: invalid format, aborting.", _zFilename);

            break;
          }
        }

        /* Valid info? */
        if(bValidInfo != orxFALSE)
        {
          orxS32  u32DataSize;
          orxU8  *au8ImageBuffer;

          /* Gets the image size (eventual mipmaps will be ignored) */
          u32DataSize = (stHeader.width * stHeader.height * u32BPP) / 8;

          /* Allocates buffer */
          au8ImageBuffer = orxMemory_Allocate(u32DataSize, orxMEMORY_TYPE_VIDEO);

          /* Reads the image content (mimaps will be ignored) */
          if(orxFile_Read(au8ImageBuffer, sizeof(orxU8), u32DataSize, pstFile) > 0)
          {
            /* Allocates bitmap */
            pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

            /* Success? */
            if(pstBitmap != orxNULL)
            {
              GLint iTexture;

              /* Pushes config section */
              orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

              /* Inits bitmap */
              pstBitmap->bSmoothing     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
              pstBitmap->fWidth         = orxU2F(stHeader.width);
              pstBitmap->fHeight        = orxU2F(stHeader.height);
              pstBitmap->u32RealWidth   = stHeader.width;
              pstBitmap->u32RealHeight  = stHeader.height;
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
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
              glASSERT();
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
              glASSERT();
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
              glASSERT();
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
              glASSERT();

              /* Compressed? */
              if(bCompressed != orxFALSE)
              {
                /* Loads compressed data */
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, eInternalFormat, stHeader.width, stHeader.height, 0, u32DataSize, au8ImageBuffer);
              }
              else
              {
                /* Loads data */
                glTexImage2D(GL_TEXTURE_2D, 0, eTextureType, stHeader.width, stHeader.height, 0, eTextureType, eInternalFormat, au8ImageBuffer);
              }
              glASSERT();

              /* Restores previous texture */
              glBindTexture(GL_TEXTURE_2D, iTexture);
              glASSERT();

              /* Pops config section */
              orxConfig_PopSection();
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: out of memory, aborting.", _zFilename);
            }
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: invalid data, aborting.", _zFilename);
          }

          /* Frees data */
          orxMemory_Free(au8ImageBuffer);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: invalid pixel format, aborting.", _zFilename);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: invalid header format, aborting.", _zFilename);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: invalid file size, aborting.", _zFilename);
    }

    /* Closes file */
    orxFile_Close(pstFile);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: file not found, aborting.", _zFilename);
  }

  /* Done! */
  return pstBitmap;
}

static void orxFASTCALL orxDisplay_iOS_DrawPrimitive(orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill, orxBOOL _bOpen)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DrawPrimitive");

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Starts no texture shader */
    orxDisplay_StartShader((orxHANDLE)sstDisplay.pstNoTextureShader);

    /* Inits it */
    orxDisplay_iOS_InitShader((orxHANDLE)sstDisplay.pstNoTextureShader);
  }
  else
  {
    /* Disables texturing */
    glDisable(GL_TEXTURE_2D);
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

orxBITMAP *orxFASTCALL orxDisplay_iOS_GetScreenBitmap()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstDisplay.pstScreen;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  const orxCHAR    *pc;
  orxU32            u32CharacterCodePoint;
  GLfloat           fX, fY, fHeight;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_iOS_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* Gets character's height */
  fHeight = _pstMap->fCharacterHeight;

  /* Prepares font for drawing */
  orxDisplay_iOS_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode);

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
        orxFLOAT                  fWidth;

        /* Gets glyph from table */
        pstGlyph = (orxCHARACTER_GLYPH *)orxHashTable_Get(_pstMap->pstCharacterTable, u32CharacterCodePoint);

        /* Valid? */
        if(pstGlyph != orxNULL)
        {
          /* Gets character width */
          fWidth = pstGlyph->fWidth;

          /* End of buffer? */
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
          {
            /* Draw arrays */
            orxDisplay_iOS_DrawArrays();
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

orxSTATUS orxFASTCALL orxDisplay_iOS_DrawLine(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvStart != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

  /* Copies vertices */
  sstDisplay.astVertexList[0].fX = (GLfloat)(_pvStart->fX);
  sstDisplay.astVertexList[0].fY = (GLfloat)(_pvStart->fY);
  sstDisplay.astVertexList[1].fX = (GLfloat)(_pvEnd->fX);
  sstDisplay.astVertexList[1].fY = (GLfloat)(_pvEnd->fY);

  /* Copies color */
  sstDisplay.astVertexList[0].stRGBA =
  sstDisplay.astVertexList[1].stRGBA = _stColor;

  /* Draws it */
  orxDisplay_iOS_DrawPrimitive(2, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_DrawPolyline(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

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
  orxDisplay_iOS_DrawPrimitive(_u32VertexNumber, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_DrawPolygon(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

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
  orxDisplay_iOS_DrawPrimitive(_u32VertexNumber, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_DrawCircle(const orxVECTOR *_pvCenter, orxFLOAT _fRadius, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxFLOAT  fAngle;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvCenter != orxNULL);
  orxASSERT(_fRadius >= orxFLOAT_0);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

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
  orxDisplay_iOS_DrawPrimitive(orxDISPLAY_KU32_CIRCLE_LINE_NUMBER, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_DrawOBox(const orxOBOX *_pstBox, orxRGBA _stColor, orxBOOL _bFill)
{
  orxVECTOR vOrigin;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBox != orxNULL);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

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
  orxDisplay_iOS_DrawPrimitive(4, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_DrawMesh(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode, orxU32 _u32VertexNumber, const orxDISPLAY_VERTEX *_astVertexList)
{
  const orxBITMAP  *pstBitmap;
  GLfloat           fWidth, fHeight, fXCoef, fYCoef, fXBorder, fYBorder;
  orxU32            i, iIndex, u32VertexNumber = _u32VertexNumber;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32VertexNumber > 2);
  orxASSERT(_astVertexList != orxNULL);

  /* Gets bitmap to use */
  pstBitmap = (_pstBitmap != orxNULL) ? _pstBitmap : sstDisplay.pstLastBitmap;

  /* Prepares bitmap for drawing */
  orxDisplay_iOS_PrepareBitmap(pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = (GLfloat)(pstBitmap->stClip.vBR.fX - pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat)(pstBitmap->stClip.vBR.fY - pstBitmap->stClip.vTL.fY);

  /* Gets X & Y coefs */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))
  {
    fXCoef = fYCoef = orxFLOAT_1;
  }
  else
  {
    fXCoef = pstBitmap->fWidth * pstBitmap->fRecRealWidth;
    fYCoef = pstBitmap->fHeight * pstBitmap->fRecRealHeight;
  }

  /* Gets X & Y border fixes */
  fXBorder = pstBitmap->fRecRealWidth * orxDISPLAY_KF_BORDER_FIX;
  fYBorder = pstBitmap->fRecRealHeight * orxDISPLAY_KF_BORDER_FIX;

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex + (2 * _u32VertexNumber) > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
  {
    /* Draw arrays */
    orxDisplay_iOS_DrawArrays();

    /* Too many vertices? */
    if(_u32VertexNumber > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE / 2)
    {
      /* Updates vertex number */
      u32VertexNumber = orxDISPLAY_KU32_VERTEX_BUFFER_SIZE / 2;

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't draw full mesh: only drawing %d vertices out of %d.", u32VertexNumber, _u32VertexNumber);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* For all vertices */
  for(i = 0, iIndex = 0; i < u32VertexNumber; i++, iIndex++)
  {
    /* Copies position */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fX = _astVertexList[i].fX;
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fY = _astVertexList[i].fY;

    /* Updates UV */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fU = (GLfloat)(fXCoef * _astVertexList[i].fU + fXBorder);
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fV = (GLfloat)(orxFLOAT_1 - (fYCoef * _astVertexList[i].fV + fYBorder));

    /* Copies color */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].stRGBA = _astVertexList[i].stRGBA;

    /* Quad extremity? */
    if((i != 1) && ((i & 1) == 1))
    {
      /* Copies last two vertices */
      orxMemory_Copy(&(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex + 1]), &(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex - 1]), sizeof(orxDISPLAY_VERTEX));
      orxMemory_Copy(&(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex + 2]), &(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex]), sizeof(orxDISPLAY_VERTEX));

      /* Updates index */
      iIndex += 2;
    }
  }

  /* Odd number of vertices in the triangle strip? */
  if(iIndex & 1)
  {
    /* Completes the quad */
    orxMemory_Copy(&(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex]), &(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex - 1]), sizeof(orxDISPLAY_VERTEX));
    iIndex++;
  }

  /* Updates index */
  sstDisplay.s32BufferIndex += iIndex;

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_iOS_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
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

orxBITMAP *orxFASTCALL orxDisplay_iOS_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
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
    pstBitmap->u32RealWidth   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Width : orxMath_GetNextPowerOfTwo(_u32Width);
    pstBitmap->u32RealHeight  = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Height : orxMath_GetNextPowerOfTwo(_u32Height);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
    glASSERT();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

orxSTATUS orxFASTCALL orxDisplay_iOS_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
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

    /* Has depth buffer? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER))
    {
      /* Clears depth buffer */
      glClear(GL_DEPTH_BUFFER_BIT);
      glASSERT();
    }

    /* Clears the color buffer with given color */
    glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
    glASSERT();
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

  /* Swaps */
  [sstDisplay.poView Swap];

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
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
    orxU8  *au8ImageBuffer;
    orxU32  i, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset;

    /* Allocates buffer */
    au8ImageBuffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_VIDEO);

    /* Gets line sizes */
    u32LineSize     = orxF2U(_pstBitmap->fWidth) * sizeof(orxRGBA);
    u32RealLineSize = _pstBitmap->u32RealWidth * sizeof(orxRGBA);

    /* Clears padding */
    orxMemory_Zero(au8ImageBuffer, u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)));

    /* For all lines */
    for(i = 0, u32SrcOffset = 0, u32DstOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - 1);
        i < u32Height;
        i++, u32SrcOffset += u32LineSize, u32DstOffset -= u32RealLineSize)
    {
      /* Copies data */
      orxMemory_Copy(au8ImageBuffer + u32DstOffset, _au8Data + u32SrcOffset, u32LineSize);

      /* Adds padding */
      orxMemory_Zero(au8ImageBuffer + u32DstOffset + u32LineSize, u32RealLineSize - u32LineSize);
    }

    /* Backups current texture */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
    glASSERT();

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, au8ImageBuffer);
    glASSERT();

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, iTexture);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(au8ImageBuffer);

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

orxSTATUS orxFASTCALL orxDisplay_iOS_GetBitmapData(orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
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

orxSTATUS orxFASTCALL orxDisplay_iOS_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
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

orxRGBA orxFASTCALL orxDisplay_iOS_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = orx2RGBA(0, 0, 0, 0);

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

orxSTATUS orxFASTCALL orxDisplay_iOS_SetDestinationBitmap(orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Different destination bitmap? */
  if(_pstBitmap != sstDisplay.pstDestinationBitmap)
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

    /* Stores it */
    sstDisplay.pstDestinationBitmap = _pstBitmap;

    /* Is valid? */
    if(_pstBitmap != orxNULL)
    {
      /* Sets OpenGL context */
      [EAGLContext setCurrentContext:sstDisplay.poView.poThreadContext];

      /* Recreates render target */
      [sstDisplay.poView CreateRenderTarget:_pstBitmap];

      /* Is screen? */
      if(sstDisplay.pstDestinationBitmap == sstDisplay.pstScreen)
      {
        /* Flushes pending commands */
        glFlush();
        glASSERT();

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

      /* Shader support? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
      {
        /* Inits projection matrix */
        orxDisplay_iOS_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.pstDestinationBitmap->fWidth, sstDisplay.pstDestinationBitmap->fHeight, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1);

        /* Passes it to shader */
        glUniformMatrix4fv(sstDisplay.pstDefaultShader->uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
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
    else
    {
     /* Updates result */
     eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

orxU32 orxFASTCALL orxDisplay_iOS_GetBitmapID(const orxBITMAP *_pstBitmap)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* Updates result */
  u32Result = (orxU32)_pstBitmap->uiTexture;

  /* Done! */
  return u32Result;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_iOS_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* No repeat? */
  if((_pstTransform->fRepeatX == orxFLOAT_1) && (_pstTransform->fRepeatY == orxFLOAT_1))
  {
    /* Draws it */
    orxDisplay_iOS_DrawBitmap(_pstSrc, &mTransform, _eSmoothing, _eBlendMode);
  }
  else
  {
    orxFLOAT  i, j, fRecRepeatX;
    GLfloat   fX, fY, fWidth, fHeight, fTop, fBottom, fLeft, fRight;

    /* Prepares bitmap for drawing */
    orxDisplay_iOS_PrepareBitmap(_pstSrc, _eSmoothing, _eBlendMode);

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
          orxDisplay_iOS_DrawArrays();
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

orxSTATUS orxFASTCALL orxDisplay_iOS_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
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
        oContext = CGBitmapContextCreate(au8ImageBuffer, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, 8, 4 * _pstBitmap->u32RealWidth, CGImageGetColorSpace(oImage), kCGImageAlphaLast | kCGBitmapByteOrder32Big);

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
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save bitmap to <%s>: couldn't grab bitmap data.", _zFilename);

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
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save bitmap to <%s>: couldn't allocate memory buffers.", _zFilename);

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

orxBITMAP *orxFASTCALL orxDisplay_iOS_LoadBitmap(const orxSTRING _zFilename)
{
  NSString   *poName;
  orxBITMAP  *pstBitmap = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets NSString */
  poName = [NSString stringWithCString:_zFilename encoding:NSASCIIStringEncoding];

  /* PVR texture? */
  if([[poName pathExtension] isEqualToString:@"pvr"] != NO)
  {
    /* Has support? */
    if([sstDisplay.poView bCompressedTextureSupport] != NO)
    {
      /* Loads texture */
      pstBitmap = orxDisplay_iOS_LoadPVRBitmap(_zFilename);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: no PVR support on this device. Retrying with PNG extension instead.", _zFilename);

      /* Defaults back to png */
      poName = [[poName stringByDeletingPathExtension] stringByAppendingPathExtension:@".png"];
    }
  }

  /* Not already loaded? */
  if(pstBitmap == orxNULL)
  {
    NSData   *poData;
    UIImage  *poSourceImage;

    /* Loads the file content */
    poData = [[NSData alloc] initWithContentsOfFile:poName];

    /* Gets image from it */
    poSourceImage = [[UIImage alloc] initWithData:poData];

    /* Success? */
    if(poSourceImage != nil)
    {
      GLuint      uiWidth, uiHeight, uiRealWidth, uiRealHeight;
      GLubyte    *au8ImageBuffer;
      CGImageRef  oImage;

      /* Gets image reference */
      oImage = poSourceImage.CGImage;

      /* Gets its size */
      uiWidth   = CGImageGetWidth(oImage);
      uiHeight  = CGImageGetHeight(oImage);

      /* Gets its real size */
      uiRealWidth   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? uiWidth : orxMath_GetNextPowerOfTwo(uiWidth);
      uiRealHeight  = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? uiHeight : orxMath_GetNextPowerOfTwo(uiHeight);

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
          CGColorSpaceRef   oColorSpace;
          CGContextRef      oContext;
          GLint             iTexture;
          orxRGBA          *pstPixel, *pstImageEnd;

          /* Creates a device color space */
          oColorSpace = CGColorSpaceCreateDeviceRGB();

          /* Creates graphic context */
          oContext = CGBitmapContextCreate(au8ImageBuffer, uiRealWidth, uiRealHeight, 8, 4 * uiRealWidth, oColorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);

          /* Clears it */
          CGContextClearRect(oContext, CGRectMake(0, 0, uiRealWidth, uiRealHeight));

          /* Inits it (flip vertically) */
          CGContextTranslateCTM(oContext, 0.0f, uiHeight);
          CGContextScaleCTM(oContext, 1.0f, -1.0f);

          /* Copies image data */
          CGContextDrawImage(oContext, CGRectMake(0, 0, uiWidth, uiHeight), oImage);

          /* For all pixels */
          for(pstPixel = (orxRGBA *)au8ImageBuffer, pstImageEnd =
              pstPixel + (uiRealWidth * uiRealHeight);
              pstPixel < pstImageEnd;
              pstPixel++)
          {
            orxCOLOR stColor;

            /* Gets its color */
            orxColor_SetRGBA(&stColor, *pstPixel);

            /* Has alpha? */
            if(stColor.fAlpha > orxFLOAT_0)
            {
              /* Updates color components */
              orxVector_Divf(&(stColor.vRGB), &(stColor.vRGB), stColor.fAlpha);
            }

            /* Updates pixel */
            *pstPixel = orxColor_ToRGBA(&stColor);
          }

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
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glASSERT();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glASSERT();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
          glASSERT();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
          glASSERT();
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, au8ImageBuffer);
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

      /* Releases source image */
      [poSourceImage release];
    }

    /* Releases texture data */
    [poData release];
  }

  /* Done! */
  return pstBitmap;
}


orxSTATUS orxFASTCALL orxDisplay_iOS_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
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

orxSTATUS orxFASTCALL orxDisplay_iOS_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
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

orxSTATUS orxFASTCALL orxDisplay_iOS_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Screen? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

    /* Is screen? */
    if(sstDisplay.pstDestinationBitmap == sstDisplay.pstScreen)
    {
      /* Sets OpenGL clipping */
      glScissor((GLint)_u32TLX, (GLint)(orxF2U(sstDisplay.pstDestinationBitmap->fHeight) - _u32BRY), (GLsizei)(_u32BRX - _u32TLX), (GLsizei)(_u32BRY - _u32TLY));
      glASSERT();
    }
    else
    {
      /* Sets OpenGL clipping */
      glScissor((GLint)_u32TLX, (GLint)(sstDisplay.pstDestinationBitmap->u32RealHeight - _u32BRY), (GLsizei)(_u32BRX - _u32TLX), (GLsizei)(_u32BRY - _u32TLY));
      glASSERT();
    }
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

orxSTATUS orxFASTCALL orxDisplay_iOS_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iOS_IsVSyncEnabled()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetFullScreen(orxBOOL _bFullScreen)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_iOS_IsFullScreen()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxU32 orxFASTCALL orxDisplay_iOS_GetVideoModeCounter()
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_iOS_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
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

orxBOOL orxFASTCALL orxDisplay_iOS_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_Init()
{
  orxSTATUS eResult;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    orxU32 i;
    GLushort u16Index;

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* Registers update function */
    eResult = orxClock_Register(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxDisplay_iOS_Update, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGHEST);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
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
        orxDISPLAY_EVENT_PAYLOAD  stPayload;
        GLint                     iWidth, iHeight;

        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Stores view instance */
        sstDisplay.poView = [orxView GetInstance];

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

        /* Creates OpenGL thread context */
        [sstDisplay.poView CreateThreadContext];

        /* Has NPOT texture support? */
        if(([sstDisplay.poView bShaderSupport] != NO) || ([sstDisplay.poView IsExtensionSupported:@"GL_APPLE_texture_2D_limited_npot"] != NO))
        {
          /* Updates status flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT, orxDISPLAY_KU32_STATIC_FLAG_NONE);
        }
        else
        {
          /* Updates status flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_NPOT);
        }

        /* Gets render buffer's size */
        glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &iWidth);
        glASSERT();
        glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &iHeight);
        glASSERT();

        /* Inits default values */
        sstDisplay.bDefaultSmoothing          = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
        sstDisplay.pstScreen                  = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);
        orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));
        sstDisplay.pstScreen->fWidth          = iWidth;
        sstDisplay.pstScreen->fHeight         = iHeight;
        sstDisplay.pstScreen->u32RealWidth    = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? orxF2U(sstDisplay.pstScreen->fWidth) : orxMath_GetNextPowerOfTwo(orxF2U(sstDisplay.pstScreen->fWidth));
        sstDisplay.pstScreen->u32RealHeight   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? orxF2U(sstDisplay.pstScreen->fHeight) : orxMath_GetNextPowerOfTwo(orxF2U(sstDisplay.pstScreen->fHeight));
        sstDisplay.pstScreen->fRecRealWidth   = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
        sstDisplay.pstScreen->fRecRealHeight  = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
        orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);
        sstDisplay.eLastBlendMode             = orxDISPLAY_BLEND_MODE_NUMBER;
        sstDisplay.dTouchTimeCorrection       = orxSystem_GetTime() - orx2D([[NSProcessInfo processInfo] systemUptime]);
        sstDisplay.u32EventInfoNumber         = 0;

        /* Updates config info */
        orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_WIDTH, sstDisplay.pstScreen->fWidth);
        orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_HEIGHT, sstDisplay.pstScreen->fHeight);
        orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_DEPTH, 32);

        /* Pops config section */
        orxConfig_PopSection();

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
          static const orxSTRING szNoTextureFragmentShaderSource =
          "precision mediump float;"
          "varying vec2 ___TexCoord___;"
          "varying vec4 ___Color___;"
          "uniform sampler2D __Texture__;"
          "void main()"
          "{"
          "  gl_FragColor = ___Color___;"
          "}";

          /* Inits flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER | orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);

          /* Creates texture for screen backup */
          glGenTextures(1, &(sstDisplay.pstScreen->uiTexture));
          glASSERT();
          glBindTexture(GL_TEXTURE_2D, sstDisplay.pstScreen->uiTexture);
          glASSERT();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glASSERT();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glASSERT();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
          glASSERT();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
          glASSERT();
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
          glASSERT();

          /* Creates default shaders */
          sstDisplay.pstDefaultShader   = orxDisplay_CreateShader(szFragmentShaderSource, orxNULL);
          sstDisplay.pstNoTextureShader = orxDisplay_CreateShader(szNoTextureFragmentShaderSource, orxNULL);

          /* Uses it */
          orxDisplay_StopShader(orxNULL);
        }
        else
        {
          /* Inits flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);
        }

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

        /* Unregisters update function */
        orxClock_Unregister(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxDisplay_iOS_Update);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_iOS_Exit()
{
  /* Was initialized? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* Unregisters update function */
    orxClock_Unregister(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxDisplay_iOS_Update);

    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Deletes default shaders */
      orxDisplay_DeleteShader(sstDisplay.pstDefaultShader);
      orxDisplay_DeleteShader(sstDisplay.pstNoTextureShader);
    }

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

orxBOOL orxFASTCALL orxDisplay_iOS_HasShaderSupport()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER)) ? orxTRUE : orxFALSE;
}

orxHANDLE orxFASTCALL orxDisplay_iOS_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
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
        sstDisplay.acShaderCodeBuffer[0]  = sstDisplay.acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1] = orxCHAR_NULL;
        pc                                = sstDisplay.acShaderCodeBuffer;
        s32Free                           = orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1;

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
          orxString_NPrint(pc, s32Free, "%s\n", _zCode);

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
        pstShader->s32ParamCounter        = 0;
        pstShader->bActive                = orxFALSE;
        pstShader->bInitialized           = orxFALSE;
        pstShader->zCode                  = orxString_Duplicate(sstDisplay.acShaderCodeBuffer);
        pstShader->astTextureInfoList     = (orxDISPLAY_TEXTURE_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO), orxMEMORY_TYPE_MAIN);
        pstShader->astParamInfoList       = (orxDISPLAY_PARAM_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_PARAM_INFO), orxMEMORY_TYPE_MAIN);
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));
        orxMemory_Zero(pstShader->astParamInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_PARAM_INFO));

        /* Compiles it */
        if(orxDisplay_iOS_CompileShader(pstShader) != orxSTATUS_FAILURE)
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

void orxFASTCALL orxDisplay_iOS_DeleteShader(orxHANDLE _hShader)
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

orxSTATUS orxFASTCALL orxDisplay_iOS_StartShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Uses program */
  glUseProgram(pstShader->uiProgram);
  glASSERT();

  /* Updates projection matrix */
  glUniformMatrix4fv(pstShader->uiProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

  /* Updates its status */
  pstShader->bActive      = orxTRUE;
  pstShader->bInitialized = orxFALSE;

  /* Updates active shader counter */
  sstDisplay.s32ActiveShaderCounter++;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_StopShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_hShader != orxHANDLE_UNDEFINED);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has shader? */
  if(pstShader != orxNULL)
  {
    /* Wasn't initialized? */
    if(pstShader->bInitialized == orxFALSE)
    {
      /* Inits it */
      orxDisplay_iOS_InitShader(pstShader);

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
      orxDisplay_iOS_DrawArrays();
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

  /* Selects it */
  glActiveTexture(GL_TEXTURE0);
  glASSERT();

  /* Done! */
  return eResult;
}

orxS32 orxFASTCALL orxDisplay_iOS_GetParameterID(const orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture)
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

    /* Inits buffer */
    acBuffer[255] = orxCHAR_NULL;

    /* Gets corresponding param info */
    pstInfo = &pstShader->astParamInfoList[pstShader->s32ParamCounter];

    /* Updates result */
    s32Result = pstShader->s32ParamCounter++;

    /* Array? */
    if(_s32Index >= 0)
    {
      /* Prints its name */
      orxString_NPrint(acBuffer, 255, "%s[%ld]", _zParam, _s32Index);

      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, acBuffer);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)_zParam);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();
    }
  }
  else
  {
    /* Array? */
    if(_s32Index >= 0)
    {
      orxCHAR acBuffer[255];

      /* Prints its name */
      orxString_NPrint(acBuffer, 255, "%s[%ld]", _zParam, _s32Index);
      acBuffer[255] = orxCHAR_NULL;

      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocation(pstShader->uiProgram, acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocation(pstShader->uiProgram, (const GLchar *)_zParam);
      glASSERT();
    }
  }

  /* Done! */
  return s32Result;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue)
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

orxSTATUS orxFASTCALL orxDisplay_iOS_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue)
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

orxSTATUS orxFASTCALL orxDisplay_iOS_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue)
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetScreenBitmap, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetDestinationBitmap, DISPLAY, SET_DESTINATION_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetBitmapID, DISPLAY, GET_BITMAP_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DrawLine, DISPLAY, DRAW_LINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DrawPolyline, DISPLAY, DRAW_POLYLINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DrawPolygon, DISPLAY, DRAW_POLYGON);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DrawCircle, DISPLAY, DRAW_CIRCLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DrawOBox, DISPLAY, DRAW_OBOX);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DrawMesh, DISPLAY, DRAW_MESH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_HasShaderSupport, DISPLAY, HAS_SHADER_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetParameterID, DISPLAY, GET_PARAMETER_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
