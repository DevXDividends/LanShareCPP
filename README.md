# ⚡ LanShareCPP

> **Secure, encrypted, real-time LAN messaging — no internet required.**

![C++17](https://img.shields.io/badge/C%2B%2B-17-00ffe7?style=flat-square&logo=cplusplus)
![Qt6](https://img.shields.io/badge/Qt-6.10.1-39ff14?style=flat-square&logo=qt)
![OpenSSL](https://img.shields.io/badge/OpenSSL-3.6-ff2d78?style=flat-square&logo=openssl)
![Boost.Asio](https://img.shields.io/badge/Boost.Asio-1.90-00ffe7?style=flat-square)
![CMake](https://img.shields.io/badge/CMake-MinGW-39ff14?style=flat-square&logo=cmake)
![Platform](https://img.shields.io/badge/Platform-Windows-ff2d78?style=flat-square&logo=windows)

---

## 📸 Screenshots

> _Screenshots coming soon._

---

## 📖 Overview

**LanShareCPP** is a desktop LAN chat application built in C++17 that enables secure, real-time communication between users on the same local network — **no internet connection, no cloud, no third-party servers.**

All messages and file transfers are encrypted end-to-end using **AES-256-GCM**.

The application features a cyberpunk-themed **Qt6 GUI**, private and group messaging, chunked encrypted file transfer, and a persistent user registry — making it suitable for closed offline environments such as labs, offices, or development teams.
---

## ✅ Features

- 🔒 **AES-256-GCM encrypted** private and group messaging
- 👥 **Group chat** with invite-only join codes (e.g. `TIGER-42`)
- 📁 **Encrypted file transfer** — chunked at 64 KB, auto-saved to Downloads
- 🗄️ **SQLite message history** — session-scoped, wiped on app close or server restart
- 🔔 **Unread message badges** — red count indicator on contact name
- 🎨 **Cyberpunk dark theme** — neon cyan, green, magenta on dark background
- 🧵 **Thread-safe write queue** — via `boost::asio::post`
---

## 🗂️ Project Architecture

### Directory Structure
```
LanShareCPP_Complete/
│
├── common/                        # Shared between server and client
│   ├── AESGCM.h                   # AES-256-GCM encryption interface
│   ├── AESGCM.cpp                 # Encryption implementation (OpenSSL backend)
│   └── Protocol.h                 # All message type definitions and opcodes
│
├── server/                        # Server-side logic (no UI)
│   ├── main.cpp                   # Server entry point
│   ├── ServerCore.h / .cpp        # Accepts connections, manages lifecycle
│   ├── ClientSession.h / .cpp     # Per-client TCP session handler
│   ├── AuthManager.h / .cpp       # Registration, login, users.db persistence
│   ├── GroupManager.h / .cpp      # Group creation, join codes, membership
│   └── MessageRouter.h / .cpp     # Routes private and group messages
│
├── client/                        # Networking layer (no UI)
│   └── ClientCore.h / .cpp        # All TCP logic, callbacks, send/receive
│
├── ui/                            # Qt6 GUI layer
│   ├── main.cpp                   # GUI entry point
│   ├── LoginWindow.h / .cpp / .ui # Registration and login screen
│   ├── ChatWindow.h / .cpp / .ui  # Main chat interface
│   ├── MessageBubble.h / .cpp     # Individual message widget (decrypt button)
│   ├── FileTransferWidget.h / .cpp# File send/receive progress widget
│   └── MessageDB.h / .cpp         # SQLite wrapper for session message storage
│
├── build/                         # CMake build output (generated)
│   ├── lanshare_server.exe        # The chat server
│   ├── lanshare_gui.exe           # The Qt GUI client
│   └── lanshare_client.exe        # CLI test client
│
├── CMakeLists.txt                 # Build configuration
├── .gitignore
```

### Layer Communication
```
[ Qt6 UI Layer ]
      ↕  callbacks (Qt signals via QMetaObject::invokeMethod)
[ ClientCore ]
      ↕  TCP / Boost.Asio (port 5555, async read/write)
[ ServerCore ]
      ↕  internal dispatch
[ MessageRouter → ClientSession → AuthManager / GroupManager ]
```

---

## 🛠️ Tech Stack

| Technology | Version | Purpose |
|---|---|---|
| C++17 | — | Core language |
| Qt6 | 6.10.1 | GUI framework (widgets, signals, events) |
| Boost.Asio | 1.90 | Async TCP networking (server + client) |
| OpenSSL | 3.6 | AES-256-GCM encryption, key derivation |
| SQLite3 | — | Session-scoped message storage |
| CMake + MinGW | — | Build system (Windows) |

---

## 📡 Protocol Reference

All communication uses a **5-byte header** followed by a payload:
```
[ 4 bytes: payload length (little-endian uint32) ][ 1 byte: message type ][ payload... ]
```

### Message Type Opcodes

| Category | Opcode | Name |
|---|---|---|
| Auth | `0x01–0x05` | Register, Login, Logout, AuthOK, AuthFail |
| Private Message | `0x10` | `MSG_PRIVATE` |
| Group Message | `0x20` | `MSG_GROUP` |
| File (Private) | `0x30–0x32` | FileMeta, FileChunk, FileEnd |
| File (Group) | `0x33–0x35` | GroupFileMeta, GroupFileChunk, GroupFileEnd |
| Group Management | `0x40–0x46` | Create, Join, Leave, List, Members, Kick, `GROUP_CODE` |
| User Management | `0x50–0x52` | UserList, UserJoined, UserLeft |
| Control | `0x60–0x63` | Ping, Pong, Error, Disconnect |

**Server port:** `5555`

---

## 🚀 Building from Source

**Platform:** Windows 10/11 (64-bit). All steps below are for Windows.  
**Estimated setup time:** ~30–45 minutes on first install.

### Step 1 — Install MSYS2

MSYS2 gives you GCC, MinGW, and `pacman` (a Linux-style package manager for Windows).

1. Download the installer from [https://www.msys2.org/](https://www.msys2.org/)
2. Install to `C:\msys64` (default)
3. Open **MSYS2 MINGW64** from the Start Menu
4. Update the package database:
```bash
pacman -Syu
# It may close the window — reopen MSYS2 MINGW64 and run again:
pacman -Su
```

### Step 2 — Install all dependencies via pacman

Open **MSYS2 MINGW64** and run:
```bash
pacman -S --needed \
  mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-boost \
  mingw-w64-x86_64-openssl \
  mingw-w64-x86_64-sqlite3 \
  git  
```

| Package | What it provides |
|---|---|
| `gcc` | C++17 compiler (GCC 13+) |
| `cmake` | Build system |
| `ninja` | Fast build tool (used by cmake) |
| `boost` | Boost.Asio (networking), Boost.Thread |
| `openssl` | AES-256-GCM encryption |
| `sqlite3` | Local message history database |
| `git` | To clone the repo |

### Step 3 — Install Qt6

Qt is too large for pacman — install it via the official Qt installer.

1. Download from [https://www.qt.io/download-qt-installer](https://www.qt.io/download-qt-installer)
2. Create a free Qt account if prompted
3. In the installer, select:
   - **Qt 6.10.1** (or latest 6.x)
   - Under it, check: ✅ **MinGW 64-bit**
   - Also check: ✅ **MinGW 13.x toolchain** (under Tools)
4. Install to `C:\Qt` (default)

> ⚠️ **Important:** Select the **MinGW** variant of Qt6, not MSVC. The project uses the MinGW compiler from MSYS2.

### Step 4 — Add MinGW to your PATH

1. Open **Start → Edit the system environment variables**
2. Click **Environment Variables**
3. Under **System variables**, find **Path** → click **Edit**
4. Click **New** and add:
   - `C:\msys64\mingw64\bin`
   - `C:\Qt\6.10.1\mingw_64\bin`
5. Click OK on all dialogs and restart any open terminals

Verify it worked — open a new Command Prompt:
```bash
g++ --version
cmake --version
```

Both should print version numbers.

### Step 5 — Clone the repository
```bash
git clone https://github.com/DevXDividends/LanShareCPP.git
cd LanShareCPP
```

### Step 6 — Configure and build
```bash
mkdir build
cd build

cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/mingw_64"

cmake --build . --parallel
```

> Replace `C:/Qt/6.10.1/mingw_64` with your actual Qt install path if different.

If the build succeeds you will see:
```
[100%] Linking CXX executable lanshare_server.exe
[100%] Linking CXX executable lanshare_gui.exe
[100%] Linking CXX executable lanshare_client.exe
```

### Step 7 — Copy Qt DLLs (first run only)

Qt apps need their DLLs next to the `.exe`. Run this from inside the `build/` folder:
```bash
C:\Qt\6.10.1\mingw_64\bin\windeployqt.exe lanshare_gui.exe
```

This copies all required Qt `.dll` files automatically.

---

## ▶️ How to Run

### Step 1 — Start the server
```bash
cd build
./lanshare_server.exe
```
The server listens on port `5555`. On startup, it wipes any existing group data and loads the user registry from `users.db`.

### Step 2 — Launch GUI clients
Open new terminal 
```bash
cd build 
./lanshare_gui.exe
```
Launch one instance per user. Each instance connects to `localhost:5555` by default (configurable at login).

### Step 3 — Register and log in
- Enter a username and password on the login screen
- First time = register; subsequent launches = login

### Step 4 — Private messaging
- Click any online user from the sidebar
- Type a message and press **Enter** to send
- Click **🔓 Decrypt** on received messages to decrypt them
- If a user is not visible click on **🔄️Refresh**

### Step 5 — Group chat
- Click **Create Group** → enter a group name → a join code (e.g. `WOLF-91`) is shown
- Share the join code with teammates
- Others click **Join Group** → enter group name + join code

### Step 6 — File transfer
- Open a chat with a user or group
- Click **📎 Attach** → select a file
- File is encrypted, chunked, and transferred automatically
- Received files are saved to the recipient's `Downloads` folder

---

## 🔐 Encryption — AES-256-GCM

LanShareCPP uses **AES-256-GCM** (Advanced Encryption Standard — Galois/Counter Mode) for all messages and file chunks. This is the same cipher used by WhatsApp, Signal, and TLS 1.3.

### Why AES-256-GCM?

Most encryption modes (like AES-CBC) only provide confidentiality — they hide the content but cannot detect tampering. GCM is an **AEAD** cipher (*Authenticated Encryption with Associated Data*), which provides three guarantees simultaneously:

| Property | Meaning |
|---|---|
| **Confidentiality** | Nobody can read the message without the key |
| **Integrity** | If even 1 bit is changed in transit, decryption fails |
| **Authenticity** | Proves the message came from someone who holds the key |

The 16-byte GCM authentication tag appended to every ciphertext acts as a cryptographic MAC — any tampering is detected before plaintext is returned.

### Encrypted Data Format

Every encrypted blob is structured as:
```
┌──────────────────┬──────────────────┬───────────────────────────┐
│  IV (12 bytes)   │  Tag (16 bytes)  │  Ciphertext (N bytes)     │
└──────────────────┴──────────────────┴───────────────────────────┘
```

- **IV:** 12 random bytes generated fresh for every single message via `RAND_bytes()`. Reusing an IV with the same key is catastrophic in GCM — this is never allowed.
- **Tag:** 16-byte authentication tag. Verified on decryption — failure throws an exception.
- **Ciphertext:** Encrypted payload, same length as plaintext.

In code (`common/AESGCM.h`):
```cpp
struct EncryptedData {
    std::vector<uint8_t> iv;          // 12 bytes
    std::vector<uint8_t> tag;         // 16 bytes
    std::vector<uint8_t> ciphertext;  // N bytes

    std::vector<uint8_t> serialize() const;
    static EncryptedData deserialize(const std::vector<uint8_t>& blob);
};
```

### Key Derivation — Keys Are Never Sent Over the Network

Both parties independently derive the same key from shared knowledge (their usernames and a hardcoded salt). No key material is ever transmitted.

Keys are derived using OpenSSL's `EVP_BytesToKey` with SHA-256, producing a 32-byte (256-bit) key:
```cpp
AESGCM::Key deriveKeyFromPassword(const std::string& secret, const std::string& salt);
```

| Context | Key input string | Notes |
|---|---|---|
| Private messages | `"userA:userB:lanshare-v1"` | Usernames sorted alphabetically |
| Group messages | `"group:groupName:lanshare-v1"` | All members derive same key |
| File transfers | `"file:userA:userB:lanshare-v1"` | Usernames sorted alphabetically |

**Why sort usernames?** If Alice sends to Bob, she computes `"alice:bob:..."`. If Bob sends to Alice, he also computes `"alice:bob:..."` (same, because sorted). Without sorting, they would derive different keys and decryption would fail.
```cpp
std::vector<std::string> ids = {userID_, toUserID};
std::sort(ids.begin(), ids.end());
AESGCM::Key key = AESGCM::deriveKeyFromPassword(
    "file:" + ids[0] + ":" + ids[1] + ":lanshare-v1",
    "lanshare-salt-2024");
```

### What the Server Cannot See

The server operates as a **relay only**. For every message it forwards, it can see:
- ✅ Who sent it (fromUserID)
- ✅ Who it is addressed to (toUserID)
- ✅ Message size
- ❌ **Cannot read the content** — it has no key and performs no decryption

### File Transfer Encryption

Each 64 KB chunk is encrypted independently with the same derived key but a **fresh IV per chunk**:
```
Chunk 0 → IV₀ (random) + Tag₀ + Ciphertext₀
Chunk 1 → IV₁ (random) + Tag₁ + Ciphertext₁
...
Chunk N → IVₙ (random) + Tagₙ + Ciphertextₙ
```

Chunks are stored in a `map<uint32_t, vector<uint8_t>>` keyed by chunk index and assembled in order when the `FILE_*_COMPLETE` message arrives.

### Known Security Limitations

This is a college project, not a production security system. The following gaps are acknowledged:

| Limitation | Notes |
|---|---|
| Static hardcoded salt `"lanshare-salt-2024"` | Weakens key derivation; production would use random per-session salts |
| Derived keys, not true E2E | Uses shared secret derivation rather than DH key exchange (X25519) |
| No certificate pinning | MITM on the same LAN is theoretically possible |
| Auth credentials sent as plaintext | TLS should wrap the auth phase in production |
| `users.db` stored unencrypted | Physical access to the server machine exposes credentials |

---

## ⚙️ Key Implementation Details

### Thread-Safe Write Queue

All network writes go through `boost::asio::post` to ensure thread safety:
```cpp
boost::asio::post(ioContext_, [this]() { writeNext(); });
```

### Session-Only Message History

Message history is **not persisted across sessions by design.** Messages are stored in RAM (`messageHistory_` map) during the session. On app close or server disconnect, all history is wiped.

### Server Restart Behaviour

| Data | On Server Restart |
|---|---|
| Users (`users.db`) | ✅ Preserved |
| Groups | ❌ Wiped |
| Chat history (client SQLite) | ❌ Wiped |
| In-memory message bubbles | ❌ Cleared |

---

## 🗺️ Roadmap — Pending Features

- [ ] Message timestamps displayed on bubbles
- [ ] Online / offline status indicator (green dot per user)
- [ ] Auto-reconnect if server crashes mid-session
- [ ] Auto-decrypt message history on contact open
- [ ] Group file transfer stress testing (3+ simultaneous senders)
- [ ] File transfer resume on disconnect
- [ ] Encrypted storage for `users.db` (currently plaintext)

---

## 📁 Database Files

| File | Contents | Persists? |
|---|---|---|
| `users.db` | Registered usernames + hashed passwords | ✅ Yes |
| `messages_<userID>.db` | Session message history + unread counts | ❌ Wiped on close |
| `groups.db` | Group membership (deleted on server start) | ❌ No |

---

---

## 🤝 Contributing

Contributions are welcome. To contribute:

1. Fork the repository
2. Create a new branch for your feature or fix
3. Make your changes and test with at least 2 client instances
4. Submit a pull request with a clear description of what was changed and why

---
<div align="center">

*LanShareCPP — Secure LAN Messaging*

</div>
