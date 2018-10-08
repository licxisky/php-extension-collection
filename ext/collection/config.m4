dnl $Id$
dnl config.m4 for extension collection

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(collection, for collection support,
Make sure that the comment is aligned:
[  --with-collection             Include collection support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(collection, whether to enable collection support,
dnl Make sure that the comment is aligned:
dnl [  --enable-collection           Enable collection support])

if test "$PHP_COLLECTION" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-collection -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/collection.h"  # you most likely want to change this
  dnl if test -r $PHP_COLLECTION/$SEARCH_FOR; then # path given as parameter
  dnl   COLLECTION_DIR=$PHP_COLLECTION
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for collection files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       COLLECTION_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$COLLECTION_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the collection distribution])
  dnl fi

  dnl # --with-collection -> add include path
  dnl PHP_ADD_INCLUDE($COLLECTION_DIR/include)

  dnl # --with-collection -> check for lib and symbol presence
  dnl LIBNAME=collection # you may want to change this
  dnl LIBSYMBOL=collection # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $COLLECTION_DIR/$PHP_LIBDIR, COLLECTION_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_COLLECTIONLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong collection lib version or lib not found])
  dnl ],[
  dnl   -L$COLLECTION_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(COLLECTION_SHARED_LIBADD)

  PHP_NEW_EXTENSION(collection, collection.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
