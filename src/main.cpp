#define SDL_MAIN_USE_CALLBACKS 1;
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <opus.h>

#include <string>
#include <vector>
// #include <chrono>
#include <format>
#include <iostream>
// #include <cstdint>
#include <thread>

#include "message.hpp"
// #include "ThreadSafeQueue.hpp"
#include "ui/theme.hpp"
#include "ui/atoms.hpp"


/**
 * @brief A way to describe a view state in 1 enum.
 *
 * This doesn't include the data, and that's why ViewState class exists.
 * I'll use this docstring for planning out the initial set of views.
 * TODO: Remove this part of docstring once the initial set of views is implemented. This part of docstring may not be needed by then.
 *
 * LANDING_VIEW:
 *  Whenever you open the app, this is the entry view for the program.
 * CONNECT_TO_NEW_SERVER_VIEW:
 *  Here user may provide data required for a connecting to an already existing server.
 * CREATE_NEW_SERVER_VIEW:
 *  Here user may set up a new server.
 * SERVER_VIEW:
 *  Displays relevant info about server that the user is currently connected to.
 *  Contains channel selection section and chat section (if a text channel has been selected)
 * USER_SETTINGS_VIEW:
 *  Here user should be able to define or configure their profile.
 * SERVER_SETTINGS_VIEW:
 *  Here a server owner will be able to configure server settings.
 */
enum class EViewState {
    AUTH_VIEW,                  // Logowanie i rejestracja
    LANDING_VIEW,               // Ekran startowy / Dashboard
    FRIENDS_LIST_VIEW,          // Lista znajomych i czaty DM (Direct Messages)
    CONNECT_TO_NEW_SERVER_VIEW, // Dołączanie do serwera przez kod/IP
    CREATE_NEW_SERVER_VIEW,     // Tworzenie nowego serwera
    CONNECTING_LOADING_VIEW,    // Ekran ładowania podczas połączenia
    SERVER_VIEW,                // Główny widok serwera (kanały + czat)
    USER_SETTINGS_VIEW,         // Ustawienia profilu / audio
    SERVER_SETTINGS_VIEW,       // Zarządzanie serwerem (role, nazwa)
    ERROR_DISCONNECTED_VIEW     // Błędy sieciowe i utrata połączenia
};


/**
 * @brief This is a container for data regarding a view state.
 *
 * Depending on a EViewState, ViewStateManager will be expecting different fields to be filled with data.
 */
class ViewState {
private:
    EViewState m_EViewState;
public:

};

/**
 * @brief This is supposed to contain what view should the app be showing at a given moment.
 *
 * This should be the sole and only source of truth about the view state.
 */
class ViewStateManager {
private:
    ViewState m_ViewState;
public:

};

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
Theme theme = Theme::createGoldGalaxyTheme();

std::vector<Message> messages = {
    {"Message 1", "User 1"},
    {"Message 2", "User 2"},
    {"Message 3", "User 1"},
    {"Message 4", "User 2"},
    {"Message 5", "User 3"},
    {"Message 6", "User 1"},
    {"Message 7", "User 2"},
};
static char message_buffer[1024] = "";

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // Initializing window and renderer with SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!SDL_CreateWindowAndRenderer("EchoHub", 800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        return SDL_APP_FAILURE;
    }
    // Initiating ImGui
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // Start rendering imgui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // The actual contents of the window
    int main_window_w, main_window_h;
    SDL_GetWindowSize(window, &main_window_w, &main_window_h);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w), static_cast<float>(main_window_h)));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("EchoHubRoot", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w) / 4.0f, static_cast<float>(main_window_h)));
        if (ImGui::Begin("EchoHubNavigation", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Navigation");
        } ImGui::End();
        ImGui::SameLine();
        ImGui::SetNextWindowPos(ImVec2((static_cast<float>(main_window_w) / 4.0f), 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w) / 2.0f, static_cast<float>(main_window_h)));
        if (ImGui::Begin("EchoHubContent", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (Atoms::Button("Zaloguj się", theme, {200, 40})) {
                // Obsługa logowania
            }
            if (ImGui::BeginChild("Messages", ImVec2(0.0f, -60.0f), false)) {
                for (Message& message : messages) {
                    if (ImGui::BeginChild(std::format("Message-{}", message.GetID()).c_str(), ImVec2(0.0f, 50.0f), true)) {
                        ImGui::Text("%s", message.GetAuthor().c_str());
                        ImGui::SameLine();
                        ImGui::Text("%s", message.GetDataTime().c_str());
                        ImGui::Text("%s", message.GetContent().c_str());
                    } ImGui::EndChild();
                }
            }
            ImGui::EndChild();
            if (ImGui::BeginChild("NewMessageInput", ImVec2(0.0f, 45), false)) {
                if (ImGui::InputText("##message_input", message_buffer, IM_ARRAYSIZE(message_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    // Message is being sent
                    SDL_Log("%s", message_buffer);
                    message_buffer[0] = '\0';
                }
                ImGui::SameLine();
                if (ImGui::Button("Send")) {
                    SDL_Log("%s", message_buffer);
                }
            } ImGui::EndChild();
        } ImGui::End();
        ImGui::SameLine();
        ImGui::SetNextWindowPos(ImVec2((static_cast<float>(main_window_w) / 4.0f * 3.0f), 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(main_window_w) / 4.0f, static_cast<float>(main_window_h)));
        if (ImGui::Begin("EchoHubDetails", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Details");
        } ImGui::End();
    } ImGui::End();

    // Rendering background
    SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
    SDL_RenderClear(renderer);

    // Render the ImGui
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    // Display the frame
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    // Passing events to ImGui so the UI can be interacted with
    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  // Exits the app
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

class VoiceClient {
private:
    // --- STAŁE PARAMETRY AUDIO ---
    static constexpr int SAMPLE_RATE = 48000;      // Wymóg Opus dla optymalnego VoIP
    static constexpr int CHANNELS = 1;             // Mono
    static constexpr int FRAME_SIZE = 960;         // 20ms dźwięku przy 48kHz (48000 * 0.02)
    
    // Ilość bajtów na jedną ramkę surowego PCM (960 próbek * 4 bajty na float)
    static constexpr int BYTES_PER_FRAME = FRAME_SIZE * sizeof(float);

    // --- ZASOBY SDL3 (Sprzęt Audio) ---
    SDL_AudioStream* micStream = nullptr;
    SDL_AudioStream* speakerStream = nullptr;

    // --- ZASOBY OPUS (Kodeki) ---
    OpusEncoder* encoder = nullptr;
    OpusDecoder* decoder = nullptr;

    // --- BUFORY ROBOCZE ---
    // Bufor na surowe próbki z mikrofonu (Float32)
    std::vector<float> pcmBuffer;
    
public:
    VoiceClient() {
        // 1. Inicjalizacja podsystemu Audio w SDL3
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            std::cerr << "Błąd inicjalizacji SDL_Audio: " << SDL_GetError() << std::endl;
            return; // W profesjonalnym kodzie warto rzucić wyjątkiem
        }

        // 2. Definicja formatu (48kHz, Mono, 32-bit Float) - taki sam dla obu urządzeń
        SDL_AudioSpec audioSpec;
        audioSpec.freq = SAMPLE_RATE;
        audioSpec.format = SDL_AUDIO_F32;
        audioSpec.channels = CHANNELS;

        // 3. Otwarcie strumieni audio (Mikrofon i Głośniki)
        micStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &audioSpec, nullptr, nullptr);
        speakerStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, nullptr, nullptr);

        if (!micStream || !speakerStream) {
            std::cerr << "Błąd otwarcia strumieni SDL: " << SDL_GetError() << std::endl;
        }

        // Uruchomienie sprzętu (rozpoczęcie nasłuchu i gotowość do odtwarzania)
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(micStream));
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(speakerStream));

        // 4. Inicjalizacja Kodeków Opus
        int error;
        // Koder: optymalizowany pod ludzki głos (VOIP)
        encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
        if (error != OPUS_OK) {
            std::cerr << "Błąd kodera Opus: " << opus_strerror(error) << std::endl;
        }

        // Dekoder: musi znać tylko częstotliwość i ilość kanałów
        decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
        if (error != OPUS_OK) {
            std::cerr << "Błąd dekodera Opus: " << opus_strerror(error) << std::endl;
        }

        // Przygotowanie bufora na wymaganą wielkość ramki
        pcmBuffer.resize(FRAME_SIZE);
    }

    ~VoiceClient() {
        // Sprzątanie pamięci przy niszczeniu obiektu, aby uniknąć wycieków
        if (micStream) SDL_DestroyAudioStream(micStream);
        if (speakerStream) SDL_DestroyAudioStream(speakerStream);
        
        if (encoder) opus_encoder_destroy(encoder);
        if (decoder) opus_decoder_destroy(decoder);
        
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    /**
     * Zbieranie danych z mikrofonu.
     * Wywołuj tę funkcję w pętli głównej programu (np. co klatkę/tik).
     * @param outEncodedPacket Wektor, do którego klasa wrzuci skompresowane dane sieciowe.
     * @return Zwraca true, jeśli zebrano pełną ramkę i ją skompresowano, false jeśli mikrofon wciąż zbiera dane.
     */
    bool RecordAndEncode(std::vector<uint8_t>& outEncodedPacket) {
        // Sprawdzamy, czy w buforze mikrofonu jest już wystarczająco dużo danych na pełną ramkę (20ms)
        if (SDL_GetAudioStreamAvailable(micStream) >= BYTES_PER_FRAME) {
            
            // Pobieramy surowe dane (PCM) do naszego wektora floatów
            SDL_GetAudioStreamData(micStream, pcmBuffer.data(), BYTES_PER_FRAME);
            
            // Przygotowujemy bufor na skompresowane dane. 
            // 1500 bajtów to bezpieczny zapas, pakiety głosowe ważą zazwyczaj < 100 bajtów.
            outEncodedPacket.resize(1500); 

            // Kompresja Opus
            int compressedBytes = opus_encode_float(
                encoder, 
                pcmBuffer.data(), 
                FRAME_SIZE, 
                outEncodedPacket.data(), 
                outEncodedPacket.size()
            );

            if (compressedBytes > 0) {
                // Skracamy wektor do rzeczywistego rozmiaru skompresowanych danych
                outEncodedPacket.resize(compressedBytes);
                return true; 
            }
        }
        return false;
    }

    /**
     * Odtwarzanie danych odebranych z sieci.
     * Wywołaj tę funkcję od razu, gdy dostaniesz przez UDP pakiet głosowy od kogoś z serwera.
     * @param inEncodedPacket Paczka bajtów prosto z sieci (UDP).
     */
    void DecodeAndPlay(const std::vector<uint8_t>& inEncodedPacket) {
        if (inEncodedPacket.empty()) return;

        // Lokaly bufor na zdekodowany sygnał (zawsze odzyskujemy pełną ramkę 960 próbek)
        std::vector<float> decodedPcm(FRAME_SIZE);

        // Dekompresja Opus
        int decodedSamples = opus_decode_float(
            decoder, 
            inEncodedPacket.data(), 
            inEncodedPacket.size(), 
            decodedPcm.data(), 
            FRAME_SIZE, 
            0 // 0 oznacza, że nie używamy mechanizmu FEC (odzyskiwania utraconych pakietów)
        );

        if (decodedSamples > 0) {
            // Wypychamy odzyskany surowy dźwięk prosto do głośników.
            // SDL3 automatycznie zadba o to, żeby sygnał został odtworzony płynnie.
            SDL_PutAudioStreamData(speakerStream, decodedPcm.data(), decodedSamples * sizeof(float));
        }
    }
    
};



// 