#include "include.h"

#include "anim/anim.h"
#include "anim/animpointer.h"
#include "camera/camera.h"
#include "camera/render.h"
#include "camera/viewport.h"
#include "core/clock.h"
#include "core/timer.h"
#include "debug/debug.h"
#include "graph/graph.h"
#include "graph/graphic.h"
#include "graph/texture.h"
#include "io/joystick.h"
#include "io/keyboard.h"
#include "io/mouse.h"
#include "object/object.h"
#include "object/frame.h"
#include "plugin/plugin.h"
#include "sound/sound.h"
#include "utils/screenshot.h"

#include "msg/msg_game.h"

#include <allegro.h>


/*
 * Début des fonctions
 */

static uint32 init(int32 argc, char **argv)
{
  /* Initialisation de la sortie debug */
  DEBUG_INIT();

  /* Initializes Core Plugins */
  plugin_init();
  timer_plugin_init();
  graph_plugin_init();
  sound_plugin_init();
  keyboard_plugin_init();
  mouse_plugin_init();
  joystick_plugin_init();

  /* Loads Core Plugins */
  if(plugin_load_ext("timer_plug", "TIMER_PLUG") == KUL_UNDEFINED)
  {
    DEBUG(D_ALL, "Problème de chargement du plugin \"timer_plug\"!!!");
  }
  if(plugin_load_ext("graph_plug", "GRAPH_PLUG") == KUL_UNDEFINED)
  {
    DEBUG(D_ALL, "Problème de chargement du plugin \"graph_plug\"!!!");
  }
  if(plugin_load_ext("sound_plug", "SOUND_PLUG") == KUL_UNDEFINED)
  {
    DEBUG(D_ALL, "Problème de chargement du plugin \"sound_plug\"!!!");
  }
  if(plugin_load_ext("keyboard_plug", "KEYBOARD_PLUG") == KUL_UNDEFINED)
  {
    DEBUG(D_ALL, "Problème de chargement du plugin \"keyboard_plug\"!!!");
  }
  if(plugin_load_ext("mouse_plug", "MOUSE_PLUG") == KUL_UNDEFINED)
  {
    DEBUG(D_ALL, "Problème de chargement du plugin \"mouse_plug\"!!!");
  }
  if(plugin_load_ext("joystick_plug", "JOYSTICK_PLUG") == KUL_UNDEFINED)
  {
    DEBUG(D_ALL, "Problème de chargement du plugin \"joystick_plug\"!!!");
  }

  /* Initialise les modules structure, object, frame & texture, etc... */
  if(timer_init() != EXIT_SUCCESS)
  {
  }
  if(clock_init() != EXIT_SUCCESS)
  {
  }
  if(structure_init() != EXIT_SUCCESS)
  {
  }
  if(object_init() != EXIT_SUCCESS)
  {
  }
  if(frame_init() != EXIT_SUCCESS)
  {
  }
  if(graphic_init() != EXIT_SUCCESS)
  {
  }
  if(texture_init() != EXIT_SUCCESS)
  {
  }
  if(camera_init() != EXIT_SUCCESS)
  {
  }
  if(viewport_init() != EXIT_SUCCESS)
  {
  }
  if(render_init() != EXIT_SUCCESS)
  {
  }
  if(animpointer_init() != EXIT_SUCCESS)
  {
  }

  /* Initialise le mode graphique */
  DEBUG(D_LOG, KZ_MSG_GAME_GRAPH_INIT_III, 800, 600, 24);
  if(graph_init() != 0)
  {
    DEBUG(D_LOG, KZ_MSG_GAME_GRAPH_INIT_FAILED_III, 800, 600, 24);

    return EXIT_FAILURE;
  }

  DEBUG(D_LOG, KZ_MSG_GAME_DONE);

  /* Initialise le mécanisme de screenshots */
  screenshot_init();

  /* Tout s'est bien passé! */
  return EXIT_SUCCESS;
}  

static void end()
{
  screenshot_exit();
  graph_exit();
  render_exit();
  viewport_exit();
  camera_exit();
  object_exit();
  graphic_exit();
  frame_exit();
  animpointer_exit();
  texture_exit();
  structure_exit();
  timer_exit();
  clock_exit();

  return;
}

static void test()
{
  graphic_st_graphic *pst_graphic = graphic_create();
  camera_st_camera *pst_camera = camera_create();
  viewport_st_viewport *pst_viewport = viewport_create();
  graph_st_bitmap *pst_graph = graph_bitmap_load("test0.bmp");
  texture_st_texture *pst_texture = texture_create_from_bitmap(pst_graph);;
  coord_st_coord st_coord;
  int32 i, j, h, w, num_w, num_h;

  graphic_struct_link(pst_graphic, (structure_st_struct *)(pst_texture));

  graph_bitmap_size_get(pst_graph, &w, &h);

  /* Inits objects */
  num_w = 16;
  num_h = 16;

  /* Inits viewport & camera */
  coord_set(&st_coord, num_w * w, num_h * h, 0);
  camera_2d_position_set(pst_camera, &st_coord);
  coord_set(&st_coord, 200, 150, 0);
  viewport_position_set(pst_viewport, &st_coord);
  coord_set(&st_coord, 800, 600, 0);
  camera_2d_size_set(pst_camera, &st_coord);
  coord_set(&st_coord, 400, 300, 0);
  viewport_size_set(pst_viewport, &st_coord);

  viewport_camera_set(pst_viewport, pst_camera);

  object_st_object *past_object_list[num_w * num_h];
  frame_st_frame *past_frame_list[num_w * num_h];

  for(i = 0; i < num_w; i++)
  {
    for(j = 0; j < num_h; j++)
    {
      past_frame_list[(i * num_h) + j] = frame_create();
      past_object_list[(i * num_h) + j] = object_create();

      st_coord.s32_x = 2 * i * w;
      st_coord.s32_y = 2 * j * h;

      frame_2d_position_set(past_frame_list[(i * num_h) + j], &st_coord);
      object_struct_link(past_object_list[(i * num_h) + j], (structure_st_struct *)(past_frame_list[(i * num_h) + j]));
      object_struct_link(past_object_list[(i * num_h) + j], (structure_st_struct *)(pst_graphic));
    }
  }

  allegro_init();
  install_keyboard();

  while(!key[KEY_ESC])
  {
    /* Updates camera */
    if(key[KEY_LEFT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_RIGHT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_UP])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_DOWN])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_PLUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f < 2.0)
      {
        camera_2d_zoom_set(pst_camera, f * 1.1);
      }
    }
    if(key[KEY_MINUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f > 0.5)
      {
        camera_2d_zoom_set(pst_camera, f / 1.1);
      }
    }
    if(key[KEY_ASTERISK])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) + 0.1);
    }
    if(key[KEY_SLASH_PAD])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) - 0.1);
    }
    if(key[KEY_F12])
    {
      screenshot_take();
    }

    render_all();

    graph_switch();
  }

  allegro_exit();

  
//  screenshot_take();
//
//  graph_clear(graph_screen_bitmap_get());
//
//  for(i = 1; i < 5; i++)
//  {
//    frame_parent_set(past_frame_list[i], past_frame_list[i-1]);
//  }
//
//  camera_view_list_update(pst_camera);
//  pst_view = camera_view_list_first_get(pst_camera);
//
//  while(pst_view != NULL)
//  {
//    frame_st_frame *pst_frame = camera_view_list_frame_get(pst_view);
//
//    fprintf(stdout, "%i :", i);
//    frame_2d_position_get(pst_frame, &st_coord, TRUE);
//    fprintf(stdout, " local (%i, %i, %i), %f, %f |", st_coord.s32_x, st_coord.s32_y, st_coord.s32_z, frame_2d_rotation_get(pst_frame, TRUE), frame_2d_scale_get(pst_frame, TRUE));
//    frame_2d_position_get(pst_frame, &st_coord, FALSE);
//    fprintf(stdout, " global (%i, %i, %i), %f, %f\n", st_coord.s32_x, st_coord.s32_y, st_coord.s32_z, frame_2d_rotation_get(pst_frame, FALSE), frame_2d_scale_get(pst_frame, FALSE));
//
//    texture_st_texture *pst_texture = camera_view_list_texture_get(pst_view);
//    graph_st_bitmap *pst_bitmap = texture_bitmap_get(pst_texture);
//    graph_bitmap_size_get(pst_bitmap, &w, &h);
//    graph_blit(pst_bitmap, graph_screen_bitmap_get(), 0, 0, st_coord.s32_x, st_coord.s32_y, w, h);
//
//    pst_view = camera_view_list_next_get(pst_camera);
////    frame_st_frame *pst_frame = (frame_st_frame *)object_struct_get(past_object_list[i], STRUCTURE_KUL_STRUCT_ID_FRAME);
////    fprintf(stdout, "%i :", i);
////    frame_2d_position_get(pst_frame, &st_coord, TRUE);
////    fprintf(stdout, " local (%i, %i, %i), %f, %f |", st_coord.s32_x, st_coord.s32_y, st_coord.s32_z, frame_2d_rotation_get(pst_frame, TRUE), frame_2d_scale_get(pst_frame, TRUE));
////    frame_2d_position_get(pst_frame, &st_coord, FALSE);
////    fprintf(stdout, " global (%i, %i, %i), %f, %f\n", st_coord.s32_x, st_coord.s32_y, st_coord.s32_z, frame_2d_rotation_get(pst_frame, FALSE), frame_2d_scale_get(pst_frame, FALSE));
////
////    graphic_st_graphic *pst_graphic = (graphic_st_graphic *)object_struct_get(past_object_list[i], STRUCTURE_KUL_STRUCT_ID_GRAPHIC);
////    texture_st_texture *pst_texture = graphic_data_get(past_graphic_list[i]);
////    graph_st_bitmap *pst_bitmap = texture_bitmap_get(pst_texture);
////
////    graph_bitmap_size_get(pst_bitmap, &w, &h);
////
////    graph_blit(pst_bitmap, graph_screen_bitmap_get(), 0, 0, st_coord.s32_x, st_coord.s32_y, w, h);
//  }
//
//  screenshot_take();
//
//
}

static void test2()
{
  int32 i, j, num_i, num_j;

  num_i = 4;
  num_j = 2;

  graphic_st_graphic *pst_graphic_list[num_i];
  camera_st_camera *pst_camera = camera_create();
  viewport_st_viewport *pst_viewport = viewport_create();
  graph_st_bitmap *pst_graph_list[num_i];
  texture_st_texture *pst_texture_list[num_i];
  coord_st_coord st_coord;

  /* Inits objects */

  for(i = 0; i < num_i; i++)
  {
    char az_name[16];
    sprintf(az_name, "test%ld.bmp", i);
    pst_graph_list[i] = graph_bitmap_load(az_name);
    pst_texture_list[i] = texture_create_from_bitmap(pst_graph_list[i]);
    pst_graphic_list[i] = graphic_create();
    graphic_struct_link(pst_graphic_list[i], (structure_st_struct *)(pst_texture_list[i]));
  }

/* Inits viewport & camera */
  coord_set(&st_coord, 0, 0, 0);
  camera_2d_position_set(pst_camera, &st_coord);
  viewport_position_set(pst_viewport, &st_coord);
  coord_set(&st_coord, 800, 600, 512);
  camera_2d_size_set(pst_camera, &st_coord);
  coord_set(&st_coord, 800, 600, 0);
  viewport_size_set(pst_viewport, &st_coord);

  viewport_camera_set(pst_viewport, pst_camera);

  object_st_object *past_object_list[num_i * num_j];
  frame_st_frame *past_frame_list[num_i * num_j];

  for(i = 0; i < num_i; i++)
  {
    for(j = 0; j < num_j; j ++)
    {
      past_frame_list[(i * num_j) + j] = frame_create();
      past_object_list[(i * num_j) + j] = object_create();

      st_coord.s32_x = 0;
      st_coord.s32_y = 0;
      st_coord.s32_z = - 10 * i;

      frame_2d_position_set(past_frame_list[(i * num_j) + j], &st_coord);
      frame_differential_scrolling_set(past_frame_list[(i * num_j) + j], 1.1 * (float)((i * num_j) + j), 1.1 * (float)((i * num_j) + j));
      object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(past_frame_list[(i * num_j) + j]));
      object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(pst_graphic_list[i]));
    }
  }

  allegro_init();
  install_keyboard();

  while(!key[KEY_ESC])
  {
    /* Updates camera */
    if(key[KEY_LEFT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_RIGHT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_UP])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_DOWN])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_PLUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f < 2.0)
      {
        camera_2d_zoom_set(pst_camera, f * 1.1);
      }
    }
    if(key[KEY_MINUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f > 0.5)
      {
        camera_2d_zoom_set(pst_camera, f / 1.1);
      }
    }
    if(key[KEY_ASTERISK])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) + 0.1);
    }
    if(key[KEY_SLASH_PAD])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) - 0.1);
    }
    if(key[KEY_F12])
    {
      screenshot_take();
    }

    render_all();

    graph_switch();
  }

  allegro_exit();
}

static void test3()
{
  int32 i, j, num_i, num_j;

  num_i = 4;
  num_j = 15;

  graphic_st_graphic *pst_graphic_list[num_i];
  camera_st_camera *pst_camera = camera_create();
  viewport_st_viewport *pst_viewport = viewport_create();
  graph_st_bitmap *pst_graph_list[num_i];
  texture_st_texture *pst_texture_list[num_i];
  coord_st_coord st_coord, st_coord2;

  /* Inits objects */

  for(i = 0; i < num_i; i++)
  {
    char az_name[16];
    sprintf(az_name, "test%ld.bmp", i);
    pst_graph_list[i] = graph_bitmap_load(az_name);
    pst_texture_list[i] = texture_create_from_bitmap(pst_graph_list[i]);
    pst_graphic_list[i] = graphic_create();
    graphic_struct_link(pst_graphic_list[i], (structure_st_struct *)(pst_texture_list[i]));
  }

/* Inits viewport & camera */
  coord_set(&st_coord, 0, 0, 0);
  camera_2d_position_set(pst_camera, &st_coord);
  viewport_position_set(pst_viewport, &st_coord);
  coord_set(&st_coord, 800, 600, 512);
  camera_2d_size_set(pst_camera, &st_coord);
  coord_set(&st_coord, 800, 600, 0);
  viewport_size_set(pst_viewport, &st_coord);

  viewport_camera_set(pst_viewport, pst_camera);

  coord_set(&st_coord, 0, 0, 0);
  coord_set(&st_coord2, 100 * num_j, 100 * num_i, 0); 
  camera_limit_set(pst_camera, &st_coord, &st_coord2);
  
  object_st_object *past_object_list[num_j * num_j];
  frame_st_frame *past_frame_list[num_j * num_j];

  for(i = 0; i < num_i; i++)
  {
    for(j = 0; j < num_j; j ++)
    {
      past_frame_list[(i * num_j) + j] = frame_create();
      past_object_list[(i * num_j) + j] = object_create();

      st_coord.s32_x = 100 * j;
      st_coord.s32_y = 100 * i;
      st_coord.s32_z = ((i + j) == 0) ? 0 : 10;

      frame_2d_position_set(past_frame_list[(i * num_j) + j], &st_coord);
      object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(past_frame_list[(i * num_j) + j]));
      if(i == 0)
      {
        if(j == 0)
        {
          object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(pst_graphic_list[0]));
        }
        else
        {
          object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(pst_graphic_list[1]));
        }
      }
      else
      {
        object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(pst_graphic_list[i]));
      }
    }
  }

  camera_link_set(pst_camera, past_object_list[0]);
  
  allegro_init();
  install_keyboard();

  while(!key[KEY_ESC])
  {
    /* Updates camera */
    if(key[KEY_LEFT])
    {
      frame_2d_position_get(past_frame_list[0], &st_coord, TRUE);
      st_coord.s32_x -= 10;
      frame_2d_position_set(past_frame_list[0], &st_coord);
    }
    if(key[KEY_RIGHT])
    {
      frame_2d_position_get(past_frame_list[0], &st_coord, TRUE);
      st_coord.s32_x += 10;
      frame_2d_position_set(past_frame_list[0], &st_coord);
    }
    if(key[KEY_UP])
    {
      frame_2d_position_get(past_frame_list[0], &st_coord, TRUE);
      st_coord.s32_y -= 10;
      frame_2d_position_set(past_frame_list[0], &st_coord);
    }
    if(key[KEY_DOWN])
    {
      frame_2d_position_get(past_frame_list[0], &st_coord, TRUE);
      st_coord.s32_y += 10;
      frame_2d_position_set(past_frame_list[0], &st_coord);

    }
    if(key[KEY_PLUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f < 2.0)
      {
        camera_2d_zoom_set(pst_camera, f * 1.1);
      }
    }
    if(key[KEY_MINUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f > 0.5)
      {
        camera_2d_zoom_set(pst_camera, f / 1.1);
      }
    }
    if(key[KEY_ASTERISK])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) + 0.1);
    }
    if(key[KEY_SLASH_PAD])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) - 0.1);
    }
    if(key[KEY_F12])
    {
      screenshot_take();
    }

    render_all();

    graph_switch();
  }

  allegro_exit();
}

void demo()
{
  camera_st_camera *pst_camera = camera_create();
  viewport_st_viewport *pst_viewport = viewport_create();
  graphic_st_graphic *pst_graphic;
  graph_st_bitmap *pst_graph;
  texture_st_texture *pst_texture;
  coord_st_coord st_coord;

  /* Inits object */

  pst_graph = graph_bitmap_load("demo.bmp");
  pst_texture = texture_create_from_bitmap(pst_graph);
  pst_graphic = graphic_create();
  graphic_struct_link(pst_graphic, (structure_st_struct *)pst_texture);

/* Inits viewport & camera */
  coord_set(&st_coord, 0, 0, 0);
  camera_2d_position_set(pst_camera, &st_coord);
  viewport_position_set(pst_viewport, &st_coord);
  coord_set(&st_coord, 800, 600, 512);
  camera_2d_size_set(pst_camera, &st_coord);
  coord_set(&st_coord, 800, 600, 0);
  viewport_size_set(pst_viewport, &st_coord);
  viewport_camera_set(pst_viewport, pst_camera);

  object_st_object *pst_object = object_create();
  frame_st_frame *pst_frame = frame_create();

  st_coord.s32_x = 0;
  st_coord.s32_y = 0;
  st_coord.s32_z = 0;

  frame_2d_position_set(pst_frame, &st_coord);
  object_struct_link(pst_object, (structure_st_struct *)pst_frame);
  object_struct_link(pst_object, (structure_st_struct *)pst_graphic);

  allegro_init();
  install_keyboard();

  while(!key[KEY_ESC])
  {
    /* Updates camera */
    if(key[KEY_LEFT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_RIGHT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_UP])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_DOWN])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_PLUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f < 10.0)
      {
        camera_2d_zoom_set(pst_camera, f * 1.1);
      }
    }
    if(key[KEY_MINUS_PAD])
    {
      float f = camera_2d_zoom_get(pst_camera);
      if(f > 0.5)
      {
        camera_2d_zoom_set(pst_camera, f / 1.1);
      }
    }
    if(key[KEY_ASTERISK])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) + 0.1);
    }
    if(key[KEY_SLASH_PAD])
    {
      camera_2d_rotation_set(pst_camera, camera_2d_rotation_get(pst_camera) - 0.1);
    }
    if(key[KEY_F12])
    {
      screenshot_take();
    }

    render_all();

    graph_switch();
  }

  allegro_exit();

  return;
}

void demo_anim()
{
  int32 i, j, num_i, num_j;

  num_i = 8;
  num_j = 8;

  camera_st_camera *pst_camera = camera_create();
  viewport_st_viewport *pst_viewport = viewport_create();
  graphic_st_graphic *past_graphic_list[num_i * num_j];
  graph_st_bitmap *past_bitmap_list[38];
  texture_st_texture *past_texture_list[38];
  anim_st_anim *past_anim_list[2];
  animset_st_animset *pst_animset;
  animpointer_st_animpointer *past_animpointer_list[num_i * num_j];
  frame_st_frame *past_frame_list[num_i * num_j];
  object_st_object *past_object_list[num_i * num_j];
  uint32 au32_anim_id[2];
  coord_st_coord st_coord;

  /* Creates anims */
  past_anim_list[0] = anim_create(ANIM_KUL_ID_FLAG_2D, 19);
  past_anim_list[1] = anim_create(ANIM_KUL_ID_FLAG_2D, 19);

  /* Creates bitmaps & inits anim */
  for(i = 0; i < 19; i++)
  {
    char az_name[16];
    sprintf(az_name, "test%ld%ld.bmp", i / 10, i % 10);
    past_bitmap_list[i] = graph_bitmap_load(az_name);
    graph_bitmap_color_key_set(past_bitmap_list[i], 0xFF, 0xFF, 0xFF, TRUE);
    past_texture_list[i] = texture_create_from_bitmap(past_bitmap_list[i]);
    anim_2d_texture_add(past_anim_list[0], past_texture_list[i], (i - 3) * 60);
  }
  for(i = 18; i >= 0; i--)
  {
    char az_name[16];
    sprintf(az_name, "test%ld%ld.bmp", i / 10, i % 10);
    past_bitmap_list[38 - i] = graph_bitmap_load(az_name);
    graph_bitmap_color_key_set(past_bitmap_list[38 - i], 0xFF, 0xFF, 0xFF, TRUE);
    past_texture_list[38 - i] = texture_create_from_bitmap(past_bitmap_list[38 - i]);
    anim_2d_texture_add(past_anim_list[1], past_texture_list[38 - i], (19 - i) * 60);
  }

  /* Inits animset */
  pst_animset = animset_create(2);
  for(i = 0; i < 2; i++)
  {
    au32_anim_id[i] = animset_anim_add(pst_animset, past_anim_list[i]);
  }
  animset_link_add(pst_animset, au32_anim_id[0], au32_anim_id[1]);
  animset_link_add(pst_animset, au32_anim_id[1], au32_anim_id[0]);

  /* Inits animpointers, graphics & objects */
  for(i = 0; i < num_i; i++)
  {
    for(j = 0; j < num_j; j ++)
    {
      past_frame_list[(i * num_j) + j] = frame_create();
      past_object_list[(i * num_j) + j] = object_create();
      past_graphic_list[(i * num_j) + j] = graphic_create();
      past_animpointer_list[(i * num_j) + j] = animpointer_create(pst_animset);
      animpointer_time_set(past_animpointer_list[(i * num_j) + j], ((i * num_j) + j) * 100);
      animpointer_frequency_set(past_animpointer_list[(i * num_j) + j], (((i * num_j) + j) * 0.1) + 1.0);

      st_coord.s32_x = 350 * j;
      st_coord.s32_y = 270 * i;
      st_coord.s32_z = ((i + j) == 0) ? 0 : 10;

      frame_2d_position_set(past_frame_list[(i * num_j) + j], &st_coord);
      graphic_struct_link(past_graphic_list[(i * num_j) + j], (structure_st_struct *)(past_animpointer_list[(i * num_j) + j]));
      object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(past_frame_list[(i * num_j) + j]));
      object_struct_link(past_object_list[(i * num_j) + j], (structure_st_struct *)(past_graphic_list[(i * num_j) + j]));
    }
  }


  /* Inits viewport & camera */
  coord_set(&st_coord, 0, 0, 0);
  camera_2d_position_set(pst_camera, &st_coord);
  viewport_position_set(pst_viewport, &st_coord);

  coord_set(&st_coord, 800, 600, 512);
  camera_2d_size_set(pst_camera, &st_coord);

  coord_set(&st_coord, 800, 600, 0);
  viewport_size_set(pst_viewport, &st_coord);

  viewport_camera_set(pst_viewport, pst_camera);

  /* Temp Allegro Keyboard Handler */
  allegro_init();
  install_keyboard();

  while(!key[KEY_ESC])
  {
    /* Updates camera */
    if(key[KEY_LEFT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_RIGHT])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_x += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_UP])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y -= 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_DOWN])
    {
      camera_2d_position_get(pst_camera, &st_coord);
      st_coord.s32_y += 10;
      camera_2d_position_set(pst_camera, &st_coord);
    }
    if(key[KEY_PLUS_PAD])
    {
     float f = timer_time_coef_get(NULL);

     if(f < 10.0)
     {
       f += 0.1;
       timer_time_coef_set(f, TIMER_KUL_COEF_TYPE_MULTIPLY);

       DEBUG(D_ALL, "Time multiplicator set to : %g.\n", f);
     }
    }
    if(key[KEY_MINUS_PAD])
    {
     float f = timer_time_coef_get(NULL);

     if(f > 0.1)
     {
       f -= 0.1;
       timer_time_coef_set(f, TIMER_KUL_COEF_TYPE_MULTIPLY);

       DEBUG(D_ALL, "Time multiplicator set to : %g.\n", f);
     }
    }
    if(key[KEY_F12])
    {
      screenshot_take();
    }

    /* Updates Time */
    timer_update();

    /* Renders all */
    render_all();

    /* Updates screen */
    graph_switch();
  }

  /* Exits from Allegro */
  allegro_exit();

  return;
}


/* FONCTION MAIN */
int32 main(int32 argc, char **argv)
{
  /* Initialisation globale */
  if(init(argc, argv) != EXIT_SUCCESS)
  {
    DEBUG(D_LOG, KZ_MSG_GAME_INIT_FAILED);
    end();

    return EXIT_FAILURE;
  }

/*
  demo();
*/

  demo_anim();

/*
  end();
  init(argc, argv);

  test();

  end();
  init(argc, argv);

  test2();

  end();
  init(argc, argv);

  test3();
*/

  /* Prépare la sortie du jeu */
  end();

  return EXIT_SUCCESS;
}
END_OF_MAIN()
