#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_SEQUENCE 34
#define HEADER 9
#define DATA 25
#define ROWS 5
#define COLUMNS 5
#define COLORS_CHAR 10

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

int main(int argc, char *argv[])
{

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
        while (!feof(fp))
        {
            int *sequence = create_sequence(fp);
            if (sequence != NULL)
            {
                int *header = create_header(sequence);
                int **data = create_data(sequence);

                int valid = validate_sequence(sequence, data);
                int ID = compute_id(data, valid);
                char *color = get_color(ID);

                FILE *fp2 = fopen("rfid_data_1.out", "a");
                if (fp2 == NULL)
                {
                    printf("Error: Could not open %s for writing.\n", "rfid_data_1.out");
                    exit(3);
                }
                output_sequence(fp2, sequence, line_count);
                output_header(fp2, header);
                output_data(fp2, data);
                output_id(fp2, ID);
                output_color(fp2, color);
                free(color);

                free_arrays(sequence, header, data);
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

int validate_sequence(int *seq, int **data)
{
    if (validate_header(seq) && validate_data(data))
        return 1;
    else
        return 0;
}

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

void output_sequence(FILE *fp2, int *seq, int seq_num)
{

    fprintf(fp2, "Sequence %d: ", seq_num);
    for (int i = 0; i < MAX_SEQUENCE; i++)
    {
        fprintf(fp2, "%d ", seq[i]);
    }
    fprintf(fp2, "\n");
}
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
void output_id(FILE *fp2, int ID)
{

    fprintf(fp2, "\tValue:\n");
    fprintf(fp2, "\t\t%d\n", ID);
}
void output_color(FILE *fp2, char *color)
{
    fprintf(fp2, "\tColor:\n");
    fprintf(fp2, "\t\t%s\n\n", color);
}

void free_data(int **data)
{

    for (int i = 4; i <= 0; i--)
    {

        free(data[i]);
    }
    free(data);
}

void free_arrays(int *seq, int *header, int **data)
{

    free(seq);
    free(header);
    free_data(data);
}