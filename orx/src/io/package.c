#include "io/package.h"

#include "memory/orxMemory.h"


#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER /* pour l'instant je ne sais pas trop quoi mettre... c pour msvc qui a besoin d'autres include */
  #include <io.h>     /* pour open/close/read/write */
  #include <direct.h> /* pour mkdir */
#endif

#ifdef __linux__
  #define mkdir(directory) mkdir(directory, S_IRWXU)
  #include <unistd.h>
  #include <sys/types.h>
#endif /* linux */

typedef struct st_pack_open_node_t
{
  orxS32 i_id;                            /* the file identifier */
  orxU8 *zFile;                          /* the file name */
  orxU8 *z_dir;                           /* the name of the ressource directory */
  orxS32 i_access;                        /* the access mode of the file. see package.h */
  orxS32 i_overwrite;                     /* the overwrite mode of the file. see package.h */
  orxS32 i_mode;                          /* the intern/extern mode. see package.h */
  struct st_pack_open_node_t *pstNext;  /* a pointer on the next node */

  /* 4 extra bytes of padding : 32 */
  orxU8 au8Unused[4];
} st_pack_open_node;



static st_pack_open_node *spst_open_linked_list = orxNULL;
/* the head of the linked which contain all the opened package */



orxS32 make_backup_extern(st_pack_open_node *_pst_pack_infos, orxU8 *_zFile);



/* This function insert a new node in the linked list of all opened package file
 * _i_id = file identifier
 * _zFile = name of the package file
 * _z_dir = name of the directory which contain the exploded version of the package
 * _i_access = access mode of the file ( PACK_O_READ - PACK_O_WRITE - PACK_O_READ | PACK_O_WRITE)
 */
orxVOID insert_open_linked_list(orxS32 _i_id, orxU8 *_zFile, orxU8 *_z_dir, orxS32 _i_access)
{
  st_pack_open_node *st_new_node;
  
  st_new_node = (st_pack_open_node *)orxMemory_Allocate(sizeof(st_pack_open_node), orxMEMORY_TYPE_MAIN);
  st_new_node->i_id = _i_id;
  st_new_node->zFile = _zFile;
  st_new_node->z_dir = _z_dir;
  st_new_node->i_access = _i_access;
  st_new_node->i_overwrite = PACK_OVR_BACKUP;
  st_new_node->i_mode = PACK_M_INTERN;
  st_new_node->pstNext = spst_open_linked_list;
  spst_open_linked_list = st_new_node;
}



/* comments in package.h */
orxS32 package_open(orxCONST orxU8 *_zFile, orxCONST orxS32 _i_access_mode)
{
  orxS32 i_id, len;
  orxU8 *zFile;
  orxU8 *z_dir_name;
  orxU8 ac_dir[256];
  orxU8 *z_pointer;

  len = (strlen(KZ_PACKAGE_RESSOURCE_DIR) + strlen(_zFile) + 1)*sizeof(char);
  zFile = (orxU8 *)orxMemory_Allocate(len, orxMEMORY_TYPE_MAIN);
  sprintf(zFile, "%s%s", KZ_PACKAGE_RESSOURCE_DIR, _zFile);

  z_pointer = strrchr(_zFile, '.');
  if (z_pointer==orxNULL)
    len = strlen(_zFile);
  else
    len = (z_pointer - _zFile);
  strncpy(ac_dir, _zFile, len);
  ac_dir[len*sizeof(char)] = '\0';
  z_dir_name = (orxU8 *)orxMemory_Allocate((len + strlen(KZ_PACKAGE_RESSOURCE_DIR) + 6)*sizeof(char), orxMEMORY_TYPE_MAIN);

#ifdef __linux__
  sprintf(z_dir_name, "%s%s_dir/", KZ_PACKAGE_RESSOURCE_DIR, ac_dir);
#else
  sprintf(z_dir_name, "%s%s_dir\\", KZ_PACKAGE_RESSOURCE_DIR, ac_dir);
#endif

  mkdir(z_dir_name);

  switch (_i_access_mode)
  {
    case PACK_O_READ:
      i_id = open(zFile, O_RDONLY, S_IREAD | S_IWRITE);
      break;
    case PACK_O_WRITE:
      i_id = open(zFile, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      break;
    case PACK_O_READ | PACK_O_WRITE:
      i_id = open(zFile, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      break;
     default:
      return -1;
      break;
  }
  if (i_id!=-1)
    insert_open_linked_list(i_id, zFile, z_dir_name, _i_access_mode);
  return i_id;
}



/* comments in package.h */
orxBOOL package_close(orxS32 _i_pack_id)
{
  st_pack_open_node *spst_open_linked_list_head;
  st_pack_open_node *st_pack_open_previous_node;
  orxBOOL found;
  orxS32 i_i;
  
  spst_open_linked_list_head = spst_open_linked_list;
  st_pack_open_previous_node = spst_open_linked_list;
  found = orxFALSE;
  i_i = 0;
  
  while (spst_open_linked_list!=orxNULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = orxTRUE;
      close(spst_open_linked_list->i_id);
      if (i_i==0)
        spst_open_linked_list_head = spst_open_linked_list->pstNext;
      else
        st_pack_open_previous_node->pstNext = spst_open_linked_list->pstNext;
      orxMemory_Free(spst_open_linked_list);
    }
    i_i++;
    spst_open_linked_list = spst_open_linked_list->pstNext;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
    return 0;
  return 1;  
}



/* JUSTE POUR LE orxDEBUGAGE !*/
orxVOID package_orxDEBUG_list()
{
  st_pack_open_node *spst_open_linked_list_head;
  spst_open_linked_list_head = spst_open_linked_list;
  printf("ID                       FILE                       DIR  ACCESS  OVERWRITE  MODE\n");
  printf("--  -------------------------  ------------------------  ------  ---------  ----\n");
  while (spst_open_linked_list!=orxNULL)
  {
    printf("%2ld  %25s %25s %7lx %10ld %5lx\n", spst_open_linked_list->i_id, spst_open_linked_list->zFile, spst_open_linked_list->z_dir, spst_open_linked_list->i_access, spst_open_linked_list->i_overwrite, spst_open_linked_list->i_mode);
    spst_open_linked_list = spst_open_linked_list->pstNext;
  }
  printf("orxNULL\n");
  spst_open_linked_list = spst_open_linked_list_head;
}
/* FIN orxDEBUG */



orxBOOL package_set_intern(orxS32 _i_pack_id)
{
  st_pack_open_node *spst_open_linked_list_head;
  orxBOOL found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = orxFALSE;
  
  while (spst_open_linked_list!=orxNULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = orxTRUE;
      spst_open_linked_list->i_mode = PACK_M_INTERN;
    }
    spst_open_linked_list = spst_open_linked_list->pstNext;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
    return 0;
  return 1;
}



orxBOOL package_set_extern(orxS32 _i_pack_id)
{
  st_pack_open_node *spst_open_linked_list_head;
  orxBOOL found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = orxFALSE;
  
  while (spst_open_linked_list!=orxNULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = orxTRUE;
      spst_open_linked_list->i_mode = PACK_M_EXTERN;
    }
    spst_open_linked_list = spst_open_linked_list->pstNext;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
  {
    errno = EBADF; /* Bad file descriptor */
     return 0;
  }
  return 1;  
}



orxBOOL package_set_overwrite(orxS32 _i_pack_id, orxS32 _i_mode)
{
  st_pack_open_node *spst_open_linked_list_head;
  orxBOOL found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = orxFALSE;
  
  if ((_i_mode!=PACK_OVR_BACKUP && _i_mode!=PACK_OVR_DELETE) && _i_mode!=PACK_OVR_ERROR)
    return 0;
  
  while (spst_open_linked_list!=orxNULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = orxTRUE;
      spst_open_linked_list->i_overwrite = _i_mode;
    }
    spst_open_linked_list = spst_open_linked_list->pstNext;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
  {
    errno = EBADF; /* Bad file descriptor */
    return 0;
  }
  return 1;  
}



orxS32 package_get_infos(orxS32 _i_pack_id, st_pack_open_node *pstInfo)
{
  st_pack_open_node *spst_open_linked_list_head;
  orxBOOL found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = orxFALSE;
  
  while (spst_open_linked_list!=orxNULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = orxTRUE;
      pstInfo->i_access = spst_open_linked_list->i_access;
      pstInfo->i_overwrite = spst_open_linked_list->i_overwrite;
      pstInfo->i_mode = spst_open_linked_list->i_mode;
      pstInfo->zFile = spst_open_linked_list->zFile;
      pstInfo->z_dir = spst_open_linked_list->z_dir;
    }
    spst_open_linked_list = spst_open_linked_list->pstNext;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  
  if (!found)
  {
    errno = EBADF; /* Bad file descriptor */
    return 0;
  }
  return 1;
}



orxS32 file_exists_intern(st_pack_open_node *_pst_pack_infos, orxU8 *_zFile)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _zFile = orxNULL;
  return -1;
}



orxS32 file_exists_extern(st_pack_open_node *_pst_pack_infos, orxU8 *_zFile)
{
  orxU8 *zFile_path;
  orxS32 i_id;
  
  zFile_path = (orxU8 *)orxMemory_Allocate((strlen(_pst_pack_infos->z_dir) + strlen(_zFile) + 1)*sizeof(char), orxMEMORY_TYPE_MAIN);
  sprintf(zFile_path, "%s%s", _pst_pack_infos->z_dir, _zFile);

  i_id = open(zFile_path, O_RDONLY);
  close(i_id);
  orxMemory_Free(zFile_path);
  if (i_id == -1)
    return 0;
  else
    return 1;
}



orxS32 package_remove_intern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_zFile)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _zFile = orxNULL;
  return 0;
}



orxS32 package_remove_extern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_zFile)
{  
  orxU8 *zFile_path;
  
  zFile_path = (orxU8 *)orxMemory_Allocate((strlen(_pst_pack_infos->z_dir) + strlen(_zFile) + 1)*sizeof(char), orxMEMORY_TYPE_MAIN);
  sprintf(zFile_path, "%s%s", _pst_pack_infos->z_dir, _zFile);

  if (remove(zFile_path)==-1)
    return 0;
  else
    return 1;
}



orxS32 package_copy_intern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_z_src_file, orxU8 *_z_dest_file)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _z_src_file = orxNULL;
  _z_dest_file = orxNULL;
  return 0;
}



orxS32 package_copy_extern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_z_src_file, orxU8 *_z_dest_file)
{
  orxS32 i_id_src;
  orxS32 i_id_dest;
  orxBOOL b_eof;
  orxBOOL b_error;
  orxS32 i_nb;
  orxVOID *p_buffer;

  orxU8 *zFile_src_path;
  orxU8 *zFile_dest_path;

  switch (_pst_pack_infos->i_overwrite)
  {
    case PACK_OVR_BACKUP:
      if (file_exists_extern(_pst_pack_infos, _z_dest_file))
        if (!make_backup_extern(_pst_pack_infos, _z_dest_file))
          return 0;
    case PACK_OVR_DELETE:
      zFile_src_path = (orxU8 *)orxMemory_Allocate((strlen(_pst_pack_infos->z_dir) + strlen(_z_src_file) + 1)*sizeof(char), orxMEMORY_TYPE_MAIN);
      sprintf(zFile_src_path, "%s%s", _pst_pack_infos->z_dir, _z_src_file);
      
      i_id_src = open(zFile_src_path, O_RDONLY);
      orxMemory_Free(zFile_src_path);
      if (i_id_src==-1)
      {
        close(i_id_src);
        return 0;
      }

      zFile_dest_path = (orxU8 *)orxMemory_Allocate((strlen(_pst_pack_infos->z_dir) + strlen(_z_dest_file) + 1)*sizeof(char), orxMEMORY_TYPE_MAIN);
      sprintf(zFile_dest_path, "%s%s", _pst_pack_infos->z_dir, _z_dest_file);

      i_id_dest = open(zFile_dest_path, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      orxMemory_Free(zFile_dest_path);
      if (i_id_dest==-1)
      {
        close(i_id_src);
        close(i_id_dest);
        return 0;
      }
       
      p_buffer = (orxVOID *)orxMemory_Allocate(256 * sizeof(char), orxMEMORY_TYPE_MAIN);
      b_eof = orxFALSE;
      b_error = orxFALSE;

      while (!b_eof && !b_error)
      {
        i_nb = read(i_id_src, p_buffer, 256);
        if (i_nb==-1)
          b_error = orxTRUE;
        else if (i_nb==0)
          b_eof = orxTRUE;
        else
        {
          i_nb = write(i_id_dest, p_buffer, i_nb);
          if (i_nb==-1)
            b_error = orxTRUE;
        }
      }
      orxMemory_Free(p_buffer);
      close(i_id_src);
      close(i_id_dest);
      if (b_error)
      {
        package_remove_extern(_pst_pack_infos, _z_dest_file);
        return 0;
      }
      break;
    case PACK_OVR_ERROR:
      errno = EEXIST; /* Fichier existant */
      return 0;
      break;
    default:
      errno = EINVAL; /* Argument invalide */
      return 0;
      break;
  }
  return 1;
}



orxS32 make_backup_intern(st_pack_open_node *_pst_pack_infos, orxU8 *_zFile)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _zFile = orxNULL;
  return 0;
}



orxS32 make_backup_extern(st_pack_open_node *_pst_pack_infos, orxU8 *_zFile)
{
  orxS32 i_i;
  orxU8 ac_backup_file[512];

  i_i = 0;
  sprintf(ac_backup_file, "%s.bak%ld", _zFile, i_i);
  while(file_exists_extern(_pst_pack_infos, ac_backup_file) && i_i<KI_MAX_BACKUP)
  {
    sprintf(ac_backup_file, "%s.bak%ld", _zFile, i_i);
    i_i++;
  }
  if (i_i<KI_MAX_BACKUP)
    return package_copy_extern(_pst_pack_infos, _zFile, ac_backup_file);
  else
  {
    errno = EMFILE;
    return 0; /* There are too many backup... or there is a problem in file_exists_extern() function */
  }
}



orxBOOL package_add_intern(st_pack_open_node *_pst_pack_infos, orxVOID *_p_data, orxU8 *_zFile, size_t _i_data_size)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _p_data = orxNULL;
  _zFile = orxNULL;
  _i_data_size = 0;
  return orxFALSE;
}



orxBOOL package_add_extern(st_pack_open_node *_pst_pack_infos, orxVOID *_p_data, orxU8 *_zFile, size_t _i_data_size)
{
  orxS32 i_id;
  orxU8 *zFile_path;
  
  zFile_path = (orxU8 *)orxMemory_Allocate((strlen(_pst_pack_infos->z_dir) + strlen(_zFile) + 1)*sizeof(char), orxMEMORY_TYPE_MAIN);
  sprintf(zFile_path, "%s%s", _pst_pack_infos->z_dir, _zFile);
   
  switch (_pst_pack_infos->i_overwrite)
  {
    case PACK_OVR_BACKUP:
      if (file_exists_extern(_pst_pack_infos, _zFile))
        if(!make_backup_extern(_pst_pack_infos, _zFile))
          return orxFALSE;
    case PACK_OVR_DELETE:
      i_id = open(zFile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      orxMemory_Free(zFile_path);
      if (i_id==-1)
      {
        close(i_id);
        return orxFALSE;
      }
      if (write(i_id, _p_data, _i_data_size)==-1)
      {
        close(i_id);
        return orxFALSE;
      }
      close(i_id);
      break;
    case PACK_OVR_ERROR:
      errno = EEXIST; /* Fichier existant */
      return orxFALSE;
      break;
    default:
      errno = EINVAL; /* Argument invalide */
      return orxFALSE;
      break;
  }
  return orxTRUE;
}



orxBOOL package_add(orxS32 _i_pack_id, orxVOID *_p_data, orxU8 *_zFile, size_t _i_data_size)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return orxFALSE;
  
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return orxFALSE;
  }

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_add_intern(&st_pack_infos, _p_data, _zFile, _i_data_size);
  else
    return package_add_extern(&st_pack_infos, _p_data, _zFile, _i_data_size);
}



orxBOOL package_copy(orxS32 _i_pack_id, orxCONST orxU8 *_z_src_file, orxU8 *_z_dest_file)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return orxFALSE;
 
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return orxFALSE;
  }

  if (strcmp(_z_src_file, _z_dest_file)==0)
    return orxTRUE;

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_copy_intern(&st_pack_infos, _z_src_file, _z_dest_file);
  else
    return package_copy_extern(&st_pack_infos, _z_src_file, _z_dest_file);
}



orxBOOL package_remove(orxS32 _i_pack_id, orxCONST orxU8 *_zFile)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return orxFALSE;
 
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return orxFALSE;
  }

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_remove_intern(&st_pack_infos, _zFile);
  else
    return package_remove_extern(&st_pack_infos, _zFile);
}



orxBOOL package_move_intern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_z_src_file, orxU8 *_z_dest_file)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _z_src_file = orxNULL;
  _z_dest_file = orxNULL;
  return orxFALSE;
}



orxBOOL package_move_extern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_z_src_file, orxU8 *_z_dest_file)
{
  orxU8 *zFile_src_path;
  orxU8 *zFile_dest_path;

  switch (_pst_pack_infos->i_overwrite)
  {
    case PACK_OVR_BACKUP:
      if (file_exists_extern(_pst_pack_infos, _z_dest_file))
        if (!make_backup_extern(_pst_pack_infos, _z_dest_file))
          return orxFALSE;
    case PACK_OVR_DELETE:
      if (!package_remove_extern(_pst_pack_infos, _z_dest_file))
        return orxFALSE;

      zFile_src_path = (orxU8 *)orxMemory_Allocate((strlen(_pst_pack_infos->z_dir) + strlen(_z_src_file) + 1)*sizeof(char), orxMEMORY_TYPE_MAIN);
      sprintf(zFile_src_path, "%s%s", _pst_pack_infos->z_dir, _z_src_file);

      zFile_dest_path = (orxU8 *)orxMemory_Allocate((strlen(_pst_pack_infos->z_dir) + strlen(_z_dest_file) + 1)*sizeof(char), orxMEMORY_TYPE_MAIN);
      sprintf(zFile_dest_path, "%s%s", _pst_pack_infos->z_dir, _z_dest_file);

      if (rename(zFile_src_path, zFile_dest_path)!=0)
      {
        orxMemory_Free(zFile_src_path);
        orxMemory_Free(zFile_dest_path);
        return orxFALSE;
      }
      else
      {
        orxMemory_Free(zFile_src_path);
        orxMemory_Free(zFile_dest_path);
        return orxTRUE;
      }
      break;
    case PACK_OVR_ERROR:
      errno = EEXIST; /* Fichier existant */
      return orxFALSE;
      break;
    default:
      errno = EINVAL; /* Argument invalide */
      return orxFALSE;
      break;
  }
  return orxTRUE;
}



orxBOOL package_move(orxS32 _i_pack_id, orxCONST orxU8 *_z_src_file, orxU8 *_z_dest_file)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return orxFALSE;
 
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return orxFALSE;
  }

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_move_intern(&st_pack_infos, _z_src_file, _z_dest_file);
  else
    return package_move_extern(&st_pack_infos, _z_src_file, _z_dest_file);
}

orxBOOL package_find_first_intern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _z_pattern = orxNULL;
  _st_fileinfos = orxNULL;
  return orxFALSE;
}

orxBOOL package_find_first_extern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _z_pattern = orxNULL;
  _st_fileinfos = orxNULL;
  return orxFALSE;
}

orxBOOL package_find_first_intern_extern(st_pack_open_node *_pst_pack_infos, orxCONST orxU8 *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = orxNULL;
  _z_pattern = orxNULL;
  _st_fileinfos = orxNULL;
  return orxFALSE;
}

orxBOOL package_find_first(orxS32 _i_pack_id, orxCONST orxU8 *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return orxFALSE;
 
  switch (st_pack_infos.i_mode)
  {
    case PACK_M_INTERN:
      return package_find_first_intern(&st_pack_infos, _z_pattern, _st_fileinfos);
      break;
    case PACK_M_EXTERN:
      return package_find_first_extern(&st_pack_infos, _z_pattern, _st_fileinfos);
      break;
    case PACK_M_EXTERN | PACK_M_INTERN:
      return package_find_first_intern_extern(&st_pack_infos, _z_pattern, _st_fileinfos);
      break;
    default:
      return orxFALSE;
      break;
  }
}



orxBOOL package_find_next_intern(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = orxNULL;
  return orxFALSE;
}

orxBOOL package_find_next_extern(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = orxNULL;
  return orxFALSE;
}

orxBOOL package_find_next_intern_extern(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = orxNULL;
  return orxFALSE;
}

orxBOOL package_find_next(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = orxNULL;
  return orxFALSE;
}
