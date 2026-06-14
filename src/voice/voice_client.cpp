#include "voice_client.hpp"
#include <iostream>

VoiceClient::VoiceClient() : pcmBuffer(FRAME_SIZE) {
    // Initialization is done in initialize() to allow error handling
}

VoiceClient::~VoiceClient() {
    shutdown();
}

bool VoiceClient::initialize() {
    // 1. Initialize SDL Audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL Audio initialization error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 2. Define audio format (48kHz, Mono, 32-bit Float)
    SDL_AudioSpec audioSpec;
    audioSpec.freq = SAMPLE_RATE;
    audioSpec.format = SDL_AUDIO_F32;
    audioSpec.channels = CHANNELS;

    // 3. Open audio streams (Microphone and Speakers)
    micStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &audioSpec, nullptr, nullptr);
    speakerStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, nullptr, nullptr);

    if (!micStream || !speakerStream) {
        std::cerr << "SDL Audio stream opening error: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }

    // 4. Start audio devices
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(micStream));
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(speakerStream));

    // 5. Initialize Opus codecs
    int error;
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
    if (error != OPUS_OK) {
        std::cerr << "Opus encoder error: " << opus_strerror(error) << std::endl;
        shutdown();
        return false;
    }

    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK) {
        std::cerr << "Opus decoder error: " << opus_strerror(error) << std::endl;
        shutdown();
        return false;
    }

    return true;
}

void VoiceClient::shutdown() {
    if (micStream) {
        SDL_DestroyAudioStream(micStream);
        micStream = nullptr;
    }
    if (speakerStream) {
        SDL_DestroyAudioStream(speakerStream);
        speakerStream = nullptr;
    }
    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
    }
    if (decoder) {
        opus_decoder_destroy(decoder);
        decoder = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool VoiceClient::recordAndEncode(std::vector<uint8_t>& outEncodedPacket) {
    if (!isInitialized()) {
        return false;
    }

    // Check if there's enough data in the microphone buffer for a full frame
    if (SDL_GetAudioStreamAvailable(micStream) >= BYTES_PER_FRAME) {
        // Get raw PCM data from microphone
        SDL_GetAudioStreamData(micStream, pcmBuffer.data(), BYTES_PER_FRAME);

        // Prepare buffer for compressed data (1500 bytes is a safe upper bound)
        outEncodedPacket.resize(1500);

        // Encode with Opus
        int compressedBytes = opus_encode_float(
            encoder,
            pcmBuffer.data(),
            FRAME_SIZE,
            outEncodedPacket.data(),
            outEncodedPacket.size()
        );

        if (compressedBytes > 0) {
            outEncodedPacket.resize(compressedBytes);
            return true;
        }
    }
    return false;
}

void VoiceClient::decodeAndPlay(const std::vector<uint8_t>& inEncodedPacket) {
    if (!isInitialized() || inEncodedPacket.empty()) {
        return;
    }

    // Local buffer for decoded PCM (always recover full frame of 960 samples)
    std::vector<float> decodedPcm(FRAME_SIZE);

    // Decode with Opus
    int decodedSamples = opus_decode_float(
        decoder,
        inEncodedPacket.data(),
        inEncodedPacket.size(),
        decodedPcm.data(),
        FRAME_SIZE,
        0  // 0 means we're not using FEC (Forward Error Correction)
    );

    if (decodedSamples > 0) {
        // Push the recovered raw audio directly to the speaker
        SDL_PutAudioStreamData(speakerStream, decodedPcm.data(), decodedSamples * sizeof(float));
    }
}
