# ======================================================================================
# Author: Emilien Kia
# RCS-ID: $Id$
#
#    http://www.gnu.org/software/autoconf/manual/autoconf-2.57/html_mono/autoconf.html
# for more info about autoconf and the autoconf macros.
# ======================================================================================


# (DON'T PUT ANYTHING BEFORE AC_INIT, JUST COMMENTS)
AC_INIT([orx], [0.1.0], [cursorstar@users.sourceforge.net])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_CC
# AC_PROG_CXX
# AC_PROG_CXXCPP

# Checks for std lib
AC_CHECK_LIB(m, cos)

#
# Options :
#

# Check if debug is enable
AC_ARG_WITH(debug,              [  --with-debug            compiles Orx with debuging options])
if test "$with_debug" = "yes" ; then
  BUILD="debug"
else
  BUILD="release"
fi
AC_SUBST(BUILD)

# Define the compilation mode as dynamic
AC_ARG_WITH(static,              [  --with-static           compiles Orx in static mode])
if test "$with_static" = "yes" ; then
  LINK="static"
else
  LINK="dynamic"
fi
AC_SUBST(LINK)


#
# Special mecanisms
#

# Check for Test
AC_ARG_ENABLE([test],
	AC_HELP_STRING([--enable-test],[enable tests]),
	[case "${enableval}" in
	  yes) with_test="yes" ;;
	  no)  with_test="no" ;;
	  *) AC_MSG_ERROR(bad value ${enableval} for --enable-test) ;;
	esac],
	[with_test="no"])
if test "$with_test" = "yes" ; then
  USE_TEST="1"
else
  USE_TEST="0"
fi


# Check for Plugins
AC_ARG_ENABLE([plugins],
	AC_HELP_STRING([--disable-plugins],[disable plugins]),
	[case "${enableval}" in
	  yes) with_plugins="yes" ;;
	  no)  with_plugins="no" ;;
	  *) AC_MSG_ERROR(bad value ${enableval} for --disable-plugins) ;;
	esac],
	[with_plugins="yes"])
if test "$with_plugins" = "yes" ; then
  USE_PLUGINS="1"
else
  USE_PLUGINS="0"
fi


#
# Plugins (requires plugins)
#
if test "$with_plugins" = "yes" ; then

# Check for SDL
PKG_CHECK_MODULES(LIBSDL, [sdl], 
            [ HAVE_SDL="yes" ],
            [ HAVE_SDL="no" ] )

AC_ARG_WITH(sdl, 
	AC_HELP_STRING([--without-sdl],[Disable SDL plugin]),
	[case "${withval}" in
	  yes) with_sdl="yes" ;;
	  no)  with_sdl="no" ;;
	  *)   with_sdl="yes" ;;
	esac],[with_sdl="yes"])
if test "$with_sdl" = "yes" -a "$HAVE_SDL" = "yes" ; then
  USE_SDL="1"
else
  USE_SDL="0"
fi
AC_SUBST(USE_SDL)


# Check for Allegro
if allegro-config --version ; then
 	HAVE_ALLEGRO="yes"
else
	HAVE_ALLEGRO="no"
fi

AC_ARG_WITH(allegro, 
	AC_HELP_STRING([--without-allegro],[Disable SDL plugin]),
	[case "${withval}" in
	  yes) with_allegro="yes" ;;
	  no)  with_allegro="no" ;;
	  *)   with_allegro="yes" ;;
	esac],[with_allegro="yes"])
if test "$with_allegro" = "yes" -a "$HAVE_ALLEGRO" = "yes" ; then
  USE_ALLEGRO="1"
else
  USE_ALLEGRO="0"
fi
AC_SUBST(USE_ALLEGRO)


# Check for FMod
AC_CHECK_LIB(fmod, [FSOUND_Init FSOUND_IsPlaying], [HAVE_FMOD="yes"], [HAVE_FMOD="no"])
if test "$HAVE_FMOD"="yes" ; then
  AC_CHECK_HEADER([fmod.h], [HAVE_FMOD="yes"], [HAVE_FMOD="no"])
fi

AC_ARG_WITH(fmod, 
	AC_HELP_STRING([--without-fmod],[Disable FMod plugin]),
	[case "${withval}" in
	  yes) with_fmod="yes" ;;
	  no)  with_fmod="no" ;;
	  *)   with_fmod="yes" ;;
	esac],[with_fmod="yes"])
if test "$with_fmod" = "yes" -a "$HAVE_FMOD" = "yes" ; then
  USE_FMOD="1"
else
  USE_FMOD="0"
fi
AC_SUBST(USE_FMOD)


fi
#
# End of plugins
#



# Configure requirements for bakefiles
AC_BAKEFILE([m4_include(autoconf_inc.m4)])

# Write makefiles.
AC_CONFIG_FILES([
         Makefile
         ])
AC_OUTPUT

echo "Compile tests        " $USE_TEST
echo "Compile plugins      " $USE_PLUGINS
if test "$USE_PLUGINS" = "1" ; then
	echo "  - SDL              " $USE_SDL
	echo "  - ALLEGRO          " $USE_ALLEGRO
	echo "  - FMOD             " $USE_FMOD
fi

