/***************************************************************************
 plugin.c
 plugin managment system
 
 begin                : 04/09/2002
 author               : (C) Gdp
 email                : david.anderson@calixo.net
                      : iarwain@ifrance.com       (v1.10->)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "plugin/plugin.h"

#include "debug/debug.h"
#include "msg/msg_plugin.h"
#include "plugin/plugin_user.h"
#include "plugin/plugin_core.h"
#include "utils/utils.h"

#ifdef LINUX
  #include <dlfcn.h>
#elif defined WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif


/*
 * Platform independant defines
 */

#define PLUGIN_KUL_FLAG_NONE    0x00000000
#define PLUGIN_KUL_FLAG_READY   0x00000001

/*
 * Platform dependant type & function defines
 */

/* WINDOWS */
#ifdef WINDOWS

#define PLUGIN_HANDLE       HMODULE

#define PLUGIN_OPEN(X)      LoadLibrary(X)
#define PLUGIN_SYM(X, Y)    GetProcAddress(X, Y)
#define PLUGIN_CLOSE(X)     FreeLibrary(X)

#define PLUGIN_LIBRARY_EXT  "dll"

/* OTHERS */
#else

#define PLUGIN_HANDLE       void *

#define PLUGIN_OPEN(X)      dlopen(X, RTLD_LAZY)
#define PLUGIN_SYM(X, Y)    dlsym(X, Y)
#define PLUGIN_CLOSE(X)     dlclose(X)

#define PLUGIN_LIBRARY_EXT  "so"

#endif


/*
 * Information structure on a plugin function
 */
typedef struct st_function_info_t
{
  plugin_function pfn_function;                       /**< Function Address : 4 */
  char z_function_args[PLUGIN_KI_FUNCTION_ARG_SIZE];  /**< Function Argument Types : 132 */

  char z_function_name[PLUGIN_KI_NAME_SIZE];          /**< Function Name : 164 */
  uint32 u32_function_id;                              /**< Function ID : 168 */

  /* 8 extra bytes of padding : 176 */
  uint8 auc_unused[8];
} plugin_st_function_info;

/*
 * Information structure on a plugin
 */
typedef struct st_plugin_info_t
{
  PLUGIN_HANDLE p_plugin_handle;            /**< Plugin handle : 4 */

  char z_plugin_name[PLUGIN_KI_NAME_SIZE];  /**< Plugin name : 36 */
  uint32 u32_plugin_id;                      /**< Plugin ID : 40 */

  map p_function_map;                       /**< Plugin Function List : 44 */

  /* 4 extra bytes of padding : 48 */
  uint8 auc_unused[4];
} plugin_st_plugin_info;



/*
 * Static information structures
 */
static plugin_core_st_function *sapst_function[PLUGIN_CORE_KI_NUMBER];
static int32 si_function_number[PLUGIN_CORE_KI_NUMBER];

static map sp_plugin_map = NULL;
static uint32 plugin_su32_flags = PLUGIN_KUL_FLAG_NONE;

/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 key_create
 
 This function creates a key for plugin/function maps. 
 Returns created key.
 ***************************************************************************/
uchar *key_create(unsigned long _u32_id, char *_z_name)
{
  uchar *puc_key = NULL;
  int32 i_shift = sizeof(unsigned long);

  /* Allocate memory for key */
  puc_key = (uchar *)malloc(i_shift + PLUGIN_KI_NAME_SIZE);

  if(puc_key != NULL)
  {
    *puc_key = (uchar)_u32_id;

    strcpy(puc_key + i_shift, _z_name);
  }

  return puc_key;
}

/***************************************************************************
 key_delete
 
 This function a key. 
 Returns nothing.
 ***************************************************************************/
void key_delete(uchar *_puc_key)
{
  if(_puc_key != NULL)
  {
    free(_puc_key);
  }
}

/***************************************************************************
 key_compare
 
 This function compares a key with plugin info. 
 Returns TRUE/FALSE based upon comparison result.
 ***************************************************************************/
bool key_compare(uchar *_puc_key1, uchar *_puc_key2)
{
  bool b_result;
  int32 i_shift = sizeof(unsigned long);

  /* ID Comparison */
  b_result = ((unsigned long) *_puc_key1 == (unsigned long) *_puc_key2) ?
             TRUE :
             FALSE;

  /* Name Compare ? */
  if(b_result == FALSE)
  {
    b_result = (strcmp((_puc_key1 + i_shift), (_puc_key2 + i_shift)) == 0) ?
              TRUE :
              FALSE;
  }

  return b_result;
}

/***************************************************************************
 function_cell_create
 
 This function creates & initiates a function_info cell. 
 Returns created cell.
 ***************************************************************************/
plugin_st_function_info *function_cell_create()
{
  plugin_st_function_info *pst_cell;

  /* Creates a function info cell */
  pst_cell = (plugin_st_function_info *) malloc(sizeof(plugin_st_function_info));

  /* Initiates it */
  pst_cell->pfn_function    = NULL;
  pst_cell->u32_function_id  = PLUGIN_KUL_INVALID_ID;

  return pst_cell;
}

/***************************************************************************
 plugin_cell_create
 
 This function creates & initiates a plugin_info cell. 
 Returns created cell.
 ***************************************************************************/
plugin_st_plugin_info *plugin_cell_create()
{
  plugin_st_plugin_info *pst_cell;

  /* Creates a plugin info cell */
  pst_cell = (plugin_st_plugin_info *) malloc(sizeof(plugin_st_plugin_info));

  /* Initiates it */
  pst_cell->p_plugin_handle   = NULL;
  pst_cell->u32_plugin_id      = PLUGIN_KUL_INVALID_ID;
  pst_cell->p_function_map    = 
    (plugin_st_function_info *)map_create(PLUGIN_KI_NAME_SIZE,
                                          sizeof(plugin_st_function_info),
                                          &key_compare);

  return pst_cell;
}

/***************************************************************************
 function_cell_delete

 This function deletes a function_info cell

 Returns nothing.
 ***************************************************************************/
void function_cell_delete(plugin_st_function_info *_pst_cell)
{
  /* Is cell valid? */
  if(_pst_cell != NULL)
  {
    /* Delete cell */
    free(_pst_cell);
  }
      
  /* Done */
  return;
}

/***************************************************************************
 plugin_cell_delete
 
 This function shuts down the given plugin.
 It should be called from plugin_unload_by_id or plugin_unload_by_name.
 
 Returns nothing.
 ***************************************************************************/
void plugin_cell_delete(plugin_st_plugin_info *_pst_cell)
{
  uchar *puc_key;

  /* Is cell valid? */
  if(_pst_cell != NULL)
  {
    /* Unload plugin */
    if(_pst_cell->p_plugin_handle != NULL)
    {
      PLUGIN_CLOSE(_pst_cell->p_plugin_handle);
    }

    /* Free function map */
    if(_pst_cell->p_function_map != NULL)
    {
      map_destroy(_pst_cell->p_function_map);
    }

    /* Compute a complete key */
    puc_key = key_create(_pst_cell->u32_plugin_id, _pst_cell->z_plugin_name);

    /* Delete plugin cell */
    map_delete(sp_plugin_map, puc_key);

    /* Delete computed key */
    key_delete(puc_key);
  }
      
  /* Done */
  return;
}

/***************************************************************************
 plugin_locate_by_id
 
 This function finds a plugin by its ID, and returns a reference to it
 if it exists, otherwise NULL.

 ***************************************************************************/
plugin_st_plugin_info *plugin_locate_by_id(unsigned long _u32_plugin_id)
{
  plugin_st_plugin_info *pst_plugin_cell;
  uchar *puc_key;

  /* Compute key with id */
  puc_key = key_create(_u32_plugin_id, NULL);

  /* Search for the requested plugin */
  pst_plugin_cell = (plugin_st_plugin_info *)map_find(sp_plugin_map, puc_key);

  /* Delete the key */
  key_delete(puc_key);

  return pst_plugin_cell;
}

/***************************************************************************
 plugin_locate_by_name
 
 This function finds a plugin by its name, and returns a reference to it
 if it exists, otherwise NULL.

 ***************************************************************************/
plugin_st_plugin_info *plugin_locate_by_name(char *_z_plugin_name)
{
  plugin_st_plugin_info *pst_plugin_cell;
  uchar *puc_key;

  /* Compute key with id */
  puc_key = key_create(0, _z_plugin_name);

  /* Search for the requested plugin */
  pst_plugin_cell = (plugin_st_plugin_info *)map_find(sp_plugin_map, puc_key);

  /* Delete the key */
  key_delete(puc_key);

  return pst_plugin_cell;
}

/***************************************************************************
 get_func_addr
 Returns a void pointer to the function located in the plugin of handle param1,
  named param2.
 
 returns: pointer if success, NULL if error (do plugin_error to find out)
 ***************************************************************************/
void *get_func_addr(PLUGIN_HANDLE _p_plugin_handle, char *_z_function_name)
{
  void *p_function_handle = NULL;

  /* Check validity of parameters */
  if(_z_function_name == NULL)
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_BAD_PARAMETERS);

    return NULL;
  }
  
  if(_p_plugin_handle == NULL)
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);

    return NULL;
  }
  
  /* Attempt to obtain the pointer to the func */
  p_function_handle = PLUGIN_SYM(_p_plugin_handle, _z_function_name);

  if(p_function_handle == NULL)
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_FUNCTION_NOT_FOUND);
  }

  /* Finally, return the void pointer to the caller */
  return p_function_handle;
}

/***************************************************************************
 function_core_register
 
 This function registers a core function.
 Returns void.
 ***************************************************************************/
void function_core_register(plugin_function _pfn_function, unsigned long _u32_function_id)
{
  plugin_core_st_function *pst_core_function;
  uint32 i_index;

  /* Locates corresponding core function info array */
  i_index = (_u32_function_id & PLUGIN_KUL_ID_INDEX_MASK) >> PLUGIN_KI_ID_INDEX_BITS;

  if(i_index < PLUGIN_CORE_KI_NUMBER)
  {
    pst_core_function = sapst_function[i_index];

    i_index = _u32_function_id & PLUGIN_KUL_FUNCTION_MASK;

//    fprintf(stdout, "ID %x index %d ppfn %p pfn %p\n", _u32_function_id, i_index, pst_core_function[i_index].pfn_function, _pfn_function);
    if(pst_core_function[i_index].pfn_function != NULL)
    {
      *(pst_core_function[i_index].pfn_function) = _pfn_function;
    }
  }

  /* Done */
  return;
}

/***************************************************************************
 plugin_register
 
 This function registers a plugin.
 Returns void.
 ***************************************************************************/
void plugin_register(PLUGIN_HANDLE _p_handle, plugin_st_plugin_info *_pst_plugin)
{
  void (*pfn_init)(int32 *, plugin_user_st_function_info **) = get_func_addr(_p_handle, PLUGIN_USER_KZ_FUNCTION_INIT);
  plugin_st_function_info *pst_function_cell;
  int32 i_function_number = 0, i;
  plugin_user_st_function_info *pst_function;
  uchar* puc_key;

  /*
   * Call plugin init function
   */
 
  if(pfn_init != NULL)
  {
    pfn_init(&i_function_number, &pst_function);
  }

  /* Creates a cell */
  pst_function_cell = function_cell_create();

  /* Adds all functions to plugin info */
  for(i = 0; i < i_function_number; i++)
  {
    if(pst_function[i].pfn_function != NULL)
    {
      /* Copies infos */
      pst_function_cell->pfn_function = pst_function[i].pfn_function;
      strcpy(pst_function_cell->z_function_args, pst_function[i].z_function_args);
      strcpy(pst_function_cell->z_function_name, pst_function[i].z_function_name);
      pst_function_cell->u32_function_id = pst_function[i].u32_function_id;

      /* Adds function info in plugin info structure */
      puc_key = key_create(pst_function_cell->u32_function_id, pst_function_cell->z_function_name);
      map_add(_pst_plugin->p_function_map, puc_key, (uchar *)pst_function_cell);
      key_delete(puc_key);

      /* Checks if it's a core plugin */
      if(pst_function_cell->u32_function_id & PLUGIN_KUL_ID_FLAG_CORE)
      {
        function_core_register(pst_function_cell->pfn_function, pst_function_cell->u32_function_id);
      }
    }
  }

  /* Deletes function cell */
  function_cell_delete(pst_function_cell);

  /* Done */
  return;
}

/***************************************************************************
 plugin_core_info_add
 
 This function adds a core plugin info structure to the global info array.
 Returns void.
 ***************************************************************************/
void plugin_core_info_add(unsigned long _u32_core_id, plugin_core_st_function *_pst_core_function, int32 _i_core_function_number)
{
  unsigned long u32_index = (_u32_core_id & PLUGIN_KUL_ID_INDEX_MASK) >> PLUGIN_KI_ID_INDEX_BITS;

  if(u32_index < PLUGIN_CORE_KI_NUMBER)
  {
    sapst_function[u32_index] = _pst_core_function;
    si_function_number[u32_index] = _i_core_function_number;
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 plugin_core_function_default
 Default function for non-initialized core plugin functions.
 Log the problem.

 returns: NULL
 ***************************************************************************/
void *plugin_core_function_default(char *_zFunctionName, char *_zFileName, uint32 _u32Line)
{
  DEBUG_FLAG_BACKUP();
  DEBUG_FLAG_SET(DEBUG_KUL_FLAG_SCREEN
                |DEBUG_KUL_FLAG_FILE
                |DEBUG_KUL_FLAG_TIMESTAMP
                |DEBUG_KUL_FLAG_TYPE,
                 DEBUG_KUL_FLAG_ALL);
  DEBUG(D_ALL, MSG_PLUGIN_KZ_DEFAULT_NOT_LOADED_ZZI, _zFunctionName, _zFileName, _u32Line);
  DEBUG_FLAG_RESTORE();

  return NULL;
}

/***************************************************************************
 plugin_load
 Loads the shared object specified as a null terminated string, and makes
 it subsequently available under the name in the 2nd param.
 
 returns: plugin ID on success, KUL_UNDEFINED if load error
 ***************************************************************************/
unsigned long plugin_load(char *_z_plugin_filename, char *_z_plugin_name)
{
  PLUGIN_HANDLE p_handle;

  plugin_st_plugin_info *pst_cell;
  uchar *puc_key;

  /* Ready to Work? */
  if(!(plugin_su32_flags & PLUGIN_KUL_FLAG_READY))
  {
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_NOT_READY);
    return KUL_UNDEFINED;
  }

  /* Check params */
  if(_z_plugin_filename == NULL)
  {
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_INVALID_FILENAME);
    return KUL_UNDEFINED;
  }
  if(_z_plugin_name == NULL)
  {
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_INVALID_NAME);
    return KUL_UNDEFINED;
  }
  
  /* Attempt to link in the shared object */
  if((p_handle = PLUGIN_OPEN(_z_plugin_filename)) == NULL)
  {
    /* Log an error message here, but I forgot the prototype ^^ */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_LOAD_FAILED);
    return KUL_UNDEFINED;
  }
  
  /*
   * Create the control structure used within the plugin module to keep
   * track of modules
   */
  if((pst_cell = plugin_cell_create()) == NULL)
  {
    PLUGIN_CLOSE(p_handle);
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_MALLOC_FAILED);
    return KUL_UNDEFINED;
  }
  
  /*
   * Fill in the control structure
   */

  /* First, plug in the reference to the dynamic code */
  pst_cell->p_plugin_handle = p_handle;

  /* Then, assign an ID */
  pst_cell->u32_plugin_id = (unsigned long)map_count(sp_plugin_map) + 1;

  /* Store info on registered functions */
  plugin_register(p_handle, pst_cell);

  /* Finally, enter the name */  
  strcpy(pst_cell->z_plugin_name, _z_plugin_name);

  /* Compute a complete key */
  puc_key = key_create(pst_cell->u32_plugin_id, _z_plugin_name);

  /* Enter the new structure into the map */
  map_add(sp_plugin_map, puc_key, (uchar *)pst_cell);

  /* Delete key */
  key_delete(puc_key);

  /* Return ID */
  return pst_cell->u32_plugin_id;
}


/***************************************************************************
 plugin_load_ext
 Loads the shared object specified as a null terminated string using OS library extension,
 and makes it subsequently available under the name in the 2nd param.

 returns: plugin ID on success, KUL_UNDEFINED if load error
 ***************************************************************************/
unsigned long plugin_load_ext(char *_z_plugin_filename, char *_z_plugin_name)
{
  char z_filename[128];

  sprintf(z_filename, "%s.%s", _z_plugin_filename, PLUGIN_LIBRARY_EXT);

  return(plugin_load(z_filename, _z_plugin_name));
}


/***************************************************************************
 plugin_unload_by_id
 Unloads the shared object designated by its ID
 
 returns: TRUE if success, FALSE if unload error (usually 'plugin not loaded')
 ***************************************************************************/
bool plugin_unload_by_id(unsigned long _u32_plugin_id)
{
  plugin_st_plugin_info *pst_node;

  if((pst_node = plugin_locate_by_id(_u32_plugin_id)) != NULL)
  {
    /* Plugin found, kill it and return okay */
    plugin_cell_delete(pst_node);

    return TRUE;
  }
  else
  {
    /* Search did not return positive, so return error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);

    return FALSE;
  }
}

/***************************************************************************
 plugin_unload_by_name
 Unloads the shared object designated by its name instead of ID
 
 returns: TRUE if success, FALSE if unload error (usually 'plugin not loaded')
 ***************************************************************************/
bool plugin_unload_by_name(char *_z_plugin_name)
{
  plugin_st_plugin_info *pst_node;

  if((pst_node = plugin_locate_by_name(_z_plugin_name)) != NULL)
  {
    /* Plugin found, kill it and return okay */
    plugin_cell_delete(pst_node);

    return TRUE;
  }
  else
  {
    /* Search did not return positive, so return error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);

    return FALSE;
  }
}

/***************************************************************************
 plugin_get_func_addr_by_id
 Returns a void pointer to the function located in the plugin of ID param1,
  named param2.
 
 returns: pointer if success, NULL if error (do plugin_error to find out)
 ***************************************************************************/
void *plugin_get_func_addr_by_id(unsigned long _u32_plugin_id, char *_z_function_name)
{
  plugin_st_plugin_info *pst_cell;
  void *p_function_handle = NULL;

  /* Gets the plugin cell */
  pst_cell = (plugin_st_plugin_info *)plugin_locate_by_id(_u32_plugin_id);

  /* Try to get the function handle */
  p_function_handle = get_func_addr(pst_cell->p_plugin_handle,
                                    _z_function_name);

  /* No function found ? */
  if(p_function_handle == NULL)
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_FUNCTION_NOT_FOUND);
  }

  /* Finally, return the void pointer to the caller */
  return p_function_handle;
}

/***************************************************************************
 plugin_get_func_addr_by_name
 Returns a void pointer to the function located in the plugin of name param1,
  named param2.
 
 returns: pointer if success, NULL if error (do plugin_error to find out)
 ***************************************************************************/
void *plugin_get_func_addr_by_name(char *_z_plugin_name, char *_z_function_name)
{
  plugin_st_plugin_info *pst_cell;
  void *p_function_handle = NULL;

  /* Gets the plugin cell */
  pst_cell = (plugin_st_plugin_info *)plugin_locate_by_name(_z_plugin_name);

  /* Plugin found? */
  if(pst_cell != NULL)
  {
    /* Try to get the function handle */
    p_function_handle = get_func_addr(pst_cell->p_plugin_handle,
                                      _z_function_name);

    /* No function found ? */
    if(p_function_handle == NULL)
    {
      /* Log an error */
      DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_FUNCTION_NOT_FOUND);
    }
  }
  else
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);
  }

  return p_function_handle;
}

/***************************************************************************
 plugin_get_id_by_name
 
 This function returns the integer ID of the plugin of given name
 
 returns ID on success, KUL_UNDEFINED on error
 ***************************************************************************/
unsigned long plugin_get_id_by_name(char *_z_plugin_name)
{
  plugin_st_plugin_info *pst_cell;

  /* Check parameter validity */
  if(_z_plugin_name == NULL)
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_BAD_PARAMETERS);

    return KUL_UNDEFINED;
  }

  if((pst_cell = plugin_locate_by_name(_z_plugin_name)) == NULL)
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);

    return KUL_UNDEFINED;
  }
  
  return(pst_cell->u32_plugin_id);
}

/***************************************************************************
 plugin_get_name_by_id
 
 This function returns the name of the plugin of given ID
 
 returns ID on success, -1 on error
 ***************************************************************************/
char *plugin_get_name_by_id(unsigned long _u32_plugin_id)
{
  plugin_st_plugin_info *pst_cell;

  if((pst_cell = plugin_locate_by_id(_u32_plugin_id)) == NULL)
  {
    /* Log an error */
    DEBUG(D_PLUGIN, MSG_PLUGIN_KZ_NOT_FOUND);

    return NULL;
  }

  return(pst_cell->z_plugin_name);
}

uint32 plugin_init()
{
  int32 i;

  if(!(plugin_su32_flags & PLUGIN_KUL_FLAG_READY))
  {
    /* Creates an empty spst_plugin_list */
    sp_plugin_map = (plugin_st_plugin_info *)map_create(PLUGIN_KI_NAME_SIZE,
                                                        sizeof(plugin_st_plugin_info),
                                                        &key_compare);

    /* Inits static core info structures */
    for(i = 0; i < PLUGIN_CORE_KI_NUMBER; i++)
    {
      sapst_function[i] = NULL;
      si_function_number[i] = 0;
    }

    /* Updates status flags */
    plugin_su32_flags |= PLUGIN_KUL_FLAG_READY;

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

void plugin_exit()
{
  uint32 i;
  plugin_st_plugin_info *pst_cell;

  /* Delete all plugin cells */
  for(i = 1; i <= map_count(sp_plugin_map); i++)
  {
    /* Gets plugin */
    pst_cell = plugin_locate_by_id((unsigned long)i);

    /* Delete plugin cell */
    plugin_cell_delete(pst_cell);
  }

  /* Destroy plugin map */
  map_destroy(sp_plugin_map);

  /* Updates status flags */
  plugin_su32_flags = PLUGIN_KUL_FLAG_NONE;

  return;
}
