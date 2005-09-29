dnl
dnl $Id$
dnl 

PHP_ARG_WITH(expect,for expect support, 
[  --with-expect[=DIR]        Include expect support (requires libexpect).])

if test "$PHP_EXPECT" != "no"; then
  for i in $PHP_EXPECT /usr/local /usr ; do
    if test -f $i/include/expect.h; then
      LIBEXPECT_DIR=$i
      LIBEXPECT_INCDIR=$i/include
    fi
  done

  if test -z "$LIBEXPECT_DIR"; then
    AC_MSG_ERROR(Cannot find libexpect)
  fi

  LIBEXPECT_LIBDIR=$LIBEXPECT_DIR/lib

  PHP_CHECK_LIBRARY(expect, exp_popen,
  	[ AC_DEFINE(HAVE_LIBEXPECT, 1, "") ],	
	[ AC_MSG_ERROR(expect module requires libexpect.) ],
	[ -L$LIBEXPECT_LIBDIR ]
  )

  PHP_ADD_LIBRARY_WITH_PATH(expect, $LIBEXPECT_LIBDIR, EXPECT_SHARED_LIBADD)
  PHP_ADD_INCLUDE($LIBEXPECT_INCDIR)

  PHP_SUBST(EXPECT_SHARED_LIBADD)
  PHP_NEW_EXTENSION(expect, expect.c expect_fopen_wrapper.c, $ext_shared)

fi
