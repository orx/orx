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

#include "include.h"

#include "msg/msg_file.h"


#define F_NORMAL 0x01 /**< normal file */
#define F_RDONLY 0x02 /**< read-only file */
#define F_HIDDEN 0x04 /**< hidden file */
#define F_SUBDIR 0x08 /**< directory */


/** structure d'informations sur le fichier. */
typedef struct st_file_infos
{
  int32 i_id;        /**< id de la recherche */
  int32 i_attrib;    /**< attributs du fichier. voir dessus */
  time_t st_time;    /**< heure de modification du fichier */
  long s32_size;     /**< taille du fichier */
  char ac_name[512]; /**< nom du fichier */
  char ac_path[512]; /**< directory path of the file */
} file_st_file_infos;

/** Teste si un fichier existe.
 * \param _z_file_name nom du fichier a tester
 * \return retourne FALSE si _z_file_name n'existe pas, sinon TRUE
 */
bool file_exist(char *_z_file_name);

/** se positionne sur le premier fichier correspondant au pattern donné.
 * \param _z_pattern definition du pattern
 * \param _pst_infos structure qui sera initialise avec les informations sur le fichier
 * \return retourn TRUE si le fichier est trouve, FALSE  dans le cas contraire
 */
bool file_find_first(char *_z_pattern, file_st_file_infos *_pst_infos);

/** se positionne sur le fichier suivant, la recherche doit avoir commencé par un file_find_first.
 * \param _pst_infos structure qui contient les informations sur le fichier precedent,
 * et qui sera rendu avec les informations du fichier suivant
 * \return retourne TRUE si un fichier suivant est trouve, FALSE sinon (en cas de FALSE, la recherche est termine
 * automatiquement, pas besoin d'appel a la fonction file_find_close)
 */
bool file_find_next(file_st_file_infos *_pst_infos);/** Termine une recherche commencé par un file_find_first.
 * \param _pst_infos dernieres informations qui ont ete rendu par la recherche qui permet de savoir de 
 * quelle recherche il s'agit.
 */
void file_find_close(file_st_file_infos *_pst_infos);

/** Copy a file.
 * \param _z_file_src source file
 * \param _z_file_dest destination file
 * \return Returns TRUE if the operation succeed, else FALSE and errno contains the error code
 */
bool file_copy(char *_z_file_src, char *_z_file_dest);

/** Delete a file.
 * \param _z_file file to delete
 * \return Returns TRUE if the operation succeed, else FALSE and errno contains the error code
 */
bool file_delete(char *_z_file);

/** Rename a file.
 * \param _z_file_src file to rename
 * \param _z_file_dest new file
 * \return Returns TRUE if the operation succeed, else FALSE and errno contains the error code
 */
bool file_rename(char *_z_file_src, char *_z_file_dest);
 
/** Create a new directory.
 * \param _z_directory_name name of the new directory
 * \return Returns TRUE if the operation succeed, else FALSE and errno contains the error code
 */
bool file_mkdir(char *_z_directory_name);
 
/** Remove a directory.
 * \param _z_directory_name name of the directory to remove
 * \return Returns TRUE if the operation succeed, else FALSE and errno contains the error code
 */
bool file_rmdir(char *_z_directory_name);

/** Delete recursively a direcory and all its subfolders.
 * \param _z_directory_name name of the directory directory to remove
 * \return Returns TRUE if the operation succeed, else FALSE and errno contains the error code
 */
bool file_deltree(char *_z_directory_name);

/** Temporary function... only for developement...
 * will be removed
 */
int32 file_list_DEBUG();

#endif /* _FILE_H_ */
