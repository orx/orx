/** 
 * \file package.h
 * Module qui permet de gérer le stockage des ressources dans le ou les
 * fichiers de packages.
 */

/*
 * begin                : 24/08/2002
 * author               : (C) Gdp
 * email                : the_bestel@free.fr
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include "include.h"


#define PACK_O_READ     0x01    /**< package can be read */
#define PACK_O_WRITE    0x02    /**< package can be written */

#define PACK_M_INTERN   0x01    /**< package in intern Mode (compiled) */
#define PACK_M_EXTERN   0x02    /**< package in extern Mode (exploded) */

#define PACK_OVR_BACKUP 0       /**< Overwrite mode - Make a backup when overwritting */
#define PACK_OVR_DELETE 1       /**< Overwrite mode - Delete the old file */
#define PACK_OVR_ERROR  2       /**< Overwrite mode - Don't change the old file */

#define PACK_A_NORMAL   0x00    /**< Normal file - No read/write restrictions */
#define PACK_A_RDONLY   0x01    /**< Read only file */
#define PACK_A_HIDDEN   0x02    /**< Hidden file */
#define PACK_A_SYSTEM   0x04    /**< System file */
#define PACK_A_SUBDIR   0x10    /**< Subdirectory */
#define PACK_A_ARCH     0x20    /**< Archive file */

#ifdef __linux__
  #define KZ_PACKAGE_RESSOURCE_DIR "../game/data/"
#else
  #define KZ_PACKAGE_RESSOURCE_DIR "..\\game\\data\\"
#endif

#define KI_MAX_BACKUP 10

/**
 * Information structure on a package
 */
typedef struct
{
  uint32 ui_id;    /**< Identifier of the current research */
  char ac_filename[260];   /**< filename of the current file */
  int32 i_attrib;          /**< attibutes of the file */
} package_st_file_infos;

/** Open a package file.
 * \param _z_file_path A string on the file to open.
 * \param _i_access_mode A combination of access_mode. they can be mixed with a
 * binary operator OR |
 * \return this function return an identifier on the opened file or -1 if the file
 * can't be opened.
 */
int32 package_open(const char *_z_file_path, const int32 _i_access_mode);

/** Close a package file.
 * \param _i_pack_id : package's identifier
 * \return this function return TRUE if there are no errors or FALSE if the file cannot be
 * closed.
 */
bool package_close(int32 _i_pack_id);

/** Set the package operation in intern mode.
 * This means that some operation will only take effect in the compiled
 * ressource's file.
 * \param _i_pack_id id of the package
 * \return If the package id is incorrect, it returns FALSE, else it returns TRUE.
 */
bool package_set_intern(int32 _i_pack_id);

/** Set the package operation in extern mode.
 * This means that some operation will only take effect in the uncompiled
 * ressource's file.
 * \param _i_pack_id id of the package
 * \return If the package id is incorrect, it returns FALSE, else it returns TRUE.
 */
bool package_set_extern(int32 _i_pack_id);

/** Add a portion of memory to the package. If the package is in intern mode,
 * the new data will be added to the compiled file, else the file will be created on
 * the disk, in the ressource directory.
 * \param _i_pack_id package's identifier
 * \param _p_data a pointer on the portion of data to add
 * \param _z_file the filename of the future file which contain the data
 * \param _i_data_size the size of the data
 * \return If the function succed, it returns TRUE, else it returns FALSE and errno is positioned with
 * the right value.
 * \todo Add the code for the intern mode (still working in extern mode only)
 */
bool package_add(int32 _i_pack_id, void *_p_data, char *_z_file, size_t _i_data_size);

/** Set the mode for overwriting. For example, if we are in rename mode, we rename
 * and save an old file before to overwrite it.
 * \param _i_pack_id package's identifier
 * \param _i_mode : PACK_OVR_BACKUP (make a backup of a file)
 * PACK_OVR_DELETE (replace the file without backup)
 * PACK_OVR_ERROR  (return an error and don't replace file)
 * \return return FALSE if the file can't be replaced in PACK_OVR_ERROR, else it returns TRUE
 */
bool package_set_overwrite(int32 _i_pack_id, int32 _i_mode);

/** Copy a file. If the mode is intern, files
 * are copied in the compiled package. Else the operation is done in the exploded
 * files.
 * \param _i_pack_id package's identifier
 * \param _z_src_file source file.
 * \param _z_dest_file destination file.
 * \return if the overwrite mode is PACK_OVR_BACKUP, and the _z_dest_file already exist, a 
 * backup of the old file is done before the copy. if the overwrite mode is
 * PACK_OVR_DELETE, the old file is replaced, and if the overwrite mode is set on
 * PACK_OVR_ERROR, the file isn't copied and the function returns FALSE. Else it returns TRUE
 * \todo Add the code for the intern mode (still working in extern mode only)
 */
bool package_copy(int32 _i_pack_id, const char *_z_src_file, char *_z_dest_file);

/** Move a file. If the mode is intern, files
 * are copied in the compiled package. Else the operation is done in the exploded
 * files.
 * \param _i_pack_id package's identifier
 * \param _z_src_file source file.
 * \param _z_dest_file destination file.
 * \return If a file exists in the compiled file with the same name
 * as _z_dest_file, operation are done with the overwite_mode's rule. (see package_copy
 * for more informations and return value)
 * \todo Add the code for the intern mode (still working in extern mode only)
 */
bool package_move(int32 _i_pack_id, const char *_z_src_file, char *_z_dest_file);

/** Remove a file. If the mode is intern, file is removed in the
 * compiled package. Else the operation is done in the exploded files.
 * \param _i_pack_id package's identifier
 * \param _z_file file.
 * \return TRUE if the operation succeed, else FALSE
 * \todo Add the code for the intern mode (still working in extern mode only)
 */
bool package_remove(int32 _i_pack_id, const char *_z_file);

/** Insert a file from the directory ressource into the package file
 * \param _i_pack_id package's identifier
 * \param _z_file the file to insert into the compiled file
 * \return If a file exists in the compiled file with the same name
 * as _z_file, operation are done with the overwite_mode's rule. (see package_copy for
 * more informations and return value)
 * \todo Implement the function body... (no current implementation)
 */
bool package_commit(int32 _i_pack_id, char *_z_file);

/** Extract a file from the package file identified to
 * the exploded directories. 
 * \param _i_pack_id package's identifier
 * \param _z_file the file to extract from the compiled file
 * \return If a file exists with the same name as _z_file
 * operation are done with the overwite_mode's rule. (see package_copy for
 * more informations and return value)
 * \todo Implement the function body... (no current implementation)
 */
bool package_checkout(int32 _i_pack_id, char *_z_file);

/** Copy a file into the memory the and returns a pointer on the allocated area.
 * If the last parameters is not set,
 * the file is taken from the mode selected (intern or extern).
 * the user can also choose to specify the mode with PACK_M_INTERN or PACK_M_EXTERN.
 * if the mode set is PACK_M_INTERN | PACK_M_EXTERN, the program will take the file in
 * the exploded ressource directory... and if it doesn't exist, it will take it in
 * the compiled file.
 * \param _pi_data_size contains the size of the data. 
 * \param _i_pack_id : package's identifier
 * \param _z_file : the file to load into memory
 * \param _pi_data_size : the size of the data loaded
 * \return If the file can't be loaded, the function returns NULL, else it return a
 * \todo Implement the function body... (no current implementation)
 * pointer on the memory data.
 */
void *package_get_file(int32 _i_pack_id, const char *_z_file, int32 *_pi_data_size, ...);

/** Insert all files which are in the ressource directory (in extern mode) into
 * the compiled file.
 * \param _i_pack_id package's identifier
 * \return If the package id is incorrect, it returns FALSE, else it returns TRUE.
 * \todo Implement the function body... (no current implementation)
 */
bool package_compile(int32 _i_pack_id);

/** Explode all files which are in the compiled package to the ressource directory.
 * \param _i_pack_id package's identifier
 * \return If the package id is incorrect, it returns FALSE, else it returns TRUE.
 * \todo Implement the function body... (no current implementation)
 */
bool package_explode(int32 _i_pack_id);

/** Zip all files which are in the ressource directory (in extern mode) and delete
 * them
 * \param _i_pack_id package's identifier
 * \return If the package id is incorrect, it returns FALSE, else it returns TRUE.
 * \todo Implement the function body... (no current implementation)
 */
bool package_archive(int32 _i_pack_id);

/** Look for the first file which correspond to a pattern
 * \param _i_pack_id package's identifier
 * \param _z_pattern pattern to recognize (e.g /ressource/img*.jpg)
 * \param _st_fileinfos informations about the file found
 * \todo Implement the function body... (no current implementation)
 * \return The function returns FALSE if the file isn't found, TRUE else.
 */
bool package_find_first(int32 _i_pack_id, const char *_z_pattern, package_st_file_infos *_st_fileinfos);
 
/** Look for the next file of a resarch. Need a call to package_find_first before.
 * \param _st_fileinfos : in = previous value of file informations, out = informations about the next file.
 * \return The function returns FALSE when the complete research is done... TRUE else.
 * \todo Implement the function body... (no current implementation)
 */
bool package_find_next(package_st_file_infos *_st_fileinfos);

/** Debug function.
 * Print a list on stdout of all opened package
 * \todo remove this function in release mode
 */
 void package_DEBUG_list();

#endif /* _PACKAGE_H_ */
