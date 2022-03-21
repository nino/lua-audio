#include <lauxlib.h>
#include <lua.h>
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
ma_device_config *playback_device_config = NULL;

#pragma mark ERRORS

void panic(const char *msg) {
    printf("Error: %s\n", msg);
    exit(1);
}

#pragma mark SAMPLE HANDLING

void push_sample(const char *filepath) {
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

    ma_result result = ma_decoder_init_file(
        filepath, NULL, &loaded_samples[num_loaded_samples - 1]);
    if (result != MA_SUCCESS) {
        panic("Couldn't add decoder");
    }
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
    puts("Seeked to the beginning");
    current_sample = decoder;
    puts("Set the correct sample");
    if (ma_device_start(device) != MA_SUCCESS) {
        printf("Failede to start playback device\n");
        ma_device_uninit(device);
        ma_decoder_uninit(decoder);
        exit(3);
    }
}

void try_playing_wav_file() {
    ma_device_config device_config;

    push_sample("assets/hihat.wav");
    push_sample("assets/hihat2.wav");

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

#pragma mark LUA

enum LUA_TYPE {
    LUA_NIL = 0,
    LUA_STRING = 4,
};

static int l_lib_info(lua_State *L) {
    lua_pushstring(L, "It's working.");
    return 1;
}

static int l_initialize(lua_State *L) {
    // Initialize samples
    int index = 1;
    int next_type;
    while (true) {
        next_type = lua_geti(L, 1, index);
        if (next_type == LUA_NIL) {
            break;
        }
        const char *value = lua_tostring(L, -1);
        push_sample(value);

        lua_remove(L, -1);
        index += 1;
    }

    // Initialize device
    playback_device_config =
        (ma_device_config *)malloc(sizeof(ma_device_config));
    *playback_device_config = ma_device_config_init(ma_device_type_playback);
    playback_device_config->playback.format = loaded_samples[0].outputFormat;
    playback_device_config->playback.channels =
        loaded_samples[0].outputChannels;
    playback_device_config->sampleRate = loaded_samples[0].outputSampleRate;
    playback_device_config->dataCallback = data_callback;

    playback_device = (ma_device *)malloc(sizeof(ma_device));
    if (playback_device == NULL) {
        free_samples();
        panic("Couldn't allocate playback device");
    }

    if (ma_device_init(NULL, playback_device_config, playback_device) !=
        MA_SUCCESS) {
        free_samples();
        free(playback_device);
        panic("Failed to open playback device");
    }

    return 0;
}

static int l_play(lua_State *L) {
    int index = lua_tointeger(L, 1) - 1; // Subtract 1 because Lua is 1-indexed
    play_device(playback_device, &loaded_samples[index]);
    return 0;
}

static const struct luaL_Reg libaudio[] = {
    {"info", l_lib_info},
    {"initialize", l_initialize},
    {"play", l_play},
    {NULL, NULL} /* sentinel */
};

int luaopen_libaudio(lua_State *L) {
    luaL_newlib(L, libaudio);
    return 1;
}

/* int main() { */
/*     printf("This is my audio test\n"); */
/*     try_playing_wav_file(); */
/*     return 0; */
/* } */
