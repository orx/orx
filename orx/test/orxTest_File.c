/**
 * @file orxTest_File.c
 * 
 * File read / write operation tests
 * 
 */
 
 /***************************************************************************
 orxTest_File.c
 File read / write operation tests
 
 begin                : 11/12/2006
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "orxInclude.h"
#include "debug/orxTest.h"
#include "io/orxFile.h"
#include "memory/orxMemory.h"
#include "io/orxTextIO.h"

/******************************************************
 * DEFINES
 ******************************************************/

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this file module
 */
orxVOID orxTest_File_Infos()
{
  orxTextIO_PrintLn("This test module will open / close, read / write in files");
}

/** Allocate memory and store allocated elements in an array
 */
orxVOID orxTest_File_Open()
{
  /* Enter the file name to open */
  orxCHAR zBufferFile[256];
  orxS32 s32Mode;
  
  /* clean buffer */
  orxMemory_Set(zBufferFile, orxCHAR_NULL, 256);

  orxTextIO_PrintLn("Open a file...");
  orxTextIO_ReadString(zBufferFile, 256, "File name to open :");
  orxTextIO_ReadS32InRange(&s32Mode, 16, 0, 7, "Enter the flag mode value (READ | WRITE | APPEND)", orxTRUE);
  
  /* Try to open the file */
  orxFILE *pstFile = orxFile_Open(zBufferFile, s32Mode);
  
  /* valid ? */
  if(pstFile != orxNULL)
  {
    /* File opened */
    orxTextIO_PrintLn("File opened at address %x\n", pstFile);
  }
  else
  {
    orxTextIO_PrintLn("Can't open file");
  }
  
  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/** Read data from a file
 */
orxVOID orxTest_File_Read()
{
  orxS32 s32Address;
  orxS32 s32Read;
  
  orxTextIO_PrintLn("Read a file address...");
  orxTextIO_ReadS32(&s32Address, 16, "File address to use :", orxTRUE);
  
  /* Convert it into an address */
  orxFILE * pstFile = (orxFILE*)s32Address;
  
  /* Print file content */
  orxTextIO_PrintLn("Print the file content :");
  
  /* Loop on file reading */
  orxCHAR acBuffer[256];
  while((s32Read = orxFile_Read(acBuffer, sizeof(orxCHAR), 256, pstFile))
  &&    (s32Read > 0))
  {
    /* Write the memory content */
    orxTextIO_Print(acBuffer);
  }
  
  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/** write data to a file
 */
orxVOID orxTest_File_Write()
{
  orxCHAR acBuffer[1024];
  orxS32 s32Address;
  orxS32 s32Write;
  
  /* clean buffer */
  orxMemory_Set(acBuffer, orxCHAR_NULL, 1024);

  orxTextIO_PrintLn("Write into a file...");
  orxTextIO_ReadString(acBuffer, 1024, "Data to write :");

  /* clean buffer */
  orxMemory_Set(acBuffer, orxCHAR_NULL, 256);

  orxTextIO_PrintLn("Read a file address...");
  orxTextIO_ReadS32(&s32Address, 16, "File address to use :", orxTRUE);
  
  /* Convert it into an address */
  orxFILE * pstFile = (orxFILE*)s32Address;

  /* Save file content */
  orxTextIO_PrintLn("Writing in file...");
  
  /* Write in file */
  s32Write = orxFile_Write(acBuffer, sizeof(orxCHAR), 1024, pstFile);
  orxTextIO_PrintLn("%d characters written", s32Write);
  
  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/** Close a file
 */
orxVOID orxTest_File_Close()
{
  orxS32 s32Address;
  
  orxTextIO_PrintLn("Close a file...");
  orxTextIO_ReadS32(&s32Address, 16, "File address :", orxTRUE);

  /* Convert address in pointer */
  orxFILE *pstFile = (orxFILE*)s32Address;
  
  /* Close the file */
  if(orxFile_Close(pstFile) == orxSTATUS_SUCCESS)
  {
    orxTextIO_PrintLn("File closed.");
  }
  else
  {
    orxTextIO_PrintLn("File not closed. Valid address ?");
  }

  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_File_Init()
{
  /* Initialize Memory module */
  orxModule_Init(orxMODULE_ID_FILE);
  
  /* Register test functions */
  orxTest_Register("File", "Display module informations", orxTest_File_Infos);
  orxTest_Register("File", "Open a file", orxTest_File_Open);
  orxTest_Register("File", "Read a file content", orxTest_File_Read);
  orxTest_Register("File", "Write into a file", orxTest_File_Write);
  orxTest_Register("File", "Close a file", orxTest_File_Close);
}

orxVOID orxTest_File_Exit()
{
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_File_Init, orxTest_File_Exit)
