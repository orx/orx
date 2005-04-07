#include "io/interaction.h"

#include "core/clock.h"


#define INTERACTION_KS32_GET_TIMER    50

orxSTATIC orxVOID interaction_get()
{

  return;
}

orxU32 interaction_init()
{
  return ((clock_cb_function_add(interaction_get, INTERACTION_KS32_GET_TIMER) == orxFALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILED);
}

orxVOID interaction_exit()
{
  clock_cb_function_remove(interaction_get, INTERACTION_KS32_GET_TIMER);

  return;
}
