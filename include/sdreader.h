#ifndef SD_READER_H
#define SD_READER_H


#include <stdint.h>
#include <stddef.h>

#define AUDIO_MAP_SIZE 32
#define DIRECTORY "/airsnare-media"

struct AudioFileMapEntry
{
    char name[16];
    int16_t *data;
    size_t len;
    size_t hash;
};

struct AudioFileMapEntry *getFileFromName(const char *fname);
void loadAllAudioFiles(void);
void mountSD(void);

#endif