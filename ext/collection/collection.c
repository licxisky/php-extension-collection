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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "php_collection.h"

#include "zend_interfaces.h"
#include <string.h>

/* If you declare any globals in php_collection.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(collection)
*/

/* True global resources - no need for thread safety here */
static int le_collection;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("collection.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_collection_globals, collection_globals)
	STD_PHP_INI_ENTRY("collection.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_collection_globals, collection_globals)
PHP_INI_END()
*/
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_collection_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_collection_init_globals(zend_collection_globals *collection_globals)
{
	collection_globals->global_value = 0;
	collection_globals->global_string = NULL;
}
*/
/* }}} */

// 定义一个类
zend_class_entry *collection_ce;

// 参数绑定
ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetSet, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetGet, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetUnset, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetExists, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

// 方法定义
ZEND_METHOD(collection, make)
{
	zval *data;

    array_init(data);

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ARRAY(data)
	ZEND_PARSE_PARAMETERS_END();

    COLLECTION_RETURN_NEW(data);
}

ZEND_METHOD(collection, __construct)
{
	zval *data;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ARRAY(data)
	ZEND_PARSE_PARAMETERS_END();

	COLLECTION_SET_PROPERTY_DATA(data);
}

ZEND_METHOD(collection, get)
{
	zval *data;
	zval *key = NULL;
	zval *def = NULL;
	zend_bool dot = 1;
	zval *entry;
	char * sk;

	ZEND_PARSE_PARAMETERS_START(0, 3)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(key)
		Z_PARAM_ZVAL(def)
		Z_PARAM_BOOL(dot)
	ZEND_PARSE_PARAMETERS_END();

	data = COLLECTION_GET_PROPERTY_DATA();

	if(key == NULL) {
		ZVAL_DEREF(data);
		ZVAL_COPY(return_value, data);
		return;
	}

	if(dot && Z_TYPE_P(key) == IS_STRING) {

		entry = data;

		int tl, kl;

		STRING_SPLIT_WHILE_START(key, sk, tl, kl, ".") {
			if(Z_TYPE_P(entry) != IS_ARRAY || (entry = zend_symtable_find(Z_ARRVAL_P(entry), zend_string_init(sk, strlen(sk), 0))) == NULL) {
				RETURN_NULL_OR_DELAULT(def);
			}
		} STRING_SPLIT_WHILE_END(sk, ".");

		if(kl + 1 == tl) {
			ZVAL_DEREF(entry);
			ZVAL_COPY(return_value, entry);
		} else {
			RETURN_NULL_OR_DELAULT(def);
		}	
	} else {
		if((entry = zend_symtable_find(Z_ARRVAL_P(data), zval_get_string(key))) != NULL) {	
			ZVAL_DEREF(entry);
			ZVAL_COPY(return_value, entry);
		} else {
			RETURN_NULL_OR_DELAULT(def);
		}
	}
}

ZEND_METHOD(collection, groupBy)
{
	zval *data;
	zval *group_by;
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
	zend_bool preserve_key = 0;

	zval function_name;
	zval args[1];
    zval retval;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_ZVAL(group_by)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC(fci, fci_cache)
		Z_PARAM_BOOL(preserve_key)
	ZEND_PARSE_PARAMETERS_END();

	data = COLLECTION_GET_PROPERTY_DATA();

	zval result;

	array_init(&result);

  	zend_hash_copy(Z_ARRVAL(result), Z_ARRVAL_P(data), NULL);

	switch(Z_TYPE_P(group_by)) {
		case IS_LONG:
		case IS_STRING:
		case IS_DOUBLE:
			group_by_key(Z_ARRVAL(result), group_by, Z_ARRVAL_P(data));
			break;

	}

    object_init_ex(return_value, collection_ce);

	ZVAL_STRING(&function_name, "__construct");

	args[0] = result;

	call_user_function_ex(EG(function_table), return_value, &function_name, &retval, 1, args, 0, NULL);
}

ZEND_METHOD(collection, count)
{
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();

	RETURN_LONG(zend_hash_num_elements(Z_ARRVAL_P(data)));
}

ZEND_METHOD(collection, offsetSet)
{
	zval *offset;
	zval *value;
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_ZVAL(offset)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	// $c[] = 1;
	if(Z_TYPE_P(offset) == IS_NULL) {
		zend_hash_next_index_insert(Z_ARRVAL_P(data), value);
		return;
	}

	// $c['k'] => 1;
	zend_symtable_update(Z_ARRVAL_P(data), zval_get_string(offset), value);
}

ZEND_METHOD(collection, offsetGet)
{
	zval *offset;
	zval *data;
	zval *entry = NULL;

	data = COLLECTION_GET_PROPERTY_DATA();
	
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(offset)
	ZEND_PARSE_PARAMETERS_END();

	if((entry = zend_symtable_find(Z_ARRVAL_P(data), zval_get_string(offset))) == NULL) {
		zend_error(E_NOTICE, "Undefined index: %s", ZSTR_VAL(zval_get_string(offset)));
		return;
	}

	ZVAL_DEREF(entry);
	ZVAL_COPY(return_value, entry);
}

ZEND_METHOD(collection, offsetUnset)
{
	zval *offset;
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(offset)
	ZEND_PARSE_PARAMETERS_END();

	zend_symtable_del(Z_ARRVAL_P(data), zval_get_string(offset));
}

ZEND_METHOD(collection, offsetExists)
{
	zval *offset;
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(offset)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL(zend_symtable_exists(Z_ARRVAL_P(data), zval_get_string(offset)));
}

ZEND_METHOD(collection, current)
{
	zval *data;
	zval *entry;

	data = COLLECTION_GET_PROPERTY_DATA();

	if ((entry = zend_hash_get_current_data(Z_ARRVAL_P(data))) == NULL) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(entry) == IS_INDIRECT) {
		entry = Z_INDIRECT_P(entry);
	}

	ZVAL_DEREF(entry);
	ZVAL_COPY(return_value, entry);
}

ZEND_METHOD(collection, key)
{
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();

	zend_hash_get_current_key_zval(Z_ARRVAL_P(data), return_value);
}

ZEND_METHOD(collection, next)
{
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();

	zend_hash_move_forward(Z_ARRVAL_P(data));
}

ZEND_METHOD(collection, rewind)
{
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
}

ZEND_METHOD(collection, valid)
{
	zval *data;

	data = COLLECTION_GET_PROPERTY_DATA();
	
	if (zend_hash_get_current_data(Z_ARRVAL_P(data)) == NULL) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

ZEND_METHOD(collection, changeKeyCase)
{
	zval *array, *entry;
	zend_string *string_key;
	zend_string *new_key;
	zend_ulong num_key;
	zend_long change_to_upper = 0;
	zval result;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(change_to_upper)
	ZEND_PARSE_PARAMETERS_END();

	array = COLLECTION_GET_PROPERTY_DATA();

	array_init_size(&result, zend_hash_num_elements(Z_ARRVAL_P(array)));

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(array), num_key, string_key, entry) {
		if (!string_key) {
			entry = zend_hash_index_update(Z_ARRVAL(result), num_key, entry);
		} else {
			if (change_to_upper) {
				new_key = php_string_toupper(string_key);
			} else {
				new_key = php_string_tolower(string_key);
			}
			entry = zend_hash_update(Z_ARRVAL(result), new_key, entry);
			zend_string_release(new_key);
		}

		zval_add_ref(entry);
	} ZEND_HASH_FOREACH_END();

	COLLECTION_RETURN_NEW(&result);
}

ZEND_METHOD(collection, chunk)
{
	int num_in;
	zend_long size, current = 0;
	zend_string *str_key;
	zend_ulong num_key;
	zend_bool preserve_keys = 0;
	zval *input = NULL;
	zval chunk;
	zval *entry;
	zval result;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_LONG(size)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(preserve_keys)
	ZEND_PARSE_PARAMETERS_END();

	input = COLLECTION_GET_PROPERTY_DATA();

	/* Do bounds checking for size parameter. */
	if (size < 1) {
		php_error_docref(NULL, E_WARNING, "Size parameter expected to be greater than 0");
		return;
	}

	num_in = zend_hash_num_elements(Z_ARRVAL_P(input));

	if (size > num_in) {
		size = num_in > 0 ? num_in : 1;
	}

	array_init_size(&result, (uint32_t)(((num_in - 1) / size) + 1));

	ZVAL_UNDEF(&chunk);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(input), num_key, str_key, entry) {
		/* If new chunk, create and initialize it. */
		if (Z_TYPE(chunk) == IS_UNDEF) {
			array_init_size(&chunk, (uint32_t)size);
		}

		/* Add entry to the chunk, preserving keys if necessary. */
		if (preserve_keys) {
			if (str_key) {
				entry = zend_hash_update(Z_ARRVAL(chunk), str_key, entry);
			} else {
				entry = zend_hash_index_update(Z_ARRVAL(chunk), num_key, entry);
			}
		} else {
			entry = zend_hash_next_index_insert(Z_ARRVAL(chunk), entry);
		}
		zval_add_ref(entry);

		/* If reached the chunk size, add it to the result array, and reset the
		 * pointer. */
		if (!(++current % size)) {
			add_next_index_zval(&result, &chunk);
			ZVAL_UNDEF(&chunk);
		}
	} ZEND_HASH_FOREACH_END();

	/* Add the final chunk if there is one. */
	if (Z_TYPE(chunk) != IS_UNDEF) {
		add_next_index_zval(&result, &chunk);
	}

	COLLECTION_RETURN_NEW(&result);
}

ZEND_METHOD(collection, combine)
{
	HashTable *values, *keys;
	uint32_t pos_values = 0;
	zval *entry_keys, *entry_values;
	int num_keys, num_values;
	zval result;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_ARRAY_HT(keys)
		Z_PARAM_ARRAY_HT(values)
	ZEND_PARSE_PARAMETERS_END();

	num_keys = zend_hash_num_elements(keys);
	num_values = zend_hash_num_elements(values);

	if (num_keys != num_values) {
		php_error_docref(NULL, E_WARNING, "Both parameters should have an equal number of elements");
		RETURN_FALSE;
	}

	array_init_size(&result, num_keys);

	if (!num_keys) {
		return;
	}

	ZEND_HASH_FOREACH_VAL(keys, entry_keys) {
		while (1) {
			if (pos_values >= values->nNumUsed) {
				break;
			} else if (Z_TYPE(values->arData[pos_values].val) != IS_UNDEF) {
				entry_values = &values->arData[pos_values].val;
				if (Z_TYPE_P(entry_keys) == IS_LONG) {
					entry_values = zend_hash_index_update(Z_ARRVAL(result),
						Z_LVAL_P(entry_keys), entry_values);
				} else {
					zend_string *key = zval_get_string(entry_keys);
					entry_values = zend_symtable_update(Z_ARRVAL(result),
						key, entry_values);
					zend_string_release(key);
				}
				zval_add_ref(entry_values);
				pos_values++;
				break;
			}
			pos_values++;
		}
	} ZEND_HASH_FOREACH_END();

	COLLECTION_RETURN_NEW(result);
}

ZEND_METHOD(collection, countValues)
{
	zval	*input,		/* Input array */
			*entry,		/* An entry in the input array */
			*tmp;
	HashTable *myht;
	zval result;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY(input)
	ZEND_PARSE_PARAMETERS_END();

	/* Initialize return array */
	array_init(&result);

	/* Go through input array and add values to the return array */
	myht = Z_ARRVAL_P(input);
	ZEND_HASH_FOREACH_VAL(myht, entry) {
		ZVAL_DEREF(entry);
		if (Z_TYPE_P(entry) == IS_LONG) {
			if ((tmp = zend_hash_index_find(Z_ARRVAL(result), Z_LVAL_P(entry))) == NULL) {
				zval data;
				ZVAL_LONG(&data, 1);
				zend_hash_index_update(Z_ARRVAL(result), Z_LVAL_P(entry), &data);
			} else {
				Z_LVAL_P(tmp)++;
			}
		} else if (Z_TYPE_P(entry) == IS_STRING) {
			if ((tmp = zend_symtable_find(Z_ARRVAL(result), Z_STR_P(entry))) == NULL) {
				zval data;
				ZVAL_LONG(&data, 1);
				zend_symtable_update(Z_ARRVAL(result), Z_STR_P(entry), &data);
			} else {
				Z_LVAL_P(tmp)++;
			}
		} else {
			php_error_docref(NULL, E_WARNING, "Can only count STRING and INTEGER values!");
		}
	} ZEND_HASH_FOREACH_END();

	COLLECTION_RETURN_NEW(&result);
}

// 类的方法 数组
static zend_function_entry collection_method[] = {

	ZEND_ME(collection, make, 			NULL,   				ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(collection, __construct,   	NULL,   				ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	ZEND_ME(collection, get,   			NULL,   				ZEND_ACC_PUBLIC)
	ZEND_ME(collection, groupBy,   		NULL,   				ZEND_ACC_PUBLIC)
	// Countable Interface
	ZEND_ME(collection, count, 			NULL,   				ZEND_ACC_PUBLIC)
	// ArrayAccess Interface
	ZEND_ME(collection, offsetSet, 		arginfo_offsetSet,   	ZEND_ACC_PUBLIC)
	ZEND_ME(collection, offsetGet, 		arginfo_offsetGet,   	ZEND_ACC_PUBLIC)
	ZEND_ME(collection, offsetUnset, 	arginfo_offsetUnset,   	ZEND_ACC_PUBLIC)
	ZEND_ME(collection, offsetExists,	arginfo_offsetExists,   ZEND_ACC_PUBLIC)
	// Iterator Interface
	ZEND_ME(collection, current, 		NULL,   				ZEND_ACC_PUBLIC)
	ZEND_ME(collection, key, 			NULL,   				ZEND_ACC_PUBLIC)
	ZEND_ME(collection, next, 			NULL,   				ZEND_ACC_PUBLIC)
	ZEND_ME(collection, rewind, 		NULL,   				ZEND_ACC_PUBLIC)
	ZEND_ME(collection, valid, 			NULL,   				ZEND_ACC_PUBLIC)
	// PHP Raw Method
	ZEND_ME(collection, changeKeyCase,  NULL, 					ZEND_ACC_PUBLIC)
	ZEND_ME(collection, chunk,  		NULL, 					ZEND_ACC_PUBLIC)
	ZEND_ME(collection, column,  		NULL, 					ZEND_ACC_PUBLIC)
	ZEND_ME(collection, combine,  		NULL, 					ZEND_ACC_PUBLIC)
	ZEND_ME(collection, countValues,  	NULL, 					ZEND_ACC_PUBLIC)
	ZEND_ME(collection, diffAssoc,  	NULL, 					ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(collection)
{
	zend_class_entry ce;

	// 初始化 类 
	INIT_CLASS_ENTRY(ce, "Collection", collection_method);

	// 在内核中注册类
	collection_ce = zend_register_internal_class(&ce TSRMLS_CC);

	// 实现 Countable ArrayAccess Iterater Ser 接口
	zend_class_implements(collection_ce TSRMLS_CC, 3,
		zend_ce_countable, zend_ce_arrayaccess, zend_ce_iterator);

	//定义属性
	zend_declare_property_null(collection_ce, "data", strlen("data"), ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(collection)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(collection)
{
#if defined(COMPILE_DL_COLLECTION) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(collection)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(collection)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Collection support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ collection_functions[]
 *
 * Every user visible function must have an entry in collection_functions[].
 */
const zend_function_entry collection_functions[] = {
	PHP_FE_END	/* Must be the last line in collection_functions[] */
};
/* }}} */

/* {{{ collection_module_entry
 */
zend_module_entry collection_module_entry = {
	STANDARD_MODULE_HEADER,
	"collection",
	collection_functions,
	PHP_MINIT(collection),
	PHP_MSHUTDOWN(collection),
	PHP_RINIT(collection),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(collection),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(collection),
	PHP_COLLECTION_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_COLLECTION
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(collection)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
/**
 *  数组操作
 */



int check_dot_string(char dot[])
{
  char *sk;
  int kl = 0, tl = 0, cnt = 0;
  kl = strlen(dot);
  sk = strtok(dot, ".");
  while (sk != NULL) {
    cnt++;
    tl += (strlen(sk) + 1);
    sk = strtok(NULL, ".");
  } 

  return tl == kl + 1 ? cnt : -1;
}

void group_by_key(HashTable *target, zval *key, HashTable *source)
{
  zval *current;
  zval *entry;

  ZEND_HASH_FOREACH_VAL(source, current) {
    if((entry = zend_symtable_find(Z_ARRVAL_P(current), zval_get_string(key))) == NULL) {
      zend_error(E_NOTICE, "Undefined index: %s", ZSTR_VAL(zval_get_string(key)));
    } else {
      zend_symtable_update(target, zval_get_string(entry), current);
    }
  } ZEND_HASH_FOREACH_END();
}

void group_by_array(HashTable *target, HashTable *keys)
{
  // zval key;
  // zval *entry;
  // HashTable source;
  // zval *val;
  // zval tmp;

  // if ((entry = zend_hash_get_current_data(keys)) == NULL) {
  //   return;
  // }

  // group_by_key(&source, entry, target);

  // zend_hash_copy(target, &source, NULL);


  // zend_hash_get_current_key_zval(keys, &key);

  // if(Z_TYPE(key) == IS_NULL) {
  //   return;
  // }

  // zend_symtable_del(keys, zval_get_string(&key));

  // if(zend_hash_num_elements(keys) == 0) {
  // 	return;
  // }

  // ZEND_HASH_FOREACH_VAL(target, val) {
  //   group_by_array(val, keys);
  // } ZEND_HASH_FOREACH_END();
}
