
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
#endif

#include "cartridge_io.h"

int count_games() {
    int count = 0;
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFileA("games\\*.tb.png", &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            count++;
        } while (FindNextFileA(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#else
    DIR *dir = opendir("./games/");
    if (dir == NULL) {
        perror("opendir");
        return 0;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len >= 7 && strcmp(name + len - 7, ".tb.png") == 0) {
            count++;
        }
    }
    closedir(dir);
#endif
    return count;
}

void load_game_by_index(int index) {
    char filepath[256] = {0};
    int count = 0;

#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFileA("games\\*.tb.png", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    bool found = false;
    do {
        if (count == index) {
            snprintf(filepath, sizeof(filepath), "games\\%s", findFileData.cFileName);
            found = true;
            break;
        }
        count++;
    } while (FindNextFileA(hFind, &findFileData) != 0);
    FindClose(hFind);

    if (!found) {
        return;
    }
#else
    DIR *dir = opendir("./games/");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    bool found = false;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len >= 7 && strcmp(name + len - 7, ".tb.png") == 0) {
            if (count == index) {
                snprintf(filepath, sizeof(filepath), "./games/%s", name);
                found = true;
                break;
            }
            count++;
        }
    }
    closedir(dir);

    if (!found) {
        printf("Game index %d not found\n", index);
        return;
    }
#endif

    load_game(filepath);
}
