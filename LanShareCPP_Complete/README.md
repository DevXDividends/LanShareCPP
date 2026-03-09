# 🛡️ LanShareCPP

> A secure, encrypted LAN messaging and file-sharing application written in C++17.  
> Private chats, group chats, file transfers — all AES-256-GCM encrypted, all local network.

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B17-orange)
![Qt](https://img.shields.io/badge/GUI-Qt6-green)

---

## 📸 What It Does

LanShareCPP lets multiple people on the **same LAN (WiFi/Ethernet)** chat and share files securely — no internet needed, no cloud, no third party.

- ✅ Register / Login with usernames and passwords
- ✅ Private 1-on-1 encrypted messages
- ✅ Group chats with join codes (e.g. `TIGER-42`)
- ✅ File transfer (any format, any size, auto-saved to Downloads)
- ✅ Message history persisted in SQLite (per user, cleared on logout)
- ✅ Unread message badge (red count on contact name)
- ✅ Progress bars for file transfers
- ✅ Discord-dark themed GUI

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                   LAN Network                       │
│                                                     │
│  ┌──────────────┐        ┌──────────────────────┐   │
│  │  GUI Client  │◄──────►│   lanshare_server    │   │
│  │ (Qt6 Window) │        │   (port 5555)        │   │
│  └──────────────┘        │                      │   │
│                          │  AuthManager         │   │
│  ┌──────────────┐        │  GroupManager        │   │
│  │Console Client│◄──────►│  MessageRouter       │   │
│  │  (terminal)  │        │  ClientSession(s)    │   │
│  └──────────────┘        └──────────────────────┘   │
└─────────────────────────────────────────────────────┘
```

### Components

| Binary | Description |
|---|---|
| `lanshare_server.exe` | Central relay server. Handles auth, routing, groups. Run once on one machine. |
| `lanshare_gui.exe` | Qt6 GUI client. The main app your friends will use. |
| `lanshare_client.exe` | Console client. For testing or headless use. |

---

## 🔐 Security Model

All messages and file chunks are encrypted with **AES-256-GCM** before leaving the sender's machine.

### Key Derivation
Keys are **never transmitted** over the network. Both parties independently derive the same key:

```
Private chat key  = HKDF("userA:userB:lanshare-v1", salt)   // usernames sorted alphabetically
Group chat key    = HKDF("group:groupName:lanshare-v1", salt)
File transfer key = HKDF("file:userA:userB:lanshare-v1", salt)
```

### Why AES-GCM?
GCM (Galois/Counter Mode) provides **authenticated encryption** — the tag proves the message wasn't tampered with in transit, not just that it's encrypted.

### What the server sees
The server routes encrypted blobs. It **cannot** read message content or file data. It only sees: sender ID, recipient ID, message type, and ciphertext.

---

## 📁 Project Structure

```
LanShareCPP_Complete/
│
├── common/
│   ├── Protocol.h          ← Message types, header format, constants
│   └── AESGCM.h / .cpp     ← AES-256-GCM encrypt/decrypt + key derivation
│
├── server/
│   ├── main.cpp            ← Server entry point
│   ├── ServerCore.h/.cpp   ← Accepts connections, owns all managers
│   ├── ClientSession.h/.cpp← Per-client TCP session (one per connected user)
│   ├── AuthManager.h/.cpp  ← Register/login, bcrypt password hashing
│   ├── GroupManager.h/.cpp ← Group CRUD, join codes, membership
│   └── MessageRouter.h/.cpp← Routes messages/files between sessions
│
├── client/
│   ├── main.cpp            ← Console client entry point
│   ├── ClientCore.h/.cpp   ← TCP connection, async I/O, all callbacks
│   └── (shared with ui/)
│
├── ui/
│   ├── main.cpp            ← GUI entry point
│   ├── LoginWindow.h/.cpp  ← Login/register screen
│   ├── ChatWindow.h/.cpp   ← Main chat UI
│   ├── MessageBubble.h/.cpp← Individual chat bubble widget
│   ├── FileTransferWidget.h/.cpp ← File send/receive progress widget
│   ├── MessageDB.h/.cpp    ← SQLite message history
│   └── ChatWindow.ui       ← Qt Designer UI layout
│
└── CMakeLists.txt          ← Build configuration
```

---

## 🚀 Building from Source

> **Platform:** Windows 10/11 (64-bit). All steps below are for Windows.  
> Estimated setup time: ~30–45 minutes on first install.

---

### Step 1 — Install MSYS2 (compiler + package manager)

MSYS2 gives you GCC, MinGW, and `pacman` (a Linux-style package manager for Windows).

1. Download the installer from **https://www.msys2.org/**
2. Run it, install to `C:\msys64` (default)
3. When it finishes, open **MSYS2 MINGW64** from the Start Menu
4. Update the package database:

```bash
pacman -Syu
# It may close the window — reopen MSYS2 MINGW64 and run again:
pacman -Su
```

---

### Step 2 — Install all dependencies via pacman

Open **MSYS2 MINGW64** and run this single command to install everything:

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

This installs:
| Package | What it provides |
|---|---|
| `gcc` | C++17 compiler (GCC 13+) |
| `cmake` | Build system |
| `ninja` | Fast build tool (used by cmake) |
| `boost` | Boost.Asio (networking), Boost.Thread |
| `openssl` | AES-256-GCM encryption |
| `sqlite3` | Local message history database |
| `git` | To clone the repo |

---

### Step 3 — Install Qt6 (for the GUI)

Qt is too large for pacman — install it via the official Qt installer.

1. Download **Qt Online Installer** from **https://www.qt.io/download-qt-installer**
2. Create a free Qt account if prompted
3. In the installer, select:
   - **Qt 6.10.1** (or latest 6.x)
   - Under it, check: ✅ **MinGW 64-bit**
   - Also check: ✅ **MinGW 13.x toolchain** (under Tools)
4. Install to `C:\Qt` (default)

> **Important:** Make sure you pick the **MinGW** variant of Qt6, not MSVC. The project uses the MinGW compiler from MSYS2.

---

### Step 4 — Add MinGW to your PATH

So Windows can find `g++`, `cmake`, etc. from the command prompt:

1. Open **Start → Edit the system environment variables**
2. Click **Environment Variables**
3. Under **System variables**, find `Path` → click **Edit**
4. Click **New** and add these two paths:
   ```
   C:\msys64\mingw64\bin
   C:\Qt\6.10.1\mingw_64\bin
   ```
5. Click OK on all dialogs
6. **Restart any open terminals** for the change to take effect

Verify it worked — open a new **Command Prompt** and run:
```cmd
g++ --version
cmake --version
```
Both should print version numbers.

---

### Step 5 — Clone the repository

Open **Command Prompt** (or Git Bash):

```bash
git clone https://github.com/YOUR_USERNAME/LanShareCPP.git
cd LanShareCPP
```

---

### Step 6 — Configure and build

```bash
# Create build directory
mkdir build
cd build

# Configure — tell CMake where Qt6 is
cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/mingw_64"

# Build everything (server + GUI + console client)
cmake --build . --parallel
```

> **Note:** Replace `C:/Qt/6.10.1/mingw_64` with your actual Qt install path if different. Check `C:\Qt\` for the version folder.

If the build succeeds you'll see:
```
[100%] Linking CXX executable lanshare_server.exe
[100%] Linking CXX executable lanshare_gui.exe
[100%] Linking CXX executable lanshare_client.exe
```

---

### Step 7 — Copy Qt DLLs (first run only)

Qt apps need their DLLs next to the `.exe`. Run this from inside the `build/` folder:

```bash
C:\Qt\6.10.1\mingw_64\bin\windeployqt.exe lanshare_gui.exe
```

This copies all required Qt `.dll` files automatically.

---

### Troubleshooting

| Error | Fix |
|---|---|
| `cmake: command not found` | Add `C:\msys64\mingw64\bin` to PATH (Step 4) |
| `Could not find Qt6` | Check `-DCMAKE_PREFIX_PATH` points to your Qt mingw_64 folder |
| `Cannot find -lsqlite3` | Run `pacman -S mingw-w64-x86_64-sqlite3` in MSYS2 |
| `Cannot find OpenSSL` | Run `pacman -S mingw-w64-x86_64-openssl` in MSYS2 |
| `DLL not found` when launching GUI | Run `windeployqt.exe lanshare_gui.exe` (Step 7) |
| Build fails after changing files | Delete `build/` folder completely and redo from Step 6 |

---

## 🖥️ Running the App

### Step 1 — Start the Server (one person does this)

```bash
cd build
./lanshare_server.exe
# Server started on port 5555
```

Find the server machine's IP:
```bash
ipconfig   # look for IPv4 Address under your WiFi adapter
# e.g. 192.168.1.5
```

### Step 2 — Launch the GUI Client (everyone else)

```bash
./lanshare_gui.exe
```

- Enter the server's IP address
- Register a new account or login
- Start chatting!

### Step 3 — Create/Join Groups

- Click **Create Group** → enter a group name → a join code like `TIGER-42` will appear
- Share the code verbally with friends
- Friends click **Join Group** → enter group name + code

---

## 🔌 Protocol Reference

See [`docs/PROTOCOL.md`](docs/PROTOCOL.md) for the full binary protocol spec.

Quick overview — every message is a 5-byte header + payload:

```
┌──────────────────────┬───────────┐
│  length (4 bytes LE) │ type (1B) │  ← MessageHeader
├──────────────────────┴───────────┤
│         payload (N bytes)        │
└──────────────────────────────────┘
```

---

## 📚 Module Documentation

| Doc | Contents |
|---|---|
| [`docs/PROTOCOL.md`](docs/PROTOCOL.md) | Binary protocol, all message types, payload formats |
| [`docs/ENCRYPTION.md`](docs/ENCRYPTION.md) | AES-GCM deep dive, key derivation, security analysis |
| [`docs/SERVER.md`](docs/SERVER.md) | Server architecture, AuthManager, GroupManager, MessageRouter |
| [`docs/CLIENT.md`](docs/CLIENT.md) | ClientCore internals, async I/O, write queue, callbacks |
| [`docs/GUI.md`](docs/GUI.md) | UI architecture, ChatWindow, SQLite history, file transfer UI |
| [`docs/FILE_TRANSFER.md`](docs/FILE_TRANSFER.md) | Chunked transfer design, encryption, corruption prevention |


## 🤝 Contributing

Pull requests welcome! If you find a bug, open an issue with:
1. What you did
2. What you expected
3. What actually happened
4. Server + client console output
---