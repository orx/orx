#include "io/interaction.h"

#include "core/clock.h"


#define INTERACTION_KI_GET_TIMER    50

static void interaction_get()
{

  return;
}

uint32 interaction_init()
{
  return ((clock_cb_function_add(interaction_get, INTERACTION_KI_GET_TIMER) == FALSE) ? EXIT_SUCCESS : EXIT_FAILURE);
}

void interaction_exit()
{
  clock_cb_function_remove(interaction_get, INTERACTION_KI_GET_TIMER);

  return;
}
