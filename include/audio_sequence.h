#ifndef AUDIO_SEQUENCE_H
#define AUDIO_SEQUENCE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define NUM_TRACKS 8
#define SAMPLE_RATE 44100

#define INIT_AUDIO(_aud_) extern unsigned char media_##_aud_##_raw[]; extern size_t media_##_aud_##_raw_len;
#define MAKE_AUDIO(_aud_, _vol_) createAudio((int16_t*) media_##_aud_##_raw, media_##_aud_##_raw_len / sizeof(int16_t), _vol_)
#define MAKE_SILENCE(_time_) createAudio(NULL, (size_t) (_time_ * SAMPLE_RATE), 0.0)

struct audio_file
{
    int16_t *audio_samples;
    size_t index;
    size_t audio_len;
    float volume;
};

struct audio_sequence
{
    struct audio_file audios[16];
    size_t index;
    size_t len;
    bool isRunning;
};

struct audio_file createAudio(int16_t *samples, size_t audio_len, float volume);

// Returns true if adding the track was successful
bool addTrack(struct audio_sequence* seq);

// Adds 'buflen' stereo samples to buf (i.e. buflen / 2 distinct samples); 'buflen' must be even
// Also advances the index of the corresponding sequence
// Returns true if the track is over
bool dumpMusic(struct audio_sequence* seq, int16_t *buf, size_t buflen);

// Dumps all the tracks onto buf
// Essentially, calls dumpMusic for each track
// The index for each track moves up
void dumpAllTracks(int16_t *buf, size_t buflen);


void debugTracks(void);

#endif