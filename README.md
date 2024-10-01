# XML to CSV Parser

This program parses an XML file and extracts specific data, saving the results in a CSV format. The data parsed includes various parameters related to an "emitor", such as status, values, and tags.

## Features

- Reads an XML file and extracts data related to the "emitor", "status", and other relevant tags.
- Outputs extracted data to a CSV file with a specified format.
- Uses the Expat library for efficient XML parsing.

## Requirements

- **C Compiler**: Ensure you have a C compiler installed (e.g., `gcc`).
- **Expat Library**: This program uses the Expat XML parsing library. Make sure it's installed on your system. You can usually install it via your package manager:
  - For Ubuntu: `sudo apt-get install libexpat1-dev`
  - For Fedora: `sudo dnf install expat-devel`

## File Structure

- **example.xml**: Input XML file containing the data to be parsed.
- **wyniki.csv**: Output CSV file where the parsed data will be saved.

## Installation

1. Clone the repository (if applicable) or create a directory for the project.
2. Ensure you have the Expat library installed.
3. Compile the program using the following command:

   ```bash
   gcc -o xml_to_csv xml_to_csv.c -lexpat
   ```

## Usage

1. Place your XML data in the `example.xml` file.
2. Run the program:

   ```bash
   ./xml_to_csv
   ```

3. The output will be saved to `wyniki.csv` in the same directory.

## Output Format

The CSV output will have the following format:

```
"YYYY-MM-DD","Hour","Emitor.Tag","Value"
```

For example:

```
"2024-09-01","1","K3.Status_Automatyczny","1013"
"2024-09-01","1","K3.Parametr.VSS.Wartosc","1167"
```
