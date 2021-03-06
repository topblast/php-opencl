/**
 * The OpenCL PHP extension
 *
 * @package     php-opencl
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2012 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */

#include "platform.h"
#include "params.h"

#define get_info ((phpcl_get_info_func_t)_get_platform_info)
#define get_info_ex NULL

/* {{{ globals */

static const phpcl_info_param_t platform_info_params[] = {
	{ "profile",    CL_PLATFORM_PROFILE,    INFO_TYPE_STRING },
	{ "version",    CL_PLATFORM_VERSION,    INFO_TYPE_STRING },
	{ "name",       CL_PLATFORM_NAME,       INFO_TYPE_STRING },
	{ "vendor",     CL_PLATFORM_VENDOR,     INFO_TYPE_STRING },
	{ "extensions", CL_PLATFORM_EXTENSIONS, INFO_TYPE_STRING },
	{ NULL, 0, 0 }
};

/* }}} */
/* {{{ _get_platform_info() */

static cl_int _get_platform_info(cl_platform_id platform,
                                 void *reserved __attribute__ ((unused)),
                                 cl_platform_info name,
                                 size_t value_size,
                                 void *value,
                                 size_t *value_size_ret)
{
	return clGetPlatformInfo(platform, name, value_size, value, value_size_ret);
}

/* }}} */
/* {{{ _get_platform_info_all() */

static void _get_platform_info_all(
	INTERNAL_FUNCTION_PARAMETERS,
	cl_platform_id platform)
{
	const phpcl_info_param_t *param = platform_info_params;
	char buf[32] = {0};

	array_init_size(return_value, 8);
	snprintf(buf, sizeof(buf), "%p", platform);
	add_assoc_string(return_value, "id", buf, 1);

	while (param->key != NULL) {
		zval *entry = phpcl_get_info(get_info, get_info_ex,
		                             platform, NULL, param TSRMLS_CC);
		if (entry) {
			add_assoc_zval(return_value, param->key, entry);
		} else {
			add_assoc_null(return_value, param->key);
		}
		param++;
	}
}

/* }}} */
/* {{{ _get_platform_info_by_name() */

static void _get_platform_info_by_name(
	INTERNAL_FUNCTION_PARAMETERS,
	cl_platform_id platform, cl_int name)
{
	const phpcl_info_param_t *param = platform_info_params;

	RETVAL_NULL();

	while (param->key != NULL) {
		if (param->name == name) {
			zval *entry = phpcl_get_info(get_info, get_info_ex,
			                             platform, NULL, param TSRMLS_CC);
			if (entry) {
				RETVAL_ZVAL(entry, 0, 1);
			}
			return;
		}
		param++;
	}
}

/* }}} */
/* {{{ mixed cl_get_platform_info(resource cl_platform platform[, int name]) */

PHPCL_FUNCTION(cl_get_platform_info)
{
	zval *zid = NULL;
	cl_platform_id platform = NULL;
	long name = 0;

	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	                          "r|l", &zid, &name) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(platform, cl_platform_id, &zid, -1,
	                    "cl_platform", phpcl_le_platform());

	if (ZEND_NUM_ARGS() == 2) {
		_get_platform_info_by_name(INTERNAL_FUNCTION_PARAM_PASSTHRU, platform, (cl_int)name);
	} else {
		_get_platform_info_all(INTERNAL_FUNCTION_PARAM_PASSTHRU, platform);
	}
}

/* }}} */
/* {{{ array cl_get_platform_ids(void) */

PHPCL_FUNCTION(cl_get_platform_ids)
{
	cl_int errcode = CL_SUCCESS;
	cl_uint num_entries = 0;
	cl_platform_id *platforms = NULL;
	cl_uint index = 0;

	RETVAL_FALSE;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	errcode = clGetPlatformIDs(0, NULL, &num_entries);
	if (errcode != CL_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"clGetPlatformIDs() failed [%s]", phpcl_errstr(errcode));
	}

	platforms = ecalloc(num_entries, sizeof(cl_platform_id));
	if (!platforms) {
		return;
	}

	errcode = clGetPlatformIDs(num_entries, platforms, NULL);
	if (errcode != CL_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"clGetPlatformIDs() failed [%s]", phpcl_errstr(errcode));
		efree(platforms);
		return;
	}

	array_init_size(return_value, num_entries);

	for (index = 0; index < num_entries; index++) {
		cl_platform_id platform = platforms[index];
		zval *entry;
		MAKE_STD_ZVAL(entry);
		ZEND_REGISTER_RESOURCE(entry, platform, phpcl_le_platform());
		add_next_index_zval(return_value, entry);
	}

	efree(platforms);
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
