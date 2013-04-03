#include "xsltp_config.h"
#include "xsltp_core.h"
#include "xsltp.h"

static void
xsltp_extension_string_join(xmlXPathParserContextPtr ctxt, int nargs) {
    xmlChar *ret = NULL, *sep = NULL, *str = NULL;
    xmlNodeSetPtr nodeSet = NULL;
    int i, j;

    if (nargs  < 2) {
        xmlXPathSetArityError(ctxt);
        return;
    }

    if (xmlXPathStackIsNodeSet(ctxt)) {
        xmlXPathSetTypeError(ctxt);
        return;
    }

    sep = xmlXPathPopString(ctxt);

    for (i = 1; i < nargs; i++) {
        if (!xmlXPathStackIsNodeSet(ctxt)) {
            str = xmlXPathPopString(ctxt);

            if (i == 1) {
                ret = str;
            }
            else {
                str = xmlStrcat(str, sep);
                str = xmlStrcat(str, ret);
                xmlFree(ret);
                ret = str;
            }
        }
        else {
            nodeSet = xmlXPathPopNodeSet(ctxt);
            if (xmlXPathCheckError(ctxt)) {
                xmlXPathSetTypeError(ctxt);
                goto fail;
            }

            for (j = nodeSet->nodeNr - 1; j >= 0; j--) {
                str = xmlXPathCastNodeToString(nodeSet->nodeTab[j]);

                if (i == 1 && j == (nodeSet->nodeNr - 1)) {
                    ret = str;
                }
                else {
                    str = xmlStrcat(str, sep);
                    str = xmlStrcat(str, ret);
                    xmlFree(ret);
                    ret = str;
                }
            }

            xmlXPathFreeNodeSet(nodeSet);
        }
    }

    xmlXPathReturnString(ctxt, ret);

fail:
    if (sep != NULL)
        xmlFree(sep);
}

#ifdef HAVE_GLIB
static void
xsltp_extension_string_uc(xmlXPathParserContextPtr ctxt, int nargs) {
    xmlChar *str;

    if (nargs != 1) {
        xmlXPathSetArityError(ctxt);
        return;
    }

    str = xmlXPathPopString(ctxt);

    xmlXPathReturnString(ctxt, (xmlChar *) g_utf8_strup((const gchar *) str, -1));

    xmlFree(str);
}

static void
xsltp_extension_string_lc(xmlXPathParserContextPtr ctxt, int nargs) {
    xmlChar *str;

    if (nargs != 1) {
        xmlXPathSetArityError(ctxt);
        return;
    }

    str = xmlXPathPopString(ctxt);

    xmlXPathReturnString(ctxt, (xmlChar *) g_utf8_strdown((const gchar *) str, -1));

    xmlFree(str);
}
#endif

static xmlChar *
_xsltp_extension_string_ltrim(xmlChar *src) {
    xmlChar *tmp = NULL, *dst = NULL, ch;

    dst = tmp = src;

    while (1) {
        switch (ch = *tmp) {
            case '\0':
                goto exit_loop;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                break;
            default:
                goto exit_loop;
        }
        tmp += sizeof(xmlChar);
    }

exit_loop:
    if (src != tmp) {
        while (1) {
            ch = *src = *tmp;
            if (ch == '\0') break;
            tmp += sizeof(xmlChar);
            src += sizeof(xmlChar);
        }
    }

    return dst;
}

static xmlChar *
_xsltp_extension_string_rtrim(xmlChar *src) {
    xmlChar *tmp = NULL, *last = NULL;

    tmp = src;

    while (*tmp != '\0') {
        if (*tmp != ' ' && *tmp != '\t' && *tmp != '\n' && *tmp != '\r') last = tmp;
        tmp++;
    }

    if (last == NULL)
        *src = '\0';
    else
        *++last = '\0';

    return src;
}

static void
xsltp_extension_string_ltrim(xmlXPathParserContextPtr ctxt, int nargs) {
    if (nargs != 1) {
        xmlXPathSetArityError(ctxt);
        return;
    }
    xmlXPathReturnString(ctxt, _xsltp_extension_string_ltrim(xmlXPathPopString(ctxt)));
}

static void
xsltp_extension_string_rtrim(xmlXPathParserContextPtr ctxt, int nargs) {
    if (nargs != 1) {
        xmlXPathSetArityError(ctxt);
        return;
    }
    xmlXPathReturnString(ctxt, _xsltp_extension_string_rtrim(xmlXPathPopString(ctxt)));
}

static void
xsltp_extension_string_trim(xmlXPathParserContextPtr ctxt, int nargs) {
    if (nargs != 1) {
        xmlXPathSetArityError(ctxt);
        return;
    }
    xmlXPathReturnString(ctxt,
        _xsltp_extension_string_rtrim(
            _xsltp_extension_string_ltrim(
                xmlXPathPopString(ctxt)
            )
        )
    );
}

static void *
xsltp_extension_ext_init(xsltTransformContextPtr ctxt, const xmlChar * URI) {
    xsltp_log_debug1("Registered plugin module : %s", (const char *) URI);
    return NULL;
}

static void
xsltp_extension_ext_shutdown(xsltTransformContextPtr ctxt,
                    const xmlChar * URI, void *data) {
    xsltp_log_debug1("Unregistered plugin module : %s", (const char *) URI);
}

static void *
xsltp_extension_style_ext_init(xsltStylesheetPtr style ATTRIBUTE_UNUSED,
                     const xmlChar * URI) {
    xsltp_log_debug1("Registered plugin module : %s", (const char *) URI);
    return NULL;
}

static void
xsltp_extension_style_ext_shutdown(xsltStylesheetPtr style ATTRIBUTE_UNUSED,
                         const xmlChar * URI, void *data)
{
    xsltp_log_debug1("Unregistered plugin module : %s", (const char *) URI);
}

int
xsltp_extension_init(void)
{
    xsltp_log_debug0("init");

    xsltRegisterExtModuleFull((const xmlChar *) XSLTP_EXTENSION_STRING_NS,
                              xsltp_extension_ext_init,
                              xsltp_extension_ext_shutdown,
                              xsltp_extension_style_ext_init,
                              xsltp_extension_style_ext_shutdown);

#ifdef HAVE_GLIB
    xsltRegisterExtModuleFunction((const xmlChar *) "uc",
                                  (const xmlChar *) XSLTP_EXTENSION_STRING_NS,
                                  xsltp_extension_string_uc);
    xsltRegisterExtModuleFunction((const xmlChar *) "lc",
                                  (const xmlChar *) XSLTP_EXTENSION_STRING_NS,
                                  xsltp_extension_string_lc);
#endif
    xsltRegisterExtModuleFunction((const xmlChar *) "join",
                                  (const xmlChar *) XSLTP_EXTENSION_STRING_NS,
                                  xsltp_extension_string_join);
    xsltRegisterExtModuleFunction((const xmlChar *) "ltrim",
                                  (const xmlChar *) XSLTP_EXTENSION_STRING_NS,
                                  xsltp_extension_string_ltrim);
    xsltRegisterExtModuleFunction((const xmlChar *) "rtrim",
                                  (const xmlChar *) XSLTP_EXTENSION_STRING_NS,
                                  xsltp_extension_string_rtrim);
    xsltRegisterExtModuleFunction((const xmlChar *) "trim",
                                  (const xmlChar *) XSLTP_EXTENSION_STRING_NS,
                                  xsltp_extension_string_trim);
    return 1;
}
