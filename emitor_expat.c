/*
 * File:        emitor_expat.c
 * Author:      Aleksandra Matysik
 * Date:        2024-10-01
 * Description: A program that parses an XML file and converts the relevant data
 *              into CSV format. It uses the Expat library for XML parsing.
 *
 * Usage:       Compile the program using gcc and link it with the Expat library:
 *              gcc -o emitor_expat emitor_expat.c -lexpat
 *              
 *              The program reads an input XML file "example.xml" and outputs
 *              the results in a CSV file "wyniki.csv" in the specified format.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <time.h>

#define INPUT_FILENAME  "example.xml"
#define OUTPUT_FILENAME "wyniki.csv"
#define ELEMENTS        30

/*
 * Structure of parsed data
 */
typedef struct {
    char emitor[10];
    int n_tags;
    char tags[4][100];
    char value[10];
} Data;

Data data;
char buffers[ELEMENTS][1024];
int n_buff = 0;

void saveData(struct tm *tm, char *str ) {
    char oneTag[300];

    strcpy(oneTag, data.emitor);
    for(int i; i<data.n_tags; i++) {
        strcat(oneTag, ".");
        strcat(oneTag, data.tags[i]);
    }
    sprintf(str, "\"%d-%02d-%02d\",\"%d\",\"%s\",\"%s\"\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, oneTag, data.value);
}

void saveOneElement( void ) {
    time_t t = time(NULL);      // System time variable
    struct tm tm = *localtime(&t);
    char output[300];

    saveData(&tm, buffers[n_buff]);
    n_buff++;
}

/**
 * @brief   Function called when the parser encounters the beginning of an XML element.
 *          The function handles the beginning of an XML element, such as 'status', 'value' or 'status'. 
 *          Assigns attribute values to the data structure to be saved later.
 *          Increases the counter of collected data when the corresponding element is found.
 * @param userData  A pointer to the user data (can be used to pass additional information to the parser, such as the resulting file).
 * @param name      Name of the currently processed XML element.
 * @param attr      An array of attributes of the XML element (alternating name and attribute value).
 */
void XMLCALL startElement(void *userData, const char *name, const char **attr) {
    if(!strcmp(name, "emitor")) {
        for (int i = 0; attr[i]; i += 2) {
            if (strcmp(attr[i], "nazwa") == 0) {
                strcpy(data.emitor, attr[i + 1]);
            }
        }
    } else if(!strcmp(name, "status") || !strcmp(name, "parametr") || !strcmp(name, "stezenie")) {
        data.n_tags = 0;
        strcpy(data.tags[data.n_tags], name);
        data.n_tags++;

        for (int i = 0; attr[i]; i += 2) {
            if (strcmp(attr[i], "typ") == 0) {
                strcpy(data.tags[data.n_tags], attr[i + 1]);
                data.n_tags++;
                break;
            }
        }
    } else if(data.n_tags!=0 && (!strcmp(name, "wartosc") || !strcmp(name, "auto") || !strcmp(name, "reka"))) {
        strcpy(data.tags[data.n_tags], name);
        data.n_tags++;
        for (int i = 0; attr[i]; i += 2) {
            if (strcmp(attr[i], "pkt") == 0) {
                strcpy(data.value, attr[i + 1]);
                break;
            }
        }
        saveOneElement();
    }
}

/**
 * @brief   Function called when the parser encounters the end of an XML element.
 * @param userData  A pointer to the user data (passed from the parser).
 * @param name      Name of the currently terminated XML element.
 */
void XMLCALL endElement(void *userData, const char *name) {
    if(data.n_tags > 0)
        data.n_tags--;
}

/**
 * @brief   Function called when the parser encounters text data in an XML element.
 * @param userData  A pointer to the user data (passed from the parser).
 * @param s         A pointer to the text data (string) contained in the XML element.
 * @param len       Length of the text data.
 */
void XMLCALL characterData(void *userData, const XML_Char *s, int len) {
    // TODO
}

int main() {
    /*
     * Support for external XML and CSV files.
     */

    FILE *inputFile = fopen(INPUT_FILENAME, "r");
    if (!inputFile) {
        perror("Nie można otworzyć pliku z danymi.\n");
        return EXIT_FAILURE;
    }

    FILE *outputFile = fopen(OUTPUT_FILENAME, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Nie można otworzyć pliku wynikowego.\n");
        return EXIT_FAILURE;
    }

    // Parser initialization
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser, characterData);

    printf("\"YYYY-MM-DD\",\"Hour\",\"Emitor.Tags\",\"Pkt_Value\"\n");
    fprintf(outputFile, "\"YYYY-MM-DD\",\"Hour\",\"Emitor.Tags\",\"Pkt_Value\"\n");

    // XML file parsing
    char buffer[1024];
    int bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFile)) > 0) {
        if (XML_Parse(parser, buffer, bytesRead, bytesRead < sizeof(buffer)) == XML_STATUS_ERROR) {
            fprintf(stderr, "Błąd: %s at line %ld\n", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
            return EXIT_FAILURE;
        } else {
            for(int i=0; i<n_buff; i++) {
                printf("%s", buffers[i]);
                fprintf(outputFile, "%s", buffers[i]);
            }
            n_buff = 0;
        }
    } 

    fclose(inputFile);
    fclose(outputFile);

    XML_ParserFree(parser);
    return EXIT_SUCCESS;
}
