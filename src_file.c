#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Define constants for program parameters
#define MAX_SEQUENCE 34 // Maximum length of the sequence data
#define HEADER 9        // Number of elements in the header
#define DATA 25         // Number of elements in the data (excluding parity bits)
#define ROWS 5          // Number of rows in the data matrix
#define COLUMNS 5       // Number of columns in the data matrix
#define COLORS_CHAR 10  // Maximum length (characters) for the color name
#define MAX_FILENAME_LEN 50

// Function prototypes
int *create_sequence(FILE *fp);
int *create_header(int *seq);
int **create_data(int *seq);

int validate_row(int **data, int row);
int validate_column(int **data, int column);
int validate_header(int *seq);
int validate_data(int **data);
int validate_sequence(int *seq, int **data);

int compute_id(int **data, int valid);
char *get_color(int id);

void output_sequence(FILE *fp2, int *seq, int seq_num);
void output_header(FILE *fp2, int *header);
void output_data(FILE *fp2, int **data);
void output_id(FILE *fp2, int ID);
void output_color(FILE *fp2, char *color);

void free_data(int **data);
void free_arrays(int *seq, int *header, int **data);

void create_output_filename(char *input_filename, char output_filename[MAX_FILENAME_LEN]);

int main(int argc, char *argv[])
{

    // Check for correct number of arguments
    if (argc != 2)
    {
        printf("Error: You must specify an input file name: %s <input file>\n", argv[0]);
        exit(1);
    }
    else
    {

        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            printf("Error: Could not open %s for reading.\n", argv[1]);
            exit(2);
        }

        int line_count = 1;

        char output_filename[MAX_FILENAME_LEN];
        create_output_filename(argv[1], output_filename);

        while (!feof(fp)) // Loop until end of file (feof)
        {
            int *sequence = create_sequence(fp);
            if (sequence != NULL) // Check if sequence was read successfully
            {
                int *header = create_header(sequence);
                int **data = create_data(sequence);

                int valid = validate_sequence(sequence, data);
                int ID = compute_id(data, valid);
                char *color = get_color(ID);

                // Open output file in append mode ("a")
                FILE *fp2 = fopen(output_filename, "a");
                if (fp2 == NULL)
                {
                    printf("Error: Could not open %s for writing.\n", output_filename);
                    exit(3);
                }
                output_sequence(fp2, sequence, line_count);
                output_header(fp2, header);
                output_data(fp2, data);
                output_id(fp2, ID);
                output_color(fp2, color);
                free(color);

                free_arrays(sequence, header, data); // Free memory before next iteration
                fclose(fp2);

                line_count++;
            }
        }
        fclose(fp);

        printf("Decoding complete. Written to file 'rfid_data_1.out'.\n");
        return 0;
    }

    return 0;
}

// function definitions
// function for creating sequence
int *create_sequence(FILE *fp)
{

    char buffer[100];
    fgets(buffer, sizeof(buffer), fp);

    int *data = calloc(MAX_SEQUENCE, sizeof(int));
    if (data == NULL)
    {
        printf("Error: Out of memory.\n");
        exit(4);
    }

    char *temp = strtok(buffer, " ");

    int i = 0;
    while (temp != NULL)
    {
        data[i] = atoi(temp);
        temp = strtok(NULL, " ");
        i++;
    }
    return data;
}

// functions for creating header
int *create_header(int *seq)
{

    int *header = calloc(HEADER, sizeof(int));
    if (header == NULL)
    {
        printf("Error: Out of memory.\n");
        exit(4);
    }

    for (int i = 0; i < HEADER; i++)
    {
        header[i] = seq[i];
    }
    return header;
}

// functions for creating data
int **create_data(int *seq)
{

    int **data = calloc(ROWS, sizeof(int *));
    if (data == NULL)
    {
        printf("Error: Out of memory.\n");
        exit(4);
    }
    for (int i = 0; i < ROWS; i++)
    {
        data[i] = calloc(COLUMNS, sizeof(int));
        if (data[i] == NULL)
        {
            printf("Error: Out of memory.\n");
            exit(4);
        }
    }

    for (int i = HEADER; i < MAX_SEQUENCE; i++)
    {
        int row = (i - HEADER) / COLUMNS;
        int col = (i - HEADER) % COLUMNS;
        data[row][col] = seq[i];
    }
    return data;
}

// validation functions
// function for sequence validation
int validate_sequence(int *seq, int **data)
{
    if (validate_header(seq) && validate_data(data))
        return 1;
    else
        return 0;
}

// function for header validation
int validate_header(int *seq)
{

    for (int i = 0; i < HEADER; i++)
    {
        if (seq[i] != 1)
        {
            return 0;
        }
    }
    return 1;
}

// function for data validation
int validate_data(int **data)
{

    if (data[ROWS - 1][COLUMNS - 1] != 0)
    {
        return 0;
    }
    else
    {
        for (int i = 0; i < ROWS - 1; i++)
        {
            if (validate_row(data, i) && validate_column(data, i))
                return 1;
            else
                return 0;
        }
    }
};

// function for validation of row and column
int validate_row(int **data, int row)
{
    int sum = 0;
    for (int i = 0; i < COLUMNS - 1; i++)
    {
        sum += data[row][i];
    }
    int calculated_parity = sum % 2;
    if (data[row][COLUMNS - 1] == calculated_parity)
        return 1;
    else
        return 0;
}

// function for validation of column
int validate_column(int **data, int column)
{

    int sum = 0;
    for (int i = 0; i < ROWS - 1; i++)
    {
        sum += data[i][column];
    }
    int calculated_parity = sum % 2;
    if (data[ROWS - 1][column] == calculated_parity)
        return 1;
    else
        return 0;
}

// function for computing ID
int compute_id(int **data, int valid)
{
    if (valid)
    {
        int id = 0;
        for (int i = 0; i < 16; i++)
        {
            int row = i / 4;
            int col = i % 4;
            int p = (15 - i);

            id += data[row][col] * pow(2, p);
        }
        return id;
    }
    else
    {
        return -1;
    }
}

// function for getting color
char *get_color(int id)
{

    char *color = calloc(COLORS_CHAR + 1, sizeof(char));
    if (color == NULL)
    {
        printf("Error: Out of memory.\n");
        exit(4);
    }
    color[COLORS_CHAR] = '\0';
    switch (id)
    {
    case 3072:
        strcpy(color, "White");
        break;
    case 3076:
        strcpy(color, "Green");
        break;
    case 3083:
        strcpy(color, "Yellow");
        break;
    case 3085:
        strcpy(color, "Red");
        break;
    case 3086:
        strcpy(color, "Blue");
        break;
    case 3087:
        strcpy(color, "Purple");
    case 3123:
        strcpy(color, "Black");
        break;
    default:
        strcpy(color, "Invalid");
        break;
    }
    return color;
}

// functions for outputting
void output_sequence(FILE *fp2, int *seq, int seq_num)
{

    fprintf(fp2, "Sequence %d: ", seq_num);
    for (int i = 0; i < MAX_SEQUENCE; i++)
    {
        fprintf(fp2, "%d ", seq[i]);
    }
    fprintf(fp2, "\n");
}

// function for outputting header
void output_header(FILE *fp2, int *header)
{

    fprintf(fp2, "\tHeader: \n");
    for (int i = 0; i < HEADER; i++)
    {
        if (i == 0)
        {
            fprintf(fp2, "\t\t%d ", header[i]);
        }
        else
        {
            fprintf(fp2, "%d ", header[i]);
        }
    }
    fprintf(fp2, "\n");
}

// function for outputting data
void output_data(FILE *fp2, int **data)
{

    fprintf(fp2, "\tData: \n");
    for (int i = 0; i < ROWS + 1; i++)
    {
        fprintf(fp2, "\t\t");
        for (int j = 0; j < COLUMNS; j++)
        {
            if (i == 4)
            {
                if (j == 4)
                    fprintf(fp2, "%s", "|--");
                else
                    fprintf(fp2, "%s", "--");
            }
            else if (i == 5)
            {
                if (j == 4)
                    fprintf(fp2, "| %d ", data[i - 1][j]);
                else
                    fprintf(fp2, "%d ", data[i - 1][j]);
            }
            else
            {
                if (j == 4)
                    fprintf(fp2, "| %d ", data[i][j]);
                else
                    fprintf(fp2, "%d ", data[i][j]);
            }
        }
        fprintf(fp2, "\n");
    }
}

// function for outputting ID
void output_id(FILE *fp2, int ID)
{

    fprintf(fp2, "\tValue:\n");
    fprintf(fp2, "\t\t%d\n", ID);
}

// function for outputting color
void output_color(FILE *fp2, char *color)
{
    fprintf(fp2, "\tColor:\n");
    fprintf(fp2, "\t\t%s\n\n", color);
}

// function for freeing arrays
void free_arrays(int *seq, int *header, int **data)
{

    free(seq);
    free(header);
    free_data(data);
}

// function for freeing data
void free_data(int **data)
{

    for (int i = 4; i <= 0; i--)
    {

        free(data[i]);
    }
    free(data);
}

void create_output_filename(char *input_filename, char output_filename[MAX_FILENAME_LEN])
{
    int len = strlen(input_filename);

    char *dot = strchr(input_filename, '.');
    if (dot != NULL)
    {
        strncpy(output_filename, input_filename, dot - input_filename);
    }
    else
    {
        strncpy(output_filename, input_filename, len);
    }

    strncat(output_filename, ".out", MAX_FILENAME_LEN - 1);
    output_filename[MAX_FILENAME_LEN - 1] = '\0';
}
