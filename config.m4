dnl
dnl $Id$
dnl 

PHP_ARG_WITH(expect,for expect support, 
[  --with-expect[=DIR]        Include expect support (requires libexpect >= 5.43.0).])

if test "$PHP_EXPECT" != "no"; then
  for i in $PHP_EXPECT /usr/local /usr ; do
    if test -f $i/include/expect.h; then
      LIBEXPECT_DIR=$i
      break
    fi
  done

  if test -z "$LIBEXPECT_DIR"; then
    AC_MSG_ERROR(expect extension requires libexpect version >= 5.43.0)
  fi

  PHP_ADD_LIBRARY_WITH_PATH(expect, $LIBEXPECT_DIR/lib, EXPECT_SHARED_LIBADD)
  PHP_ADD_INCLUDE($LIBEXPECT_DIR/include)

  PHP_NEW_EXTENSION(expect, expect.c expect_fopen_wrapper.c, $ext_shared)
  PHP_SUBST(EXPECT_SHARED_LIBADD)
fi
