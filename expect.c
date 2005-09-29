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

/* {{{ expect_functions[] */
function_entry expect_functions[] = {
	PHP_FE(expect_popen,	NULL)
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
	"0.0.1", 
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EXPECT
ZEND_GET_MODULE(expect)
#endif

PHP_INI_BEGIN()
PHP_INI_END()

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(expect)
{
	php_register_url_stream_wrapper("expect", &php_expect_wrapper TSRMLS_CC);

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
	php_info_print_table_row(2, "Expect support", "enabled");
	php_info_print_table_row(2, "Stream wrapper support", "expect://");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{
 * proto resource expect_popen(string command)
 */
PHP_FUNCTION(expect_popen)
{
	char *command = NULL;
	int command_len;
	FILE *fp;
	php_stream *stream = NULL;

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
	php_stream_to_zval(stream, return_value);
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
