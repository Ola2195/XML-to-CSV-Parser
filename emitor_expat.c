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

#define INPUT_FILENAME "example.xml"
#define OUTPUT_FILENAME "wyniki.csv"
#define ELEMENTS 30
#define MAX_TAG 5

#define STR_SIZE 15

/*
 * Structure of parsed data
 */
typedef struct
{
    char emitor[STR_SIZE];
    int n_tags;
    char tags[MAX_TAG][STR_SIZE];
    char value[STR_SIZE];
} Data;

char dataBuffers[ELEMENTS][1024];
int n_buff = 0;

/**
 * @brief   Formats and saves the collected data into a CSV format.
 *
 * The function concatenates the 'emitor' and its associated tags into a single string,
 * then formats the data into a CSV string with a timestamp (YYYY-MM-DD, Hour), tags, and value.
 *
 * @param tm   A pointer to a struct containing the current time data (year, month, day, hour).
 * @param str  A pointer to a string buffer where the formatted CSV line will be stored.
 */
void saveData(struct tm *tm, Data *data, char *str)
{
    char oneTag[300];

    strcpy(oneTag, data->emitor);
    for (int i = 0; i < data->n_tags; i++)
    {
        strcat(oneTag, ".");
        strcat(oneTag, data->tags[i]);
    }
    sprintf(str, "\"%d-%02d-%02d\",\"%d\",\"%s\",\"%s\"\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, oneTag, data->value);
}

/**
 * @brief   Saves one element of data by appending a timestamp and calling the saveData() function.
 *
 * The function retrieves the current system time, formats the data using saveData(),
 * and increments the buffer counter for storing the next entry.
 */
void saveOneElement(Data *data)
{
    time_t t = time(NULL); // System time variable
    struct tm tm = *localtime(&t);

    saveData(&tm, data, dataBuffers[n_buff]);
    if (n_buff < ELEMENTS - 1)
    {
        n_buff++;
    }
    else
    {
        fprintf(stderr, "Przekroczono limit bufforów!");
    }
}

void addTag(Data *data, const char *tag)
{
    if (data->n_tags < MAX_TAG - 1)
    {
        strcpy(data->tags[data->n_tags], tag);
        data->n_tags++;
    }
    else
    {
        fprintf(stderr, "Przekroczono limit tagów!\n");
    }
}

/**
 * @brief   Function called when the parser encounters the beginning of an XML element.
 *
 * The function handles the beginning of an XML element, such as 'status', 'value' or 'status'.
 * Assigns attribute values to the data structure to be saved later.
 * Increases the counter of collected data when the corresponding element is found.
 *
 * @param userData  A pointer to the user data (can be used to pass additional information to the parser, such as the resulting file).
 * @param name      Name of the currently processed XML element.
 * @param attr      An array of attributes of the XML element (alternating name and attribute value).
 */
void XMLCALL startElement(void *userData, const char *name, const char **attr)
{
    Data *data = (Data *)userData;

    if (!strcmp(name, "emitor"))
    {
        for (int i = 0; attr[i]; i += 2)
        {
            if (strcmp(attr[i], "nazwa") == 0)
            {
                strcpy(data->emitor, attr[i + 1]);
            }
        }
    }
    else if (!strcmp(name, "status") || !strcmp(name, "parametr") || !strcmp(name, "stezenie"))
    {
        data->n_tags = 0;
        addTag(data, name);

        for (int i = 0; attr[i]; i += 2)
        {
            if (strcmp(attr[i], "typ") == 0)
            {
                addTag(data, attr[i + 1]);
                break;
            }
        }
    }
    else if (data->n_tags != 0 && (!strcmp(name, "wartosc") || !strcmp(name, "auto") || !strcmp(name, "reka")))
    {
        addTag(data, name);
        for (int i = 0; attr[i]; i += 2)
        {
            if (strcmp(attr[i], "pkt") == 0)
            {
                strcpy(data->value, attr[i + 1]);
                break;
            }
        }
        saveOneElement(data);
    }
}

/**
 * @brief   Function called when the parser encounters the end of an XML element.
 *
 * @param userData  A pointer to the user data (passed from the parser).
 * @param name      Name of the currently terminated XML element.
 */
void XMLCALL endElement(void *userData, const char *name)
{
    Data *data = (Data *)userData;
    if (data->n_tags > 0)
    {
        data->n_tags--;
    }
}

/**
 * @brief   Function called when the parser encounters text data in an XML element.
 *
 * @param userData  A pointer to the user data (passed from the parser).
 * @param s         A pointer to the text data (string) contained in the XML element.
 * @param len       Length of the text data->
 */
void XMLCALL characterData(void *userData, const XML_Char *s, int len)
{
    // TODO
}

int main()
{
    Data data;

    /*
     * Support for external XML and CSV files.
     */
    FILE *inputFile = fopen(INPUT_FILENAME, "r");
    if (!inputFile)
    {
        fprintf(stderr, "Nie można otworzyć pliku z danymi.\n");
        return EXIT_FAILURE;
    }

    FILE *outputFile = fopen(OUTPUT_FILENAME, "w");
    if (outputFile == NULL)
    {
        fprintf(stderr, "Nie można otworzyć pliku wynikowego.\n\n");
        fclose(inputFile);
        return EXIT_FAILURE;
    }

    // Parser initialization
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser, characterData);
    XML_SetUserData(parser, &data);

    printf("\"YYYY-MM-DD\",\"Hour\",\"Emitor.Tags\",\"Pkt_Value\"\n");
    if (fprintf(outputFile, "\"YYYY-MM-DD\",\"Hour\",\"Emitor.Tags\",\"Pkt_Value\"\n") < 0)
    {
        fprintf(stderr, "Błąd podczas zapisu do pliku wynikowego.\n");
        fclose(inputFile);
        fclose(outputFile);
        XML_ParserFree(parser);
        return EXIT_FAILURE;
    }

    // XML file parsing
    char buffer[1024];
    int bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFile)) > 0)
    {
        if (XML_Parse(parser, buffer, bytesRead, bytesRead < sizeof(buffer)) == XML_STATUS_ERROR)
        {
            fprintf(stderr, "Błąd: %s at line %ld\n", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
            fclose(inputFile);
            fclose(outputFile);
            XML_ParserFree(parser);
            return EXIT_FAILURE;
        }
        else
        {
            for (int i = 0; i < n_buff; i++)
            {
                printf("%s", dataBuffers[i]);
                if (fprintf(outputFile, "%s", dataBuffers[i]) < 0)
                {
                    fprintf(stderr, "Błąd podczas zapisu do pliku wynikowego.\n");
                    fclose(inputFile);
                    fclose(outputFile);
                    XML_ParserFree(parser);
                    return EXIT_FAILURE;
                }
            }
            n_buff = 0;
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    XML_ParserFree(parser);
    return EXIT_SUCCESS;
}
