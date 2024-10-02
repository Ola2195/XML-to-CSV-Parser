# XML to CSV Parser

This program parses an XML file and converts specific data into CSV format. It processes data related to an "emitor" and its associated tags such as status, values, and concentrations, storing the results in a CSV file. The program uses the **Expat** library for efficient XML parsing.

## Features

- **XML Parsing**: Extracts data related to "emitor", "status", "parametr", and "stezenie" tags.
- **CSV Output**: Saves parsed data in a CSV file with a formatted timestamp, including emitter tags and their associated values.
- **Expat Library**: Utilizes Expat, a fast and non-validating XML parser, for efficient handling of large XML files.

## Requirements

- **C Compiler**: Ensure you have a C compiler installed (e.g., `gcc`).
- **Expat Library**: This program uses the Expat XML parsing library. Make sure it's installed on your system. You can usually install it via your package manager:
  - For Ubuntu: `sudo apt-get install libexpat1-dev`
  - For Fedora: `sudo dnf install expat-devel`

## File Structure

- **example.xml**: Input XML file containing the data to be parsed.
- **wyniki.csv**: Output CSV file where the parsed data will be saved.

## Compilation

To compile the program, use the following command:

```console
gcc -o expat_example emitor_expat.c -lexpat
```

Ensure that the Expat library is linked correctly, as shown above (`-lexpat`).

## Usage

1. Place your XML data in a file named `example.xml` in the same directory as the compiled program.
2. Run the program:

   ```console
   ./expat_example
   ```

3. The program will generate a CSV file named `wyniki.csv` in the current directory. The CSV will include a timestamp and data from the XML file in the following format:

   ```csv
   "YYYY-MM-DD","Hour","Emitor.Tags","Pkt_Value"
   ```

## Example

For instance, after processing `example.xml`, the `wyniki.csv` file might contain:

```csv
"2024-10-01","10","K3.status.reka","1012"
"2024-10-01","10","K3.parametr.VSS.wartosc","1167"
"2024-10-01","10","K3.stezenie.CO2.wartosc","1319"
```

## Troubleshooting

- **Compilation Issues**: Ensure the Expat development libraries are installed. On Linux, use the provided package manager commands. If you're using macOS or another platform, make sure the appropriate headers are available.
  
- **File Errors**: If the program reports an error opening the XML file (`example.xml`), check the file's location and ensure it's in the same directory as the executable.

## Notes

- The program assumes that the input XML file follows a specific structure where "emitor" elements contain tags and values. Ensure the XML file is correctly formatted for successful parsing.
- The program dynamically allocates memory for storing tags and buffers, ensuring efficient handling of XML files with varying tag structures and data points.
   - **Tag Management**: Tags are allocated in blocks (defined by `ADD_TAG`), and the memory is expanded as more tags are encountered.
   - **Buffer Management**: Buffers for CSV output are allocated dynamically in blocks (defined by `ADD_BUFFOR`), ensuring efficient memory use and avoiding overflow.