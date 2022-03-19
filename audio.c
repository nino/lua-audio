#include <stdio.h>
#include <stdlib.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

const size_t INITIAL_SAMPLES_CAPACITY = 16;

ma_decoder *loaded_samples = NULL;
size_t num_loaded_samples = 0;
size_t samples_capacity = 0;
ma_decoder *current_sample = NULL;
ma_device *playback_device = NULL;

#pragma mark ERRORS

void panic(const char *msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

#pragma mark SAMPLE HANDLING

void push_sample(ma_decoder sample) {
    num_loaded_samples += 1;

    if (loaded_samples == NULL) {
        samples_capacity = INITIAL_SAMPLES_CAPACITY;
        loaded_samples =
            (ma_decoder *)malloc(INITIAL_SAMPLES_CAPACITY * sizeof(ma_decoder));
        if (loaded_samples == NULL) {
            panic("Can't allocate samples cache");
        }
    }

    if (num_loaded_samples >= samples_capacity) {
        samples_capacity *= 2;
        loaded_samples = (ma_decoder *)realloc(
            loaded_samples, samples_capacity * sizeof(ma_decoder));
        if (loaded_samples == NULL) {
            panic("Failed to grow samples cache");
        }
    }

    loaded_samples[num_loaded_samples - 1] = sample;
}

void free_device() {
    if (playback_device != NULL) {
        ma_device_uninit(playback_device);
        free(playback_device);
        playback_device = NULL;
    }
}

void free_samples() {
    if (loaded_samples != NULL) {
        for (size_t i = 0; i < num_loaded_samples; i += 1) {
            ma_decoder_uninit(&loaded_samples[i]);
        }

        free(loaded_samples);
        loaded_samples = NULL;
    }
}

void free_all() {
    free_device();
    free_samples();
}

#pragma mark PLAYBACK

void data_callback(ma_device *device, void *output, const void *input,
                   ma_uint32 framecount) {
    ma_decoder_read_pcm_frames(current_sample, output, framecount, NULL);
}

ma_decoder decoder_with_file(const char *filepath) {
    ma_decoder decoder;

    ma_result result = ma_decoder_init_file(filepath, NULL, &decoder);
    if (result != MA_SUCCESS) {
        panic("Couldn't initialize decoder");
    }

    return decoder;
}

void play_device(ma_device *device, ma_decoder *decoder) {
    ma_decoder_seek_to_pcm_frame(decoder, 0);
    current_sample = decoder;
    if (ma_device_start(device) != MA_SUCCESS) {
        printf("Failede to start playback device\n");
        ma_device_uninit(device);
        ma_decoder_uninit(decoder);
        exit(3);
    }
}

void try_playing_wav_file() {
    ma_device_config device_config;

    push_sample(decoder_with_file("assets/hihat.wav"));
    push_sample(decoder_with_file("assets/hihat2.wav"));

    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format = loaded_samples[0].outputFormat;
    device_config.playback.channels = loaded_samples[0].outputChannels;
    device_config.sampleRate = loaded_samples[0].outputSampleRate;
    device_config.dataCallback = data_callback;

    playback_device = (ma_device *)malloc(sizeof(ma_device));
    if (playback_device == NULL) {
        free_samples();
        panic("Couldn't allocate playback device");
    }

    if (ma_device_init(NULL, &device_config, playback_device) != MA_SUCCESS) {
        free_samples();
        free(playback_device);
        panic("Failed to open playback device");
    }

    char a;
    for (int i = 0; a != 'q'; i += 1) {
        printf("Press return to play\n");
        if (i % 2 == 0) {
            play_device(playback_device, &loaded_samples[0]);
        } else {
            play_device(playback_device, &loaded_samples[1]);
        }
        a = getchar();
    }

    printf("Press return to quit\n");
    getchar();
    getchar();

    free_all();
}

int main() {
    printf("This is my audio test\n");
    try_playing_wav_file();
    return 0;
}
