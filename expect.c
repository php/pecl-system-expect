/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2004 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Michael Spector <michael@zend.com>                           |
  +----------------------------------------------------------------------+
*/

/* $ Id: $ */ 

#include "php_expect.h"
#include <string.h>
#include <errno.h>

/* {{{ expect_functions[] */
function_entry expect_functions[] = {
	PHP_FE(expect_popen,	NULL)
	PHP_FE(expect_expectl,	third_arg_force_ref)
	{ NULL, NULL, NULL }
};
/* }}} */


/* {{{ expect_module_entry
 */
zend_module_entry expect_module_entry = {
	STANDARD_MODULE_HEADER,
	"expect",
	expect_functions,
	PHP_MINIT(expect),
	PHP_MSHUTDOWN(expect),
	NULL,
	NULL,
	PHP_MINFO(expect),
	PHP_EXPECT_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EXPECT
ZEND_GET_MODULE(expect)
#endif

/* {{{ PHP_INI_MH
 *  */
static PHP_INI_MH(OnSetExpectTimeout)
{
	if (new_value) {
		exp_timeout = atoi(new_value);
	}
}
/* }}} */


/* {{{ PHP_INI_MH
 *  */
static PHP_INI_MH(OnSetExpectLogUser)
{
	if (new_value) {
		if (strncasecmp("on", new_value, sizeof("on"))) {
			exp_loguser = atoi(new_value);
		} else {
			exp_loguser = 1;
		}
	}
}
/* }}} */


/* {{{ PHP_INI_MH
 *  */
static PHP_INI_MH(OnSetExpectLogFile)
{
	if (new_value_length > 0) {
		php_stream *stream = php_stream_open_wrapper (new_value, "a", 0, NULL);
		if (!stream) {
			php_error_docref (NULL TSRMLS_CC, E_ERROR, "could not open log file for writing");
			return FAILURE;
		}
		stream->flags |= PHP_STREAM_FLAG_NO_SEEK;
		if (php_stream_cast(stream, PHP_STREAM_AS_STDIO, (void **) &exp_logfile, REPORT_ERRORS) != SUCCESS) {
			return FAILURE;
		}
		exp_logfile_all = 1;
	} else {
		exp_logfile = NULL;
		exp_logfile_all = 0;
	}
	return SUCCESS;
}
/* }}} */


PHP_INI_BEGIN()
	PHP_INI_ENTRY("expect.timeout", "10", PHP_INI_ALL, OnSetExpectTimeout)
	PHP_INI_ENTRY_EX("expect.loguser", "1", PHP_INI_ALL, OnSetExpectLogUser, php_ini_boolean_displayer_cb)
	PHP_INI_ENTRY("expect.logfile", "", PHP_INI_ALL, OnSetExpectLogFile)
PHP_INI_END()


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(expect)
{
	php_register_url_stream_wrapper("expect", &php_expect_wrapper TSRMLS_CC);

	REGISTER_LONG_CONSTANT("EXP_GLOB", exp_glob, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXP_EXACT", exp_exact, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXP_REGEXP", exp_regexp, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXP_EOF", EXP_EOF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXP_TIMEOUT", EXP_TIMEOUT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EXP_FULLBUFFER", EXP_FULLBUFFER, CONST_CS | CONST_PERSISTENT);

	REGISTER_INI_ENTRIES();
	
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(expect)
{
	php_unregister_url_stream_wrapper("expect" TSRMLS_CC);

	UNREGISTER_INI_ENTRIES();
	
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(expect)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Expect support", "enabled");
	php_info_print_table_row(2, "Version", PHP_EXPECT_VERSION);
	php_info_print_table_row(2, "Stream wrapper support", "expect://");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{
 * proto resource expect_popen (string command)
 */
PHP_FUNCTION(expect_popen)
{
	char *command = NULL;
	int command_len;
	FILE *fp;
	php_stream *stream = NULL;
	zval *z_pid;

	if (ZEND_NUM_ARGS() != 1) { WRONG_PARAM_COUNT; }

	if (zend_parse_parameters (ZEND_NUM_ARGS() TSRMLS_CC, "s", &command, &command_len) == FAILURE) {
		return;
	}

	if ((fp = exp_popen (command)) != NULL) {
		stream = php_stream_fopen_from_pipe (fp, "");
	}
	if (!stream) {
		RETURN_FALSE;
	}

	stream->flags |= PHP_STREAM_FLAG_NO_SEEK;

	MAKE_STD_ZVAL (z_pid);
	ZVAL_LONG (z_pid, exp_pid);
	stream->wrapperdata = z_pid;

	php_stream_to_zval(stream, return_value);
}
/* }}} */


/* {{{
 * proto mixed expect_expectl (resource stream, array expect_cases [, array match])
 */
PHP_FUNCTION(expect_expectl)
{
	struct exp_case *ecases, *ec, matchedcase;
	zval *z_stream, *z_cases, *z_match=NULL, **z_case, **z_value;
	php_stream *stream;
	int fd, argc;
	ulong key;
	
	if (ZEND_NUM_ARGS() < 2 || ZEND_NUM_ARGS() > 3) { WRONG_PARAM_COUNT; }

	if (zend_parse_parameters (ZEND_NUM_ARGS() TSRMLS_CC, "ra|z", &z_stream, &z_cases, &z_match) == FAILURE) {
		return;
	}

	php_stream_from_zval (stream, &z_stream);

	if (!stream->wrapperdata) {
		php_error_docref (NULL TSRMLS_CC, E_ERROR, "supplied argument is not a valid stream resource");
		return;
	}

	if (php_stream_cast (stream, PHP_STREAM_AS_FD, (void*)&fd, REPORT_ERRORS) != SUCCESS || fd < 0) {
		return;
	}

	argc = zend_hash_num_elements (Z_ARRVAL_P(z_cases));
	ecases = (struct exp_case*) safe_emalloc (argc + 1, sizeof(struct exp_case), 0);

	ec = ecases;
	zend_hash_internal_pointer_reset (Z_ARRVAL_P(z_cases));

	while (zend_hash_get_current_data (Z_ARRVAL_P(z_cases), (void **)&z_case) == SUCCESS)
	{
		zval **z_pattern, **z_exp_type;
		zend_hash_get_current_key(Z_ARRVAL_P(z_cases), NULL, &key, 0);

		if (Z_TYPE_PP(z_case) != IS_ARRAY) {
			efree (ecases);
			php_error_docref (NULL TSRMLS_CC, E_ERROR, "expect case must be an array");
			return;
		}

		ec->re = NULL;
		ec->type = exp_glob;

		/* Gather pattern */
		if (zend_hash_index_find(Z_ARRVAL_PP(z_case), 0, (void **)&z_pattern) != SUCCESS) {
			efree (ecases);
			php_error_docref (NULL TSRMLS_CC, E_ERROR, "missing parameter for pattern at index: 0");
			return;
		}
		if (Z_TYPE_PP(z_pattern) != IS_STRING) {
			efree (ecases);
			php_error_docref (NULL TSRMLS_CC, E_ERROR, "pattern must be of string type");
			return;
		}
		ec->pattern = Z_STRVAL_PP(z_pattern);

		/* Gather value */
		if (zend_hash_index_find(Z_ARRVAL_PP(z_case), 1, (void **)&z_value) != SUCCESS) {
			efree (ecases);
			php_error_docref (NULL TSRMLS_CC, E_ERROR, "missing parameter for value at index: 1");
			return;
		}
		ec->value = key;

		/* Gather expression type (optional, default: EXPECT_GLOB) */
		if (zend_hash_index_find(Z_ARRVAL_PP(z_case), 2, (void **)&z_exp_type) == SUCCESS) {
			if (Z_TYPE_PP(z_exp_type) != IS_LONG) {
				efree (ecases);
				php_error_docref (NULL TSRMLS_CC, E_ERROR, "expression type must be an integer constant");
				return;
			}
			if (Z_LVAL_PP(z_exp_type) != exp_glob && Z_LVAL_PP(z_exp_type) != exp_exact && Z_LVAL_PP(z_exp_type) != exp_regexp) {
				efree (ecases);
				php_error_docref (NULL TSRMLS_CC, E_ERROR, "expression type must be either EXPECT_GLOB, EXPECT_EXACT or EXPECT_REGEXP");
				return;
			}
			ec->type = Z_LVAL_PP(z_exp_type);
		}

		ec++;
		zend_hash_move_forward(Z_ARRVAL_P(z_cases));
	}
	ec->type = exp_end;

	key = exp_expectv (fd, ecases);

	int exp_match_len = exp_match_end - exp_match;
	if (z_match && exp_match && exp_match_len > 0) {
		char *tmp = (char *)emalloc (sizeof(char) * (exp_match_len + 1));
		strlcpy (tmp, exp_match, exp_match_len + 1);
		zval_dtor (z_match);
		array_init(z_match);
		add_index_string(z_match, 0, tmp, 1);
		/* Get case that was matched */
		matchedcase = ecases[key];
		/* If there are subpattern matches ... */
		if (matchedcase.re->startp != NULL) {
			int i;
			/* iterate across all possible 9 subpatterns (a limitation of libexpect)
			   and add matching substring to matches array */
			for (i = 1; i <= 9; i++) {
				if (matchedcase.re->startp[i] != NULL) {
					int sub_match_len = matchedcase.re->endp[i] - matchedcase.re->startp[i];
					char *sub_match = (char *)emalloc (sizeof(char) * (sub_match_len + 1));
					strlcpy (sub_match, matchedcase.re->startp[i], sub_match_len + 1);
					add_next_index_string(z_match, sub_match, 1);
					efree (sub_match);
				}
			}
		}
		efree (tmp);
	}

	if (zend_hash_index_find (Z_ARRVAL_P(z_cases), key, (void **)&z_case) == SUCCESS) {
		if (zend_hash_index_find(Z_ARRVAL_PP(z_case), 1, (void **)&z_value) == SUCCESS) {
			*return_value = **z_value;
			zval_copy_ctor (return_value);
		}
	}
	else {
		RETURN_LONG (key);
	}

	efree (ecases);
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
