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

#define STR_SIZE    15  // Maximum length of strings for emitter names, tags, and values
#define ADD_TAG     5   // Number of additional tags to allocate when more space is needed
#define ADD_BUFFOR  5   // Number of additional buffers to allocate when more space is needed

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
    int nTags;
    int allocatedTags;
    char value[STR_SIZE];
} Data;

char **dataBuffers = NULL; // Dynamic array to hold pointers to data buffers
int nBuff = 0;             // Number of buffers currently in use
int allocatedBuffers = 0;  // Total number of buffers allocated

/**
 * @brief   Checks if a given string is present in a predefined array of tag names.

 * @param val   Pointer to the string to be checked.
 * @return  Returns 1 if the value is found in the array, otherwise 0.
 */
int valueInArray(const char *val)
{
    const char *tagNames[] = {"auto", "reka", "wartosc", "status", "niepewnosc", "standard"};

    int arraySize = sizeof(tagNames) / sizeof(tagNames[0]);
    for (int i = 0; i < arraySize; i++)
    {
        if (!strcmp(tagNames[i], val))
        {
            return 1;
        }
    }
    return 0;
}


/**
 * @brief   Reallocates memory for a dynamic array when needed.
 *
 * This function checks if more memory is required and reallocates memory in chunks,
 * based on the provided increment size.
 *
 * @param array          A pointer to the array that needs to be resized.
 * @param currentSize    The current number of elements in the array.
 * @param allocatedSize  A pointer to the current allocated size (it will be updated).
 * @param increment      The number of elements to add during reallocation.
 * @param elementSize    The size of each element in the array.
 * @return  A pointer to the newly reallocated array.
 */
void *relocateMemmory(void *array, int currentSize, int *allocatedSize, int increment, size_t elementSize)
{
    if (currentSize >= *allocatedSize)
    {
        *allocatedSize += increment;
        array = realloc(array, *allocatedSize * elementSize);
        if (!array)
        {
            perror("Błąd relokacji pamięci!");
            exit(EXIT_FAILURE);
        }
    }
    return array;
}

/**
 * @brief   Allocates new memory for a dynamic array when needed.
 *
 * @param array          A pointer to the array that needs to be sized.
 * @param sizeToAllocate The number of elements to add during allocation.
 * @param elementSize    The size of each element in the array.
 * @return  A pointer to the allocated array.
 */
void *alocateNewMemmory(void *array, int sizeToAllocate, size_t elementSize)
{
    array = malloc(sizeToAllocate * elementSize);
    if (!array)
    {
        perror("Błąd alokacji pamięci!");
        exit(EXIT_FAILURE);
    }
    return array;
}

/**
 * @brief   Frees the dynamically allocated memory for an array of strings.
 *
 * @param array   A pointer to the array of strings to be freed.
 * @param size    The number of elements (strings) in the array.
 */
void freeMemmory(char **array, int size)
{
    for (int i = 0; i < size; i++)
    {
        free(array);
    }
    free(array);
}

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
    data->nTags = 0;
    data->allocatedTags = ADD_TAG;
    data->tags = alocateNewMemmory(data->tags, data->allocatedTags, sizeof(char *));
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
    relocateMemmory(data->tags, data->nTags, &data->allocatedTags, ADD_TAG, sizeof(char *));
    data->tags[data->nTags] = alocateNewMemmory(data->tags[data->nTags], (strlen(tag) + 1), sizeof(char));
    strcpy(data->tags[data->nTags], tag);
    data->nTags++;
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
    for (int i = 0; i < data->nTags; i++)
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

    dataBuffers = relocateMemmory(dataBuffers, nBuff, &allocatedBuffers, ADD_BUFFOR, sizeof(char *));

    for (int i = nBuff; i < allocatedBuffers; i++)
    {
        dataBuffers[i] = alocateNewMemmory(dataBuffers[i], 1024, sizeof(char));
    }

    saveData(&tm, data, dataBuffers[nBuff]);
    nBuff++;
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
    else if (data->nTags == 0 && (!strcmp(name, "status") || !strcmp(name, "parametr") || !strcmp(name, "stezenie")))
    {
        data->nTags = 0;
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
    else if (data->nTags != 0 && valueInArray(name))
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
    if (data->nTags > 0)
    {
        data->nTags--;
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
            for (int i = 0; i < nBuff; i++)
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
            nBuff = 0;
        }
    }

    freeMemmory(dataBuffers, nBuff);
    freeMemmory(data.tags, data.nTags);

    fclose(inputFile);
    fclose(outputFile);

    XML_ParserFree(parser);
    return EXIT_SUCCESS;
}
