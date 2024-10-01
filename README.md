# XML to CSV Parser

This program parses an XML file and extracts specific data, saving the results in a CSV format. The data parsed includes various parameters related to an "emitor", such as status, values, and concentration.

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

## Usage

1. Compile the program using the following command:
   
     ```bash
     gcc -o expat_example expat_example.c -lexpat
     ```
     
3. Place your XML data in the `example.xml` file.
4. Run the program:

   ```bash
   ./expat_example
   ```

5. The output will be saved to `wyniki.csv` in the same directory.

## Output Format

The CSV output will have the following format:

```
"YYYY-MM-DD","Hour","Emitor.Tags","Value"
```

For example:

```
"2024-10-01","10","K3.status.reka","1012"
"2024-10-01","10","K3.parametr.VSS.wartosc","1167"
"2024-10-01","10","K3.stezenie.CO2.wartosc","1319"
```
