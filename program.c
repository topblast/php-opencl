/**
 * The OpenCL PHP extension
 *
 * @package     php-opencl
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2012 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */

#include "program.h"

#define get_info ((phpcl_get_info_func_t)_get_program_info)
#define get_info_ex NULL

/* {{{ globals */

static const phpcl_info_param_t program_info_params[] = {
	{ NULL, 0, 0 }
};

/* }}} */
/* {{{ _get_program_info() */

static cl_int _get_program_info(cl_program program,
                                void *reserved __attribute__ ((unused)),
                                cl_program_info name,
                                size_t value_size,
                                void *value,
                                size_t *value_size_ret)
{
	return clGetProgramInfo(program, name, value_size, value, value_size_ret);
}

/* }}} */
/* {{{ _get_program_info_all() */

static void _get_program_info_all(INTERNAL_FUNCTION_PARAMETERS,
	cl_program program)
{
	const phpcl_info_param_t *param = program_info_params;
	char buf[32] = {0};

	array_init_size(return_value, 8);
	snprintf(buf, sizeof(buf), "%p", program);
	add_assoc_string(return_value, "id", buf, 1);

	while (param->key != NULL) {
		zval *entry = phpcl_get_info(get_info, get_info_ex,
		                             program, NULL, param TSRMLS_CC);
		if (entry) {
			add_assoc_zval(return_value, param->key, entry);
		} else {
			add_assoc_null(return_value, param->key);
		}
		param++;
	}
}

/* }}} */
/* {{{ _get_program_info_by_name() */

static void _get_program_info_by_name(INTERNAL_FUNCTION_PARAMETERS,
	cl_program program, cl_int name)
{
	const phpcl_info_param_t *param = program_info_params;

	RETVAL_NULL();

	while (param->key != NULL) {
		if (param->name == name) {
			zval *entry = phpcl_get_info(get_info, get_info_ex,
			                             program, NULL, param TSRMLS_CC);
			if (entry) {
				RETVAL_ZVAL(entry, 0, 1);
			}
			return;
		}
		param++;
	}
}

/* }}} */
/* {{{ mixed cl_get_program_info(resource cl_program program[, int name]) */

PHP_FUNCTION(cl_get_program_info)
{
	zval *zid = NULL;
	cl_program program = NULL;
	long name = 0;

	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	                          "r|l", &zid, &name) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(program, cl_program, &zid, -1,
	                    "cl_program", phpcl_le_program());

	if (ZEND_NUM_ARGS() == 2) {
		_get_program_info_by_name(INTERNAL_FUNCTION_PARAM_PASSTHRU, program, (cl_int)name);
	} else {
		_get_program_info_all(INTERNAL_FUNCTION_PARAM_PASSTHRU, program);
	}
}

/* }}} */
/* {{{ resource cl_program cl_create_program_with_source(resource cl_context context, mixed source[, int &errcode]); */

PHP_FUNCTION(cl_create_program_with_source)
{
	cl_int errcode = CL_SUCCESS;
	cl_program program = NULL;
	zval *zcontext = NULL;
	phpcl_context_t *ctx = NULL;
	zval *zstring = NULL;
	zval *zerrcode = NULL;

	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	                          "rz|z", &zcontext, &zstring, &zerrcode) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(ctx, phpcl_context_t *, &zcontext, -1,
	                    "cl_context", phpcl_le_context());

	if (Z_TYPE_P(zstring) == IS_STRING) {
		const char *strings[] = { (const char *)Z_STRVAL_P(zstring) };
		const size_t lengths[] = { (const size_t)Z_STRLEN_P(zstring) };
		program = clCreateProgramWithSource(ctx->context, 1, strings, lengths, &errcode);
	} else if (Z_TYPE_P(zstring) == IS_ARRAY) {
		/* TODO: support multiple sources
		cl_uint count = 0;
		char **strings = NULL;
		size_t *lengths = NULL;
		program = clCreateProgramWithSource(ctx->context, count, strings, lengths, &errcode);
		efree(strings);
		efree(lengths);
		*/
	}

	if (zerrcode) {
		zval_dtor(zerrcode);
		ZVAL_LONG(zerrcode, (long)errcode);
	}

	if (program) {
		ZEND_REGISTER_RESOURCE(return_value, program, phpcl_le_program());
	}
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
