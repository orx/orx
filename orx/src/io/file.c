#include "io/file.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#ifdef __linux__ 
  #include <unistd.h>
  #include <dirent.h>
#else
  #include <io.h>
  #include <direct.h>
#endif


#include "debug/debug.h"


typedef struct st_search_node_t
{
  int32 i_id;      /* Pointer on file informations */
  
#ifdef __linux__
  DIR *pst_directory;                /* DIR* structure for (for opendir/readdir) */
  char ac_pattern[512];              /* Pattern of the search */
#else
  int32 i_find_handler;                /* Handler for search (_findfirst/_findnext) */
#endif

  struct st_search_node_t *pst_next; /* pointer on the next node */
  
  /* 4 extra bytes of padding : 16 (+512) */
  uint8 auc_unused[4];
} file_st_search_node;


/*
 * linked list of all current search
 * this list is sorted on id
 */
static file_st_search_node *spst_search_list = NULL;


/* génère un numéro d'identifiant valide */
int32 file_generate_id()
{
  int32 i_id_free;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */

  i_id_free = 0;
  while(spst_search_list != NULL)
  {
    if(i_id_free == spst_search_list->i_id)
      i_id_free++;
    spst_search_list = spst_search_list->pst_next;
  }
  spst_search_list = pst_tmp_list;            /* on repositionne le pointeur en début de liste */

  return i_id_free;
}

/* rend le pointeur */
file_st_search_node *file_get_ptr(int32 _i_id)
{
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */
  file_st_search_node *pst_out_file;

  if(spst_search_list == NULL)
  {
    DEBUG(D_FILE, KZ_FILE_NO_SEARCH_FOUND);
    return NULL;
  }

  while((spst_search_list->pst_next != NULL) && (spst_search_list->i_id != _i_id))
  {
    spst_search_list = spst_search_list->pst_next;
  }
  
  if((spst_search_list->pst_next != NULL) && (spst_search_list->i_id != _i_id))
  {
    DEBUG(D_FILE, KZ_FILE_READ_SEARCH_ERROR);
    spst_search_list = pst_tmp_list;
    return NULL;
  }
  else
  {
    pst_out_file = spst_search_list;
    spst_search_list = pst_tmp_list;
    return pst_out_file;
  }
}

/* insere un nouveau noeud de recherche dans la liste trié
 * insertion dès que next->id est > à l'id du nouveau noeud
 * le dernier parametre : ptr_DirInfos ne sert que pour la fonction openDir afin d'avoir une
 * référence sur les infos rendus et permettre de libérer la mémoire qd l'utilisation est fini
 */
#ifdef __linux__
  file_st_search_node *file_insert_search(int32 _i_id, DIR *_pst_dir, char *_z_pattern)
#else
  file_st_search_node *file_insert_search(int32 _i_id, int32 _i_find_handler)
#endif
{
  file_st_search_node *pst_new_node;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */

  pst_new_node = (file_st_search_node *) malloc(sizeof(file_st_search_node));
  
    pst_new_node->i_id=_i_id;
    #ifdef __linux__
      pst_new_node->pst_directory = _pst_dir;
      strcpy(pst_new_node->ac_pattern, _z_pattern);
    #else
      pst_new_node->i_find_handler = _i_find_handler;
    #endif
    pst_new_node->pst_next = NULL;
    
  if(spst_search_list == NULL)
    return pst_new_node;
  
  /* on se positionne au bon endroit de la liste pour insérer le nouveau noeud */

  if(spst_search_list->i_id > _i_id)
  {
    pst_new_node->pst_next = spst_search_list;
    return pst_new_node;
  }
  else
  {
    while((spst_search_list->pst_next != NULL) && (spst_search_list->pst_next->i_id < _i_id))
      spst_search_list = spst_search_list->pst_next;
    
    pst_new_node->pst_next = spst_search_list->pst_next;
    spst_search_list->pst_next = pst_new_node;
    return pst_tmp_list;
  }
}

/* supprime un noeud de recherche dans la liste triée */
file_st_search_node *file_delete_search(int32 _i_id)
{
  file_st_search_node *pst_previous_node = NULL;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */
  if(spst_search_list == NULL)
  {
    DEBUG(D_FILE, KZ_FILE_CANT_REMOVE_SEARCH);
    return NULL;
  }
  else if(spst_search_list->pst_next == NULL)
  {
    DEBUG(D_FILE, KZ_FILE_ONE_SEARCH_FOUND);
    if(spst_search_list->i_id == _i_id)
    {
      DEBUG(D_FILE, "%s : %i", KZ_FILE_REMOVING_SEARCH, _i_id);
#ifdef __linux__
  closedir((file_get_ptr(_i_id))->pst_directory);
#else
  _findclose(file_get_ptr(_i_id)->i_find_handler);
#endif
      free(spst_search_list);
      return NULL;
    }
    else
    {
      DEBUG(D_FILE, KZ_FILE_NO_MATCHING_ID);
      return pst_tmp_list;
    }
  }
  else
  {
    DEBUG(D_FILE, KZ_FILE_MULTIPLE_SEARCH_FOUND);
    if(spst_search_list->i_id == _i_id)
    {
      DEBUG(D_FILE, "%s : %i", KZ_FILE_REMOVING_SEARCH, _i_id);
      pst_tmp_list = spst_search_list->pst_next;
      free(spst_search_list);
      return pst_tmp_list;
    }
    else
    {
      while((pst_tmp_list->pst_next != NULL) && (pst_tmp_list->i_id != _i_id))
      {
        pst_previous_node = pst_tmp_list;
        pst_tmp_list = pst_tmp_list->pst_next;
      }
      
      if((pst_tmp_list->pst_next == NULL) && (pst_tmp_list->i_id != _i_id))
      {
        DEBUG(D_FILE, KZ_FILE_NO_MATCHING_ID);
        return spst_search_list;
      }
      else
      {
        DEBUG(D_FILE, "%s : %i ***\n",KZ_FILE_REMOVING_SEARCH, _i_id);
       #ifdef __linux__
         closedir((file_get_ptr(_i_id))->pst_directory);
       #else
         _findclose(file_get_ptr(_i_id)->i_find_handler);
       #endif
        pst_previous_node->pst_next = pst_tmp_list->pst_next;
        free(pst_tmp_list);
        return spst_search_list;
      }
    }
  }
}

bool file_exist(char *_z_file_name)
{
#ifdef _MSC_VER
  struct _stat st_stat;
  stat(_z_file_name, &st_stat);
  return ((st_stat.st_mode & _S_IFREG) || (st_stat.st_mode & _S_IFDIR));
#else
  struct stat st_stat;
  stat(_z_file_name, &st_stat);
  return ((st_stat.st_mode & S_IFREG) || (st_stat.st_mode & S_IFDIR));
#endif
}

#ifdef __linux__
/* returns TRUE if the pattern _z_pattern is correct for _z_name
 * example : a?bc*.j?g is correct for aubcaze.jpg
 * returns FALSE if the pattern is not correct
 */
bool file_pattern_match(char *_z_pattern, char *_z_name)
{
  int32 i_pos_patt;
  int32 i_pos_name;
  int32 i_pos;
  int32 i_length_patt;
  int32 i_length_name;
  bool b_ok;
  bool b_exit;
  
  if(_z_pattern==NULL || _z_name==NULL)
    return FALSE;
  
  /* The first thing to do is to correct the pattern if there are errors (two '*' followings) */
  i_pos_patt = 0;
  i_length_patt = strlen(_z_pattern);
  while(i_pos_patt<i_length_patt)
  {
    if(_z_pattern[i_pos_patt]=='*' && _z_pattern[i_pos_patt+1]=='*')
    {
      i_pos = i_pos_patt+1;
      while (i_pos<i_length_patt)
      {
        _z_pattern[i_pos] = _z_pattern[i_pos+1];
        ++i_pos;
      }
    }
    ++i_pos_patt;
  }
  
  i_pos_patt = 0;
  i_pos_name = 0;
  i_length_patt = strlen(_z_pattern);
  i_length_name = strlen(_z_name);
  
  b_ok = TRUE;
  b_exit = FALSE;
  
  while ((b_ok && !b_exit) && (i_pos_patt<=i_length_patt && i_pos_name<=i_length_name))
  {
    if (_z_pattern[i_pos_patt]=='?')
    {
      /* We test nothing */
    }
    else if(_z_pattern[i_pos_patt]=='*')
    {
      /* first case : '*' is the last pattern to find.. all is ok */
      if (i_pos_patt+1==i_length_patt)
      {
        b_ok = TRUE; 
        b_exit = TRUE;
      }
      /* second case : there are other characters after '*' */
      else
      {
        ++i_pos_patt;
        while (_z_name[i_pos_name]!=_z_pattern[i_pos_patt] && i_pos_name<=i_length_name)
        {
          ++i_pos_name;
        }
        if (_z_name[i_pos_name]!=_z_pattern[i_pos_patt])
          b_ok = FALSE;
      }
    }
    else
    {
      if (_z_name[i_pos_name]!=_z_pattern[i_pos_patt])
        b_ok = FALSE;
    }
    ++i_pos_patt;
    ++i_pos_name;      
  }
  
  if (b_ok)
    return TRUE;
  else
    return FALSE;
}

#endif /* __linux__ */

bool file_find_first(char *_z_pattern, file_st_file_infos *_pst_infos)
{
  bool b_return;
  char ac_directory[512];
  char ac_full_path[512];
  char ac_cwd[256];
  char *z_cwd;
  char *z_pattern;
  char *pc_pos;
  int32 i_length;
      
#ifdef __linux__
  DIR *pst_dir;
  struct dirent *pst_dirent;
  bool b_continue;
  struct stat st_stat;
#else
  int32 i_id;
  struct _finddata_t pst_finddata;
#endif
 
  if (_z_pattern==NULL)
    return FALSE;
    
  /* The first thing to do is to recreate the absolute path
   * and construct a full path with a correct format
   * 1 - If the user doesn't give the absolute path, we get
   *     the current working directory, and we join it with
   *     the given pattern
   * 2 - We check that the new path end with a correct pattern.
   *     If it ends with / or \, we add / *.* or \*.*
   */
  
#ifdef __linux__
  if(_z_pattern[0]!='/')
  {
    /* This is a relative path, we have to transform it to a full absolute path */
    z_cwd = getcwd(ac_cwd, 256*sizeof(char));
    if (z_cwd==NULL)
    {
      DEBUG(D_FILE, KZ_FILE_CANT_GET_CWD);
      return FALSE;
    }
  
    if (z_cwd[strlen(z_cwd)-1]=='/')
      sprintf(ac_full_path, "%s%s", z_cwd, _z_pattern);
    else
      sprintf(ac_full_path, "%s/%s", z_cwd, _z_pattern);
  }
  else
    strcpy(ac_full_path, _z_pattern);
  
  if (ac_full_path[strlen(ac_full_path)-1]=='/')
    sprintf(ac_full_path, "%s*", ac_full_path); /* !! ATTENTION DANS CETTE PARTIE DU CODE !! BUFFER OVERFLOW A TESTER !! */

#else
  /* This is a relative path, we have to transform it to a full absolute path */
  if(_z_pattern[1]!=':' || _z_pattern[2]!='\\')
  {
    z_cwd = _getcwd(ac_cwd, 256*sizeof(char));
    if (z_cwd==NULL)
    {
      DEBUG(D_FILE, KZ_FILE_CANT_GET_CWD);
      return FALSE;
    }
  
    printf("CWD = %s\n", z_cwd);

    if (z_cwd[strlen(z_cwd)-1]=='\\')
      sprintf(ac_full_path, "%s%s", z_cwd, _z_pattern);
    else
      sprintf(ac_full_path, "%s\\%s", z_cwd, _z_pattern);
  }
  else
    strcpy(ac_full_path, _z_pattern);
  
  if (ac_full_path[strlen(ac_full_path)-1]=='\\')
    sprintf(ac_full_path, "%s*.*", ac_full_path); /* !! ATTENTION DANS CETTE PARTIE DU CODE !! BUFFER OVERFLOW A TESTER !! */

#endif

 
#ifdef __linux__
  pc_pos = strrchr(ac_full_path, '/')+sizeof(char); /* we only want things after the last '/' */
#else
  pc_pos = strrchr(ac_full_path, '\\')+sizeof(char); /* we only want things after the last '\' */
#endif
  z_pattern = strdup(pc_pos);
  i_length = strlen(ac_full_path)-strlen(z_pattern)-1;

  strncpy(ac_directory, ac_full_path, i_length);
  if (i_length!=0)
    ac_directory[i_length]='\0';
  else
    strcpy(ac_directory, "/");

#ifdef __linux__

  pst_dir = opendir(ac_directory);
  if (pst_dir == NULL)
  {
    DEBUG(D_FILE, "file_find_first : directory  %s not found\n", ac_directory);
    return FALSE;
  }

  b_continue = TRUE;
  b_return = FALSE;
  
  do
  {
    pst_dirent = readdir(pst_dir);
    if (pst_dirent!=NULL)
    {
      if (file_pattern_match(z_pattern, pst_dirent->d_name))
      {
        stat(_z_pattern, &st_stat);
        _pst_infos->i_id = file_generate_id();
        _pst_infos->i_attrib = 0x00;
        if (st_stat.st_mode & S_IFREG) _pst_infos->i_attrib |= F_NORMAL;
        if (st_stat.st_mode & S_IFDIR) _pst_infos->i_attrib |= F_SUBDIR;
        if ((st_stat.st_mode & S_IRUSR) && !(st_stat.st_mode & S_IWUSR)) _pst_infos->i_attrib |= F_RDONLY;
        if (pst_dirent->d_name[0]=='.') _pst_infos->i_attrib |= F_HIDDEN;
        _pst_infos->s32_size = st_stat.st_size;
        _pst_infos->st_time = st_stat.st_ctime;
        strcpy(_pst_infos->ac_name, pst_dirent->d_name);
        strcpy(_pst_infos->ac_path, ac_directory);
        spst_search_list = file_insert_search(_pst_infos->i_id, pst_dir, z_pattern);
        b_continue = FALSE;
        b_return = TRUE;
      }
    }
    else
    {
      b_continue = FALSE;
      closedir(pst_dir);  
    }
  } while (b_continue);

#else
  i_id = _findfirst(_z_pattern, &pst_finddata);
  if (i_id==-1)
  {
    DEBUG(D_FILE, "file_find_first : file  %s not found\n", _z_pattern);
    b_return = FALSE;
  }
  else
  {
    _pst_infos->i_id = file_generate_id();
    _pst_infos->i_attrib = 0x00;
    if (pst_finddata.attrib & _A_NORMAL) _pst_infos->i_attrib |= F_NORMAL;
    if (pst_finddata.attrib & _A_SUBDIR) _pst_infos->i_attrib |= F_SUBDIR;
    if (pst_finddata.attrib & _A_RDONLY) _pst_infos->i_attrib |= F_RDONLY;
    if (pst_finddata.attrib & _A_HIDDEN) _pst_infos->i_attrib |= F_HIDDEN;
    _pst_infos->s32_size = pst_finddata.size;
    _pst_infos->st_time = pst_finddata.time_write;
    strcpy(_pst_infos->ac_path, ac_directory);
    strcpy(_pst_infos->ac_name, pst_finddata.name);
    spst_search_list = file_insert_search(_pst_infos->i_id, i_id);
    b_return = TRUE;
  }
#endif
  free(z_pattern);
  return b_return;
}

bool file_find_next(file_st_file_infos *_pst_infos)
{
  /* Variable declarations */
  
  bool b_return;
  
#ifdef __linux__
  struct dirent *pst_dirent;
  char ac_path_name[1024];
  bool b_continue;
  struct stat st_stat;
#else
  struct _finddata_t pst_finddata;
#endif


  file_st_search_node *pst_search_node = file_get_ptr(_pst_infos->i_id);
  
  /* Code */
#ifdef __linux__
  
  b_return = FALSE;
  b_continue = TRUE;
  
  do
  {
    pst_dirent = readdir(pst_search_node->pst_directory);
    if (pst_dirent!=NULL)
    {
      if (file_pattern_match(pst_search_node->ac_pattern, pst_dirent->d_name))
      {
        sprintf(ac_path_name, "%s/%s", _pst_infos->ac_path, pst_dirent->d_name);
        stat(ac_path_name, &st_stat);
        _pst_infos->i_attrib = 0x00;
        if (st_stat.st_mode & S_IFREG) _pst_infos->i_attrib |= F_NORMAL;
        if (st_stat.st_mode & S_IFDIR) _pst_infos->i_attrib |= F_SUBDIR;
        if ((st_stat.st_mode & S_IRUSR) && !(st_stat.st_mode & S_IWUSR)) _pst_infos->i_attrib |= F_RDONLY;
        if (pst_dirent->d_name[0]=='.') _pst_infos->i_attrib |= F_HIDDEN;
        _pst_infos->s32_size = st_stat.st_size;
        _pst_infos->st_time = st_stat.st_ctime;
        strcpy(_pst_infos->ac_name, pst_dirent->d_name);
        b_continue = FALSE;
        b_return = TRUE;
      }
    }
    else
    {
      b_continue = FALSE;
       file_find_close(_pst_infos);
      DEBUG(D_FILE, KZ_FILE_SEARCH_FINISHED);
    }
  } while (b_continue);

#else
  if(_findnext(pst_search_node->i_find_handler, &pst_finddata)==-1)
  {
    DEBUG(D_FILE, KZ_FILE_SEARCH_FINISHED);
    b_return = FALSE;
    file_find_close(_pst_infos);
  }
  else
  {
    _pst_infos->i_attrib = 0x00;
    if (pst_finddata.attrib & _A_NORMAL) _pst_infos->i_attrib |= F_NORMAL;
    if (pst_finddata.attrib & _A_SUBDIR) _pst_infos->i_attrib |= F_SUBDIR;
    if (pst_finddata.attrib & _A_RDONLY) _pst_infos->i_attrib |= F_RDONLY;
    if (pst_finddata.attrib & _A_HIDDEN) _pst_infos->i_attrib |= F_HIDDEN;
    _pst_infos->s32_size = pst_finddata.size;
    _pst_infos->st_time = pst_finddata.time_write;
    strcpy(_pst_infos->ac_name, pst_finddata.name);
    b_return = TRUE;
  }

#endif
  return b_return;
}

void file_find_close(file_st_file_infos *_pst_infos)
{
  spst_search_list = file_delete_search(_pst_infos->i_id);
}

bool file_copy(char *_z_file_src, char *_z_file_dest)
{
  FILE *pst_src_file;   /* A pointer on the source file */
  FILE *pst_des_file;   /* A pointer on the destination */
  int32 i_nb_read;        /* The number of values read by fread */
  char ac_buffer[1024]; /* A buffer where are stored read characters */
  
  printf("pst_src_file = fopen(%s, \"r\");\n", _z_file_src);
  pst_src_file = fopen(_z_file_src, "r");
  if (pst_src_file==NULL)
  {
    /* Can't open the source file */
    DEBUG(D_FILE, "file_copy : can't open file %s in read mode.\n", _z_file_src);
//    printf("ERROR : file_copy : can't open file %s in read mode.\n", _z_file_src);
    return FALSE;
  }
  
  pst_des_file = fopen(_z_file_dest, "w");
  if (pst_des_file==NULL)
  {
    /* Can't open the destination file */
    DEBUG(D_FILE, "file_copy : can't open file %s in write mode.\n", _z_file_dest);
//    fclose(pst_src_file);
    return FALSE;
  }
  
  while ((i_nb_read = fread(ac_buffer, sizeof(char), 1024, pst_src_file)))
    fwrite(ac_buffer, sizeof(char), i_nb_read, pst_des_file);
  
  fclose(pst_des_file);
  fclose(pst_src_file);
  return TRUE;
}

bool file_delete(char *_z_file)
{
  if (_z_file==NULL)
  {
    DEBUG(D_FILE, "file_delete : the file name is NULL\n");
    return FALSE;
  }
    
  if (remove(_z_file)==0)
    return TRUE;
  else
  {
    DEBUG(D_FILE, "%s %s", KZ_FILE_CANT_REMOVE_FILE, _z_file);
    return FALSE;
  }
}

bool file_rename(char *_z_file_src, char *_z_file_dest)
{
  if (_z_file_src==NULL)
  {
    DEBUG(D_FILE, KZ_FILE_SOURCE_NULL);
    return FALSE;
  }
  
  if (_z_file_dest==NULL)
  {
    DEBUG(D_FILE, KZ_FILE_DESTINATION_NULL);
    return FALSE;
  }
    
  if (rename(_z_file_src, _z_file_dest)==0)
    return TRUE;
  else
  {
    DEBUG(D_FILE, "%s %s.\n", KZ_FILE_CANT_RENAME_FILE, _z_file_src);
    return FALSE;
  }
}
 
bool file_mkdir(char *_z_directory_name)
{
  int32 i_res;

  if (_z_directory_name==NULL)
  {
    DEBUG(D_FILE, KZ_FILE_DIRECTORY_NULL);
    return FALSE;
  }
  
  #ifdef __linux__
    i_res = mkdir(_z_directory_name, S_IRWXU);
  #else
    i_res = mkdir(_z_directory_name);
  #endif
  
  if(i_res==0)
    return TRUE;
  else
  {
    DEBUG(D_FILE, "%s %s.\n", KZ_FILE_CANT_CREATE_DIR, _z_directory_name);
    return FALSE;
  }
}
 
bool file_rmdir(char *_z_directory_name)
{
  if (_z_directory_name==NULL)
  {
    DEBUG(D_FILE, KZ_FILE_DIRECTORY_NULL);
    return FALSE;
  }
#ifdef __linux__
  if (rmdir(_z_directory_name)==0)
#else
  if (_rmdir(_z_directory_name)==0)
#endif
    return TRUE;
  else
  {
    DEBUG(D_FILE, "%s %s.\n", KZ_FILE_CANT_REMOVE_DIR, _z_directory_name);
    return FALSE;
  }
}

bool file_deltree(char *_z_directory_name)
{
  char ac_delete_path[1024];
  char ac_path[1024];
  file_st_file_infos st_infos;
  bool b_ok;
  
  if (_z_directory_name==NULL)
  {
    DEBUG(D_FILE, KZ_FILE_DIRECTORY_NULL);
    return FALSE;
  }

  if (_z_directory_name[strlen(_z_directory_name)-1]=='/' || _z_directory_name[strlen(_z_directory_name)-1]=='\\')
    _z_directory_name[strlen(_z_directory_name)-1]='\0';
    
  #ifdef __linux__
    sprintf(ac_delete_path, "%s/*", _z_directory_name);
  #else
    sprintf(ac_delete_path, "%s\\*.*", _z_directory_name);
  #endif
  
  if (!file_find_first(ac_delete_path, &st_infos))
  {
    DEBUG(D_FILE, "file_deltree : %s not found.\n", ac_delete_path);
    return FALSE;
  }
  
  b_ok = TRUE;
  
  do
  {
    if (strcmp(st_infos.ac_name, ".")!=0 && strcmp(st_infos.ac_name, "..")!=0)
    {

#ifdef __linux__
      sprintf(ac_path, "%s/%s", _z_directory_name, st_infos.ac_name);
#else
      sprintf(ac_path, "%s\\%s", _z_directory_name, st_infos.ac_name);
#endif

      if (st_infos.i_attrib & F_SUBDIR)
      {
        printf("%s is a directory\n", st_infos.ac_name);
        b_ok = file_deltree(ac_path);
      }
      else
      {
        printf("delete = %s\n", ac_path);
        b_ok = file_delete(ac_path);
      }
    }
  }
  while (file_find_next(&st_infos) && b_ok);
  
//  if (b_ok && file_rmdir(_z_directory_name))
//    return TRUE;
  if (!b_ok)
  {
    DEBUG(D_FILE, "%s %s\n",KZ_FILE_REMOVE_RECURSIVELY, _z_directory_name);
    return FALSE;
  }
  
  if (!file_rmdir(_z_directory_name))
  {
    DEBUG(D_FILE, "%s %s\n",KZ_FILE_REMOVE_RECURSIVELY, _z_directory_name);
    return FALSE;
  }
  return TRUE;
}


int32 file_list_DEBUG()
{
  int32 i_id_free;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */

  i_id_free = 0;
  while(spst_search_list != NULL)
  {
    printf("------------------------------\n");
    printf("ADRESS : %d\n", (int)spst_search_list);
#ifdef __linux__
    printf("DIRECTORY : %d\n", (int)spst_search_list->pst_directory);
    printf("PATTERN : %s\n", spst_search_list->ac_pattern);
#else
    printf("HANDLER : %ld\n", spst_search_list->i_find_handler);
#endif
    printf("NEXT : %d\n", (int)spst_search_list->pst_next);
    spst_search_list = spst_search_list->pst_next;
  }
  spst_search_list = pst_tmp_list;            /* on repositionne le pointeur en début de liste */

  return i_id_free;
}
