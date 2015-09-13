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
 * @file orxDisplay.m
 * @date 23/01/2010
 * @author iarwain@orx-project.org
 *
 * iOS display plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include "webp/decode.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#if defined(__orxARM__)
  #define STBI_NEON
#endif /* __orxARM__ */
#include "stb_image.h"
#if defined(__orxARM__)
  #undef STBI_NEON
#endif /* __orxARM__ */
#undef STBI_NO_PNM
#undef STBI_NO_PIC
#undef STBI_NO_HDR
#undef STBI_NO_GIF
#undef STBI_NO_PSD
#undef STB_IMAGE_IMPLEMENTATION
#undef STBI_NO_STDIO

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#import <QuartzCore/QuartzCore.h>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000  /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001  /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHADER      0x00000002  /**< Shader support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER 0x00000004  /**< Has depth buffer support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_NPOT        0x00000008  /**< NPOT texture support flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_FLAG_NONE        0x00000000  /** No flags */

#define orxDISPLAY_KU32_BITMAP_FLAG_LOADING     0x00000001  /**< Loading flag */
#define orxDISPLAY_KU32_BITMAP_FLAG_DELETE      0x00000002  /**< Delete flag */

#define orxDISPLAY_KU32_BITMAP_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        128
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        16

#define orxDISPLAY_KU32_VERTEX_BUFFER_SIZE      (4 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_INDEX_BUFFER_SIZE       (6 * 1024)  /**< 1024 items batch capacity */
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      65536

#define orxDISPLAY_KF_BORDER_FIX                0.1f

#define orxDISPLAY_KU32_TOUCH_NUMBER            16

#define orxDISPLAY_KU32_EVENT_INFO_NUMBER       32

#define orxDISPLAY_KU32_CIRCLE_LINE_NUMBER      32

#define orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER 32


/**  Misc defines
 */
#define orxDISPLAY_KU32_PVR_TEXTURE_MASK_TYPE   0xFF

#define glUNIFORM(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__); glASSERT();}} while(orxFALSE)

#ifdef __orxDEBUG__

#define glASSERT()                                                      \
do                                                                      \
{                                                                       \
  GLenum eError = glGetError();                                         \
  orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: 0x%X", eError);\
} while(orxFALSE)

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__); glGetError();}} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__);}} while(orxFALSE)

#endif /* __orxDEBUG__ */

//If the symbol for iOS 8 isnt defined, define it.
#ifndef NSFoundationVersionNumber_iOS_8_0
#define NSFoundationVersionNumber_iOS_8_0 1134.10 //extracted with NSLog(@"%f", NSFoundationVersionNumber)
#endif

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

/** Internal vertex structure
 */
typedef struct __orxDISPLAY_IOS_VERTEX_t
{
  GLfloat fX, fY;
  GLfloat fU, fV;
  orxRGBA stRGBA;

} orxDISPLAY_IOS_VERTEX;

/** Internal bitmap structure
 */
struct __orxBITMAP_t
{
  GLuint                    uiTexture;
  orxBOOL                   bSmoothing;
  orxFLOAT                  fWidth, fHeight;
  orxAABOX                  stClip;
  orxU32                    u32RealWidth, u32RealHeight, u32Depth;
  orxFLOAT                  fRecRealWidth, fRecRealHeight;
  orxU32                    u32DataSize;
  orxRGBA                   stColor;
  const orxSTRING           zLocation;
  orxU32                    u32FilenameID;
  orxU32                    u32Flags;
};

/** Internal bitmap save info structure
 */
typedef struct __orxDISPLAY_SAVE_INFO_t
{
  orxU8  *pu8ImageData;
  orxU32  u32Width;
  orxU32  u32Height;
  orxU32  u32FilenameID;

} orxDISPLAY_SAVE_INFO;

/** Internal bitmap load info structure
 */
typedef struct __orxDISPLAY_LOAD_INFO_t
{
  orxU8      *pu8ImageBuffer;
  orxU8      *pu8ImageSource;
  orxS64      s64Size;
  orxBITMAP  *pstBitmap;
  orxU32      u32DataSize;
  GLuint      uiWidth;
  GLuint      uiHeight;
  GLuint      uiDepth;
  GLuint      uiRealWidth;
  GLuint      uiRealHeight;
  GLenum      eTextureType;
  GLenum      eInternalFormat;
  orxBOOL     bCompressed;
  orxBOOL     bIsPVRTC;

} orxDISPLAY_LOAD_INFO;

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
  orxLINKLIST_NODE          stNode;
  GLuint                    uiProgram;
  GLint                     iTextureLocation;
  GLint                     iProjectionMatrixLocation;
  GLint                     iTextureCounter;
  orxS32                    s32ParamCounter;
  orxBOOL                   bPending;
  orxBOOL                   bUseCustomParam;
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
  orxLINKLIST               stActiveShaderList;
  orxBOOL                   bDefaultSmoothing;
  orxBITMAP                *pstScreen;
  const orxBITMAP          *pstTempBitmap;
  orxBITMAP                *pstDestinationBitmap;
  orxRGBA                   stLastColor;
  orxU32                    u32LastClipX, u32LastClipY, u32LastClipWidth, u32LastClipHeight;
  orxDISPLAY_BLEND_MODE     eLastBlendMode;
  orxS32                    s32PendingShaderCounter;
  GLint                     iLastViewportX, iLastViewportY;
  GLsizei                   iLastViewportWidth, iLastViewportHeight;
  orxFLOAT                  fLastOrthoRight, fLastOrthoBottom;
  orxDISPLAY_SHADER        *pstDefaultShader;
  orxDISPLAY_SHADER        *pstNoTextureShader;
  GLuint                    uiIndexBuffer;
  GLint                     iTextureUnitNumber;
  orxS32                    s32BufferIndex;
  orxDOUBLE                 dTouchTimeCorrection;
  orxU32                    u32EventInfoNumber;
  orxView                  *poView;
  orxU32                    u32Flags;
  orxS32                    s32ActiveTextureUnit;
  stbi_io_callbacks         stSTBICallbacks;
  const orxBITMAP          *apstBoundBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxDOUBLE                 adMRUBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxDISPLAY_PROJ_MATRIX    mProjectionMatrix;
  orxDISPLAY_IOS_VERTEX     astVertexList[orxDISPLAY_KU32_VERTEX_BUFFER_SIZE];
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
static orxDISPLAY_STATIC    sstDisplay;

static orxCHAR              sacPVRTextureTag[4] = "PVR!";


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Prototypes
 */
orxSTATUS orxFASTCALL orxDisplay_iOS_StartShader(orxHANDLE _hShader);
orxSTATUS orxFASTCALL orxDisplay_iOS_StopShader(orxHANDLE _hShader);
orxSTATUS orxFASTCALL orxDisplay_iOS_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode);
orxSTATUS orxFASTCALL orxDisplay_iOS_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number);
orxSTATUS orxFASTCALL orxDisplay_iOS_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode);


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
  if((NSFoundationVersionNumber < NSFoundationVersionNumber_iOS_8_0) && UIInterfaceOrientationIsLandscape(eOrientation))
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
  /* Is initialized? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    /* Not overflowing? */
    if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
    {
      orxSYSTEM_EVENT_PAYLOAD stPayload;

      /* Inits payload */
      orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
      stPayload.stTouch.fPressure = orxFLOAT_1;

      /* Updates it */
      stPayload.stAccelerometer.dTime = _poAcceleration.timestamp + sstDisplay.dTouchTimeCorrection;
      orxVector_Set(&(stPayload.stAccelerometer.vAcceleration), orx2F(_poAcceleration.x), orx2F(-_poAcceleration.y), orx2F(-_poAcceleration.z));

      /* Queues event */
      [self QueueEvent:orxSYSTEM_EVENT_ACCELERATE WithPayload:&stPayload];
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping accelerometer event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
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
    /* Wasn't already bound? */
    if(sstDisplay.pstDestinationBitmap != sstDisplay.pstScreen)
    {
      /* Binds screen frame buffer */
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiScreenFrameBuffer);
      glASSERT();
    }

    /* Updates result */
    bResult = (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES) ? YES : NO;
    glASSERT();
  }
  else
  {
    /* Wasn't already bound? */
    if((sstDisplay.pstDestinationBitmap == sstDisplay.pstScreen)
    || (sstDisplay.pstDestinationBitmap == orxNULL))
    {
      /* Binds texture frame buffer */
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, uiTextureFrameBuffer);
      glASSERT();
    }

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

- (void) QueueEvent:(orxENUM)_ID WithPayload:(orxSYSTEM_EVENT_PAYLOAD *)_pstPayload
{
  @synchronized(self)
  {
    orxSYSTEM_EVENT_PAYLOAD *pstPayload;

    /* Gets stored payload */
    pstPayload = &(sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber].stPayload);

    /* Inits it */
    if(_pstPayload != nil)
    {
      orxMemory_Copy(pstPayload, _pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
    }
    else
    {
      orxMemory_Zero(pstPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
    }

    /* Stores event info */
    sstDisplay.astEventInfoList[sstDisplay.u32EventInfoNumber++].eID = _ID;
  }
}

- (void) touchesBegan:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
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
        CGPoint                 vViewPosition;
        orxSYSTEM_EVENT_PAYLOAD stPayload;
        orxU32                  u32ID;

        /* Finds first empty slot */
        for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != nil); u32ID++);

        /* Checks */
        orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

        /* Stores touch */
        sstDisplay.astTouchInfoList[u32ID].poTouch = poTouch;

        /* Gets its position inside view */
        vViewPosition = [poTouch locationInView:self];

        /* Inits payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_1;

        /* Updates it */
        stPayload.stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
        stPayload.stTouch.u32ID = u32ID;
        stPayload.stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
        stPayload.stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

        /* Queues event */
        [self QueueEvent:orxSYSTEM_EVENT_TOUCH_BEGIN WithPayload:&stPayload];
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch begin event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
      }
    }
  }

  /* Done! */
  return;
}

- (void) touchesMoved:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
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
        CGPoint                 vViewPosition;
        orxSYSTEM_EVENT_PAYLOAD stPayload;
        orxU32                  u32ID;

        /* Finds corresponding slot */
        for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != poTouch); u32ID++);

        /* Checks */
        orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

        /* Gets its position inside view */
        vViewPosition = [poTouch locationInView:self];

        /* Inits payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_1;

        /* Updates it */
        stPayload.stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
        stPayload.stTouch.u32ID = u32ID;
        stPayload.stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
        stPayload.stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

        /* Queues event */
        [self QueueEvent:orxSYSTEM_EVENT_TOUCH_MOVE WithPayload:&stPayload];
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch move event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
      }
    }
  }

  /* Done! */
  return;
}

- (void) touchesEnded:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
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
        CGPoint                 vViewPosition;
        orxSYSTEM_EVENT_PAYLOAD stPayload;
        orxU32                  u32ID;

        /* Finds corresponding slot */
        for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != poTouch); u32ID++);

        /* Checks */
        orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

        /* Removes touch */
        sstDisplay.astTouchInfoList[u32ID].poTouch = nil;

        /* Gets its position inside view */
        vViewPosition = [poTouch locationInView:self];

        /* Inits payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_0;

        /* Updates it */
        stPayload.stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
        stPayload.stTouch.u32ID = u32ID;
        stPayload.stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
        stPayload.stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

        /* Queues event */
        [self QueueEvent:orxSYSTEM_EVENT_TOUCH_END WithPayload:&stPayload];
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch end event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
      }
    }
  }

  /* Done! */
  return;
}

- (void) touchesCancelled:(NSSet *)_poTouchList withEvent:(UIEvent *)_poEvent
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
        CGPoint                 vViewPosition;
        orxSYSTEM_EVENT_PAYLOAD stPayload;
        orxU32                  u32ID;

        /* Finds corresponding slot */
        for(u32ID = 0; (u32ID < orxDISPLAY_KU32_TOUCH_NUMBER) && (sstDisplay.astTouchInfoList[u32ID].poTouch != poTouch); u32ID++);

        /* Checks */
        orxASSERT(u32ID < orxDISPLAY_KU32_TOUCH_NUMBER);

        /* Removes touch */
        sstDisplay.astTouchInfoList[u32ID].poTouch = nil;

        /* Gets its position inside view */
        vViewPosition = [poTouch locationInView:self];

        /* Inits payload */
        orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
        stPayload.stTouch.fPressure = orxFLOAT_0;

        /* Updates it */
        stPayload.stTouch.dTime = poTouch.timestamp + sstDisplay.dTouchTimeCorrection;
        stPayload.stTouch.u32ID = u32ID;
        stPayload.stTouch.fX    = orx2F(self.contentScaleFactor * vViewPosition.x);
        stPayload.stTouch.fY    = orx2F(self.contentScaleFactor * vViewPosition.y);

        /* Queues event */
        [self QueueEvent:orxSYSTEM_EVENT_TOUCH_END WithPayload:&stPayload];
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping touch cancel event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
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
    /* Is initialized? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY))
    {
      /* Not overflowing? */
      if(sstDisplay.u32EventInfoNumber < orxDISPLAY_KU32_EVENT_INFO_NUMBER)
      {
        /* Queues event */
        [self QueueEvent:orxSYSTEM_EVENT_MOTION_SHAKE WithPayload:nil];
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Too many touch/accelerometer events received this frame (limit is %d), dropping motion event.", orxDISPLAY_KU32_EVENT_INFO_NUMBER);
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

static orxINLINE void orxDisplay_iOS_BindBitmap(const orxBITMAP *_pstBitmap)
{
  orxDOUBLE dBestTime;
  orxS32    i, s32BestCandidate;

  /* For all texture units */
  for(i = 0, s32BestCandidate = -1, dBestTime = orxDOUBLE_MAX; i < (orxS32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Found? */
    if(sstDisplay.apstBoundBitmapList[i] == _pstBitmap)
    {
      /* Stops */
      break;
    }
    /* Is first empty? */
    else if((dBestTime != orxDOUBLE_0) && (sstDisplay.apstBoundBitmapList[i] == orxNULL))
    {
      /* Selects it */
      s32BestCandidate = i;
      dBestTime = orxDOUBLE_0;
    }
    /* Older candidate? */
    else if(sstDisplay.adMRUBitmapList[i] < dBestTime)
    {
      /* Selects it */
      s32BestCandidate = i;
      dBestTime = sstDisplay.adMRUBitmapList[i];
    }
  }

  /* Found? */
  if(i < (orxS32)sstDisplay.iTextureUnitNumber)
  {
    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Selects unit */
      glActiveTexture(GL_TEXTURE0 + i);
      glASSERT();
    }

    /* Updates MRU timestamp */
    sstDisplay.adMRUBitmapList[i] = orxSystem_GetSystemTime();

    /* Updates active texture unit */
    sstDisplay.s32ActiveTextureUnit = i;
  }
  else
  {
    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Selects unit */
      glActiveTexture(GL_TEXTURE0 + s32BestCandidate);
      glASSERT();
    }

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Stores texture */
    sstDisplay.apstBoundBitmapList[s32BestCandidate] = _pstBitmap;

    /* Updates MRU timestamp */
    sstDisplay.adMRUBitmapList[s32BestCandidate] = orxSystem_GetSystemTime();

    /* Updates active texture unit */
    sstDisplay.s32ActiveTextureUnit = s32BestCandidate;
  }

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

static orxSTATUS orxFASTCALL orxDisplay_iOS_GetPVRTCInfo(orxHANDLE _hResource, int *_piWidth, int *_piHeight)
{
  PVRTexHeader  stHeader;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Loads PVR header from file */
  if(orxResource_Read(_hResource, sizeof(PVRTexHeader), &stHeader, orxNULL, orxNULL) == sizeof(PVRTexHeader))
  {
    orxU32 *pu32;

    /* Swaps the header's bytes to host format */
    for(pu32 = (orxU32 *)&stHeader; pu32 < (orxU32 *)&stHeader + sizeof(PVRTexHeader); pu32++)
    {
      *pu32 = CFSwapInt32LittleToHost(*pu32);
    }

    /* Is a valid PVR header? */
    if((sacPVRTextureTag[0] == ((stHeader.pvrTag >>  0) & 0xFF))
    && (sacPVRTextureTag[1] == ((stHeader.pvrTag >>  8) & 0xFF))
    && (sacPVRTextureTag[2] == ((stHeader.pvrTag >> 16) & 0xFF))
    && (sacPVRTextureTag[3] == ((stHeader.pvrTag >> 24) & 0xFF)))
    {
      /* Stores dimensions */
      *_piWidth   = (int)stHeader.width;
      *_piHeight  = (int)stHeader.height;

      /* Updates result */
      eResult = orxTRUE;
    }
    else
    {
      /* Resets resource cursor */
      orxResource_Seek(_hResource, 0, orxSEEK_OFFSET_WHENCE_START);
    }
  }

  /* Done! */
  return eResult;
}

static int orxDisplay_iOS_ReadSTBICallback(void *_hResource, char *_pBuffer, int _iSize)
{
  /* Reads data */
  return (int)orxResource_Read((orxHANDLE)_hResource, _iSize, (orxU8 *)_pBuffer, orxNULL, orxNULL);
}

static void orxDisplay_iOS_SkipSTBICallback(void *_hResource, int _iOffset)
{
  /* Seeks offset */
  orxResource_Seek((orxHANDLE)_hResource, _iOffset, orxSEEK_OFFSET_WHENCE_CURRENT);

  /* Done! */
  return;
}

static int orxDisplay_iOS_EOFSTBICallback(void *_hResource)
{
  /* End of buffer? */
  return (orxResource_Tell((orxHANDLE)_hResource) == orxResource_GetSize(_hResource)) ? 1 : 0;
}

static orxSTATUS orxFASTCALL orxDisplay_iOS_DecompressBitmapCallback(void *_pContext)
{
  orxDISPLAY_EVENT_PAYLOAD  stPayload;
  orxDISPLAY_LOAD_INFO     *pstInfo;
  orxU32                    i;
  orxSTATUS                 eResult = orxSTATUS_SUCCESS;

  /* Gets load info */
  pstInfo = (orxDISPLAY_LOAD_INFO *)_pContext;

  /* Inits bitmap */
  pstInfo->pstBitmap->fWidth         = orxU2F(pstInfo->uiWidth);
  pstInfo->pstBitmap->fHeight        = orxU2F(pstInfo->uiHeight);
  pstInfo->pstBitmap->u32RealWidth   = (orxU32)pstInfo->uiRealWidth;
  pstInfo->pstBitmap->u32RealHeight  = (orxU32)pstInfo->uiRealHeight;
  pstInfo->pstBitmap->u32Depth       = (orxU32)pstInfo->uiDepth;
  pstInfo->pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstInfo->pstBitmap->u32RealWidth);
  pstInfo->pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstInfo->pstBitmap->u32RealHeight);
  pstInfo->pstBitmap->u32DataSize    = pstInfo->u32DataSize;
  orxVector_Copy(&(pstInfo->pstBitmap->stClip.vTL), &orxVECTOR_0);
  orxVector_Set(&(pstInfo->pstBitmap->stClip.vBR), pstInfo->pstBitmap->fWidth, pstInfo->pstBitmap->fHeight, orxFLOAT_0);

  /* Tracks video memory */
  orxMEMORY_TRACK(VIDEO, pstInfo->pstBitmap->u32DataSize, orxTRUE);

  /* Creates new texture */
  glGenTextures(1, &(pstInfo->pstBitmap->uiTexture));
  glASSERT();
  glBindTexture(GL_TEXTURE_2D, pstInfo->pstBitmap->uiTexture);
  glASSERT();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glASSERT();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glASSERT();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstInfo->pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
  glASSERT();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstInfo->pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
  glASSERT();

  /* Compressed? */
  if(pstInfo->bCompressed != orxFALSE)
  {
    /* Loads compressed data */
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, pstInfo->eInternalFormat, (GLsizei)pstInfo->uiRealWidth, (GLsizei)pstInfo->uiRealHeight, 0, (GLsizei)pstInfo->u32DataSize, pstInfo->pu8ImageBuffer);
  }
  else
  {
    /* Loads data */
    glTexImage2D(GL_TEXTURE_2D, 0, pstInfo->eInternalFormat, (GLsizei)pstInfo->uiRealWidth, (GLsizei)pstInfo->uiRealHeight, 0, pstInfo->eInternalFormat, pstInfo->eTextureType, pstInfo->pu8ImageBuffer);
  }
  glASSERT();

  /* Restores previous texture */
  glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
  glASSERT();

  /* For all bound bitmaps */
  for(i = 0; i < (orxU32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Is decompressed bitmap? */
    if(sstDisplay.apstBoundBitmapList[i] == pstInfo->pstBitmap)
    {
      /* Resets it */
      sstDisplay.apstBoundBitmapList[i] = orxNULL;
      sstDisplay.adMRUBitmapList[i]     = orxDOUBLE_0;
    }
  }

  /* Is PVRTC? */
  if(pstInfo->bIsPVRTC != orxFALSE)
  {
    /* Frees source */
    orxMemory_Free(pstInfo->pu8ImageSource);
    pstInfo->pu8ImageSource = orxNULL;
    pstInfo->pu8ImageBuffer = orxNULL;
  }
  else
  {
    /* Frees image buffer */
    if(pstInfo->pu8ImageBuffer != pstInfo->pu8ImageSource)
    {
      orxMemory_Free(pstInfo->pu8ImageBuffer);
    }
    pstInfo->pu8ImageBuffer = orxNULL;

    /* Frees source */
    stbi_image_free(pstInfo->pu8ImageSource);
    pstInfo->pu8ImageSource = orxNULL;
  }

  /* Inits payload */
  stPayload.stBitmap.zLocation      = pstInfo->pstBitmap->zLocation;
  stPayload.stBitmap.u32FilenameID  = pstInfo->pstBitmap->u32FilenameID;
  stPayload.stBitmap.u32ID          = (orxU32)pstInfo->pstBitmap->uiTexture;

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_LOAD_BITMAP, pstInfo->pstBitmap, orxNULL, &stPayload);

  /* Clears loading flag */
  orxFLAG_SET(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);
  orxMEMORY_BARRIER();

  /* Asked for deletion? */
  if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_DELETE))
  {
    /* Deletes it */
    orxDisplay_DeleteBitmap(pstInfo->pstBitmap);
  }

  /* Frees load info */
  orxMemory_Free(pstInfo);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxDisplay_iOS_DecompressBitmap(void *_pContext)
{
  PVRTexHeader         *pstHeader;
  orxDISPLAY_LOAD_INFO *pstInfo;
  orxSTATUS             eResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DecompressBitmap");

  /* Gets load info */
  pstInfo = (orxDISPLAY_LOAD_INFO *)_pContext;

  /* Gets PRVTC header */
  pstHeader = (PVRTexHeader *)pstInfo->pu8ImageSource;

  /* Is a valid PVR header? */
  if((sacPVRTextureTag[0] == ((pstHeader->pvrTag >>  0) & 0xFF))
  && (sacPVRTextureTag[1] == ((pstHeader->pvrTag >>  8) & 0xFF))
  && (sacPVRTextureTag[2] == ((pstHeader->pvrTag >> 16) & 0xFF))
  && (sacPVRTextureTag[3] == ((pstHeader->pvrTag >> 24) & 0xFF)))
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;

    /* Depending on its type */
    switch(pstHeader->flags & orxDISPLAY_KU32_PVR_TEXTURE_MASK_TYPE)
    {
      case kPVRTextureFlagTypeOGLARGB4444:
      {
        /* Updates info */
        pstInfo->uiDepth          = 16;
        pstInfo->eTextureType     = GL_UNSIGNED_SHORT_4_4_4_4;
        pstInfo->eInternalFormat  = GL_RGBA;
        pstInfo->bCompressed      = orxFALSE;

        break;
      }

      case kPVRTextureFlagTypeOGLARGB1555:
      {
        /* Updates info */
        pstInfo->uiDepth          = 16;
        pstInfo->eTextureType     = GL_UNSIGNED_SHORT_5_5_5_1;
        pstInfo->eInternalFormat  = GL_RGBA;
        pstInfo->bCompressed      = orxFALSE;

        break;
      }

      case kPVRTextureFlagTypeOGLARGB8888:
      {
        /* Updates info */
        pstInfo->uiDepth          = 32;
        pstInfo->eTextureType     = GL_UNSIGNED_BYTE;
        pstInfo->eInternalFormat  = GL_RGBA;
        pstInfo->bCompressed      = orxFALSE;

        break;
      }

      case kPVRTextureFlagTypeOGLRGB565:
      {
        /* Updates info */
        pstInfo->uiDepth          = 16;
        pstInfo->eTextureType     = GL_UNSIGNED_SHORT_5_6_5;
        pstInfo->eInternalFormat  = GL_RGB;
        pstInfo->bCompressed      = orxFALSE;

        break;
      }

      case kPVRTextureFlagTypeOGLRGB888:
      {
        /* Updates info */
        pstInfo->uiDepth          = 24;
        pstInfo->eTextureType     = GL_UNSIGNED_BYTE;
        pstInfo->eInternalFormat  = GL_RGB;
        pstInfo->bCompressed      = orxFALSE;

        break;
      }

      case kPVRTextureFlagTypePVRTC_2:
      {
        /* Updates info */
        pstInfo->uiDepth          = 2;
        pstInfo->eInternalFormat  = (pstHeader->bitmaskAlpha != 0) ? GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        pstInfo->bCompressed      = orxTRUE;

        break;
      }

      case kPVRTextureFlagTypePVRTC_4:
      {
        /* Updates info */
        pstInfo->uiDepth          = 4;
        pstInfo->eInternalFormat  = (pstHeader->bitmaskAlpha != 0) ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        pstInfo->bCompressed      = orxTRUE;

        break;
      }

      case kPVRTextureFlagTypeOGLRGB555:
      default:
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't load PVR texture <%s>: invalid format, aborting.", pstInfo->pstBitmap->zLocation);

        /* Asynchronous call? */
        if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't process data for bitmap <%s>: temp texture will remain in use.", pstInfo->pstBitmap->zLocation);
        }

        /* Clears loading flag */
        orxFLAG_SET(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);

        /* Frees original source from resource */
        orxMemory_Free(pstInfo->pu8ImageSource);
        pstInfo->pu8ImageSource = orxNULL;
        pstInfo->pu8ImageBuffer = orxNULL;

        /* Frees load info */
        orxMemory_Free(pstInfo);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;

        break;
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates its status */
        pstInfo->bIsPVRTC = orxTRUE;

        /* Uses source minus header as buffer */
        pstInfo->pu8ImageBuffer = pstInfo->pu8ImageSource + sizeof(PVRTexHeader);

        /* Stores its real size */
        pstInfo->uiRealWidth  = pstInfo->uiWidth;
        pstInfo->uiRealHeight = pstInfo->uiHeight;

        /* Stores its data size */
        pstInfo->u32DataSize = (orxU32)(pstInfo->uiWidth * pstInfo->uiHeight * pstInfo->uiDepth) / 8;
      }
    }
  }
  else
  {
    unsigned char *pu8ImageData;
    GLuint         uiBytesPerPixel;

    /* Updates its status */
    pstInfo->bIsPVRTC = orxFALSE;

    /* Loads image */
    pu8ImageData = stbi_load_from_memory((unsigned char *)pstInfo->pu8ImageSource, (int)pstInfo->s64Size, (int *)&(pstInfo->uiWidth), (int *)&(pstInfo->uiHeight), (int *)&uiBytesPerPixel, STBI_rgb_alpha);

    /* Valid? */
    if(pu8ImageData != NULL)
    {
      /* Checks */
      orxASSERT((uiBytesPerPixel == 3) || (uiBytesPerPixel == 4));

      /* Has NPOT texture support? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))
      {
        /* Uses image buffer */
        pstInfo->pu8ImageBuffer = pu8ImageData;

        /* Stores real size */
        pstInfo->uiRealWidth  = pstInfo->uiWidth;
        pstInfo->uiRealHeight = pstInfo->uiHeight;
      }
      else
      {
        GLuint i, uiSrcOffset, uiDstOffset, uiLineSize, uiRealLineSize;

        /* Gets real size */
        pstInfo->uiRealWidth  = (GLuint)orxMath_GetNextPowerOfTwo(pstInfo->uiWidth);
        pstInfo->uiRealHeight = (GLuint)orxMath_GetNextPowerOfTwo(pstInfo->uiHeight);

        /* Allocates buffer */
        pstInfo->pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(pstInfo->uiRealWidth * pstInfo->uiRealHeight * uiBytesPerPixel, orxMEMORY_TYPE_MAIN);

        /* Checks */
        orxASSERT(pstInfo->pu8ImageBuffer != orxNULL);

        /* Gets line sizes */
        uiLineSize      = pstInfo->uiWidth * uiBytesPerPixel;
        uiRealLineSize  = pstInfo->uiRealWidth * uiBytesPerPixel;

        /* Clears padding */
        orxMemory_Zero(pstInfo->pu8ImageBuffer, uiRealLineSize * (pstInfo->uiRealHeight - pstInfo->uiHeight));

        /* For all lines */
        for(i = 0, uiSrcOffset = 0, uiDstOffset = 0;
            i < pstInfo->uiHeight;
            i++, uiSrcOffset += uiLineSize, uiDstOffset += uiRealLineSize)
        {
          /* Copies data */
          orxMemory_Copy(pstInfo->pu8ImageBuffer + uiDstOffset, pu8ImageData + uiSrcOffset, uiLineSize);

          /* Adds padding */
          orxMemory_Zero(pstInfo->pu8ImageBuffer + uiDstOffset + uiLineSize, uiRealLineSize - uiLineSize);
        }
      }

      /* Frees original source from resource */
      orxMemory_Free(pstInfo->pu8ImageSource);

      /* Stores uncompressed data as new source */
      pstInfo->pu8ImageSource = pu8ImageData;

      /* Updates info */
      pstInfo->u32DataSize      = (orxU32)(pstInfo->uiRealWidth * pstInfo->uiRealHeight * uiBytesPerPixel);
      pstInfo->uiDepth          = uiBytesPerPixel * 8;
      pstInfo->eInternalFormat  = (uiBytesPerPixel == 4) ? GL_RGBA : GL_RGB;
      pstInfo->eTextureType     = GL_UNSIGNED_BYTE;
      pstInfo->bCompressed      = orxFALSE;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Asynchronous call? */
      if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't process data for bitmap <%s>: temp texture will remain in use.", pstInfo->pstBitmap->zLocation);
      }

      /* Clears loading flag */
      orxFLAG_SET(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);

      /* Frees original source from resource */
      orxMemory_Free(pstInfo->pu8ImageSource);
      pstInfo->pu8ImageSource = orxNULL;

      /* Frees load info */
      orxMemory_Free(pstInfo);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_iOS_ReadResourceCallback(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, void *_pContext)
{
  orxDISPLAY_LOAD_INFO *pstInfo;

  /* Allocates load info */
  pstInfo = (orxDISPLAY_LOAD_INFO *)orxMemory_Allocate(sizeof(orxDISPLAY_LOAD_INFO), orxMEMORY_TYPE_TEMP);

  /* Checks */
  orxASSERT(pstInfo != orxNULL);

  /* Inits it */
  orxMemory_Zero(pstInfo, sizeof(orxDISPLAY_LOAD_INFO));
  pstInfo->pu8ImageSource = (orxU8 *)_pBuffer;
  pstInfo->s64Size        = _s64Size;
  pstInfo->pstBitmap      = (orxBITMAP *)_pContext;

  /* Asynchronous? */
  if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
  {
    /* Runs asynchronous task */
    if(orxThread_RunTask(&orxDisplay_iOS_DecompressBitmap, orxDisplay_iOS_DecompressBitmapCallback, orxNULL, (void *)pstInfo) == orxSTATUS_FAILURE)
    {
      /* Frees load info */
      orxMemory_Free(pstInfo);
    }
  }
  else
  {
    /* Decompresses bitmap */
    if(orxDisplay_iOS_DecompressBitmap(pstInfo) != orxSTATUS_FAILURE)
    {
      /* Upload texture */
      orxDisplay_iOS_DecompressBitmapCallback(pstInfo);
    }
  }

  /* Closes resource */
  orxResource_Close(_hResource);
}

static orxSTATUS orxFASTCALL orxDisplay_iOS_SaveBitmapData(void *_pContext)
{
  orxDISPLAY_SAVE_INFO *pstInfo;
  const orxCHAR        *zExtension;
  const orxSTRING       zFilename;
  orxU32                u32Length;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets save info */
  pstInfo = (orxDISPLAY_SAVE_INFO *)_pContext;

  /* Gets filename */
  zFilename = orxString_GetFromID(pstInfo->u32FilenameID);

  /* Gets file name's length */
  u32Length = orxString_GetLength(zFilename);

  /* Gets extension */
  zExtension = (u32Length > 3) ? zFilename + u32Length - 3 : orxSTRING_EMPTY;

  /* PNG? */
  if(orxString_ICompare(zExtension, "png") == 0)
  {
    /* Saves image to disk */
    eResult = stbi_write_png(zFilename, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData, 0) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  /* BMP? */
  else if(orxString_ICompare(zExtension, "bmp") == 0)
  {
    /* Saves image to disk */
    eResult = stbi_write_bmp(zFilename, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  /* TGA */
  else
  {
    /* Saves image to disk */
    eResult = stbi_write_tga(zFilename, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Deletes data */
  orxMemory_Free(pstInfo->pu8ImageData);

  /* Deletes save info */
  orxMemory_Free(pstInfo);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxDisplay_iOS_LoadBitmapData(orxBITMAP *_pstBitmap)
{
  orxHANDLE hResource;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Opens resource */
  hResource = orxResource_Open(_pstBitmap->zLocation, orxFALSE);

  /* Success? */
  if(hResource != orxHANDLE_UNDEFINED)
  {
    orxS64  s64Size;
    orxU8  *pu8Buffer;

    /* Gets its size */
    s64Size = orxResource_GetSize(hResource);

    /* Checks */
    orxASSERT((s64Size > 0) && (s64Size < 0xFFFFFFFF));

    /* Allocates buffer */
    pu8Buffer = (orxU8 *)orxMemory_Allocate((orxU32)s64Size, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(pu8Buffer != orxNULL)
    {
      /* Asynchronous? */
      if(sstDisplay.pstTempBitmap != orxNULL)
      {
        int iWidth, iHeight, iComponent;

        /* Gets its info */
        if((orxDisplay_iOS_GetPVRTCInfo(hResource, &iWidth, &iHeight) != orxSTATUS_FAILURE)
        || (stbi_info_from_callbacks(&(sstDisplay.stSTBICallbacks), (void *)hResource, &iWidth, &iHeight, &iComponent) != 0))
        {
          /* Resets resource cursor */
          orxResource_Seek(hResource, 0, orxSEEK_OFFSET_WHENCE_START);

          /* Updates asynchronous loading flag */
          orxFLAG_SET(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING, orxDISPLAY_KU32_BITMAP_FLAG_NONE);

          /* Loads data from resource */
          s64Size = orxResource_Read(hResource, s64Size, pu8Buffer, orxDisplay_iOS_ReadResourceCallback, (void *)_pstBitmap);

          /* Successful asynchronous call? */
          if(s64Size < 0)
          {
            /* Inits bitmap info using temp */
            _pstBitmap->uiTexture       = sstDisplay.pstTempBitmap->uiTexture;
            _pstBitmap->fWidth          = orxS2F(iWidth);
            _pstBitmap->fHeight         = orxS2F(iHeight);
            _pstBitmap->u32RealWidth    = sstDisplay.pstTempBitmap->u32RealWidth;
            _pstBitmap->u32RealHeight   = sstDisplay.pstTempBitmap->u32RealHeight;
            _pstBitmap->u32Depth        = sstDisplay.pstTempBitmap->u32Depth;
            _pstBitmap->fRecRealWidth   = sstDisplay.pstTempBitmap->fRecRealWidth;
            _pstBitmap->fRecRealHeight  = sstDisplay.pstTempBitmap->fRecRealHeight;
            _pstBitmap->u32DataSize     = sstDisplay.pstTempBitmap->u32DataSize;
            orxVector_Copy(&(_pstBitmap->stClip.vTL), &(sstDisplay.pstTempBitmap->stClip.vTL));
            orxVector_Copy(&(_pstBitmap->stClip.vBR), &(sstDisplay.pstTempBitmap->stClip.vBR));

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }
          else
          {
            /* Frees buffer */
            orxMemory_Free(pu8Buffer);

            /* Closes resource */
            orxResource_Close(hResource);
          }
        }
        else
        {
          /* Frees buffer */
          orxMemory_Free(pu8Buffer);

          /* Closes resource */
          orxResource_Close(hResource);
        }
      }
      else
      {
        /* Loads data from resource */
        s64Size = orxResource_Read(hResource, s64Size, pu8Buffer, orxNULL, orxNULL);

        /* Success? */
        if(s64Size != 0)
        {
          /* Processes data */
          orxDisplay_iOS_ReadResourceCallback(hResource, s64Size, (void *)pu8Buffer, (void *)_pstBitmap);

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Frees buffer */
          orxMemory_Free(pu8Buffer);

          /* Closes resource */
          orxResource_Close(hResource);
        }
      }
    }
    else
    {
      /* Closes resource */
      orxResource_Close(hResource);
    }
  }

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_iOS_DeleteBitmapData(orxBITMAP *_pstBitmap)
{
  orxS32 i;

  /* For all bound bitmaps */
  for(i = 0; i < (orxS32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Is deleted bitmap? */
    if(sstDisplay.apstBoundBitmapList[i] == _pstBitmap)
    {
      /* Resets it */
      sstDisplay.apstBoundBitmapList[i] = orxNULL;
      sstDisplay.adMRUBitmapList[i]     = orxDOUBLE_0;
    }
  }

  /* Tracks video memory */
  orxMEMORY_TRACK(VIDEO, _pstBitmap->u32DataSize, orxFALSE);

  /* Deletes its texture */
  glDeleteTextures(1, &(_pstBitmap->uiTexture));
  glASSERT();

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxDisplay_iOS_CompileShader(orxDISPLAY_SHADER *_pstShader)
{
  static const orxSTRING szVertexShaderSource =
  "attribute vec2 __vPosition__;"
  "uniform mat4 __mProjection__;"
  "attribute mediump vec2 __vTexCoord__;"
  "varying mediump vec2 ___TexCoord___;"
  "attribute mediump vec4 __vColor__;"
  "varying mediump vec4 ___Color;"
  "void main()"
  "{"
  "  mediump float fCoef = 1.0 / 255.0;"
  "  gl_Position      = __mProjection__ * vec4(__vPosition__.xy, 0.0, 1.0);"
  "  ___TexCoord___   = __vTexCoord__;"
  "  ___Color         = fCoef * __vColor__;"
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
      _pstShader->iTextureLocation = glGetUniformLocation(uiProgram, "__Texture__");
      glASSERT();

      /* Gets projection matrix location */
      _pstShader->iProjectionMatrixLocation = glGetUniformLocation(uiProgram, "__mProjection__");
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
        glGetProgramInfoLog(uiProgram, sizeof(acBuffer) - 1, NULL, (GLchar *)acBuffer);
        glASSERT();
        acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

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
      glGetShaderInfoLog(uiFragmentShader, sizeof(acBuffer) - 1, NULL, (GLchar *)acBuffer);
      glASSERT();
      acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

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
    glGetShaderInfoLog(uiVertexShader, sizeof(acBuffer) - 1, NULL, (GLchar *)acBuffer);
    glASSERT();
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

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
    GLint i;

    /* For all defined textures */
    for(i = 0; i < _pstShader->iTextureCounter; i++)
    {
      /* Binds bitmap */
      orxDisplay_iOS_BindBitmap(_pstShader->astTextureInfoList[i].pstBitmap);

      /* Updates shader uniform */
      glUNIFORM(1i, _pstShader->astTextureInfoList[i].iLocation, sstDisplay.s32ActiveTextureUnit);
    }
  }

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
    if(orxLinkList_GetCounter(&(sstDisplay.stActiveShaderList)) > 0)
    {
      orxDISPLAY_SHADER *pstShader, *pstNextShader;

      /* For all active shaders */
      for(pstShader = (orxDISPLAY_SHADER *)orxLinkList_GetFirst(&(sstDisplay.stActiveShaderList));
          pstShader != orxNULL;
          pstShader = pstNextShader)
      {
        /* Inits shader */
        orxDisplay_iOS_InitShader(pstShader);

        /* Draws elements */
        glDrawElements(GL_TRIANGLE_STRIP, sstDisplay.s32BufferIndex + (sstDisplay.s32BufferIndex >> 1), GL_UNSIGNED_SHORT, 0);
        glASSERT();

        /* Gets next shader */
        pstNextShader = (orxDISPLAY_SHADER *)orxLinkList_GetNext(&(pstShader->stNode));

        /* Was pending removal? */
        if(pstShader->bPending != orxFALSE)
        {
          /* Clears its texture counter */
          pstShader->iTextureCounter = 0;

          /* Clears its texture info list */
          orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

          /* Removes its pending status */
          pstShader->bPending = orxFALSE;

          /* Removes it from active list */
          orxLinkList_Remove(&(pstShader->stNode));

          /* Updates counter */
          sstDisplay.s32PendingShaderCounter--;
        }
      }

      /* Uses default shader */
      orxDisplay_iOS_StopShader(orxNULL);
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

  /* Has pending shaders? */
  if(sstDisplay.s32PendingShaderCounter != 0)
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

    /* Checks */
    orxASSERT(sstDisplay.s32PendingShaderCounter == 0);
  }

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit])
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

    /* Binds bitmap */
    orxDisplay_iOS_BindBitmap(_pstBitmap);

    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* No other shader active? */
      if(orxLinkList_GetCounter(&(sstDisplay.stActiveShaderList)) == 0)
      {
        /* Updates shader uniform */
        glUNIFORM(1i, sstDisplay.pstDefaultShader->iTextureLocation, sstDisplay.s32ActiveTextureUnit);
      }
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

  /* Sets blend mode */
  orxDisplay_iOS_SetBlendMode(_eBlendMode);

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
  if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
  {
    /* Draws arrays */
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
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(_pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(_pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));

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

static void orxFASTCALL orxDisplay_iOS_DrawPrimitive(orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill, orxBOOL _bOpen)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DrawPrimitive");

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Starts no texture shader */
    orxDisplay_iOS_StartShader((orxHANDLE)sstDisplay.pstNoTextureShader);

    /* Inits it */
    orxDisplay_iOS_InitShader(sstDisplay.pstNoTextureShader);
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

    /* Updates blend mode */
    sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_ALPHA;
  }
  else
  {
    /* Disables alpha blending */
    glDisable(GL_BLEND);
    glASSERT();

    /* Updates blend mode */
    sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NONE;
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
    /* Bypasses the full screen rendering when stopping shader */
    sstDisplay.s32BufferIndex = -1;

    /* Stops current shader */
    orxDisplay_iOS_StopShader((orxHANDLE)sstDisplay.pstNoTextureShader);

    /* Resets buffer index */
    sstDisplay.s32BufferIndex = 0;
  }
  else
  {
    /* Reenables texturing */
    glEnable(GL_TEXTURE_2D);
    glASSERT();
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxDisplay_iOS_EventHandler(const orxEVENT *_pstEvent)
{
  /* Render stop? */
  if(_pstEvent->eID == orxRENDER_EVENT_STOP)
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
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
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
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
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU  = (GLfloat)(_pstFont->fRecRealWidth * (pstGlyph->fX + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU  = (GLfloat)(_pstFont->fRecRealWidth * (pstGlyph->fX + fWidth - orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(_pstFont->fRecRealHeight * (pstGlyph->fY + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(_pstFont->fRecRealHeight * (pstGlyph->fY + fHeight - orxDISPLAY_KF_BORDER_FIX));

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
  orxFLOAT          fWidth, fHeight, fTop, fLeft, fXCoef, fYCoef;
  orxU32            i, iIndex, u32VertexNumber = _u32VertexNumber;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32VertexNumber > 2);
  orxASSERT(_astVertexList != orxNULL);

  /* Gets bitmap to use */
  pstBitmap = (_pstBitmap != orxNULL) ? _pstBitmap : sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit];

  /* Prepares bitmap for drawing */
  orxDisplay_iOS_PrepareBitmap(pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = pstBitmap->stClip.vBR.fX - pstBitmap->stClip.vTL.fX;
  fHeight = pstBitmap->stClip.vBR.fY - pstBitmap->stClip.vTL.fY;

  /* Gets top-left corner  */
  fTop  = pstBitmap->fRecRealHeight * pstBitmap->stClip.vTL.fY;
  fLeft = pstBitmap->fRecRealWidth * pstBitmap->stClip.vTL.fX;

  /* Gets X & Y coefs */
  fXCoef = pstBitmap->fRecRealWidth * fWidth;
  fYCoef = pstBitmap->fRecRealHeight * fHeight;

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex + (2 * _u32VertexNumber) > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 3)
  {
    /* Draws arrays */
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
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fU = (GLfloat)(fLeft + (fXCoef * _astVertexList[i].fU));
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fV = (GLfloat)(fTop + (fYCoef * _astVertexList[i].fV));

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

  /* Not enough vertices for a final quad in the triangle strip? */
  while(iIndex & 3)
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
    /* Loading? */
    if(orxFLAG_TEST(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
    {
      /* Asks for deletion */
      orxFLAG_SET(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_DELETE, orxDISPLAY_KU32_BITMAP_FLAG_NONE);
    }
    else
    {
      /* Delete its data */
      orxDisplay_iOS_DeleteBitmapData(_pstBitmap);

      /* Is temp bitmap? */
      if(_pstBitmap == sstDisplay.pstTempBitmap)
      {
        /* Clears temp bitmap */
        sstDisplay.pstTempBitmap = orxNULL;
      }

      /* Deletes it */
      orxBank_Free(sstDisplay.pstBitmapBank, _pstBitmap);
    }
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
    /* Inits it */
    pstBitmap->bSmoothing     = sstDisplay.bDefaultSmoothing;
    pstBitmap->fWidth         = orxU2F(_u32Width);
    pstBitmap->fHeight        = orxU2F(_u32Height);
    pstBitmap->u32RealWidth   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Width : orxMath_GetNextPowerOfTwo(_u32Width);
    pstBitmap->u32RealHeight  = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Height : orxMath_GetNextPowerOfTwo(_u32Height);
    pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
    pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
    pstBitmap->u32DataSize    = pstBitmap->u32RealWidth * pstBitmap->u32RealHeight * 4 * sizeof(orxU8);
    pstBitmap->stColor        = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
    pstBitmap->zLocation      = orxSTRING_EMPTY;
    pstBitmap->u32FilenameID  = 0;
    pstBitmap->u32Flags       = orxDISPLAY_KU32_BITMAP_FLAG_NONE;
    orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

    /* Tracks video memory */
    orxMEMORY_TRACK(VIDEO, pstBitmap->u32DataSize, orxTRUE);

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
    glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
    glASSERT();
  }

  /* Done! */
  return pstBitmap;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Clears current destinations? */
  if(_pstBitmap == orxNULL)
  {
    /* Different clear color? */
    if(_stColor.u32RGBA != sstDisplay.stLastColor.u32RGBA)
    {
      /* Updates it */
      glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
      glASSERT();

      /* Stores it */
      sstDisplay.stLastColor.u32RGBA = _stColor.u32RGBA;
    }

    /* Clears the color buffer with given color */
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();
  }
  else
  {
    orxBITMAP *pstBackupBitmap;

    /* Backups current destination */
    pstBackupBitmap = sstDisplay.pstDestinationBitmap;

    /* Sets new destination bitmap */
    if(orxDisplay_iOS_SetDestinationBitmaps(&_pstBitmap, 1) != orxSTATUS_FAILURE)
    {
      /* Different clear color? */
      if(_stColor.u32RGBA != sstDisplay.stLastColor.u32RGBA)
      {
        /* Updates it */
        glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
        glASSERT();

        /* Stores it */
        sstDisplay.stLastColor.u32RGBA = _stColor.u32RGBA;
      }

      /* Clears the color buffer with given color */
      glClear(GL_COLOR_BUFFER_BIT);
      glASSERT();

      /* Is screen and has depth buffer? */
      if((_pstBitmap == sstDisplay.pstScreen) && (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER)))
      {
        /* Clears depth buffer */
        glClear(GL_DEPTH_BUFFER_BIT);
        glASSERT();
      }

      /* Restores previous destination */
      orxDisplay_iOS_SetDestinationBitmaps(&pstBackupBitmap, 1);
    }
    /* Not screen? */
    else if(_pstBitmap != sstDisplay.pstScreen)
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
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)_pstBitmap->u32RealWidth, (GLsizei)_pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, astBuffer);
      glASSERT();

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
      glASSERT();

      /* Frees buffer */
      orxMemory_Free(astBuffer);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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

      case orxDISPLAY_BLEND_MODE_PREMUL:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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
    orxU8 *pu8ImageBuffer;

    /* Has NPOT texture support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))
    {
      /* Uses sources bitmap */
      pu8ImageBuffer = (orxU8 *)_au8Data;
    }
    else
    {
      orxU32 i, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset;

      /* Allocates buffer */
      pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

      /* Gets line sizes */
      u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
      u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

      /* Clears padding */
      orxMemory_Zero(pu8ImageBuffer, u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)));

      /* For all lines */
      for(i = 0, u32SrcOffset = 0, u32DstOffset = 0;
          i < u32Height;
          i++, u32SrcOffset += u32LineSize, u32DstOffset += u32RealLineSize)
      {
        /* Copies data */
        orxMemory_Copy(pu8ImageBuffer + u32DstOffset, _au8Data + u32SrcOffset, u32LineSize);

        /* Adds padding */
        orxMemory_Zero(pu8ImageBuffer + u32DstOffset + u32LineSize, u32RealLineSize - u32LineSize);
      }
    }

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
    glASSERT();

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
    glASSERT();

    /* Needs to free buffer? */
    if(pu8ImageBuffer != _au8Data)
    {
      /* Frees it */
      orxMemory_Free(pu8ImageBuffer);
    }

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

orxSTATUS orxFASTCALL orxDisplay_iOS_GetBitmapData(const orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
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
    orxBITMAP *pstBackupBitmap;

    /* Backups current destination */
    pstBackupBitmap = sstDisplay.pstDestinationBitmap;

    /* Sets new destination bitmap */
    if((eResult = orxDisplay_iOS_SetDestinationBitmaps((orxBITMAP **)&_pstBitmap, 1)) != orxSTATUS_FAILURE)
    {
      orxU32  u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset, i;
      orxU8  *pu8ImageBuffer;

      /* Allocates buffer */
      pu8ImageBuffer = ((_pstBitmap != sstDisplay.pstScreen) && (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))) ? _au8Data : (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

      /* Checks */
      orxASSERT(pu8ImageBuffer != orxNULL);

      /* Reads OpenGL data */
      glReadPixels(0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
      glASSERT();

      /* Gets line sizes */
      u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
      u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

      /* Screen? */
      if(_pstBitmap == sstDisplay.pstScreen)
      {
        orxRGBA stOpaque;

        /* Sets opaque pixel */
        stOpaque = orx2RGBA(0x00, 0x00, 0x00, 0xFF);

        /* For all lines */
        for(i = 0, u32SrcOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)), u32DstOffset = u32LineSize * (orxF2U(_pstBitmap->fHeight) - 1);
            i < orxF2U(_pstBitmap->fHeight);
            i++, u32SrcOffset += u32RealLineSize, u32DstOffset -= u32LineSize)
        {
          orxU32 j;

          /* For all columns */
          for(j = 0; j < orxF2U(_pstBitmap->fWidth); j++)
          {
            orxRGBA stPixel;

            /* Gets opaque pixel */
            stPixel.u32RGBA = ((orxRGBA *)(pu8ImageBuffer + u32SrcOffset))[j].u32RGBA | stOpaque.u32RGBA;

            /* Stores it */
            ((orxRGBA *)(_au8Data + u32DstOffset))[j] = stPixel;
          }
        }

        /* Deletes buffer */
        orxMemory_Free(pu8ImageBuffer);
      }
      else
      {
        /* Doesn't have NPOT texture support? */
        if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))
        {
          /* For all lines */
          for(i = 0, u32SrcOffset = 0, u32DstOffset = 0;
              i < orxF2U(_pstBitmap->fHeight);
              i++, u32SrcOffset += u32RealLineSize, u32DstOffset += u32LineSize)
          {
            /* Copies data */
            orxMemory_Copy(_au8Data + u32DstOffset, pu8ImageBuffer + u32SrcOffset, u32LineSize);
          }

          /* Deletes buffer */
          orxMemory_Free(pu8ImageBuffer);
        }
      }

      /* Restores previous destination */
      orxDisplay_iOS_SetDestinationBitmaps(&pstBackupBitmap, 1);
    }
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

orxSTATUS orxFASTCALL orxDisplay_iOS_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number)
{
  orxFLOAT  fOrthoRight, fOrthoBottom;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32Number == 1);

  /* Different destination bitmap? */
  if(_apstBitmapList[0] != sstDisplay.pstDestinationBitmap)
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

    /* Is valid? */
    if(_apstBitmapList[0] != orxNULL)
    {
      GLint   iX, iY;
      GLsizei iWidth, iHeight;

      /* Recreates render target */
      [sstDisplay.poView CreateRenderTarget:_apstBitmapList[0]];

      /* Is screen? */
      if(_apstBitmapList[0] == sstDisplay.pstScreen)
      {
        /* Updates viewport info */
        iX      = 0;
        iY      = 0;
        iWidth  = (GLsizei)orxF2S(_apstBitmapList[0]->fWidth);
        iHeight = (GLsizei)orxF2S(_apstBitmapList[0]->fHeight);

        /* Updates ortho info */
        fOrthoRight   = _apstBitmapList[0]->fWidth;
        fOrthoBottom  = _apstBitmapList[0]->fHeight;
      }
      else
      {
        /* Updates viewport info */
        iX      = 0;
        iY      = 0;
        iWidth  = (GLsizei)orxF2S(_apstBitmapList[0]->fWidth);
        iHeight = (GLsizei)orxF2S(_apstBitmapList[0]->fHeight);

        /* Updates ortho info */
        fOrthoRight   = _apstBitmapList[0]->fWidth;
        fOrthoBottom  = -_apstBitmapList[0]->fHeight;
      }

      /* Should update viewport? */
      if((iX != sstDisplay.iLastViewportX)
      || (iY != sstDisplay.iLastViewportY)
      || (iWidth != sstDisplay.iLastViewportWidth)
      || (iHeight != sstDisplay.iLastViewportHeight))
      {
        /* Inits viewport */
        glViewport(iX, iY, iWidth, iHeight);
        glASSERT();

        /* Stores its info */
        sstDisplay.iLastViewportX       = iX;
        sstDisplay.iLastViewportY       = iY;
        sstDisplay.iLastViewportWidth   = iWidth;
        sstDisplay.iLastViewportHeight  = iHeight;
      }

      /* Should update the orthogonal projection? */
      if((fOrthoRight != sstDisplay.fLastOrthoRight)
      || (fOrthoBottom != sstDisplay.fLastOrthoBottom))
      {
        /* Stores data */
        sstDisplay.fLastOrthoRight  = fOrthoRight;
        sstDisplay.fLastOrthoBottom = fOrthoBottom;

        /* Shader support? */
        if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
        {
          /* Inits projection matrix */
          (fOrthoBottom >= orxFLOAT_0)
          ? orxDisplay_iOS_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, fOrthoRight, fOrthoBottom, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1)
          : orxDisplay_iOS_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, fOrthoRight, orxFLOAT_0, -fOrthoBottom, -orxFLOAT_1, orxFLOAT_1);

          /* Passes it to shader */
          glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
        }
        else
        {
          /* Inits matrices */
          glMatrixMode(GL_PROJECTION);
          glASSERT();
          glLoadIdentity();
          glASSERT();
          (fOrthoBottom >= orxFLOAT_0)
          ? glOrthof(0.0f, (GLfloat)fOrthoRight, (GLfloat)fOrthoBottom, 0.0f, -1.0f, 1.0f)
          : glOrthof(0.0f, (GLfloat)fOrthoRight, 0.0f, -(GLfloat)fOrthoBottom, -1.0f, 1.0f);
          glASSERT();
          glMatrixMode(GL_MODELVIEW);
          glASSERT();
          glLoadIdentity();
          glASSERT();
        }
      }
    }
    else
    {
     /* Updates result */
     eResult = orxSTATUS_FAILURE;
    }

    /* Stores it */
    sstDisplay.pstDestinationBitmap = _apstBitmapList[0];
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
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSrc != sstDisplay.pstScreen);
  orxASSERT((_pstSrc == orxNULL) || (_pstTransform != orxNULL));

  /* No bitmap? */
  if(_pstSrc == orxNULL)
  {
    /* Has something to display? */
    if(sstDisplay.s32BufferIndex > 0)
    {
      /* Draws arrays */
      orxDisplay_iOS_DrawArrays();
    }

    /* Defines the vertex list */
    sstDisplay.astVertexList[0].fX  =
    sstDisplay.astVertexList[1].fX  = sstDisplay.pstDestinationBitmap->stClip.vTL.fX;
    sstDisplay.astVertexList[2].fX  =
    sstDisplay.astVertexList[3].fX  = sstDisplay.pstDestinationBitmap->stClip.vBR.fX;
    sstDisplay.astVertexList[1].fY  =
    sstDisplay.astVertexList[3].fY  = sstDisplay.pstDestinationBitmap->stClip.vTL.fY;
    sstDisplay.astVertexList[0].fY  =
    sstDisplay.astVertexList[2].fY  = sstDisplay.pstDestinationBitmap->stClip.vBR.fY;

    /* Defines the texture coord list */
    sstDisplay.astVertexList[0].fU  =
    sstDisplay.astVertexList[1].fU  = 0.0f;
    sstDisplay.astVertexList[2].fU  =
    sstDisplay.astVertexList[3].fU  = 1.0f;
    sstDisplay.astVertexList[1].fV  =
    sstDisplay.astVertexList[3].fV  = 0.0f;
    sstDisplay.astVertexList[0].fV  =
    sstDisplay.astVertexList[2].fV  = 1.0f;

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
  else
  {
    orxDISPLAY_MATRIX mTransform;

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
      fLeft   = _pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX);
      fTop    = _pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX);

      /* For all lines */
      for(fY = 0.0f, i = _pstTransform->fRepeatY, fRecRepeatX = orxFLOAT_1 / _pstTransform->fRepeatX; i > orxFLOAT_0; i -= orxFLOAT_1, fY += fHeight)
      {
        /* Partial line? */
        if(i < orxFLOAT_1)
        {
          /* Updates height */
          fHeight *= (GLfloat)i;

          /* Resets texture coords */
          fRight  = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
          fBottom = (GLfloat)(_pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + (i * (_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY)) - orxDISPLAY_KF_BORDER_FIX));
        }
        else
        {
          /* Resets texture coords */
          fRight  = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
          fBottom = (GLfloat)(_pstSrc->fRecRealHeight * (_pstSrc->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));
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
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
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
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxU32    u32BufferSize;
  orxU8    *pu8ImageData;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Gets buffer size */
  u32BufferSize = orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8);

  /* Allocates buffer */
  pu8ImageData = (orxU8 *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN);

  /* Valid? */
  if(pu8ImageData != orxNULL)
  {
    orxDISPLAY_SAVE_INFO *pstInfo = orxNULL;

    /* Gets bitmap data */
    if(orxDisplay_iOS_GetBitmapData(_pstBitmap, pu8ImageData, u32BufferSize) != orxSTATUS_FAILURE)
    {
      /* Allocates save info */
      pstInfo = (orxDISPLAY_SAVE_INFO *)orxMemory_Allocate(sizeof(orxDISPLAY_SAVE_INFO), orxMEMORY_TYPE_TEMP);

      /* Valid? */
      if(pstInfo != orxNULL)
      {
        /* Inits it */
        pstInfo->pu8ImageData   = pu8ImageData;
        pstInfo->u32FilenameID  = orxString_GetID(_zFilename);
        pstInfo->u32Width       = orxF2U(_pstBitmap->fWidth);
        pstInfo->u32Height      = orxF2U(_pstBitmap->fHeight);

        /* Runs asynchronous task */
        eResult = orxThread_RunTask(&orxDisplay_iOS_SaveBitmapData, orxNULL, orxNULL, (void *)pstInfo);
      }
    }

    /* Failure? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Frees save info */
      if(pstInfo != orxNULL)
      {
        orxMemory_Free(pstInfo);
      }

      /* Frees buffer */
      orxMemory_Free(pu8ImageData);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetTempBitmap(const orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Stores it */
  sstDisplay.pstTempBitmap = _pstBitmap;

  /* Done! */
  return eResult;
}

const orxBITMAP *orxFASTCALL orxDisplay_iOS_GetTempBitmap()
{
  const orxBITMAP *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  pstResult = sstDisplay.pstTempBitmap;

  /* Done! */
  return pstResult;
}

orxBITMAP *orxFASTCALL orxDisplay_iOS_LoadBitmap(const orxSTRING _zFilename)
{
  const orxSTRING zResourceLocation;
  orxBITMAP      *pstBitmap = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Locates resource */
  zResourceLocation = orxResource_Locate(orxTEXTURE_KZ_RESOURCE_GROUP, _zFilename);

  /* Success? */
  if(zResourceLocation != orxNULL)
  {
    /* Allocates bitmap */
    pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

    /* Valid? */
    if(pstBitmap != orxNULL)
    {
      /* Inits it */
      pstBitmap->bSmoothing     = sstDisplay.bDefaultSmoothing;
      pstBitmap->zLocation      = zResourceLocation;
      pstBitmap->u32FilenameID  = orxString_GetID(_zFilename);
      pstBitmap->u32Flags       = orxDISPLAY_KU32_BITMAP_FLAG_NONE;

      /* Loads its data */
      if(orxDisplay_iOS_LoadBitmapData(pstBitmap) == orxSTATUS_FAILURE)
      {
        /* Deletes it */
        orxBank_Free(sstDisplay.pstBitmapBank, pstBitmap);

        /* Updates result */
        pstBitmap = orxNULL;
      }
    }
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

  /* Destination bitmap? */
  if(_pstBitmap == sstDisplay.pstDestinationBitmap)
  {
    orxU32 u32ClipX, u32ClipY, u32ClipWidth, u32ClipHeight;

    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

    /* Gets new clipping values */
    u32ClipX      = _u32TLX;
    u32ClipY      = (_pstBitmap == sstDisplay.pstScreen) ? orxF2U(sstDisplay.pstDestinationBitmap->fHeight) - _u32BRY : _u32TLY;
    u32ClipWidth  = _u32BRX - _u32TLX;
    u32ClipHeight = _u32BRY - _u32TLY;

    /* Different clipping? */
    if((u32ClipX != sstDisplay.u32LastClipX)
    || (u32ClipY != sstDisplay.u32LastClipY)
    || (u32ClipWidth != sstDisplay.u32LastClipWidth)
    || (u32ClipHeight != sstDisplay.u32LastClipHeight))
    {
      /* Sets OpenGL clipping */
      glScissor((GLint)u32ClipX, (GLint)u32ClipY, (GLsizei)u32ClipWidth, (GLsizei)u32ClipHeight);
      glASSERT();

      /* Stores clipping values */
      sstDisplay.u32LastClipX       = u32ClipX;
      sstDisplay.u32LastClipY       = u32ClipY;
      sstDisplay.u32LastClipWidth   = u32ClipWidth;
      sstDisplay.u32LastClipHeight  = u32ClipHeight;
    }
  }

  /* Stores clip coords */
  orxVector_Set(&(_pstBitmap->stClip.vTL), orxU2F(_u32TLX), orxU2F(_u32TLY), orxFLOAT_0);
  orxVector_Set(&(_pstBitmap->stClip.vBR), orxU2F(_u32BRX), orxU2F(_u32BRY), orxFLOAT_0);

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
  orxU32 u32Result = 1;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_iOS_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Gets default mode */
  _pstVideoMode->u32Width       = orxF2U(sstDisplay.pstScreen->fWidth);
  _pstVideoMode->u32Height      = orxF2U(sstDisplay.pstScreen->fHeight);
  _pstVideoMode->u32Depth       = 32;
  _pstVideoMode->u32RefreshRate = 60;
  _pstVideoMode->bFullScreen    = orxTRUE;

  /* Updates result */
  pstResult = _pstVideoMode;

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_iOS_DrawArrays();

  /* Shader support? */
  if([sstDisplay.poView bShaderSupport] != NO)
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

    /* Updates active texture unit */
    sstDisplay.s32ActiveTextureUnit = 0;

    /* Selects first texture unit */
    glActiveTexture(GL_TEXTURE0);
    glASSERT();

    /* Uses default shader */
    orxDisplay_iOS_StopShader(orxNULL);
  }
  else
  {
    /* Selects arrays */
    glVertexPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
    glASSERT();
    glTexCoordPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
    glASSERT();
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
    glASSERT();
  }

  /* For all texture units */
  for(i = 0; i < (orxU32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Clears its bound bitmap */
    sstDisplay.apstBoundBitmapList[i] = orxNULL;

    /* Clears its MRU timestamp */
    sstDisplay.adMRUBitmapList[i] = orxDOUBLE_0;
  }

  /* Shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Inits projection matrix */
    (sstDisplay.fLastOrthoBottom >= orxFLOAT_0)
    ? orxDisplay_iOS_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.fLastOrthoRight, sstDisplay.fLastOrthoBottom, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1)
    : orxDisplay_iOS_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.fLastOrthoRight, orxFLOAT_0, -sstDisplay.fLastOrthoBottom, -orxFLOAT_1, orxFLOAT_1);

    /* Passes it to shader */
    glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
  }
  else
  {
    /* Inits matrices */
    glMatrixMode(GL_PROJECTION);
    glASSERT();
    glLoadIdentity();
    glASSERT();
    (sstDisplay.fLastOrthoBottom >= orxFLOAT_0)
    ? glOrthof(0.0f, (GLfloat)sstDisplay.fLastOrthoRight, (GLfloat)sstDisplay.fLastOrthoBottom, 0.0f, -1.0f, 1.0f)
    : glOrthof(0.0f, (GLfloat)sstDisplay.fLastOrthoRight, 0.0f, -(GLfloat)sstDisplay.fLastOrthoBottom, -1.0f, 1.0f);
    glASSERT();
    glMatrixMode(GL_MODELVIEW);
    glASSERT();
    glLoadIdentity();
    glASSERT();
  }

  /* Clears last blend mode */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;

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

    /* Stores stbi callbacks */
    sstDisplay.stSTBICallbacks.read = orxDisplay_iOS_ReadSTBICallback;
    sstDisplay.stSTBICallbacks.skip = orxDisplay_iOS_SkipSTBICallback;
    sstDisplay.stSTBICallbacks.eof  = orxDisplay_iOS_EOFSTBICallback;

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

        /* Adds event handler */
        orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxDisplay_iOS_EventHandler);

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
        sstDisplay.pstScreen->u32DataSize     = sstDisplay.pstScreen->u32RealWidth * sstDisplay.pstScreen->u32RealHeight * 4 * sizeof(orxU8);
        orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);
        sstDisplay.eLastBlendMode             = orxDISPLAY_BLEND_MODE_NUMBER;
        sstDisplay.dTouchTimeCorrection       = orxSystem_GetTime() - orx2D([[NSProcessInfo processInfo] systemUptime]);
        sstDisplay.u32EventInfoNumber         = 0;

        /* Updates bound texture */
        sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] = orxNULL;

        /* Updates config info */
        orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_WIDTH, sstDisplay.pstScreen->fWidth);
        orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_HEIGHT, sstDisplay.pstScreen->fHeight);
        orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_DEPTH, 32);

        /* Pops config section */
        orxConfig_PopSection();

        /* Gets max texture unit number */
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &(sstDisplay.iTextureUnitNumber));
        glASSERT();
        sstDisplay.iTextureUnitNumber = orxMIN(sstDisplay.iTextureUnitNumber, orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER);

        /* Has shader support? */
        if([sstDisplay.poView bShaderSupport] != NO)
        {
          static const orxSTRING szFragmentShaderSource =
          "precision mediump float;"
          "varying vec2 ___TexCoord___;"
          "varying vec4 ___Color;"
          "uniform sampler2D __Texture__;"
          "void main()"
          "{"
          "  gl_FragColor = ___Color.rgba * texture2D(__Texture__, ___TexCoord___).rgba;"
          "}";
          static const orxSTRING szNoTextureFragmentShaderSource =
          "precision mediump float;"
          "varying vec2 ___TexCoord___;"
          "varying vec4 ___Color;"
          "uniform sampler2D __Texture__;"
          "void main()"
          "{"
          "  gl_FragColor = ___Color;"
          "}";

          /* Inits flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER | orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);

          /* Creates default shaders */
          sstDisplay.pstDefaultShader   = (orxDISPLAY_SHADER *)orxDisplay_CreateShader(szFragmentShaderSource, orxNULL, orxFALSE);
          sstDisplay.pstNoTextureShader = (orxDISPLAY_SHADER *)orxDisplay_CreateShader(szNoTextureFragmentShaderSource, orxNULL, orxTRUE);

          /* Uses it */
          orxDisplay_iOS_StopShader(orxNULL);
        }
        else
        {
          /* Updates texture unit number */
          sstDisplay.iTextureUnitNumber = 1;

          /* Inits flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);
        }

        /* Pushes config section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Stores texture unit and draw buffer numbers */
        orxConfig_SetU32("TextureUnitNumber", (orxU32)sstDisplay.iTextureUnitNumber);
        orxConfig_SetU32("DrawBufferNumber", 1);

        /* Pops config section */
        orxConfig_PopSection();

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
        stPayload.stVideoMode.u32Width    = orxF2U(sstDisplay.pstScreen->fWidth);
        stPayload.stVideoMode.u32Height   = orxF2U(sstDisplay.pstScreen->fHeight);
        stPayload.stVideoMode.u32Depth    = 32;
        stPayload.stVideoMode.bFullScreen = orxTRUE;

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
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_iOS_EventHandler);

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

orxHANDLE orxFASTCALL orxDisplay_iOS_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList, orxBOOL _bUseCustomParam)
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
          s32Offset = orxString_NPrint(pc, s32Free, "precision mediump float;\nvarying vec2 ___TexCoord___;\nvarying vec4 ___Color;\n");
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

          /* For all gl_Color */
          for(pcReplace = (orxCHAR *)orxString_SearchString(sstDisplay.acShaderCodeBuffer, "gl_Color");
              pcReplace != orxNULL;
              pcReplace = (orxCHAR *)orxString_SearchString(pcReplace + 8 * sizeof(orxCHAR), "gl_Color"))
          {
            /* Replaces it */
            orxMemory_Copy(pcReplace, "___Color", 8 * sizeof(orxCHAR));
          }
        }
        else
        {
          /* Adds code */
          orxString_NPrint(pc, s32Free, "%s\n", _zCode);
        }

        /* Inits shader */
        orxMemory_Zero(&(pstShader->stNode), sizeof(orxLINKLIST_NODE));
        pstShader->uiProgram              = (GLuint)orxHANDLE_UNDEFINED;
        pstShader->iTextureCounter        = 0;
        pstShader->s32ParamCounter        = 0;
        pstShader->bPending               = orxFALSE;
        pstShader->bUseCustomParam        = _bUseCustomParam;
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

  /* Deletes its param info list */
  orxMemory_Free(pstShader->astParamInfoList);

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

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Not pending or use custom param? */
  if((pstShader->bPending == orxFALSE) || (pstShader->bUseCustomParam != orxFALSE))
  {
    /* Draws remaining items */
    orxDisplay_iOS_DrawArrays();

    /* Adds it to the active list */
    orxLinkList_AddEnd(&(sstDisplay.stActiveShaderList), &(pstShader->stNode));
  }
  else
  {
    /* Resets its pending status */
    pstShader->bPending = orxFALSE;

    /* Updates counter */
    sstDisplay.s32PendingShaderCounter--;
  }

  /* Uses its program */
  glUseProgram(pstShader->uiProgram);
  glASSERT();

  /* Updates projection matrix */
  glUNIFORM(Matrix4fv, pstShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_iOS_StopShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxBOOL             bResetShader = orxTRUE;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_hShader != orxHANDLE_UNDEFINED);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has shader? */
  if(pstShader != orxNULL)
  {
    /* Not already removed? */
    if((pstShader != orxNULL)
    && (orxLinkList_GetList(&(pstShader->stNode)) != orxNULL)
    && (pstShader->bPending == orxFALSE))
    {
      /* Empty buffer? */
      if(sstDisplay.s32BufferIndex == 0)
      {
        orxDISPLAY_SHADER *pstActive, *pstNextActive;

        /* For all active shaders */
        for(pstActive = (orxDISPLAY_SHADER *)orxLinkList_GetFirst(&(sstDisplay.stActiveShaderList));
            pstActive != orxNULL;
            pstActive = pstNextActive)
        {
          /* Gets next shader */
          pstNextActive = (orxDISPLAY_SHADER *)orxLinkList_GetNext(&(pstActive->stNode));

          /* Was pending removal? */
          if(pstActive->bPending != orxFALSE)
          {
            /* Clears its texture counter */
            pstActive->iTextureCounter = 0;

            /* Clears its texture info list */
            orxMemory_Zero(pstActive->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

            /* Removes its pending status */
            pstActive->bPending = orxFALSE;

            /* Removes it from active list */
            orxLinkList_Remove(&(pstActive->stNode));

            /* Updates counter */
            sstDisplay.s32PendingShaderCounter--;
          }
        }

        /* Clears texture counter */
        pstShader->iTextureCounter = 0;

        /* Clears its texture info list */
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

        /* Removes it from active list */
        orxLinkList_Remove(&(pstShader->stNode));
      }
      /* Using custom param? */
      else if(pstShader->bUseCustomParam != orxFALSE)
      {
        /* Has something to display? */
        if(sstDisplay.s32BufferIndex > 0)
        {
          /* Draws arrays */
          orxDisplay_iOS_DrawArrays();

          /* Don't reset shader */
          bResetShader = orxFALSE;
        }

        /* Clears texture counter */
        pstShader->iTextureCounter = 0;

        /* Clears texture info list */
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

        /* Removes it from active list */
        orxLinkList_Remove(&(pstShader->stNode));
      }
      else
      {
        /* Marks it as pending */
        pstShader->bPending = orxTRUE;

        /* Updates counter */
        sstDisplay.s32PendingShaderCounter++;
      }

      /* Updates projection matrix */
      glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
    }
    else
    {
      /* Don't reset shader */
      bResetShader = orxFALSE;
    }
  }

  /* Should reset shader? */
  if(bResetShader != orxFALSE)
  {
    /* Uses default program */
    glUseProgram(sstDisplay.pstDefaultShader->uiProgram);
    glASSERT();

    /* Updates its texture unit */
    glUNIFORM(1i, sstDisplay.pstDefaultShader->iTextureLocation, sstDisplay.s32ActiveTextureUnit);

    /* Updates projection matrix */
    glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
  }

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
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

    /* Gets corresponding param info */
    pstInfo = &pstShader->astParamInfoList[pstShader->s32ParamCounter];

    /* Updates result */
    s32Result = pstShader->s32ParamCounter++;

    /* Array? */
    if(_s32Index >= 0)
    {
      /* Prints its name */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s[%ld]", _zParam, _s32Index);

      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, acBuffer);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)_zParam);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const GLchar *)acBuffer);
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
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s[%ld]", _zParam, _s32Index);
      acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

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
  orxSTATUS           eResult = orxSTATUS_FAILURE;
  orxS32              i;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT((_pstValue != sstDisplay.pstScreen) && "Can't use screen bitmap as texture parameter (ID <%d>) for fragment shader.", _s32ID);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* For all already used texture units */
  for(i = 0; i < pstShader->iTextureCounter; i++)
  {
    /* Same location? */
    if(pstShader->astTextureInfoList[i].iLocation == pstShader->astParamInfoList[_s32ID].iLocation)
    {
      /* Different texture? */
      if(pstShader->astTextureInfoList[i].pstBitmap != _pstValue)
      {
        /* Draws remaining items */
        orxDisplay_iOS_DrawArrays();

        /* Updates texture info */
        pstShader->astTextureInfoList[i].pstBitmap = _pstValue;

        /* Updates corner values */
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationTop, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;

      break;
    }
  }

  /* Not already done? */
  if(eResult == orxSTATUS_FAILURE)
  {
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
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationTop, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));

        /* Updates texture counter */
        pstShader->iTextureCounter++;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter (ID <%d>) for fragment shader: invalid ID.", _s32ID);
      }
    }
    else
    {
      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter (ID <%d>) for fragment shader: all the texture units are used.", _s32ID);
    }
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
    /* Updates its value (no glASSERT() as this can be set more than once per use and would trigger it) */
    glUNIFORM_NO_ASSERT(1f, (GLint)_s32ID, (GLfloat)_fValue);

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
    /* Updates its value (no glASSERT() as this can be set more than once per use and would trigger it) */
    glUNIFORM_NO_ASSERT(3f, (GLint)_s32ID, (GLfloat)_pvValue->fX, (GLfloat)_pvValue->fY, (GLfloat)_pvValue->fZ);

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetTempBitmap, DISPLAY, SET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_GetTempBitmap, DISPLAY, GET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetDestinationBitmaps, DISPLAY, SET_DESTINATION_BITMAPS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_iOS_SetBlendMode, DISPLAY, SET_BLEND_MODE);
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
