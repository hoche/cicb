dnl File:        aclocal.m4
dnl Revision:    1.0
dnl Created:     00/09/29
dnl Author:      Michel Hoche-Mong (hoche@grok.com)
dnl $Id: aclocal.m4,v 1.24 2009/04/04 09:08:40 hoche Exp $

dnl
dnl I got the idea for the readline- and tcl-finding scripts from the ones 
dnl originally written by Rob Savoye <rob@cygnus.com> for gdb. I have, 
dnl however, modified them extensively. -Michel Hoche-Mong <hoche@grok.com>
dnl



dnl
dnl Define a substition variable ($1) to be a reasonable option to
dnl link the binary static. if $1 is set, it will just leave it alone.
dnl
AC_DEFUN(ICB_AC_SET_STATIC,
[
	if test x"$$1" != x ; then
		if test x"$CC" = x"gcc"; then
			$1="-static"
		elif test x"$UNAME" != x; then
			os_system=`$UNAME -s`
			if test x"$os_system" = x"IRIX"; then
				$1="-non_shared"
			else
				$1="-Bstatic"
			fi
		else
			$1="-Bstatic"
		fi
	fi
])


dnl
dnl ICB_AC_PROMPT_USER_NO_CACHE(VARIABLE,PROMPT,[DEFAULT])
dnl
dnl
AC_DEFUN(ICB_AC_PROMPT_USER_NO_CACHE,
[
	if test "x$defaults" = "xno"; then
		echo $ac_n "$2 ($3): $ac_c"
		read tmpinput
		if test "$tmpinput" = "" -a "$3" != ""; then
		  tmpinput="$3"
		fi
		eval $1=\"$tmpinput\"
		else
		tmpinput="$3"
		eval $1=\"$tmpinput\"
	fi
])


dnl
dnl ICB_AC_PROMPT_USER(VARIABLE,PROMPT,[DEFAULT])
dnl
dnl
AC_DEFUN(ICB_AC_PROMPT_USER,
[
	MSG_CHECK=`echo "$2" | tail -1`
	AC_CACHE_CHECK($MSG_CHECK, ac_cv_user_prompt_$1,
	[
		echo ""
		ICB_AC_PROMPT_USER_NO_CACHE($1,[$2],$3)
		eval ac_cv_user_prompt_$1=\$$1
		echo $ac_n "setting $MSG_CHECK to...  $ac_c"
	])
])




dnl
dnl Readline Stuff
dnl

dnl ICB_AC_PATH_READLINEH finds the readline headers and does three things:
dnl      1) it sets CPPFLAGS to include the directory with the readline headers.
dnl      2) it checks to see if there's a history.h file available in the
dnl         same directory as readline.h, and sets HAVE_READLINE_HISTORY_H
dnl         if so.
dnl      3) it sets HAVE_READLINE_2 if the readline 2 API is detected. (After 
dnl         readline 2, several functions were added, and an extra parameter 
dnl         was added to many of the calls).
dnl
AH_TEMPLATE([HAVE_READLINE_HISTORY_H], [Defined if readline/history.h is available.])
AC_DEFUN(ICB_AC_PATH_READLINEH, [

  #
  # Ok, lets find the readline include directory.
  # (The alternative search directory is invoked by --with-readlineinclude)
  #
  no_readline=true
  AC_ARG_WITH(readlineinclude, [  --with-readlineinclude  directory containing the readline headers], 
		with_readlineinclude=${withval})

  AC_MSG_CHECKING(for readline headers)
  AC_CACHE_VAL(icb_ac_cv_readline_h_dir,[

	# first check to see if --with-readlineinclude was specified
	if test x"${with_readlineinclude}" != x ; then
	  if test -f ${with_readlineinclude}/readline/readline.h ; then
		icb_ac_cv_readline_h_dir=`(cd ${with_readlineinclude}; pwd)`
	  else
		AC_MSG_ERROR([no headers found in ${with_readlineinclude} directory])
	  fi
	fi

	# next check in a few common install locations
	#
	if test x"${prefix}" = xNONE ; then
		 rlprefix=.
	else
		 rlprefix=${prefix}
	fi

	if test x"${icb_ac_cv_readline_h_dir}" = x ; then
	  for i in ${rlprefix}/include \
			  /sw/include \
			  /usr/pkg/include \
			  /usr/local/include \
			  /usr/include \
			  ; do
		if test -f $i/readline/readline.h ; then
		  icb_ac_cv_readline_h_dir=`(cd $i; pwd)`
		  break
		fi
	  done
	fi
  ])


  dnl ok, we've either found it, or we're hosed.
  dnl
  READLINEHDIR=
  if test x"${icb_ac_cv_readline_h_dir}" = x ; then
	AC_MSG_ERROR([no readline headers found])
  elif test x"${icb_ac_cv_readline_h_dir}" = x"/usr/include" ; then
	no_readline=""
	AC_MSG_RESULT([found in the normal location: ${icb_ac_cv_readline_h_dir}])
  else
	no_readline=""
	# this hack is cause the READLINEHDIR won't print if there is a "-I" in it.
	READLINEHDIR="-I${icb_ac_cv_readline_h_dir}"
	AC_MSG_RESULT([found in ${icb_ac_cv_readline_h_dir}])
  fi


  dnl Next, check to see if we have readline/history.h...
  dnl
  AC_MSG_CHECKING(for readline/history.h)
  AC_CACHE_VAL(icb_ac_cv_have_readline_history_h, [
	  if test -f "${icb_ac_cv_readline_h_dir}/readline/history.h" ; then
		icb_ac_cv_have_readline_history_h="yes"
	  else
		icb_ac_cv_have_readline_history_h="no"

      fi
  ])

  if test x"${icb_ac_cv_have_readline_history_h}" = "xyes" ; then
	  AC_MSG_RESULT([yes])
	  AC_DEFINE(HAVE_READLINE_HISTORY_H)
  else
	  AC_MSG_RESULT([no])
  fi


  AC_PROVIDE([$0])
  AC_SUBST(READLINEHDIR)
])



dnl ICB_AD_PATH_READLINELIB finds the readline lib and sets READLINELIB.
dnl
AH_TEMPLATE([HAVE_READLINE_2], [Defined if Readline 2 or earlier is detected.])
AC_DEFUN(ICB_AC_PATH_READLINELIB, [
  #
  # Ok, lets find the readline library
  # the alternative search directory is invoked by --with-readlinelib
  #

  # unfortunately, i can't think of a good way to detect which readline library
  # to link to if there's more than one. rl_library_version isn't part of the
  # includes -- it's only available as a variable in the library itself.

dnl "readlineroot" means what version of readline we have.. generally 
dnl pointless, but it might come in handy later.  an accidental feature.

  readlineroot=readline

  if test x"${no_readline}" = x ; then
	# we reset no_readline incase something fails here
	no_readline=true
	AC_ARG_WITH(readlinelib, [  --with-readlinelib      directory containing readline library],
		   with_readlinelib=${withval})

	AC_MSG_CHECKING([for readline library])
	AC_CACHE_VAL(icb_ac_cv_readlinelib,[

		# First check to see if --with-readlinelib was specified.
		if test x"${with_readlinelib}" != x ; then
		  if test -f "${with_readlinelib}/lib$readlineroot.so" -a \
				-f "${with_readlinelib}/lib$readlineroot.a" ; then
			icb_ac_cv_readlinelib="-L`(cd ${with_readlinelib}; pwd)` -l$readlineroot"
		  elif test -f "${with_readlinelib}/lib$readlineroot.so" ; then
			icb_ac_cv_readlinelib=`(cd ${with_readlinelib}; pwd)`/lib$readlineroot.so
		  elif test -f "${with_readlinelib}/lib$readlineroot.a" ; then
			icb_ac_cv_readlinelib=`(cd ${with_readlinelib}; pwd)`/lib$readlineroot.a
		  else
			AC_MSG_ERROR([No libraries found in ${with_readlinelib} directory])
		  fi
		fi

		# check in a few common install locations
		if test x"${prefix}" = xNONE ; then
             rlprefix=.
		else
			 rlprefix=${prefix}
        fi
		if test x"${icb_ac_cv_readlinelib}" = x ; then
		  for i in ${rlprefix}/lib \
				  /sw/lib \
				  /usr/pkg/lib \
				  /usr/local/lib \
				  /usr/lib \
				  ; do
			  for j in ${i} \
					   `ls -dr ${i}/readline[[0-9]]* 2>/dev/null`  \
					   ${i}/readline \
					   `ls -dr ${i}/readline/readline[[0-9]]* 2>/dev/null`  \
					  ; do
				if test -f "$j/lib$readlineroot.so" -a \
					-f "$j/lib$readlineroot.a" ; then
				  icb_ac_cv_readlinelib="-L`(cd $j; pwd)` -l$readlineroot"
				  break;
				elif test -f "$j/lib$readlineroot.so" ; then
				  icb_ac_cv_readlinelib=`(cd $j; pwd)`/lib$readlineroot.so
				  break;
				elif test -f "$j/lib$readlineroot.a" ; then
				  icb_ac_cv_readlinelib=`(cd $j; pwd)`/lib$readlineroot.a
				  break;
				fi
			  done
			  if test x"${icb_ac_cv_readlinelib}" != x ; then
				  break;
			  fi
		  done
		fi

	])

	#
	# ok, done searching. did we find it?
	if test x"${icb_ac_cv_readlinelib}" = x ; then
	  READLINELIB="# no readline library found"
	  AC_MSG_WARN(no readline library found)
	else
	  READLINELIB=${icb_ac_cv_readlinelib}
	  AC_MSG_RESULT(found $READLINELIB)
	  no_readline=
	fi
  fi


  dnl Now, we're going to try a compile, and we don't want to add the 
  dnl readline header directory to our CFLAGS ('cause the top level config
  dnl will do that later - we may need to do some other test compiles in some
  dnl other subroutines, so we want to keep CFLAGS as simple as possible until
  dnl the very end of the configure run), so save it off before we add 
  dnl READLINEHDIR to it. ditto with LDFLAGS.

  old_cflags=${CFLAGS}
  CFLAGS="$CFLAGS $READLINEHDIR"
  old_libs=${LIBS}
  LIBS="$READLINELIB $LIBS"

  dnl check for readline 2.2 by trying to compile a bit of test code 
  dnl with a function that we know wasn't added until later versions.
  dnl if it fails, we try to link with a function we *know* exists,
  dnl just to make sure we don't have other link problems.
  dnl
  AC_MSG_CHECKING(for old readline 2.x)
  AC_CACHE_VAL(icb_ac_cv_readline2,[
    dnl first try a 4.x function
    AC_TRY_LINK([
      #include <stdio.h>
      #include <readline/readline.h>
    ], [
        rl_resize_terminal();
    ], [
      dnl success. we must have readline 4.x or later. therefore we don't
      dnl have readline 2.
      icb_ac_cv_readline2="no",
    ], [
      dnl failure. we might have readline 2.x, or we might have other link
      dnl problems. try to link with something that we know always is there
      dnl to make sure that it's not the latter.
      AC_TRY_LINK([
        #include <stdio.h>
        #include <readline/readline.h>
      ], [
        readline("");
      ], [
         dnl success. we don't have link problems, therefore we might
         dnl have readline 2.x., but lets make sure.

         AC_TRY_LINK([
           #include <stdio.h>
           #include <readline/readline.h>
         ], [
           rl_getc_function();
         ], [
	   dnl okay, it really is readline 2.x
           icb_ac_cv_readline2="yes"
         ], [
           AC_MSG_ERROR(Holy Cow! That looks like readline 1.0!)
         ])
      ], [ 
         dnl failure. we have problems linking to libreadline. bomb out.
         AC_MSG_ERROR(Can't link to $READLINELIB!) 
      ])
    ])
  ])

  if test x"${icb_ac_cv_readline2}" = "xyes" ; then
    AC_MSG_RESULT([readline 2.x found. Yuck!])
    echo "**************************************************************"
    echo "Warning: An older version of readline has been detected. This"
    echo "isn't a critical error, but you may see some bizarre behavior"
    echo "at runtime. We strongly recommend that you upgrade to readline"
    echo "4.0 or better."
    echo "**************************************************************"
    AC_DEFINE(HAVE_READLINE_2)
  else
    AC_MSG_RESULT([no. you have a later version. good.])
  fi

  dnl and reset the CFLAGS and LDFLAGS back to what they were.
  CFLAGS=${old_cflags}
  LIBS=${old_libs}

  AC_PROVIDE([$0])
  AC_SUBST(READLINELIB)
])


dnl This macro just calls the following two.
AC_DEFUN(ICB_AC_PATH_READLINE, [
  ICB_AC_PATH_READLINEH
  ICB_AC_PATH_READLINELIB
])


dnl
dnl TCL Stuff
dnl


dnl ICB_AC_PATH_TCLH finds the Tcl headers and sets CPPFLAGS to include the 
dnl proper directory.
dnl
AC_DEFUN(ICB_AC_PATH_TCLH, [
  #
  # Ok, lets find the tcl include trees so we can use the headers
  # Warning: transition of version 9 to 10 will break this algorithm
  # because 10 sorts before 9. We also look for just tcl. We have to
  # be careful that we don't match stuff like tclX by accident.
  # the alternative search directory is invoked by --with-tclinclude
  #
  no_tcl=true
  AC_ARG_WITH(tclinclude, [  --with-tclinclude       directory containing tcl headers], 
		with_tclinclude=${withval})

  AC_MSG_CHECKING(for Tcl headers)
  AC_CACHE_VAL(icb_ac_cv_tclh,[

	# first check to see if --with-tclinclude was specified
	if test x"${with_tclinclude}" != x ; then
	  if test -f ${with_tclinclude}/tcl.h ; then
		icb_ac_cv_tclh=`(cd ${with_tclinclude}; pwd)`
	  else
		AC_MSG_ERROR([no headers found in ${with_tclinclude} directory])
	  fi
	fi

	# next check in a few common install locations
	#
	if test x"${prefix}" = xNONE ; then
		 tclprefix=.
	else
		 tclprefix=${prefix}
	fi
	if test x"${icb_ac_cv_tclh}" = x ; then
	  for i in ${tclprefix}/include \
			  /usr/pkg/include \
			  /usr/local/include \
			  /usr/include \
			  ; do
		  for j in ${i} \
				   `ls -dr ${i}/tcl[[0-9]]* 2>/dev/null`  \
				   ${i}/tcl \
				   `ls -dr ${i}/tcl/tcl[[0-9]]* 2>/dev/null`  \
				  ; do
			if test -f $j/tcl.h ; then
			  icb_ac_cv_tclh=`(cd $j; pwd)`
			  break
			fi
		done
		if test x"${icb_ac_cv_tclh}" != x ; then
			break;
		fi
	  done
	fi
  ])

  if test x"${icb_ac_cv_tclh}" = x ; then
	TCLHDIR="# no Tcl headers found"
	AC_MSG_ERROR([no Tcl headers found])
  fi

  if test x"${icb_ac_cv_tclh}" != x ; then
	no_tcl=""
	if test x"${icb_ac_cv_tclh}" = x"/usr/include" ; then
		AC_MSG_RESULT([found in the normal location: ${icb_ac_cv_tclh}])
	else
		AC_MSG_RESULT([found in ${icb_ac_cv_tclh}])
		# this hack is cause the TCLHDIR won't print if
		# there is a "-I" in it.
		TCLHDIR="-I${icb_ac_cv_tclh}"
	fi
  fi

  if test x"${TCLHDIR}" != x ; then
	CPPFLAGS="$CPPFLAGS $TCLHDIR"
  fi

  AC_MSG_CHECKING([Tcl version])
  orig_includes="$CPPFLAGS"

  # Get major and minor versions of Tcl.  Use funny names to avoid
  # clashes with eg SunOS.
  cat > conftest.c <<'EOF'
#include "tcl.h"
MaJor = TCL_MAJOR_VERSION
MiNor = TCL_MINOR_VERSION
EOF

  tclmajor=
  tclminor=
  if (eval "$CPP $CPPFLAGS conftest.c") 2>/dev/null >conftest.out; then
	 # Success.
	 tclmajor=`egrep '^MaJor = ' conftest.out | sed -e 's/^MaJor = *//' -e 's/ *$//'`
	 tclminor=`egrep '^MiNor = ' conftest.out | sed -e 's/^MiNor = *//' -e 's/ *$//'`
  fi
  rm -f conftest.c conftest.out

  if test -z "$tclmajor" || test -z "$tclminor"; then
	 AC_MSG_RESULT([fatal error: could not find major or minor version number of Tcl])
	 exit 1
  fi
  AC_MSG_RESULT(${tclmajor}.${tclminor})

  CPPFLAGS="${orig_includes}"

  AC_PROVIDE([$0])
  AC_SUBST(TCLHDIR)
])



dnl ICB_AD_PATH_TCLLIB finds the Tcl lib and sets TCLLIB.
dnl 
AC_DEFUN(ICB_AC_PATH_TCLLIB, [
  #
  # Ok, lets find the tcl library
  # First, look for one uninstalled.  
  # the alternative search directory is invoked by --with-tcllib
  #

  # tcl versions greater than 7.4 have the library installed with the
  # major/minor numbers as part of the name. Some OS's separate them
  # with a dot, some (notably freebsd) don't. guh.
  if test $tclmajor -gt 7 ; then
	dottedtcllibroot=tcl$tclmajor.$tclminor
	undottedtcllibroot=tcl$tclmajor$tclminor
  elif test	$tclmajor -eq 7 -a $tclminor -ge 4 ; then
	dottedtcllibroot=tcl$tclmajor.$tclminor
	undottedtcllibroot=tcl$tclmajor$tclminor
  else
	dottedtcllibroot=tcl
	undottedtcllibroot=tcl
  fi

  if test x"${no_tcl}" = x ; then
	# we reset no_tcl incase something fails here
	no_tcl=true
	AC_ARG_WITH(tcllib, [  --with-tcllib           directory containing the tcl library],
		   with_tcllib=${withval})

	AC_MSG_CHECKING([for Tcl library])
	AC_CACHE_VAL(icb_ac_cv_tcllib,[

		if test x"${with_tcllib}" != x ; then

		  if test -f "${with_tcllib}/lib$dottedtcllibroot.so" -a \
				-f "${with_tcllib}/lib$dottedtcllibroot.a" ; then
			icb_ac_cv_tcllib="-L`(cd ${with_tcllib}; pwd)` -l$dottedtcllibroot"
		  elif test -f "${with_tcllib}/lib$dottedtcllibroot.so" ; then
			icb_ac_cv_tcllib=`(cd ${with_tcllib}; pwd)`/lib$dottedtcllibroot.so
		  elif test -f "${with_tcllib}/lib$dottedtcllibroot.a" ; then
			icb_ac_cv_tcllib=`(cd ${with_tcllib}; pwd)`/lib$dottedtcllibroot.a

		  elif test -f "${with_tcllib}/lib$undottedtcllibroot.so" -a \
				-f "${with_tcllib}/lib$undottedtcllibroot.a" ; then
			icb_ac_cv_tcllib="-L`(cd ${with_tcllib}; pwd)` -l$undottedtcllibroot"
		  elif test -f "${with_tcllib}/lib$undottedtcllibroot.so" ; then
			icb_ac_cv_tcllib=`(cd ${with_tcllib}; pwd)`/lib$undottedtcllibroot.so
		  elif test -f "${with_tcllib}/lib$undottedtcllibroot.a" ; then
			icb_ac_cv_tcllib=`(cd ${with_tcllib}; pwd)`/lib$undottedtcllibroot.a

		  elif test -f "${with_tcllib}/libtcl.so" -a \
				-f "${with_tcllib}/libtcl.a" ; then
			icb_ac_cv_tcllib="-L`(cd ${with_tcllib}; pwd)` -ltcl"
		  elif test -f "${with_tcllib}/libtcl.so" ; then
			icb_ac_cv_tcllib=`(cd ${with_tcllib}; pwd)`/libtcl.so
		  elif test -f "${with_tcllib}/libtcl.a" ; then
			icb_ac_cv_tcllib=`(cd ${with_tcllib}; pwd)`/libtcl.a

		  else
			AC_MSG_ERROR([No libraries found in ${with_tcllib} directory])
		  fi
		fi

		# check in a few common install locations
		if test x"${prefix}" = xNONE ; then
			 tclprefix=.
		else
			 tclprefix=${prefix}
		fi
		if test x"${icb_ac_cv_tcllib}" = x ; then
		  for i in ${tclprefix}/lib \
				  /usr/pkg/lib \
				  /usr/local/lib \
				  /usr/lib \
				  ; do
			  for j in ${i} \
					   `ls -dr ${i}/tcl[[0-9]]* 2>/dev/null`  \
					   ${i}/tcl \
					   `ls -dr ${i}/tcl/tcl[[0-9]]* 2>/dev/null`  \
					  ; do
				if test -f "$j/lib$dottedtcllibroot.so" -a \
					-f "$j/lib$dottedtcllibroot.a" ; then
				  icb_ac_cv_tcllib="-L`(cd $j; pwd)` -l$dottedtcllibroot"
				  break;
				elif test -f "$j/lib$dottedtcllibroot.so" ; then
				  icb_ac_cv_tcllib=`(cd $j; pwd)`/lib$dottedtcllibroot.so
				  break;
				elif test -f "$j/lib$dottedtcllibroot.a" ; then
				  icb_ac_cv_tcllib=`(cd $j; pwd)`/lib$dottedtcllibroot.a
				  break;

				elif test -f "$j/lib$undottedtcllibroot.so" -a \
					-f "$j/lib$undottedtcllibroot.a" ; then
				  icb_ac_cv_tcllib="-L`(cd $j; pwd)` -l$undottedtcllibroot"
				  break;
				elif test -f "$j/lib$undottedtcllibroot.so" ; then
				  icb_ac_cv_tcllib=`(cd $j; pwd)`/lib$undottedtcllibroot.so
				  break;
				elif test -f "$j/lib$undottedtcllibroot.a" ; then
				  icb_ac_cv_tcllib=`(cd $j; pwd)`/lib$undottedtcllibroot.a
				  break;

				fi
			  done
			  if test x"${icb_ac_cv_tcllib}" != x ; then
				  break;
			  fi
		  done
		fi

		# see if one is conveniently installed with the compiler
		# we only need check for the dotted version here, i think
		if test x"${icb_ac_cv_tcllib}" = x ; then	
		  orig_libs="$LIBS"
		  LIBS="$LIBS -l$dottedtcllibroot -lm"    
		  AC_TRY_RUN([
		  int main(int argc, char** argv) { Tcl_CreateInterp(); return 0; }
		  ], icb_ac_cv_tcllib="-l$dottedtcllibroot", icb_ac_cv_tcllib=""
		  , ac_cv_c_tclib="-l$dottedtcllibroot")
		  LIBS="${orig_libs}"
		fi

	])

	#
	# ok, done searching. did we find it?
	if test x"${icb_ac_cv_tcllib}" = x ; then
	  TCLLIB="# no Tcl library found"
	  AC_MSG_WARN(no Tcl library found)
	else
	  TCLLIB=${icb_ac_cv_tcllib}
	  AC_MSG_RESULT(found $TCLLIB)
	  no_tcl=
	fi
  fi

  AC_PROVIDE([$0])
  AC_SUBST(TCLLIB)
])


AC_DEFUN(ICB_AC_PATH_TCL, [
  ICB_AC_PATH_TCLH
  ICB_AC_PATH_TCLLIB
])



dnl ICB_AC_PATH_SSL_H finds the openssl headers and sets CPPFlags
dnl to include the directory with the headers.
dnl
AC_DEFUN(ICB_AC_PATH_SSL_H, [

  #
  # Ok, lets find the include directory.
  # (The alternative search directory is invoked by --with-ssl-include)
  #
  no_ssl=true
  AC_ARG_WITH(ssl_include, [  --with-ssl-include      directory containing the openssl headers], 
		with_ssl_include=${withval})

  AC_MSG_CHECKING(for openssl headers)
  AC_CACHE_VAL(icb_ac_cv_ssl_h_dir,[

	# first check to see if --with-ssl-include was specified
	if test x"${with_ssl_include}" != x ; then
	  if test -f ${with_ssl_include}/openssl/ssl.h ; then
		icb_ac_cv_ssl_h_dir=`(cd ${with_ssl_include}; pwd)`
	  else
		AC_MSG_ERROR([no headers found in ${with_ssl_include} directory])
	  fi
	fi

	# next check in a few common install locations
	#
	if test x"${prefix}" = xNONE ; then
		 ssl_prefix=.
	else
		 ssl_prefix=${prefix}
	fi

	if test x"${icb_ac_cv_ssl_h_dir}" = x ; then
	  for i in ${ssl_prefix}/include \
			  /usr/pkg/include \
			  /usr/local/ssl/include \
			  /usr/local/include \
			  /usr/include \
			  ; do
		if test -f $i/openssl/ssl.h ; then
		  icb_ac_cv_ssl_h_dir=`(cd $i; pwd)`
		  break
		fi
	  done
	fi
  ])


  dnl ok, we've either found it, or we're hosed.
  dnl
  SSL_H_DIR=
  if test x"${icb_ac_cv_ssl_h_dir}" = x ; then
	AC_MSG_ERROR([no openssl headers found])
  elif test x"${icb_ac_cv_ssl_h_dir}" = x"/usr/include" ; then
	no_ssl=""
	AC_MSG_RESULT([found in the normal location: ${icb_ac_cv_ssl_h_dir}])
  else
	no_ssl=""
	# this hack is cause the SSL_H_DIR won't print if there is a "-I" in it.
	SSL_H_DIR="-I${icb_ac_cv_ssl_h_dir}"
	AC_MSG_RESULT([found in ${icb_ac_cv_ssl_h_dir}])
  fi

  AC_PROVIDE([$0])
  AC_SUBST(SSL_H_DIR)
])



dnl ICB_AD_PATH_SSL_LIB finds the openssl libs and sets SSL_LIB.
dnl
AC_DEFUN(ICB_AC_PATH_SSL_LIB, [
  #
  # the alternative search directory is invoked by --with-ssl-lib
  #

  if test x"${no_ssl}" = x ; then
	# we reset no_ssl incase something fails here
	no_ssl=true
	AC_ARG_WITH(ssl_lib, [  --with-ssl-lib          directory containing the openssl library],
		   with_ssl_lib=${withval})

	AC_MSG_CHECKING([for openssl library])
	AC_CACHE_VAL(icb_ac_cv_ssl_lib,[

		# First check to see if --with-ssl-lib was specified.
		if test x"${with_ssl_lib}" != x ; then
		  if test -f "${with_ssl_lib}/libssl.so" ; then
			icb_ac_cv_ssl_lib="-L`(cd ${with_ssl_lib}; pwd)` -lssl -lcrypto"
		  elif test -f "${with_ssl_lib}/libssl.a" ; then
			icb_ac_cv_ssl_lib="`(cd ${with_ssl_lib}; pwd)`/libssl.a `(cd ${with_ssl_lib}; pwd)`/libcrypto.a"
		  else
			AC_MSG_ERROR([No libraries found in ${with_ssl_lib} directory])
		  fi
		fi

		# check in a few common install locations
		if test x"${prefix}" = xNONE ; then
             ssl_prefix=.
		else
			 ssl_prefix=${prefix}
        fi
		if test x"${icb_ac_cv_ssl_lib}" = x ; then
		  for i in ${ssl_prefix}/lib \
				  /usr/pkg/lib \
				  /usr/local/ssl/lib \
				  /usr/local/lib \
				  /usr/lib \
				  ; do
				if test -f "$i/libssl.so" ; then
				  icb_ac_cv_ssl_lib="-L`(cd $i; pwd)` -lssl -lcrypto"
				  break;
				elif test -f "$i/libssl.a" ; then
				  icb_ac_cv_ssl_lib="`(cd $i; pwd)`/libssl.a `(cd $i; pwd)`/libcrypto.a"
				  break;
				fi
			  if test x"${icb_ac_cv_ssl_lib}" != x ; then
				  break;
			  fi
		  done
		fi

	])

	#
	# ok, done searching. did we find it?
	if test x"${icb_ac_cv_ssl_lib}" = x ; then
	  SSL_LIB="# no openssl libraries found"
	  AC_MSG_WARN(no openssl libraries found)
	else
	  SSL_LIB=${icb_ac_cv_ssl_lib}
	  AC_MSG_RESULT(found $SSL_LIB)
	  no_ssl=
	fi
  fi

  AC_PROVIDE([$0])
  AC_SUBST(SSL_LIB)
])


dnl main entry point
AC_DEFUN(ICB_AC_PATH_SSL, [
  ICB_AC_PATH_SSL_H
  ICB_AC_PATH_SSL_LIB
])


