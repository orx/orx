#include "io/package.h"

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
  int32 i_id;                            /* the file identifier */
  char *z_file;                          /* the file name */
  char *z_dir;                           /* the name of the ressource directory */
  int32 i_access;                        /* the access mode of the file. see package.h */
  int32 i_overwrite;                     /* the overwrite mode of the file. see package.h */
  int32 i_mode;                          /* the intern/extern mode. see package.h */
  struct st_pack_open_node_t *pst_next;  /* a pointer on the next node */

  /* 4 extra bytes of padding : 32 */
  uint8 auc_unused[4];
} st_pack_open_node;



static st_pack_open_node *spst_open_linked_list = NULL;
/* the head of the linked which contain all the opened package */



int32 make_backup_extern(st_pack_open_node *_pst_pack_infos, char *_z_file);



/* This function insert a new node in the linked list of all opened package file
 * _i_id = file identifier
 * _z_file = name of the package file
 * _z_dir = name of the directory which contain the exploded version of the package
 * _i_access = access mode of the file ( PACK_O_READ - PACK_O_WRITE - PACK_O_READ | PACK_O_WRITE)
 */
void insert_open_linked_list(int32 _i_id, char *_z_file, char *_z_dir, int32 _i_access)
{
  st_pack_open_node *st_new_node;
  
  st_new_node = (st_pack_open_node *)malloc(sizeof(st_pack_open_node));
  st_new_node->i_id = _i_id;
  st_new_node->z_file = _z_file;
  st_new_node->z_dir = _z_dir;
  st_new_node->i_access = _i_access;
  st_new_node->i_overwrite = PACK_OVR_BACKUP;
  st_new_node->i_mode = PACK_M_INTERN;
  st_new_node->pst_next = spst_open_linked_list;
  spst_open_linked_list = st_new_node;
}



/* comments in package.h */
int32 package_open(const char *_z_file, const int32 _i_access_mode)
{
  int32 i_id, len;
  char *z_file;
  char *z_dir_name;
  char ac_dir[256];
  char *z_pointer;

  len = (strlen(KZ_PACKAGE_RESSOURCE_DIR) + strlen(_z_file) + 1)*sizeof(char);
  z_file = (char *)malloc(len);
  sprintf(z_file, "%s%s", KZ_PACKAGE_RESSOURCE_DIR, _z_file);

  z_pointer = strrchr(_z_file, '.');
  if (z_pointer==NULL)
    len = strlen(_z_file);
  else
    len = (z_pointer - _z_file);
  strncpy(ac_dir, _z_file, len);
  ac_dir[len*sizeof(char)] = '\0';
  z_dir_name = (char *)malloc((len + strlen(KZ_PACKAGE_RESSOURCE_DIR) + 6)*sizeof(char));

#ifdef __linux__
  sprintf(z_dir_name, "%s%s_dir/", KZ_PACKAGE_RESSOURCE_DIR, ac_dir);
#else
  sprintf(z_dir_name, "%s%s_dir\\", KZ_PACKAGE_RESSOURCE_DIR, ac_dir);
#endif

  mkdir(z_dir_name);

  switch (_i_access_mode)
  {
    case PACK_O_READ:
      i_id = open(z_file, O_RDONLY, S_IREAD | S_IWRITE);
      break;
    case PACK_O_WRITE:
      i_id = open(z_file, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      break;
    case PACK_O_READ | PACK_O_WRITE:
      i_id = open(z_file, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      break;
     default:
      return -1;
      break;
  }
  if (i_id!=-1)
    insert_open_linked_list(i_id, z_file, z_dir_name, _i_access_mode);
  return i_id;
}



/* comments in package.h */
bool package_close(int32 _i_pack_id)
{
  st_pack_open_node *spst_open_linked_list_head;
  st_pack_open_node *st_pack_open_previous_node;
  bool found;
  int32 i_i;
  
  spst_open_linked_list_head = spst_open_linked_list;
  st_pack_open_previous_node = spst_open_linked_list;
  found = FALSE;
  i_i = 0;
  
  while (spst_open_linked_list!=NULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = TRUE;
      close(spst_open_linked_list->i_id);
      if (i_i==0)
        spst_open_linked_list_head = spst_open_linked_list->pst_next;
      else
        st_pack_open_previous_node->pst_next = spst_open_linked_list->pst_next;
      free(spst_open_linked_list);
    }
    i_i++;
    spst_open_linked_list = spst_open_linked_list->pst_next;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
    return 0;
  return 1;  
}



/* JUSTE POUR LE DEBUGAGE !*/
void package_DEBUG_list()
{
  st_pack_open_node *spst_open_linked_list_head;
  spst_open_linked_list_head = spst_open_linked_list;
  printf("ID                       FILE                       DIR  ACCESS  OVERWRITE  MODE\n");
  printf("--  -------------------------  ------------------------  ------  ---------  ----\n");
  while (spst_open_linked_list!=NULL)
  {
    printf("%2ld  %25s %25s %7lx %10ld %5lx\n", spst_open_linked_list->i_id, spst_open_linked_list->z_file, spst_open_linked_list->z_dir, spst_open_linked_list->i_access, spst_open_linked_list->i_overwrite, spst_open_linked_list->i_mode);
    spst_open_linked_list = spst_open_linked_list->pst_next;
  }
  printf("NULL\n");
  spst_open_linked_list = spst_open_linked_list_head;
}
/* FIN DEBUG */



bool package_set_intern(int32 _i_pack_id)
{
  st_pack_open_node *spst_open_linked_list_head;
  bool found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = FALSE;
  
  while (spst_open_linked_list!=NULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = TRUE;
      spst_open_linked_list->i_mode = PACK_M_INTERN;
    }
    spst_open_linked_list = spst_open_linked_list->pst_next;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
    return 0;
  return 1;
}



bool package_set_extern(int32 _i_pack_id)
{
  st_pack_open_node *spst_open_linked_list_head;
  bool found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = FALSE;
  
  while (spst_open_linked_list!=NULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = TRUE;
      spst_open_linked_list->i_mode = PACK_M_EXTERN;
    }
    spst_open_linked_list = spst_open_linked_list->pst_next;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
  {
    errno = EBADF; /* Bad file descriptor */
     return 0;
  }
  return 1;  
}



bool package_set_overwrite(int32 _i_pack_id, int32 _i_mode)
{
  st_pack_open_node *spst_open_linked_list_head;
  bool found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = FALSE;
  
  if ((_i_mode!=PACK_OVR_BACKUP && _i_mode!=PACK_OVR_DELETE) && _i_mode!=PACK_OVR_ERROR)
    return 0;
  
  while (spst_open_linked_list!=NULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = TRUE;
      spst_open_linked_list->i_overwrite = _i_mode;
    }
    spst_open_linked_list = spst_open_linked_list->pst_next;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  if (!found)
  {
    errno = EBADF; /* Bad file descriptor */
    return 0;
  }
  return 1;  
}



int32 package_get_infos(int32 _i_pack_id, st_pack_open_node *pst_infos)
{
  st_pack_open_node *spst_open_linked_list_head;
  bool found;
  
  spst_open_linked_list_head = spst_open_linked_list;
  found = FALSE;
  
  while (spst_open_linked_list!=NULL && !found)
  {
    if (spst_open_linked_list->i_id==_i_pack_id)
    {
      found = TRUE;
      pst_infos->i_access = spst_open_linked_list->i_access;
      pst_infos->i_overwrite = spst_open_linked_list->i_overwrite;
      pst_infos->i_mode = spst_open_linked_list->i_mode;
      pst_infos->z_file = spst_open_linked_list->z_file;
      pst_infos->z_dir = spst_open_linked_list->z_dir;
    }
    spst_open_linked_list = spst_open_linked_list->pst_next;
  }
  spst_open_linked_list = spst_open_linked_list_head;
  
  if (!found)
  {
    errno = EBADF; /* Bad file descriptor */
    return 0;
  }
  return 1;
}



int32 file_exists_intern(st_pack_open_node *_pst_pack_infos, char *_z_file)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_file = NULL;
  return -1;
}



int32 file_exists_extern(st_pack_open_node *_pst_pack_infos, char *_z_file)
{
  char *z_file_path;
  int32 i_id;
  
  z_file_path = (char *)malloc((strlen(_pst_pack_infos->z_dir) + strlen(_z_file) + 1)*sizeof(char));
  sprintf(z_file_path, "%s%s", _pst_pack_infos->z_dir, _z_file);

  i_id = open(z_file_path, O_RDONLY);
  close(i_id);
  free(z_file_path);
  if (i_id == -1)
    return 0;
  else
    return 1;
}



int32 package_remove_intern(st_pack_open_node *_pst_pack_infos, const char *_z_file)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_file = NULL;
  return 0;
}



int32 package_remove_extern(st_pack_open_node *_pst_pack_infos, const char *_z_file)
{  
  char *z_file_path;
  
  z_file_path = (char *)malloc((strlen(_pst_pack_infos->z_dir) + strlen(_z_file) + 1)*sizeof(char));
  sprintf(z_file_path, "%s%s", _pst_pack_infos->z_dir, _z_file);

  if (remove(z_file_path)==-1)
    return 0;
  else
    return 1;
}



int32 package_copy_intern(st_pack_open_node *_pst_pack_infos, const char *_z_src_file, char *_z_dest_file)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_src_file = NULL;
  _z_dest_file = NULL;
  return 0;
}



int32 package_copy_extern(st_pack_open_node *_pst_pack_infos, const char *_z_src_file, char *_z_dest_file)
{
  int32 i_id_src;
  int32 i_id_dest;
  bool b_eof;
  bool b_error;
  int32 i_nb;
  void *p_buffer;

  char *z_file_src_path;
  char *z_file_dest_path;

  switch (_pst_pack_infos->i_overwrite)
  {
    case PACK_OVR_BACKUP:
      if (file_exists_extern(_pst_pack_infos, _z_dest_file))
        if (!make_backup_extern(_pst_pack_infos, _z_dest_file))
          return 0;
    case PACK_OVR_DELETE:
      z_file_src_path = (char *)malloc((strlen(_pst_pack_infos->z_dir) + strlen(_z_src_file) + 1)*sizeof(char));
      sprintf(z_file_src_path, "%s%s", _pst_pack_infos->z_dir, _z_src_file);
      
      i_id_src = open(z_file_src_path, O_RDONLY);
      free(z_file_src_path);
      if (i_id_src==-1)
      {
        close(i_id_src);
        return 0;
      }

      z_file_dest_path = (char *)malloc((strlen(_pst_pack_infos->z_dir) + strlen(_z_dest_file) + 1)*sizeof(char));
      sprintf(z_file_dest_path, "%s%s", _pst_pack_infos->z_dir, _z_dest_file);

      i_id_dest = open(z_file_dest_path, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      free(z_file_dest_path);
      if (i_id_dest==-1)
      {
        close(i_id_src);
        close(i_id_dest);
        return 0;
      }
       
      p_buffer = (void *)malloc(256 * sizeof(char));
      b_eof = FALSE;
      b_error = FALSE;

      while (!b_eof && !b_error)
      {
        i_nb = read(i_id_src, p_buffer, 256);
        if (i_nb==-1)
          b_error = TRUE;
        else if (i_nb==0)
          b_eof = TRUE;
        else
        {
          i_nb = write(i_id_dest, p_buffer, i_nb);
          if (i_nb==-1)
            b_error = TRUE;
        }
      }
      free(p_buffer);
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



int32 make_backup_intern(st_pack_open_node *_pst_pack_infos, char *_z_file)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_file = NULL;
  return 0;
}



int32 make_backup_extern(st_pack_open_node *_pst_pack_infos, char *_z_file)
{
  int32 i_i;
  char ac_backup_file[512];

  i_i = 0;
  sprintf(ac_backup_file, "%s.bak%ld", _z_file, i_i);
  while(file_exists_extern(_pst_pack_infos, ac_backup_file) && i_i<KI_MAX_BACKUP)
  {
    sprintf(ac_backup_file, "%s.bak%ld", _z_file, i_i);
    i_i++;
  }
  if (i_i<KI_MAX_BACKUP)
    return package_copy_extern(_pst_pack_infos, _z_file, ac_backup_file);
  else
  {
    errno = EMFILE;
    return 0; /* There are too many backup... or there is a problem in file_exists_extern() function */
  }
}



bool package_add_intern(st_pack_open_node *_pst_pack_infos, void *_p_data, char *_z_file, size_t _i_data_size)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _p_data = NULL;
  _z_file = NULL;
  _i_data_size = 0;
  return FALSE;
}



bool package_add_extern(st_pack_open_node *_pst_pack_infos, void *_p_data, char *_z_file, size_t _i_data_size)
{
  int32 i_id;
  char *z_file_path;
  
  z_file_path = (char *)malloc((strlen(_pst_pack_infos->z_dir) + strlen(_z_file) + 1)*sizeof(char));
  sprintf(z_file_path, "%s%s", _pst_pack_infos->z_dir, _z_file);
   
  switch (_pst_pack_infos->i_overwrite)
  {
    case PACK_OVR_BACKUP:
      if (file_exists_extern(_pst_pack_infos, _z_file))
        if(!make_backup_extern(_pst_pack_infos, _z_file))
          return FALSE;
    case PACK_OVR_DELETE:
      i_id = open(z_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
      free(z_file_path);
      if (i_id==-1)
      {
        close(i_id);
        return FALSE;
      }
      if (write(i_id, _p_data, _i_data_size)==-1)
      {
        close(i_id);
        return FALSE;
      }
      close(i_id);
      break;
    case PACK_OVR_ERROR:
      errno = EEXIST; /* Fichier existant */
      return FALSE;
      break;
    default:
      errno = EINVAL; /* Argument invalide */
      return FALSE;
      break;
  }
  return TRUE;
}



bool package_add(int32 _i_pack_id, void *_p_data, char *_z_file, size_t _i_data_size)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return FALSE;
  
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return FALSE;
  }

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_add_intern(&st_pack_infos, _p_data, _z_file, _i_data_size);
  else
    return package_add_extern(&st_pack_infos, _p_data, _z_file, _i_data_size);
}



bool package_copy(int32 _i_pack_id, const char *_z_src_file, char *_z_dest_file)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return FALSE;
 
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return FALSE;
  }

  if (strcmp(_z_src_file, _z_dest_file)==0)
    return TRUE;

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_copy_intern(&st_pack_infos, _z_src_file, _z_dest_file);
  else
    return package_copy_extern(&st_pack_infos, _z_src_file, _z_dest_file);
}



bool package_remove(int32 _i_pack_id, const char *_z_file)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return FALSE;
 
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return FALSE;
  }

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_remove_intern(&st_pack_infos, _z_file);
  else
    return package_remove_extern(&st_pack_infos, _z_file);
}



bool package_move_intern(st_pack_open_node *_pst_pack_infos, const char *_z_src_file, char *_z_dest_file)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_src_file = NULL;
  _z_dest_file = NULL;
  return FALSE;
}



bool package_move_extern(st_pack_open_node *_pst_pack_infos, const char *_z_src_file, char *_z_dest_file)
{
  char *z_file_src_path;
  char *z_file_dest_path;

  switch (_pst_pack_infos->i_overwrite)
  {
    case PACK_OVR_BACKUP:
      if (file_exists_extern(_pst_pack_infos, _z_dest_file))
        if (!make_backup_extern(_pst_pack_infos, _z_dest_file))
          return FALSE;
    case PACK_OVR_DELETE:
      if (!package_remove_extern(_pst_pack_infos, _z_dest_file))
        return FALSE;

      z_file_src_path = (char *)malloc((strlen(_pst_pack_infos->z_dir) + strlen(_z_src_file) + 1)*sizeof(char));
      sprintf(z_file_src_path, "%s%s", _pst_pack_infos->z_dir, _z_src_file);

      z_file_dest_path = (char *)malloc((strlen(_pst_pack_infos->z_dir) + strlen(_z_dest_file) + 1)*sizeof(char));
      sprintf(z_file_dest_path, "%s%s", _pst_pack_infos->z_dir, _z_dest_file);

      if (rename(z_file_src_path, z_file_dest_path)!=0)
      {
        free(z_file_src_path);
        free(z_file_dest_path);
        return FALSE;
      }
      else
      {
        free(z_file_src_path);
        free(z_file_dest_path);
        return TRUE;
      }
      break;
    case PACK_OVR_ERROR:
      errno = EEXIST; /* Fichier existant */
      return FALSE;
      break;
    default:
      errno = EINVAL; /* Argument invalide */
      return FALSE;
      break;
  }
  return TRUE;
}



bool package_move(int32 _i_pack_id, const char *_z_src_file, char *_z_dest_file)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return FALSE;
 
  if (st_pack_infos.i_access==PACK_O_READ)
  {
    errno = EROFS; /* Read only file system */
    return FALSE;
  }

  if (st_pack_infos.i_mode==PACK_M_INTERN)
    return package_move_intern(&st_pack_infos, _z_src_file, _z_dest_file);
  else
    return package_move_extern(&st_pack_infos, _z_src_file, _z_dest_file);
}

bool package_find_first_intern(st_pack_open_node *_pst_pack_infos, const char *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_pattern = NULL;
  _st_fileinfos = NULL;
  return FALSE;
}

bool package_find_first_extern(st_pack_open_node *_pst_pack_infos, const char *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_pattern = NULL;
  _st_fileinfos = NULL;
  return FALSE;
}

bool package_find_first_intern_extern(st_pack_open_node *_pst_pack_infos, const char *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _pst_pack_infos = NULL;
  _z_pattern = NULL;
  _st_fileinfos = NULL;
  return FALSE;
}

bool package_find_first(int32 _i_pack_id, const char *_z_pattern, package_st_file_infos *_st_fileinfos)
{
  st_pack_open_node st_pack_infos;

  if (!package_get_infos(_i_pack_id, &st_pack_infos))
    return FALSE;
 
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
      return FALSE;
      break;
  }
}



bool package_find_next_intern(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = NULL;
  return FALSE;
}

bool package_find_next_extern(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = NULL;
  return FALSE;
}

bool package_find_next_intern_extern(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = NULL;
  return FALSE;
}

bool package_find_next(package_st_file_infos *_st_fileinfos)
{
  /* TODO */
  errno = ENOSYS; /* Fonction non implémentée */
  _st_fileinfos = NULL;
  return FALSE;
}
