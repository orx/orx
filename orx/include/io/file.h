/** 
 * \file file.h
 * 
 * Module de gestion des fichiers et des répertoires.
 * Module qui permet de gérer tout ce qui concerne la gestion des fichiers
 * (necessaire pour gerer les fichiers de maniere transparente quelque soit
 * la plateforme de destination)
 * \todo Vérifier s'il n'y a pas de fuites de mémoires et regarder s'il n'est pas possible de faire
 * de buffer overflow. (regarder en particulier les buffers pour les noms de fichiers ou les patterns)
 * Nettoyer le code qui est un peu sale (les tests conditionels sur les plateformes peuvent être mieux
 * écrits.
 * Traduire le morceau de documentation de ce package qui est encore en Francais vvers l'anglais.
 * Ajouter des fonctions de lecture et écriture typés comme l'a proposé Cursor (C.f. Mailing liste du 7/2/2003)
 */

/*
 * begin                : 15/03/2002
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

#ifndef _FILE_H_
#define _FILE_H_

#include <time.h>

#include "orxInclude.h"

#include "msg/msg_file.h"


#define F_NORMAL 0x01 /**< normal file */
#define F_RDONLY 0x02 /**< read-only file */
#define F_HIDDEN 0x04 /**< hidden file */
#define F_SUBDIR 0x08 /**< directory */


/** structure d'informations sur le fichier. */
typedef struct st_file_infos
{
  orxS32 i_id;        /**< id de la recherche */
  orxS32 i_attrib;    /**< attributs du fichier. voir dessus */
  time_t st_time;    /**< heure de modification du fichier */
  long s32_size;     /**< taille du fichier */
  orxU8 ac_name[512]; /**< nom du fichier */
  orxU8 ac_path[512]; /**< directory path of the file */
} file_st_file_infos;

/** Teste si un fichier existe.
 * \param _zFile_name nom du fichier a tester
 * \return retourne orxFALSE si _zFile_name n'existe pas, sinon orxTRUE
 */
orxBOOL file_exist(orxU8 *_zFile_name);

/** se positionne sur le premier fichier correspondant au pattern donné.
 * \param _z_pattern definition du pattern
 * \param _pstInfo structure qui sera initialise avec les informations sur le fichier
 * \return retourn orxTRUE si le fichier est trouve, orxFALSE  dans le cas contraire
 */
orxBOOL file_find_first(orxU8 *_z_pattern, file_st_file_infos *_pstInfo);

/** se positionne sur le fichier suivant, la recherche doit avoir commencé par un file_find_first.
 * \param _pstInfo structure qui contient les informations sur le fichier precedent,
 * et qui sera rendu avec les informations du fichier suivant
 * \return retourne orxTRUE si un fichier suivant est trouve, orxFALSE sinon (en cas de orxFALSE, la recherche est termine
 * automatiquement, pas besoin d'appel a la fonction file_find_close)
 */
orxBOOL file_find_next(file_st_file_infos *_pstInfo);

/** Termine une recherche commencé par un file_find_first.
 * \param _pstInfo dernieres informations qui ont ete rendu par la recherche qui permet de savoir de 
 * quelle recherche il s'agit.
 */
orxVOID file_find_close(file_st_file_infos *_pstInfo);

/** Copy a file.
 * \param _zFile_src source file
 * \param _zFile_dest destination file
 * \return Returns orxTRUE if the operation succeed, else orxFALSE and errno contains the error code
 */
orxBOOL file_copy(orxU8 *_zFile_src, orxU8 *_zFile_dest);

/** Delete a file.
 * \param _zFile file to delete
 * \return Returns orxTRUE if the operation succeed, else orxFALSE and errno contains the error code
 */
orxBOOL file_delete(orxU8 *_zFile);

/** Rename a file.
 * \param _zFile_src file to rename
 * \param _zFile_dest new file
 * \return Returns orxTRUE if the operation succeed, else orxFALSE and errno contains the error code
 */
orxBOOL file_rename(orxU8 *_zFile_src, orxU8 *_zFile_dest);
 
/** Create a new directory.
 * \param _z_directory_name name of the new directory
 * \return Returns orxTRUE if the operation succeed, else orxFALSE and errno contains the error code
 */
orxBOOL file_mkdir(orxU8 *_z_directory_name);
 
/** Remove a directory.
 * \param _z_directory_name name of the directory to remove
 * \return Returns orxTRUE if the operation succeed, else orxFALSE and errno contains the error code
 */
orxBOOL file_rmdir(orxU8 *_z_directory_name);

/** Delete recursively a direcory and all its subfolders.
 * \param _z_directory_name name of the directory directory to remove
 * \return Returns orxTRUE if the operation succeed, else orxFALSE and errno contains the error code
 */
orxBOOL file_deltree(orxU8 *_z_directory_name);

/** Temporary function... only for developement...
 * will be removed
 */
orxS32 file_list_orxDEBUG_LOG();

#endif /* _FILE_H_ */
