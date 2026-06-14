#include "voice_client.hpp"
#include <iostream>
#include <chrono>
#include <cstring>

VoiceClient::VoiceClient() : pcmBuffer(FRAME_SIZE), playbackBuffer_(FRAME_SIZE) {
    // Initialization is done in initialize() to allow error handling
}

VoiceClient::~VoiceClient() {
    // Use stop() which properly signals the thread and waits
    stop();
    
    // Then shutdown audio resources
    shutdown();
}

bool VoiceClient::initialize() {
    // 1. Initialize SDL Audio subsystem (must be called from main thread)
    // Note: SDL_InitSubSystem returns true if already initialized or if initialization succeeded
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            std::cerr << "SDL Audio initialization error: " << SDL_GetError() << std::endl;
            return false;
        }
    }

    // 2. Define our desired audio format (48kHz, Mono, 32-bit Float)
    SDL_AudioSpec desiredSpec;
    desiredSpec.format = SDL_AUDIO_F32;
    desiredSpec.channels = CHANNELS;
    desiredSpec.freq = SAMPLE_RATE;

    // 3. Open microphone for capture
    micDeviceId_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &desiredSpec);
    if (micDeviceId_ == 0) {
        std::cerr << "SDL Microphone opening error: " << SDL_GetError() << std::endl;
        std::cerr << "Note: No microphone device available. Voice chat will not work." << std::endl;
        return false;
    }

    // 4. Open speaker for playback
    speakerDeviceId_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desiredSpec);
    if (speakerDeviceId_ == 0) {
        std::cerr << "SDL Speaker opening error: " << SDL_GetError() << std::endl;
        std::cerr << "Note: No speaker device available. Voice chat will not work." << std::endl;
        SDL_CloseAudioDevice(micDeviceId_);
        micDeviceId_ = 0;
        return false;
    }

    // 5. Get the actual device formats
    SDL_AudioSpec micDeviceSpec, speakerDeviceSpec;
    int micSampleFrames, speakerSampleFrames;
    if (!SDL_GetAudioDeviceFormat(micDeviceId_, &micDeviceSpec, &micSampleFrames) ||
        !SDL_GetAudioDeviceFormat(speakerDeviceId_, &speakerDeviceSpec, &speakerSampleFrames)) {
        std::cerr << "SDL Get device format error: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }

    // 6. Create audio streams for capture and playback
    // For mic: convert from device format to our desired format
    // For speaker: convert from our desired format to device format
    micStream = SDL_CreateAudioStream(&micDeviceSpec, &desiredSpec);
    speakerStream = SDL_CreateAudioStream(&desiredSpec, &speakerDeviceSpec);
    
    if (!micStream || !speakerStream) {
        std::cerr << "SDL Audio stream creation error: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }

    // 7. Bind streams to devices
    // In SDL3, devices start unpaused, so binding a stream will start audio flowing
    if (!SDL_BindAudioStream(micDeviceId_, micStream)) {
        std::cerr << "SDL Bind mic stream error: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }
    if (!SDL_BindAudioStream(speakerDeviceId_, speakerStream)) {
        std::cerr << "SDL Bind speaker stream error: " << SDL_GetError() << std::endl;
        shutdown();
        return false;
    }

    // 8. Initialize Opus codecs
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
    // Stop the voice thread first (if running)
    // Note: stop() already unbinds streams, so we don't need to do it again here
    stop();
    
    // Destroy streams (already unbound by stop())
    if (micStream) {
        SDL_DestroyAudioStream(micStream);
        micStream = nullptr;
    }
    if (speakerStream) {
        SDL_DestroyAudioStream(speakerStream);
        speakerStream = nullptr;
    }
    
    // Close audio devices
    if (micDeviceId_ != 0) {
        SDL_CloseAudioDevice(micDeviceId_);
        micDeviceId_ = 0;
    }
    if (speakerDeviceId_ != 0) {
        SDL_CloseAudioDevice(speakerDeviceId_);
        speakerDeviceId_ = 0;
    }
    
    // Clean up Opus codecs
    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
    }
    if (decoder) {
        opus_decoder_destroy(decoder);
        decoder = nullptr;
    }
    
    // Note: We don't quit SDL_INIT_AUDIO here because:
    // 1. It was initialized in the main thread
    // 2. Other components might need it
    // 3. SDL_QuitSubSystem should be called from the main thread
    // SDL audio will be properly cleaned up when the application exits
}

bool VoiceClient::recordAndEncode(std::vector<uint8_t>& outEncodedPacket) {
    if (!isInitialized()) {
        return false;
    }

    // Check if there's enough data in the microphone stream for a full frame
    int available = SDL_GetAudioStreamAvailable(micStream);
    if (available >= BYTES_PER_FRAME) {
        // Get raw PCM data from microphone stream
        int got = SDL_GetAudioStreamData(micStream, pcmBuffer.data(), BYTES_PER_FRAME);
        if (got <= 0) {
            return false;
        }

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
        // Queue the decoded audio for playback
        playbackAudioQueue_.push(decodedPcm);
    }
}

// Note: SDL3 uses audio streams with SDL_PutAudioStreamData/SDL_GetAudioStreamData
// We'll use a simpler approach: poll the mic stream in the voice thread
// and put data to speaker stream directly

bool VoiceClient::start(VoicePacketCallback onVoicePacket, std::function<bool()> shouldSendPacket) {
    if (running_) {
        std::cerr << "Voice client is already running!" << std::endl;
        return false;
    }

    if (!isInitialized()) {
        std::cerr << "Voice client not initialized! Call initialize() first in the main thread." << std::endl;
        return false;
    }

    onVoicePacketCallback_ = onVoicePacket;
    shouldSendPacketCallback_ = shouldSendPacket;
    running_ = true;
    
    // Use jthread with stop_token for safe thread interruption
    voiceThread_ = std::jthread([this, stopToken = std::stop_token()]() {
        voiceThreadFunction(stopToken);
    });
    
    std::cout << "Voice client started in thread " << voiceThread_.get_id() << std::endl;
    return true;
}

void VoiceClient::stop() {
    if (!running_) return;

    // Signal the thread to stop FIRST
    // The thread will unbind streams itself when it sees the signal
    std::cout << "[VOICE] Signaling thread to stop..." << std::endl;
    stopRequested_ = true;
    running_ = false;
    
    if (voiceThread_.joinable()) {
        voiceThread_.request_stop();
        
        // Wait for thread to stop with timeout
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        while (voiceThread_.joinable() && 
               std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (voiceThread_.joinable()) {
            std::cerr << "[VOICE] ERROR: Voice thread did not stop within 500ms!" << std::endl;
            std::cerr << "[VOICE] This may cause issues on program exit." << std::endl;
        } else {
            std::cout << "[VOICE] Thread stopped successfully" << std::endl;
        }
    }
    
    std::cout << "Voice client stopped" << std::endl;
}

void VoiceClient::queueVoicePacket(const std::vector<uint8_t>& packet) {
    if (isInitialized() && running_) {
        incomingVoicePackets_.push(packet);
    }
}

void VoiceClient::voiceThreadFunction(std::stop_token stopToken) {
    std::cout << "[VOICE THREAD] Started" << std::endl;
    
    while (!stopToken.stop_requested() && !stopRequested_ && running_) {
        // 1. Process incoming voice packets (decode and queue for playback)
        std::vector<uint8_t> incomingPacket;
        if (incomingVoicePackets_.tryPop(incomingPacket)) {
            decodeAndPlay(incomingPacket);
        }
        
        // 2. Process captured audio (encode and send)
        std::vector<uint8_t> encodedPacket;
        if (recordAndEncode(encodedPacket)) {
            // Only send if we should (e.g., there are other users in the channel)
            if (shouldSendPacketCallback_() && onVoicePacketCallback_) {
                onVoicePacketCallback_(encodedPacket);
            }
        }
        
        // 3. Process playback queue (send to speaker)
        std::vector<float> playbackFrame;
        if (playbackAudioQueue_.tryPop(playbackFrame)) {
            if (speakerStream) {
                SDL_PutAudioStreamData(speakerStream, playbackFrame.data(), playbackFrame.size() * sizeof(float));
            }
        }
        
        // 4. Small sleep to prevent CPU overload
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    // Clean up: Unbind streams in the voice thread itself to avoid race conditions
    std::cout << "[VOICE THREAD] Exiting, unbinding streams..." << std::endl;
    if (micStream) {
        SDL_UnbindAudioStream(micStream);
    }
    if (speakerStream) {
        SDL_UnbindAudioStream(speakerStream);
    }
    std::cout << "[VOICE THREAD] Exited cleanly" << std::endl;
}
