/* Orx - Portable Game Engine
 *
 * Copyright (c) 2010 Orx-Project
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
 * @file orxFile.c
 * @date 01/05/2005
 * @author bestel@arcallians.org
 * @author iarwain@orx-project.org
 *
 */


#include "io/orxFile.h"
#include "memory/orxMemory.h"
#include "debug/orxDebug.h"
#include "utils/orxString.h"


#ifdef __orxMSVC__

  #pragma warning(disable : 4996)

#endif /* __orxMSVC__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

#define orxFILE_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxFILE_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */

typedef struct __orxFILE_STATIC_t
{
  orxU32 u32Flags;
} orxFILE_STATIC;

struct __orxFILE_t
{
  FILE *pstFile;
};

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/
static orxFILE_STATIC sstFile;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** File module setup */
void orxFASTCALL orxFile_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_MEMORY);

  return;
}

/** Inits the File Module
 */
orxSTATUS orxFASTCALL orxFile_Init()
{
  /* Module not already initialized ? */
  orxASSERT(!(sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Zero(&sstFile, sizeof(orxFILE_STATIC));

	/* Set module has ready */
	sstFile.u32Flags = orxFILE_KU32_STATIC_FLAG_READY;

  /* Module successfully initialized ? */
  if(sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}

/** Exits from the File Module
 */
void orxFASTCALL orxFile_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Module not ready now */
  sstFile.u32Flags = orxFILE_KU32_STATIC_FLAG_NONE;
}

/** Opens a file for later read or write operation
 * @param[in] _zPath               Full file's path to open
 * @param[in] _u32OpenFlags        List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
orxFILE *orxFASTCALL orxFile_Open(const orxSTRING _zPath, orxU32 _u32OpenFlags)
{
  /* Convert the open flags into a string */
  orxCHAR acMode[4];
  orxBOOL bBinaryMode;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Fills with null terminated characters */
  orxMemory_Zero(acMode, 4 * sizeof(orxCHAR));

  /*** LIB C MODES :
   * r   : Open text file for reading.
   *       The stream is positioned at the beginning of the file.
   * r+  : Open for reading and writing.
   *       The stream is positioned at the beginning of the file.
   * w   : Truncate file to zero length or create text file for writing.
   *       The stream is positioned at the beginning of the file.
   * w+  : Open for reading and writing.
   *       The file is created if it does not exist, otherwise it is truncated.
   *       The stream is positioned at the beginning of the file.
   * a   : Open for appending (writing at end of file).
   *       The file is created if it does not exist.
   *       The stream is positioned at the end of the file.
   * a+  : Open for reading and appending (writing at end of file).
   *       The file is created if it does not exist.
   *       The initial file position for reading is at the beginning of the file, but output is always appended to the end of the file.
       *
   *** AVAILABLE CONVERSIONS :
   * READ | WRITE | APPEND | result
   *  X   |       |        | r
   *      |  X    |        | w
   *      |       |   X    | a
   *      |  X    |   X    | a
   *  X   |  X    |        | w+
   *  X   |       |   X    | a+
   *  X   |  X    |   X    | a+
   */

  /* Binary? */
  if(_u32OpenFlags & orxFILE_KU32_FLAG_OPEN_BINARY)
  {
    /* Removes it */
    _u32OpenFlags &= ~orxFILE_KU32_FLAG_OPEN_BINARY;

    /* Updates binary status*/
    bBinaryMode = orxTRUE;
  }
  else
  {
    /* Updates binary status*/
    bBinaryMode = orxFALSE;
  }

  /* Read only? */
  if(_u32OpenFlags == orxFILE_KU32_FLAG_OPEN_READ)
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "rb");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "r");
    }
  }
  /* Write only ?*/
  else if(_u32OpenFlags == orxFILE_KU32_FLAG_OPEN_WRITE)
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "wb");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "w");
    }
  }
  /* Append only ? */
  else if((_u32OpenFlags == orxFILE_KU32_FLAG_OPEN_APPEND)
       || (_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_APPEND)))
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "ab");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "a");
    }
  }
  else if(_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_WRITE))
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "wb+");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "w+");
    }
  }
  else if((_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_APPEND))
       || (_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_APPEND)))
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "ab+");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "a+");
    }
  }

  /* Open the file */
  return(orxFILE*)fopen(_zPath, acMode);
}

/** Reads data from a file
 * @param[out] _pReadData          Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
orxU32 orxFASTCALL orxFile_Read(void *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  /* Default return value */
  orxU32 u32Ret = 0;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid input ? */
  if(_pstFile != orxNULL)
  {
    u32Ret = (orxU32)fread(_pReadData, _u32ElemSize, _u32NbElem, (FILE*)_pstFile);
  }

  /* Returns the number of read elements */
  return u32Ret;
}

/** writes data to a file
 * @param[in] _pDataToWrite        Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
orxU32 orxFASTCALL orxFile_Write(void *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  /* Default return value */
  orxU32 u32Ret = 0;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Checks inputs */
  orxASSERT(_pstFile != orxNULL);

  /* Valid input ? */
  if(_pstFile != orxNULL)
  {
    u32Ret = (orxU32)fwrite(_pDataToWrite, _u32ElemSize, _u32NbElem, (FILE*)_pstFile);
  }

  /* Returns the number of read elements */
  return u32Ret;
}

/** Prints a formatted string to a file
 * @param[in] _pstFile             Pointer on the file descriptor
 * @param[in] _zString             Formatted string
 * @return Returns the number of written characters
 */
orxS32 orxCDECL orxFile_Print(orxFILE *_pstFile, orxSTRING _zString, ...)
{
  orxS32 s32Result;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);

  /* Valid input? */
  if(_pstFile != orxNULL)
  {
    va_list stArgs;

    /* Gets variable arguments & print the string */
    va_start(stArgs, _zString);
    s32Result = vfprintf((FILE *)_pstFile, _zString, stArgs);
    va_end(stArgs);
  }

  /* Done! */
  return s32Result;
}

/** Gets text line from a file
 * @param[out] _zBuffer       Pointer where will be stored datas
 * @param[in] _u32Size        Size of buffer
 * @param[in] _pstFile        Pointer on the file descriptor
 * @return Returns orxTRUE if a line has been read, else returns orxFALSE.
 */
orxBOOL orxFASTCALL orxFile_ReadLine(orxSTRING _zBuffer, orxU32 _u32Size, orxFILE *_pstFile)
{
  /* Default return value */
  orxBOOL bRet = orxFALSE;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Checks inputs */
  orxASSERT(_pstFile != orxNULL);

  /* Valid input ? */
  if(_pstFile != orxNULL)
  {
    /* Try to read a line */
    if(fgets(_zBuffer, _u32Size, (FILE*)_pstFile))
    {
      bRet = orxTRUE;
    }
    else
    {
      bRet = orxFALSE;
    }
  }

  /* Returns orxTRUE if a line has been read, else orxFALSE */
  return bRet;
}

/** Closes an oppened file
 * @param[in] _pstFile             File's pointer to close
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxFile_Close(orxFILE *_pstFile)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Checks inputs */
  orxASSERT(_pstFile != orxNULL);

  /* valid ? */
  if(_pstFile != orxNULL)
  {
    /* Close file pointer */
    if(fclose((FILE*)_pstFile) == 0)
    {
      /* Success ! */
      eRet = orxSTATUS_SUCCESS;
    }
  }

  /* return success status */
  return eRet;
}


#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */
