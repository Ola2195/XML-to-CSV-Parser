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

#define STR_SIZE 15  // Maximum length of strings for emitter names, tags, and values
#define ADD_TAG 5    // Number of additional tags to allocate when more space is needed
#define ADD_BUFFOR 5 // Number of additional buffers to allocate when more space is needed

/*
 * Structure to store parsed data from the XML file.
 * It includes:
 * - emitter name,
 * - dynamically allocated array of tags,
 * - value associated with the current element.
 */
typedef struct
{
    char emitor[STR_SIZE];
    char **tags;
    int n_tags;
    int allocated_tags;
    char value[STR_SIZE];
} Data;

char **dataBuffers = NULL; // Dynamic array to hold pointers to data buffers
int n_buff = 0;            // Number of buffers currently in use
int allocated_buffers = 0; // Total number of buffers allocated

/**
 * @brief   Initializes the Data structure, allocating memory for the tags.
 *
 * The function sets the initial number of tags to zero and allocates memory for
 * the array of tags based on a pre-defined number of tags (ADD_TAG).
 *
 * @param data  A pointer to the Data structure to be initialized.
 */
void initData(Data *data)
{
    data->n_tags = 0;
    data->allocated_tags = ADD_TAG;
    data->tags = malloc(data->allocated_tags * sizeof(char *));
    if (!data->tags)
    {
        perror("Błąd alokacji pamięci dla tagów!");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief   Reallocates memory for tags in the Data structure when needed.
 *
 * The function checks if the number of stored tags has reached the allocated limit.
 * If so, it increases the memory allocation for the tags array to accommodate more tags.
 *
 * @param data  A pointer to the Data structure where tags are stored.
 */
void reallocTags(Data *data)
{
    if (data->n_tags >= data->allocated_tags)
    {
        data->allocated_tags += ADD_TAG;
        data->tags = realloc(data->tags, data->allocated_tags * sizeof(char *));
        if (!data->tags)
        {
            perror("Błąd alokacji pamięci dla tagów!");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief   Adds a new tag to the Data structure.
 *
 * The function dynamically allocates memory for a new tag and copies the given tag string
 * into the Data structure's tag array. It reallocates memory if needed.
 *
 * @param data  A pointer to the Data structure where the tag will be added.
 * @param tag   A string representing the tag to be added.
 */
void addTag(Data *data, const char *tag)
{
    reallocTags(data);
    data->tags[data->n_tags] = malloc((strlen(tag) + 1) * sizeof(char));
    if (!data->tags[data->n_tags])
    {
        perror("Błąd alokacji pamięci dla pojedynczego tagu!");
        exit(EXIT_FAILURE);
    }
    strcpy(data->tags[data->n_tags], tag);
    data->n_tags++;
}

/**
 * @brief   Frees the dynamically allocated memory for the tags in the Data structure.
 *
 * This function releases the memory for each tag in the Data structure and then
 * frees the memory allocated for the tags array itself.
 *
 * @param data  A pointer to the Data structure whose tags are to be freed.
 */
void freeTags(Data *data)
{
    for (int i = 0; i < data->n_tags; i++)
    {
        free(data->tags[i]);
    }
    free(data->tags);
}

/**
 * @brief   Allocates or expands the memory for data buffers.
 *
 * This function checks if there is enough allocated space for new data.
 * If not, it allocates more memory for additional buffers.
 * The allocation happens in blocks defined by ADD_BUFFOR to optimize performance.
 */
void relocateBuffers(void)
{
    if (n_buff >= allocated_buffers)
    {
        allocated_buffers += ADD_BUFFOR;
        dataBuffers = realloc(dataBuffers, allocated_buffers * sizeof(char *));
        if (!dataBuffers)
        {
            perror("Błąd alokacji pamięci!");
            exit(EXIT_FAILURE);
        }

        for (int i = n_buff; i < allocated_buffers; i++)
        {
            dataBuffers[i] = malloc(1024 * sizeof(char));
            if (!dataBuffers[i])
            {
                perror("Błąd alokacji pamięci dla bufora!");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/**
 * @brief   Frees all dynamically allocated memory for data buffers.
 *
 * This function iterates through the allocated buffers and frees each one,
 * then frees the array holding the buffer pointers.
 */
void freeBuffers(void)
{
    for (int i = 0; i < allocated_buffers; i++)
    {
        free(dataBuffers[i]);
    }
    free(dataBuffers);
}

/**
 * @brief   Formats and saves the collected data into CSV format.
 *
 * The function concatenates the 'emitor' and its associated tags into a single string,
 * then formats the data into a CSV string with a timestamp (YYYY-MM-DD, Hour), emitter tags, and value.
 *
 * @param tm   A pointer to a struct containing the current time data (year, month, day, hour).
 * @param data A pointer to the Data struct containing emitter and tag information.
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
    sprintf(str, "\"%d-%02d-%02d\",\"%d\",\"%s\",\"%s\"\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
            oneTag, data->value);
}

/**
 * @brief   Adds a new element of data, appending a timestamp and calling saveData().
 *
 * The function retrieves the current system time, formats the data using saveData(),
 * and increments the buffer counter for storing the next entry.
 *
 * @param data  A pointer to the Data struct containing parsed XML data to be saved.
 */
void saveOneElement(Data *data)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    relocateBuffers();

    saveData(&tm, data, dataBuffers[n_buff]);
    n_buff++;
}

/**
 * @brief   Function called when the parser encounters the beginning of an XML element.
 *
 * This function handles the start of an XML element and extracts relevant data such as
 * emitter names, tags, and values from the element's attributes.
 * It assigns attribute values to the Data struct and prepares data for saving when needed.
 *
 * @param userData  A pointer to user data (the Data struct in this case).
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
 * This function decreases the tag counter when an XML element ends.
 *
 * @param userData  A pointer to user data (the Data struct in this case).
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
 * This function is not currently implemented, but it could be used to handle text content
 * within XML elements.
 *
 * @param userData  A pointer to user data (the Data struct in this case).
 * @param s         A pointer to the text data (string) contained in the XML element.
 * @param len       Length of the text data.
 */
void XMLCALL characterData(void *userData, const XML_Char *s, int len)
{
    // TODO: Implementation for handling character data in XML elements
}

int main()
{
    Data data;
    initData(&data); // Initialize the data structure to store parsed information

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

    // Initialize the XML parser
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser, characterData);
    XML_SetUserData(parser, &data);

    // Write the CSV header to both the console and the output file
    printf("\"YYYY-MM-DD\",\"Hour\",\"Emitor.Tags\",\"Pkt_Value\"\n");
    if (fprintf(outputFile, "\"YYYY-MM-DD\",\"Hour\",\"Emitor.Tags\",\"Pkt_Value\"\n") < 0)
    {
        fprintf(stderr, "Błąd podczas zapisu do pliku wynikowego.\n");
        fclose(inputFile);
        fclose(outputFile);
        XML_ParserFree(parser);
        return EXIT_FAILURE;
    }

    // Read the XML file and parse its contents
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

    freeBuffers();
    freeTags(&data);

    fclose(inputFile);
    fclose(outputFile);

    XML_ParserFree(parser);
    return EXIT_SUCCESS;
}
