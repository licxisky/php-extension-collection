/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_COLLECTION_H
#define PHP_COLLECTION_H

extern zend_module_entry collection_module_entry;
#define phpext_collection_ptr &collection_module_entry

#define PHP_COLLECTION_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_COLLECTION_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_COLLECTION_API __attribute__ ((visibility("default")))
#else
#	define PHP_COLLECTION_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(collection)
	zend_long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(collection)
*/

/* Always refer to the globals in your function as COLLECTION_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define COLLECTION_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(collection, v)

#if defined(ZTS) && defined(COMPILE_DL_COLLECTION)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_COLLECTION_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

#define COLLECTION_RETURN_THIS() \
        RETURN_ZVAL(getThis(), 1, 0)

#define COLLECTION_RETURN_NEW(data) do { \
        zval function_name;         \
        zval args[1];               \
        zval retval;                \
        object_init_ex(return_value, collection_ce);  \
        ZVAL_STRING(&function_name, "__construct");   \
        args[0] = *data;            \
        call_user_function_ex(EG(function_table), return_value, &function_name, &retval, 1, args, 0, NULL); \
        } while (0) 


#define COLLECTION_GET_PROPERTY(name, len) \
        zend_read_property(collection_ce, getThis(), name, len, 0 TSRMLS_DC, NULL)

#define COLLECTION_SET_PROPERTY(name, len, data) \
        zend_update_property(collection_ce, getThis(), name, len, data TSRMLS_CC)

#define COLLECTION_GET_PROPERTY_DATA() \
        COLLECTION_GET_PROPERTY("data", sizeof("data") - 1)

#define COLLECTION_SET_PROPERTY_DATA(data) \
        COLLECTION_SET_PROPERTY("data", sizeof("data") - 1, data)

#define RETURN_NULL_OR_DELAULT(def) \
        if(def == NULL) { RETURN_NULL(); } \
        else { RETURN_ZVAL(def, 0, 0); } 

#define STRING_SPLIT_WHILE_START(key, sk, tl, kl, ex) do { \
    sk = strtok(Z_STRVAL_P(key), ex);                 \
    kl = Z_STRLEN_P(key);                             \
    tl = 0;                                           \
    while(sk != NULL) {          \
      tl += (strlen(sk) + 1);   

#define STRING_SPLIT_WHILE_END(sk, ex) \
      sk = strtok(NULL, ex); \
    } \
  } while (0)

/**
 * 成员方法
 */
PHP_METHOD(collection, public_method);
PHP_METHOD(collection, __construct);
PHP_METHOD(collection, get);

PHP_METHOD(collection, count);

PHP_METHOD(collection, offsetSet);
PHP_METHOD(collection, offsetGet);
PHP_METHOD(collection, offsetUnset);
PHP_METHOD(collection, offsetExists);

PHP_METHOD(collection, current);
PHP_METHOD(collection, key);
PHP_METHOD(collection, next);
PHP_METHOD(collection, rewind);
PHP_METHOD(collection, valid);

int check_dot_string(char dot[]);

void group_by_key(HashTable *target, zval *key, HashTable *source);

void group_by_array(HashTable *target, HashTable *keys);