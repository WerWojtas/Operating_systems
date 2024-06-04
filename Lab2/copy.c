#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *input_file, *output_file;
    input_file = fopen("my_copy.txt", "rb");
    output_file = fopen("output.txt", "wb");
    if (input_file == NULL) {
        perror("Cannot open input file");
        return EXIT_FAILURE;
    }

    fseek(input_file, 0, SEEK_END); // ustawianie wskaźnika na koniec pliku
    long fileSize = ftell(input_file); // odczytywanie długości
    rewind(input_file); // ustawianie wskaźnika na początek pliku

    char *buffer = (char*)malloc(fileSize);
    if (buffer == NULL) {
        perror("Error in allocating memory");
        fclose(input_file);
        fclose(output_file);
        return EXIT_FAILURE;
    }

    fread(buffer, 1, fileSize, input_file); // odczytywanie pliku do bufora

    for (int i = fileSize - 1; i >= 0; i--) {
        fwrite(&buffer[i], 1, 1, output_file); // zapisywanie pliku z bufora w odwrotnej kolejności
    }

    free(buffer);
    fclose(input_file);
    fclose(output_file);

    return EXIT_SUCCESS;
}
