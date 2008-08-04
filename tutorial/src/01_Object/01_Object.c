/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file 01_Object.c
 * @date 04/08/2008
 * @author iarwain@orx-project.org
 *
 * Object creation tutorial
 */


#include "orx.h"


/** Inits the tutorial
 */
orxSTATIC orxSTATUS Init()
{
  /* Loads config file and selects its section */
  orxConfig_Load("../../01_Object/01_Object.ini");

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates object */
  orxObject_CreateFromConfig("Object");

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);
