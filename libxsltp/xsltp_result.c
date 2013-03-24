#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static xsltp_bool_t
xsltp_result_init(xsltp_t *processor, xsltp_result_t *result)
{
    result->processor = processor;

    return TRUE;
}

xsltp_result_t *
xsltp_result_create(xsltp_t *processor)
{
    xsltp_result_t *result;

    xsltp_log_debug0("create a result");

    if ((result = xsltp_malloc(sizeof(xsltp_result_t))) == NULL) {
        return NULL;
    }
    memset(result, 0, sizeof(xsltp_result_t));

    if (! xsltp_result_init(processor, result)) {
        xsltp_result_destroy(result);
        return NULL;
    }

    return result;
}

int
xsltp_result_save(xsltp_result_t *result, xmlOutputBufferPtr output)
{
    return xsltSaveResultTo(output, result->doc, result->xsltp_stylesheet->stylesheet);
}

int
xsltp_result_save_to_file(xsltp_result_t *result, char *filename)
{
    return xsltSaveResultToFilename(filename, result->doc, result->xsltp_stylesheet->stylesheet, 0);
}

int
xsltp_result_save_to_string(xsltp_result_t *result, char **buf, int *buf_len)
{
    return xsltSaveResultToString((xmlChar **) buf, buf_len, result->doc, result->xsltp_stylesheet->stylesheet);
}

void
xsltp_result_destroy(xsltp_result_t *result)
{
    xsltp_log_debug0("destroy");

    if (result != NULL) {
        if (result->doc != NULL) {
            xmlFreeDoc(result->doc);
        }

        if (!result->processor->stylesheet_caching_enable) {
            xsltp_stylesheet_parser_destroy_stylesheet(result->xsltp_stylesheet);
        }

        if (result->profiler_result != NULL) {
            xsltp_profiler_result_destroy(result->profiler_result);
        }

        xsltp_free(result);
    }
}
