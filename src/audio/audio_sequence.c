#include "audio_sequence.h"
#include "printf.h"

struct audio_sequence all_tracks[NUM_TRACKS];

struct audio_file createAudio(int16_t *samples, size_t audio_len, float volume)
{
    // printf("Creating audio with samples at %p, audio_len=%d, volue=%d%%\n", samples, audio_len, (int) (volume * 100));
    struct audio_file f;
    f.audio_samples = samples;
    f.audio_len = audio_len;
    f.volume = volume;
    f.index = 0;
    return f;
}

bool addTrack(struct audio_sequence* seq)
{
    for (size_t i = 0; i < NUM_TRACKS; ++i) {
        if (!all_tracks[i].isRunning) {
            seq->isRunning = false;  // Make sure that we don't do anything thread-unsafe; the track won't play until after the copy
            all_tracks[i] = *seq;    // Copy onto all_tracks[i]
            all_tracks[i].isRunning = true;
            return true;
        }
    }
    return false;
}

static int16_t clamp(int32_t sample) {
    if (sample > INT16_MAX) return INT16_MAX;
    else if (sample < INT16_MIN) return INT16_MIN;
    else return (int16_t) sample;
}

bool dumpMusic(struct audio_sequence* seq, int16_t *buf, size_t buflen)
{
    for (size_t i = 0; i < buflen; i += 2) {
        if (seq->index < seq->len) {
            struct audio_file* aud = &seq->audios[seq->index];
            if (aud->audio_samples != NULL) {
                int32_t sample = (int32_t)buf[i] + aud->audio_samples[aud->index] * aud->volume;
                buf[i] = buf[i+1] = clamp(sample);
            }
            if (++aud->index == aud->audio_len) {
                seq->index++;
            }
        } else return true;
    }
    return seq->index >= seq->len;
}

void dumpAllTracks(int16_t *buf, size_t buflen)
{
    for (size_t i = 0; i < buflen; ++i) buf[i] = 0;
    for (size_t i = 0; i < NUM_TRACKS; ++i) {
        if (all_tracks[i].isRunning && dumpMusic(&all_tracks[i], buf, buflen)) {
            all_tracks[i].isRunning = false;
        }
    }
}

void debugTracks(void)
{
    printf("\n\n\n");
    for (size_t i = 0; i < NUM_TRACKS; ++i) {
        if (all_tracks[i].isRunning) {
            printf("Track %d: index=%d len=%d audios[0]=%p", i, all_tracks[i].index, all_tracks[i].len, all_tracks[i].audios[0].audio_samples);
            printf("\n");
        }
    }
}

