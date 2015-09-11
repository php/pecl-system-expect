dnl
dnl $Id$
dnl 

PHP_ARG_WITH(expect,for expect support, 
[  --with-expect[=DIR]        Include expect support (requires libexpect >= 5.43.0).])

PHP_ARG_WITH(tcldir,specify path to Tcl needed by expect, 
[  --with-tcldir[=DIR]        Specify path to Tcl config script (tclConfig.sh).])

if test "$PHP_EXPECT" != "no"; then

  for i in $PHP_EXPECT/include/expect.h $PHP_EXPECT/include/*/expect.h \
        /usr/local/include/expect.h /usr/local/include/*/expect.h \
        /usr/include/expect.h /usr/include/*/expect.h ;
  do
    if test -f $i; then
      LIBEXPECT_INCLUDE_DIR=`dirname $i`
      LIBEXPECT_DIR=`dirname $LIBEXPECT_INCLUDE_DIR`
      break
    fi
  done

  AC_MSG_CHECKING(for tcl version)
  for i in $PHP_TCLDIR/tclConfig.sh /usr/lib/tcl*/tclConfig.sh \
        /usr/$PHP_LIBDIR/tcl*/tclConfig.sh \
        /usr/local/lib/tcl*/tclConfig.sh \
	/System/Library/Frameworks/Tcl.framework/Versions/Current/tclConfig.sh;
  do
    if test -f $i; then
      . $i
      break
    fi
  done
  if test -n "$TCL_VERSION" ; then
    AC_MSG_RESULT($TCL_VERSION in $TCL_PREFIX)
  else
    AC_MSG_ERROR([not found])
  fi

  PHP_ADD_LIBRARY_WITH_PATH(tcl$TCL_VERSION, $TCL_PREFIX/$PHP_LIBDIR, EXPECT_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(expect, $LIBEXPECT_DIR/$PHP_LIBDIR, EXPECT_SHARED_LIBADD)
  PHP_ADD_INCLUDE($LIBEXPECT_INCLUDE_DIR)

  PHP_NEW_EXTENSION(expect, expect.c expect_fopen_wrapper.c, $ext_shared)
  PHP_SUBST(EXPECT_SHARED_LIBADD)
fi

