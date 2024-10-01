#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>
#include <time.h>

#define INPUT_FILENAME  "example.xml"
#define OUTPUT_FILENAME "wyniki.csv"
#define ELEMENTS        30

/*
 * Struktura parsowanych danych
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
    time_t t = time(NULL);      // Zmienna czasu systemowego
    struct tm tm = *localtime(&t);
    char output[300];

    saveData(&tm, buffers[n_buff]);
    n_buff++;
}

/**
 * @brief   Funkcja wywoływana, gdy parser napotyka początek elementu XML.
 *          Funkcja obsługuje początkowy element XML, takie jak 'status', 'wartosc' czy 'stezenie'. 
 *          Przypisuje wartości atrybutów do struktury danych, które później zostaną zapisane.
 *          Zwiększa licznik zebranych danych po znalezieniu odpowiedniego elementu.
 * @param userData  Wskaźnik na dane użytkownika (może być użyty do przekazania dodatkowych informacji do parsera, np. pliku wynikowego).
 * @param name      Nazwa aktualnie przetwarzanego elementu XML.
 * @param attr      Tablica atrybutów elementu XML (naprzemiennie nazwa i wartość atrybutu).
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
 * @brief   Funkcja wywoływana, gdy parser napotyka koniec elementu XML.
 * @param userData  Wskaźnik na dane użytkownika (przekazywany z parsera).
 * @param name      Nazwa aktualnie zakończonego elementu XML.
 */
void XMLCALL endElement(void *userData, const char *name) {
    if(data.n_tags > 0)
        data.n_tags--;
}

/**
 * @brief   Funkcja wywoływana, gdy parser napotyka dane tekstowe w elemencie XML.
 * @param userData  Wskaźnik na dane użytkownika (przekazywany z parsera).
 * @param s         Wskaźnik na dane tekstowe (ciąg znaków) znajdujące się w elemencie XML.
 * @param len       Długość danych tekstowych.
 */
void XMLCALL characterData(void *userData, const XML_Char *s, int len) {
    // TODO
}

int main() {
    

    /*
     * Obsługa zewnętrznych plików XML i CSV.
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

    // Inicjalizacja parsera
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser, characterData);

    // Parsowanie pliku XML
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
