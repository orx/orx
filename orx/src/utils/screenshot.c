#include "utils/screenshot.h"

#include "debug/orxDebug.h"
#include "graph/graph.h"
#include "io/file.h"

#include <string.h>
#include <stdio.h>

/*
 * Platform independant defines
 */

#define SCREENSHOT_KU32_FLAG_NONE  0x00000000
#define SCREENSHOT_KU32_FLAG_READY 0x00000001

/*
 * Static information structures
 */

orxSTATIC orxS32 si_screenshot_number = 0;
orxSTATIC orxU32 screenshot_su32Flags = SCREENSHOT_KU32_FLAG_NONE;


orxU32 screenshot_init()
{
  orxU8 ac_find_file[512];
  file_st_file_infos st_file;

  strcpy(ac_find_file, SCREENSHOT_KZ_DIRECTORY);
  if(!(file_exist(SCREENSHOT_KZ_DIRECTORY)))
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_DIRECTORY_MISSING_S, SCREENSHOT_KZ_DIRECTORY);
    strcpy(ac_find_file, ".");

    if(!(file_exist(".")))
    {
	    orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_DIRECTORY_UNREADABLE_S, SCREENSHOT_KZ_DIRECTORY);

      screenshot_su32Flags &= ~SCREENSHOT_KU32_FLAG_READY;

	    return orxSTATUS_FAILED;
    }
  }

  /* Everything's fine! :) */
  screenshot_su32Flags |= SCREENSHOT_KU32_FLAG_READY;

  strcat(ac_find_file, "/");
  strcat(ac_find_file, SCREENSHOT_KZ_PREFIX);
  strcat(ac_find_file, "*.*");

  if(file_find_first(ac_find_file, &st_file))
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_SHOT_FOUND_S, st_file.ac_name);
    si_screenshot_number++;
    while(file_find_next(&st_file))
    {
      orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_SHOT_FOUND_S, st_file.ac_name);
      si_screenshot_number++;
    }
  }

  orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_INIT_DONE_SSI, SCREENSHOT_KZ_PREFIX, SCREENSHOT_KZ_DIRECTORY, si_screenshot_number);

  return orxSTATUS_SUCCESS;
}

orxVOID screenshot_exit()
{
  /* !!! MSG !!! */

  return;
}

orxVOID screenshot_take()
{
  orxU8 ac_name[256];

  if(screenshot_su32Flags & SCREENSHOT_KU32_FLAG_READY)
  {
    sprintf(ac_name, "%s/%s-%ld%ld%ld%ld.%s", SCREENSHOT_KZ_DIRECTORY, SCREENSHOT_KZ_PREFIX, (si_screenshot_number/1000)%10000, (si_screenshot_number/100)%1000, (si_screenshot_number/10)%100, si_screenshot_number%10, SCREENSHOT_KZ_EXT);

    orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_SHOT_TAKEN_S, ac_name);

    graph_bitmap_save(ac_name, graph_screen_bitmap_get());
  }
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_NOT_ACTIVATED);
  }

  si_screenshot_number++;

  return;
}
