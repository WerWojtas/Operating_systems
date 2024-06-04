#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int main() {
    DIR *dir;
    struct dirent *entry;
    struct stat buf;
    long long total_size = 0;

    dir = opendir("."); // otwieranie aktualnego katalogu
    if (dir == NULL) {
        perror("Cannot open this directory");
        return EXIT_FAILURE;
    }

    entry = readdir(dir); 
    while (entry != NULL) { // odczytywanie plików z katalogu
        if (stat(entry->d_name, &buf) == -1) { // zapisywanie statystyk pliku do buf
            perror("Cannot get file stats");
            continue;
        }

        if (!S_ISDIR(buf.st_mode)) { // printowanie rozmiaru jeśli plik nie jest katalogiem
            printf("%lld %s\n", (long long)buf.st_size, entry->d_name);
            total_size += buf.st_size;
        }
        entry = readdir(dir);
    }

    closedir(dir);

    printf("Size of files in bytes: %lld bytes\n", total_size);

    return EXIT_SUCCESS;
}
