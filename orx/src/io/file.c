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


#include "debug/orxDebug.h"
#include "memory/orxMemory.h"


typedef struct st_search_node_t
{
  orxS32 i_id;      /* Pointer on file informations */
  
#ifdef __linux__
  DIR *pst_directory;                /* DIR* structure for (for opendir/readdir) */
  orxU8 ac_pattern[512];              /* Pattern of the search */
#else
  orxS32 i_find_handler;                /* Handler for search (_findfirst/_findnext) */
#endif

  struct st_search_node_t *pstNext; /* pointer on the next node */
  
  /* 4 extra bytes of padding : 16 (+512) */
  orxU8 au8Unused[4];
} file_st_search_node;


/*
 * linked list of all current search
 * this list is sorted on id
 */
static file_st_search_node *spst_search_list = orxNULL;


/* génère un numéro d'identifiant valide */
orxS32 file_generate_id()
{
  orxS32 i_id_free;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */

  i_id_free = 0;
  while(spst_search_list != orxNULL)
  {
    if(i_id_free == spst_search_list->i_id)
      i_id_free++;
    spst_search_list = spst_search_list->pstNext;
  }
  spst_search_list = pst_tmp_list;            /* on repositionne le pointeur en début de liste */

  return i_id_free;
}

/* rend le pointeur */
file_st_search_node *file_get_ptr(orxS32 _i_id)
{
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */
  file_st_search_node *pst_out_file;

  if(spst_search_list == orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_NO_SEARCH_FOUND);
    return orxNULL;
  }

  while((spst_search_list->pstNext != orxNULL) && (spst_search_list->i_id != _i_id))
  {
    spst_search_list = spst_search_list->pstNext;
  }
  
  if((spst_search_list->pstNext != orxNULL) && (spst_search_list->i_id != _i_id))
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_READ_SEARCH_ERROR);
    spst_search_list = pst_tmp_list;
    return orxNULL;
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
  file_st_search_node *file_insert_search(orxS32 _i_id, DIR *_pst_dir, orxU8 *_z_pattern)
#else
  file_st_search_node *file_insert_search(orxS32 _i_id, orxS32 _i_find_handler)
#endif
{
  file_st_search_node *pst_new_node;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */

  pst_new_node = (file_st_search_node *) orxMemory_Allocate(sizeof(file_st_search_node), orxMEMORY_TYPE_MAIN);
  
    pst_new_node->i_id=_i_id;
    #ifdef __linux__
      pst_new_node->pst_directory = _pst_dir;
      strcpy(pst_new_node->ac_pattern, _z_pattern);
    #else
      pst_new_node->i_find_handler = _i_find_handler;
    #endif
    pst_new_node->pstNext = orxNULL;
    
  if(spst_search_list == orxNULL)
    return pst_new_node;
  
  /* on se positionne au bon endroit de la liste pour insérer le nouveau noeud */

  if(spst_search_list->i_id > _i_id)
  {
    pst_new_node->pstNext = spst_search_list;
    return pst_new_node;
  }
  else
  {
    while((spst_search_list->pstNext != orxNULL) && (spst_search_list->pstNext->i_id < _i_id))
      spst_search_list = spst_search_list->pstNext;
    
    pst_new_node->pstNext = spst_search_list->pstNext;
    spst_search_list->pstNext = pst_new_node;
    return pst_tmp_list;
  }
}

/* supprime un noeud de recherche dans la liste triée */
file_st_search_node *file_delete_search(orxS32 _i_id)
{
  file_st_search_node *pstPrevious_node = orxNULL;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */
  if(spst_search_list == orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_CANT_REMOVE_SEARCH);
    return orxNULL;
  }
  else if(spst_search_list->pstNext == orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_ONE_SEARCH_FOUND);
    if(spst_search_list->i_id == _i_id)
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s : %i", KZ_FILE_REMOVING_SEARCH, _i_id);
#ifdef __linux__
  closedir((file_get_ptr(_i_id))->pst_directory);
#else
  _findclose(file_get_ptr(_i_id)->i_find_handler);
#endif
      orxMemory_Free(spst_search_list);
      return orxNULL;
    }
    else
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_NO_MATCHING_ID);
      return pst_tmp_list;
    }
  }
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_MULTIPLE_SEARCH_FOUND);
    if(spst_search_list->i_id == _i_id)
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s : %i", KZ_FILE_REMOVING_SEARCH, _i_id);
      pst_tmp_list = spst_search_list->pstNext;
      orxMemory_Free(spst_search_list);
      return pst_tmp_list;
    }
    else
    {
      while((pst_tmp_list->pstNext != orxNULL) && (pst_tmp_list->i_id != _i_id))
      {
        pstPrevious_node = pst_tmp_list;
        pst_tmp_list = pst_tmp_list->pstNext;
      }
      
      if((pst_tmp_list->pstNext == orxNULL) && (pst_tmp_list->i_id != _i_id))
      {
        orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_NO_MATCHING_ID);
        return spst_search_list;
      }
      else
      {
        orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s : %i ***\n",KZ_FILE_REMOVING_SEARCH, _i_id);
       #ifdef __linux__
         closedir((file_get_ptr(_i_id))->pst_directory);
       #else
         _findclose(file_get_ptr(_i_id)->i_find_handler);
       #endif
        pstPrevious_node->pstNext = pst_tmp_list->pstNext;
        orxMemory_Free(pst_tmp_list);
        return spst_search_list;
      }
    }
  }
}

orxBOOL file_exist(orxU8 *_zFile_name)
{
#ifdef _MSC_VER
  struct _stat st_stat;
  stat(_zFile_name, &st_stat);
  return ((st_stat.st_mode & _S_IFREG) || (st_stat.st_mode & _S_IFDIR));
#else
  struct stat st_stat;
  stat(_zFile_name, &st_stat);
  return ((st_stat.st_mode & S_IFREG) || (st_stat.st_mode & S_IFDIR));
#endif
}

#ifdef __linux__
/* returns orxTRUE if the pattern _z_pattern is correct for _z_name
 * example : a?bc*.j?g is correct for aubcaze.jpg
 * returns orxFALSE if the pattern is not correct
 */
orxBOOL file_pattern_match(orxU8 *_z_pattern, orxU8 *_z_name)
{
  orxS32 i_pos_patt;
  orxS32 i_pos_name;
  orxS32 i_pos;
  orxS32 i_length_patt;
  orxS32 i_length_name;
  orxBOOL b_ok;
  orxBOOL b_exit;
  
  if(_z_pattern==orxNULL || _z_name==orxNULL)
    return orxFALSE;
  
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
  
  b_ok = orxTRUE;
  b_exit = orxFALSE;
  
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
        b_ok = orxTRUE; 
        b_exit = orxTRUE;
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
          b_ok = orxFALSE;
      }
    }
    else
    {
      if (_z_name[i_pos_name]!=_z_pattern[i_pos_patt])
        b_ok = orxFALSE;
    }
    ++i_pos_patt;
    ++i_pos_name;      
  }
  
  if (b_ok)
    return orxTRUE;
  else
    return orxFALSE;
}

#endif /* __linux__ */

orxBOOL file_find_first(orxU8 *_z_pattern, file_st_file_infos *_pstInfo)
{
  orxBOOL b_return;
  orxU8 ac_directory[512];
  orxU8 ac_full_path[512];
  orxU8 ac_cwd[256];
  orxU8 *z_cwd;
  orxU8 *z_pattern;
  orxU8 *pc_pos;
  orxS32 i_length;
      
#ifdef __linux__
  DIR *pst_dir;
  struct dirent *pst_dirent;
  orxBOOL b_continue;
  struct stat st_stat;
#else
  orxS32 i_id;
  struct _finddata_t pst_finddata;
#endif
 
  if (_z_pattern==orxNULL)
    return orxFALSE;
    
  /* The first thing to do is to recreate the absolute path
   * and orxCONSTruct a full path with a correct format
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
    if (z_cwd==orxNULL)
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_CANT_GET_CWD);
      return orxFALSE;
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
    if (z_cwd==orxNULL)
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_CANT_GET_CWD);
      return orxFALSE;
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
  if (pst_dir == orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "file_find_first : directory  %s not found\n", ac_directory);
    return orxFALSE;
  }

  b_continue = orxTRUE;
  b_return = orxFALSE;
  
  do
  {
    pst_dirent = readdir(pst_dir);
    if (pst_dirent!=orxNULL)
    {
      if (file_pattern_match(z_pattern, pst_dirent->d_name))
      {
        stat(_z_pattern, &st_stat);
        _pstInfo->i_id = file_generate_id();
        _pstInfo->i_attrib = 0x00;
        if (st_stat.st_mode & S_IFREG) _pstInfo->i_attrib |= F_NORMAL;
        if (st_stat.st_mode & S_IFDIR) _pstInfo->i_attrib |= F_SUBDIR;
        if ((st_stat.st_mode & S_IRUSR) && !(st_stat.st_mode & S_IWUSR)) _pstInfo->i_attrib |= F_RDONLY;
        if (pst_dirent->d_name[0]=='.') _pstInfo->i_attrib |= F_HIDDEN;
        _pstInfo->s32_size = st_stat.st_size;
        _pstInfo->st_time = st_stat.st_ctime;
        strcpy(_pstInfo->ac_name, pst_dirent->d_name);
        strcpy(_pstInfo->ac_path, ac_directory);
        spst_search_list = file_insert_search(_pstInfo->i_id, pst_dir, z_pattern);
        b_continue = orxFALSE;
        b_return = orxTRUE;
      }
    }
    else
    {
      b_continue = orxFALSE;
      closedir(pst_dir);  
    }
  } while (b_continue);

#else
  i_id = _findfirst(_z_pattern, &pst_finddata);
  if (i_id==-1)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "file_find_first : file  %s not found\n", _z_pattern);
    b_return = orxFALSE;
  }
  else
  {
    _pstInfo->i_id = file_generate_id();
    _pstInfo->i_attrib = 0x00;
    if (pst_finddata.attrib & _A_NORMAL) _pstInfo->i_attrib |= F_NORMAL;
    if (pst_finddata.attrib & _A_SUBDIR) _pstInfo->i_attrib |= F_SUBDIR;
    if (pst_finddata.attrib & _A_RDONLY) _pstInfo->i_attrib |= F_RDONLY;
    if (pst_finddata.attrib & _A_HIDDEN) _pstInfo->i_attrib |= F_HIDDEN;
    _pstInfo->s32_size = pst_finddata.size;
    _pstInfo->st_time = pst_finddata.time_write;
    strcpy(_pstInfo->ac_path, ac_directory);
    strcpy(_pstInfo->ac_name, pst_finddata.name);
    spst_search_list = file_insert_search(_pstInfo->i_id, i_id);
    b_return = orxTRUE;
  }
#endif
  orxMemory_Free(z_pattern);
  return b_return;
}

orxBOOL file_find_next(file_st_file_infos *_pstInfo)
{
  /* Variable declarations */
  
  orxBOOL b_return;
  
#ifdef __linux__
  struct dirent *pst_dirent;
  orxU8 ac_path_name[1024];
  orxBOOL b_continue;
  struct stat st_stat;
#else
  struct _finddata_t pst_finddata;
#endif


  file_st_search_node *pst_search_node = file_get_ptr(_pstInfo->i_id);
  
  /* Code */
#ifdef __linux__
  
  b_return = orxFALSE;
  b_continue = orxTRUE;
  
  do
  {
    pst_dirent = readdir(pst_search_node->pst_directory);
    if (pst_dirent!=orxNULL)
    {
      if (file_pattern_match(pst_search_node->ac_pattern, pst_dirent->d_name))
      {
        sprintf(ac_path_name, "%s/%s", _pstInfo->ac_path, pst_dirent->d_name);
        stat(ac_path_name, &st_stat);
        _pstInfo->i_attrib = 0x00;
        if (st_stat.st_mode & S_IFREG) _pstInfo->i_attrib |= F_NORMAL;
        if (st_stat.st_mode & S_IFDIR) _pstInfo->i_attrib |= F_SUBDIR;
        if ((st_stat.st_mode & S_IRUSR) && !(st_stat.st_mode & S_IWUSR)) _pstInfo->i_attrib |= F_RDONLY;
        if (pst_dirent->d_name[0]=='.') _pstInfo->i_attrib |= F_HIDDEN;
        _pstInfo->s32_size = st_stat.st_size;
        _pstInfo->st_time = st_stat.st_ctime;
        strcpy(_pstInfo->ac_name, pst_dirent->d_name);
        b_continue = orxFALSE;
        b_return = orxTRUE;
      }
    }
    else
    {
      b_continue = orxFALSE;
       file_find_close(_pstInfo);
      orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_SEARCH_FINISHED);
    }
  } while (b_continue);

#else
  if(_findnext(pst_search_node->i_find_handler, &pst_finddata)==-1)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_SEARCH_FINISHED);
    b_return = orxFALSE;
    file_find_close(_pstInfo);
  }
  else
  {
    _pstInfo->i_attrib = 0x00;
    if (pst_finddata.attrib & _A_NORMAL) _pstInfo->i_attrib |= F_NORMAL;
    if (pst_finddata.attrib & _A_SUBDIR) _pstInfo->i_attrib |= F_SUBDIR;
    if (pst_finddata.attrib & _A_RDONLY) _pstInfo->i_attrib |= F_RDONLY;
    if (pst_finddata.attrib & _A_HIDDEN) _pstInfo->i_attrib |= F_HIDDEN;
    _pstInfo->s32_size = pst_finddata.size;
    _pstInfo->st_time = pst_finddata.time_write;
    strcpy(_pstInfo->ac_name, pst_finddata.name);
    b_return = orxTRUE;
  }

#endif
  return b_return;
}

orxVOID file_find_close(file_st_file_infos *_pstInfo)
{
  spst_search_list = file_delete_search(_pstInfo->i_id);
}

orxBOOL file_copy(orxU8 *_zFile_src, orxU8 *_zFile_dest)
{
  FILE *pst_src_file;   /* A pointer on the source file */
  FILE *pst_des_file;   /* A pointer on the destination */
  orxS32 i_nb_read;        /* The number of values read by fread */
  orxU8 ac_buffer[1024]; /* A buffer where are stored read characters */
  
  printf("pst_src_file = fopen(%s, \"r\");\n", _zFile_src);
  pst_src_file = fopen(_zFile_src, "r");
  if (pst_src_file==orxNULL)
  {
    /* Can't open the source file */
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "file_copy : can't open file %s in read mode.\n", _zFile_src);
//    printf("ERROR : file_copy : can't open file %s in read mode.\n", _zFile_src);
    return orxFALSE;
  }
  
  pst_des_file = fopen(_zFile_dest, "w");
  if (pst_des_file==orxNULL)
  {
    /* Can't open the destination file */
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "file_copy : can't open file %s in write mode.\n", _zFile_dest);
//    fclose(pst_src_file);
    return orxFALSE;
  }
  
  while ((i_nb_read = fread(ac_buffer, sizeof(char), 1024, pst_src_file)))
    fwrite(ac_buffer, sizeof(char), i_nb_read, pst_des_file);
  
  fclose(pst_des_file);
  fclose(pst_src_file);
  return orxTRUE;
}

orxBOOL file_delete(orxU8 *_zFile)
{
  if (_zFile==orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "file_delete : the file name is orxNULL\n");
    return orxFALSE;
  }
    
  if (remove(_zFile)==0)
    return orxTRUE;
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s %s", KZ_FILE_CANT_REMOVE_FILE, _zFile);
    return orxFALSE;
  }
}

orxBOOL file_rename(orxU8 *_zFile_src, orxU8 *_zFile_dest)
{
  if (_zFile_src==orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_SOURCE_orxNULL);
    return orxFALSE;
  }
  
  if (_zFile_dest==orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_DESTINATION_orxNULL);
    return orxFALSE;
  }
    
  if (rename(_zFile_src, _zFile_dest)==0)
    return orxTRUE;
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s %s.\n", KZ_FILE_CANT_RENAME_FILE, _zFile_src);
    return orxFALSE;
  }
}
 
orxBOOL file_mkdir(orxU8 *_z_directory_name)
{
  orxS32 i_res;

  if (_z_directory_name==orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_DIRECTORY_orxNULL);
    return orxFALSE;
  }
  
  #ifdef __linux__
    i_res = mkdir(_z_directory_name, S_IRWXU);
  #else
    i_res = mkdir(_z_directory_name);
  #endif
  
  if(i_res==0)
    return orxTRUE;
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s %s.\n", KZ_FILE_CANT_CREATE_DIR, _z_directory_name);
    return orxFALSE;
  }
}
 
orxBOOL file_rmdir(orxU8 *_z_directory_name)
{
  if (_z_directory_name==orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_DIRECTORY_orxNULL);
    return orxFALSE;
  }
#ifdef __linux__
  if (rmdir(_z_directory_name)==0)
#else
  if (_rmdir(_z_directory_name)==0)
#endif
    return orxTRUE;
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s %s.\n", KZ_FILE_CANT_REMOVE_DIR, _z_directory_name);
    return orxFALSE;
  }
}

orxBOOL file_deltree(orxU8 *_z_directory_name)
{
  orxU8 ac_delete_path[1024];
  orxU8 ac_path[1024];
  file_st_file_infos st_infos;
  orxBOOL b_ok;
  
  if (_z_directory_name==orxNULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, KZ_FILE_DIRECTORY_orxNULL);
    return orxFALSE;
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
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "file_deltree : %s not found.\n", ac_delete_path);
    return orxFALSE;
  }
  
  b_ok = orxTRUE;
  
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
//    return orxTRUE;
  if (!b_ok)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s %s\n",KZ_FILE_REMOVE_RECURSIVELY, _z_directory_name);
    return orxFALSE;
  }
  
  if (!file_rmdir(_z_directory_name))
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_FILE, "%s %s\n",KZ_FILE_REMOVE_RECURSIVELY, _z_directory_name);
    return orxFALSE;
  }
  return orxTRUE;
}


orxS32 file_list_orxDEBUG_LOG()
{
  orxS32 i_id_free;
  file_st_search_node *pst_tmp_list = spst_search_list; /* on sauvegarde le pointeur de début de liste */

  i_id_free = 0;
  while(spst_search_list != orxNULL)
  {
    printf("------------------------------\n");
    printf("ADRESS : %d\n", (int)spst_search_list);
#ifdef __linux__
    printf("DIRECTORY : %d\n", (int)spst_search_list->pst_directory);
    printf("PATTERN : %s\n", spst_search_list->ac_pattern);
#else
    printf("HANDLER : %ld\n", spst_search_list->i_find_handler);
#endif
    printf("NEXT : %d\n", (int)spst_search_list->pstNext);
    spst_search_list = spst_search_list->pstNext;
  }
  spst_search_list = pst_tmp_list;            /* on repositionne le pointeur en début de liste */

  return i_id_free;
}
