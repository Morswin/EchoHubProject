# EchoHubProject
This is university project, where we aim to write our own version of a desktop voice/text chat application (Discord/TeamSpeak clone).  
(*In case that this app name is already taken, just let us know and we will change this project's name.*)

## 📋 Project Overview

EchoHub is a **self-hosted** voice and text chat application built with:
- **C++23** (main language)
- **SDL3** (windowing and audio)
- **Dear ImGui** (UI framework)
- **Asio** (networking)
- **Opus** (voice codec)
- **nlohmann/json** (message serialization)

### Key Features
- **Text Chat**: Real-time messaging in multiple channels
- **Voice Chat**: Low-latency voice communication using Opus codec
- **Self-Hosted**: Any user can host their own server
- **Multi-Channel**: Support for both text and voice channels
- **Threaded**: Uses threads for non-blocking audio and network operations

## 🏗️ Architecture

### Network Protocol
- **Transport**: TCP for text messages, UDP for voice packets
- **Message Format**: JSON (using nlohmann/json library)
- **Ports**:
  - TCP: 9987 (default for text chat)
  - UDP: 9988 (default for voice)

### Message Types
All messages follow this JSON structure:
```json
{
  "type": "message_type",
  "data": { ... }
}
```

Supported message types:
- `connect_request` / `connect_response` - Connection management
- `login_request` / `login_response` - User authentication
- `text_message` - Chat messages
- `voice_packet` - Voice data (base64 encoded)
- `join_channel` / `leave_channel` - Channel management
- `user_joined` / `user_left` - User presence
- `channel_list_request` / `channel_list_response` - Channel listing

### User Authentication
- Each server maintains its own user database (in-memory for now)
- Users are identified by username + password
- No central authentication server (self-hosted model)

### Voice Processing
- **Codec**: Opus (optimized for VoIP)
- **Sample Rate**: 48kHz
- **Channels**: Mono
- **Frame Size**: 20ms (960 samples)
- **Compression**: Opus encoder/decoder for efficient transmission

## 📁 Project Structure
```
.
├── src/
│   ├── main.cpp              # Main application entry point
│   ├── app_state.hpp          # Application state management
│   ├── message.hpp/cpp        # Message class
│   ├── users.hpp/cpp          # User management
│   ├── view_states.hpp        # View state enumeration
│   ├── voice/                 # Voice client implementation
│   │   ├── voice_client.hpp
│   │   └── voice_client.cpp
│   ├── network/               # Networking components
│   │   ├── protocol.hpp       # Message protocol definitions
│   │   ├── server.hpp/cpp     # Server implementation
│   │   └── client.hpp/cpp     # Client implementation
│   └── utils/                 # Utility classes
│       └── thread_safe_queue.hpp  # Thread-safe queue
├── ui/                       # UI components
│   ├── atoms.hpp             # Basic UI elements
│   ├── molecules.hpp         # Composite UI components
│   ├── theme.hpp             # Theme management
│   └── views.hpp             # Application views
├── CMakeLists.txt            # Build configuration
└── README.md                 # This file
```

## 🛠️ Cheat sheet 
## Cloning
Cloning with submodules:  
```shell
git clone --recurse-submodules https://github.com/Morswin/EchoHubProject
```
Activating submodules if forgotten to do that when cloning:  
```shell
git submodule update --init --recursive
```

## Compiling
If you're trying to compile this on windows, try running this batch file, or read it's [contents](https://github.com/Morswin/EchoHubProject/blob/main/compile.bat) to see how we suggest to use cmake to help yourself compile this repo.
```shell
compile.bat
```

### Prerequisites
You need to have installed `cmake` to use this script. It also assumes that you're using windows. You can get the compiler with the Visual Studio Installer.  
For tips how to compile this on macos or linux please raise an issue in this project.

### Manual Compilation (Linux/macOS)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./echo_hub
```

## 🚀 Usage

### Starting a Server
1. Run the application
2. Click "Stwórz nowy serwer" (Create new server)
3. The server will start in a background thread
4. Share your IP address and port with friends

### Connecting to a Server
1. Run the application
2. Click "Dołącz do istniejącego serwera" (Join existing server)
3. Enter the server address (IP:port) and your username
4. Connect and start chatting

### Voice Chat
1. Join a voice channel
2. Your microphone will automatically start capturing audio
3. Voice packets are sent to the server and broadcast to other users in the same channel

## 🎯 Implementation Notes

### Threading Model
- **Server**: Runs in a separate thread, accepts connections asynchronously
- **Client**: Runs in a separate thread, handles incoming messages
- **Voice Client**: Designed to work in a separate thread for non-blocking audio processing

### Current Status
- ✅ UI Framework (ImGui + SDL3)
- ✅ Voice Client (Opus + SDL3 Audio)
- ✅ Network Protocol (JSON-based)
- ✅ Server Implementation (TCP + UDP)
- ✅ Client Implementation (TCP + UDP)
- ✅ Thread-Safe Queue
- 🔄 Integration of all components (in progress)
- ⏳ Testing and optimization

## 📝 Decyzje projektowe (Design Decisions)

### 1. Sieć (Network)
- **Protokół**: JSON przez TCP (czat tekstowy) i UDP (głos)
- **Porty**: 9987 (TCP), 9988 (UDP)
- **Uzasadnienie**: JSON jest łatwy do debugowania i wystarczająco wydajny dla czatu tekstowego. UDP dla głosu minimalizuje opóźnienia.

### 2. Autentykacja (Authentication)
- **Model**: Każdy serwer trzyma własną bazę użytkowników (w pamięci)
- **Identyfikacja**: Nazwa użytkownika + hasło
- **Uzasadnienie**: Prostsze niż IP-based, bardziej elastyczne (użytkownik może zmienić IP)

### 3. Głos (Voice)
- **Kodek**: Opus (VOIP mode)
- **Przesyłanie**: Przez serwer (klient → serwer → inni klienci)
- **Uzasadnienie**: Opus jest zoptymalizowany dla głosu, serwer jako pośrednik uproszcza implementację (brak P2P = brak NAT traversal)

### 4. Wątki (Threads)
- **Serwer**: Działa w oddzielnym wątku, akceptuje połączenia asynchronicznie
- **Klient**: Działa w oddzielnym wątku, obsługuje przychodzące wiadomości
- **Głos**: `VoiceClient` zaprojektowany do pracy w wątku
- **Uzasadnienie**: Wymaganie zaliczeniowe + lepsza responsywność UI

### 5. Kanały (Channels)
- **Typy**: Tekstowe (czat) i głosowe (VoIP)
- **Zarządzanie**: Serwer trzyma listę kanałów i użytkowników na każdym kanale
- **Uzasadnienie**: Podobne do Discorda, intuicyjne dla użytkowników
