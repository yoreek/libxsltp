#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

/*
 * <profiler repeat="20">
 *   <stylesheet uri="main.xsl" time="444">
 *     <profile>
 *       <template name="" match="*" mode="" ... />
 *       ...
 *     </profile>
 *     <document>
 *       <root>
 *         ...
 *       </root>
 *     </document>
 *     <params>
 *       <param name="name1" value="'value1'" />
 *       ...
 *     </params>
 *   </stylesheet>
 *   ...
 * </profiler>
 */

static xsltp_bool_t
xsltp_profiler_result_init(xsltp_t *processor, xsltp_profiler_result_t *profiler_result)
{
    xmlNodePtr root;
    char       strbuf[100];

    profiler_result->doc = xmlNewDoc((const xmlChar*) "1.0");
    profiler_result->doc->encoding = (const xmlChar*) xmlStrdup((const xmlChar*) "utf-8");

    root = xmlNewDocNode(profiler_result->doc, NULL, BAD_CAST "profiler", NULL);
    xmlDocSetRootElement(profiler_result->doc, root);

    sprintf(strbuf, "%d", processor->profiler_repeat);
    xmlSetProp(root, BAD_CAST "repeat", BAD_CAST strbuf);

    return TRUE;
}

xsltp_profiler_result_t *
xsltp_profiler_result_create(xsltp_t *processor)
{
    xsltp_profiler_result_t *profiler_result;

    xsltp_log_debug0("create");

    if ((profiler_result = xsltp_malloc(sizeof(xsltp_profiler_result_t))) == NULL) {
        return NULL;
    }
    memset(profiler_result, 0, sizeof(xsltp_profiler_result_t));

    if (! xsltp_profiler_result_init(processor, profiler_result)) {
        xsltp_profiler_result_destroy(profiler_result);
        return NULL;
    }

    return profiler_result;
}

void
xsltp_profiler_result_destroy(xsltp_profiler_result_t *profiler_result)
{
    xsltp_log_debug0("destroy");

    if (profiler_result != NULL) {
        if (profiler_result->doc != NULL) {
            xmlFreeDoc(profiler_result->doc);
        }

        xsltp_free(profiler_result);
    }
}

xsltp_bool_t
xsltp_profiler_result_apply(xsltp_profiler_t *profiler, xsltp_profiler_result_t *profiler_result, xmlDocPtr doc)
{
    xmlNodePtr   root;
    xmlDocPtr    res;
    xsltp_bool_t status = TRUE;

    if (profiler->stylesheet == NULL) {
        return TRUE;
    }

    res = xsltApplyStylesheet(profiler->stylesheet, profiler_result->doc, NULL);

    if (res) {
        root = xmlDocCopyNode(xmlDocGetRootElement(res), doc, 1);

        if (root) {
            xmlAddChild(xmlDocGetRootElement(doc)->last, root);
        }
        else {
            perror("No root element in the profile info");
            status = FALSE;
        }

        xmlFreeDoc(res);
    }
    else {
        perror("No profile info");
        status = FALSE;
    }

    return status;
}

void
xsltp_profiler_result_update(xsltp_profiler_result_t *profiler_result,
    xsltp_stylesheet_t *xsltp_stylesheet, const char **params,
    long spent, xmlDocPtr doc, xmlDocPtr profiler_info)
{
    xmlNodePtr root, child, child2, child3;
    char       strbuf[100];

    root = xmlDocGetRootElement(profiler_result->doc);

    /* add stylesheet info */
    child = xmlNewChild(root, NULL, BAD_CAST "stylesheet", NULL);
    xmlSetProp(child, BAD_CAST "uri", BAD_CAST xsltp_stylesheet->uri);

    sprintf(strbuf, "%ld", spent);
    xmlSetProp(child, BAD_CAST "time", BAD_CAST strbuf);

    /* add profile info */
    xmlAddChild(child, xmlDocCopyNode(xmlDocGetRootElement(profiler_info), profiler_result->doc, 1));

    /* add document */
    child2 = xmlNewChild(child, NULL, BAD_CAST "document", NULL);
    xmlAddChild(child2, xmlDocCopyNode(xmlDocGetRootElement(doc), profiler_result->doc, 1));

    /* add params */
    child2 = xmlNewChild(child, NULL, BAD_CAST "params", NULL);
    if (params != NULL) {
        for(; *params != '\0'; params++) {
            child3 = xmlNewChild(child2, NULL, BAD_CAST "param", NULL);

            xmlSetProp(child3, BAD_CAST "name", BAD_CAST *params);
            params++;
            xmlSetProp(child3, BAD_CAST "value", BAD_CAST *params);
        }
    }
}

static xsltp_bool_t
xsltp_profiler_init(xsltp_t *processor, xsltp_profiler_t *profiler)
{
    profiler->processor = processor;
    profiler->repeat    = processor->profiler_repeat;

    profiler->handle = fopen("/dev/null", "w");
    if (profiler->handle == NULL) {
        perror("Failed to create file handle");
        return FALSE;
    }

    if (processor->profiler_stylesheet != NULL) {
        profiler->stylesheet = xsltParseStylesheetFile((const xmlChar *) processor->profiler_stylesheet);
        if (profiler->stylesheet == NULL) {
            return FALSE;
        }
    }

    return TRUE;
}

xsltp_profiler_t *
xsltp_profiler_create(xsltp_t *processor)
{
    xsltp_profiler_t *profiler;

    xsltp_log_debug0("create");

    if ((profiler = xsltp_malloc(sizeof(xsltp_profiler_t))) == NULL) {
        return NULL;
    }
    memset(profiler, 0, sizeof(xsltp_profiler_t));

    if (! xsltp_profiler_init(processor, profiler)) {
        xsltp_profiler_destroy(profiler);
        return NULL;
    }

    return profiler;
}

void
xsltp_profiler_destroy(xsltp_profiler_t *profiler)
{
    xsltp_log_debug0("destroy");

    if (profiler != NULL) {
        if (profiler->stylesheet != NULL) {
            xsltFreeStylesheet(profiler->stylesheet);
        }

        if (profiler->handle != NULL) {
            fclose(profiler->handle);
        }

        xsltp_free(profiler);
    }
}
