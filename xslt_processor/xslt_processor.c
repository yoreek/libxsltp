#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include <libxsltp/xsltp_config.h>
#include <libxsltp/xsltp_core.h>
#include <libxsltp/xsltp.h>

int run(char *stylesheet, char *xml) {
    xmlDocPtr       xml_doc;
    xsltp_t        *processor;
    xsltp_result_t *result;
    char           *buf;
    int             buf_len;
    int             status = 0;

    for ( ;; ) {
        processor = xsltp_create();
        if (processor == NULL) {
            perror("Can't to create processor");
            status = -1;
            break;
        }

        for ( ;; ) {
            xml_doc = xmlParseFile(xml);
            if (xml_doc == NULL) {
                perror("Error to parse xml");
                status = -1;
                break;
            }

            for ( ;; ) {
                result = xsltp_transform(processor, stylesheet, xml_doc, NULL, NULL);
                if (result == NULL) {
                    status = -1;
                    perror("Error to transform");
                    break;
                }

                for ( ;; ) {
                    if (xsltp_result_save_to_string(result, &buf, &buf_len) == -1) {
                        status = -1;
                        perror("Error to transform");
                        break;
                    }

                    if (fwrite(buf, 1, buf_len, stdout) != buf_len) {
                        perror("Error to write result");
                        status = -1;
                    }

                    xmlFree(buf);
                    break;
                }

                xsltp_result_destroy(result);
                break;
            }

            xmlFreeDoc(xml_doc);
            break;
        }

        xsltp_destroy(processor);
        break;
    }

    return status;
}

int main(int argc, char **argv) {
    int status;

    if (argc <= 2) {
        printf("Usage: xslt_processor <stylesheet> <xml>\n");
        return -1;
    }

    status = run(argv[1], argv[2]);

    return status;
}
