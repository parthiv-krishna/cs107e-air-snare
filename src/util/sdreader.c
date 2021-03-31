#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ff.h"
#include "printf.h"
#include "malloc.h"
#include "strings.h"
#include "sdreader.h"

static struct AudioFileMapEntry fileMap[AUDIO_MAP_SIZE];

static unsigned int hashString(const char *str)
{
    const unsigned int seed = 314159;
    unsigned int result = 27182818;
    while (*str)
    {
        result = result * seed + *str++;
    }
    return result;
}

static bool insertIntoMap(const struct AudioFileMapEntry* f)
{
    unsigned int modHash = f->hash % AUDIO_MAP_SIZE;
    for (size_t i = 0; i < AUDIO_MAP_SIZE; ++i) {
        size_t index = (i + modHash) % AUDIO_MAP_SIZE;
        if (fileMap[index].name == NULL) {
            fileMap[index] = *f;
            return true;
        }
        else if (fileMap[index].hash == f->hash && strcmp(fileMap[index].name, f->name) == 0) {
            return false;
        }
    }
    return false;
}

struct AudioFileMapEntry *getFileFromName(const char *fname)
{
    unsigned int hash = hashString(fname);
    unsigned int modHash = hash % AUDIO_MAP_SIZE;
    for (size_t i = 0; i < AUDIO_MAP_SIZE; ++i) {
        size_t index = (i + modHash) % AUDIO_MAP_SIZE;
        if (fileMap[index].name == NULL)
            break;
        else if (fileMap[index].hash == hash && strcmp(fileMap[index].name, fname) == 0)
            return &fileMap[index];
    }
    printf("Warning: could not find table entry for %s\n", fname);
    return NULL;
}

static bool loadAudioFile(FILINFO *fi)
{
    FRESULT res;
    unsigned int size = fi->fsize;

    FIL fp;
    res = f_open(&fp, fi->fname, FA_READ);
    if (res != FR_OK)
        return false;

    // Dump the file contents into data
    char *data = malloc(size);
    unsigned int nread;
    res = f_read(&fp, data, size, &nread);
    if (res != FR_OK || nread != size)
        return false;

    struct AudioFileMapEntry f;
    memcpy(f.name, fi->fname, sizeof(fi->fname));
    f.hash = hashString(f.name);
    f.data = (int16_t *)data;
    f.len = size / sizeof(int16_t);

    f_close(&fp);

    return insertIntoMap(&f);
}


void mountSD(void) {
    FATFS fs;
    FRESULT res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        printf("Could not mount SD card.\n");
        return;
    }
}

void loadAllAudioFiles(void)
{
    FRESULT res;
    DIR dir;
    
    res = f_opendir(&dir, DIRECTORY);
    if (res != FR_OK) {
        printf("Could not open %s directory. Make sure to run download.c\n", DIRECTORY);
        return;
    }

    FILINFO fi;
    f_chdir(DIRECTORY);
    for (;;) {
        res = f_readdir(&dir, &fi);
        if (res != FR_OK) {
            printf("Read dir error\n");
            return;
        }
        if (fi.fname[0] == 0)
            break;

        printf("Loading audio file %s\n", fi.fname);
        if (!loadAudioFile(&fi))
            printf("An error occured with %s.\n"
                   "Make sure the file exists and the table size is large enough.\n",
                   fi.fname);
    }
}
