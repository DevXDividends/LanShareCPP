# LanShareCPP - VS Code Setup with Existing g++

## ✅ Great News - You Can Use Your Existing g++!

Since you already have g++ working, you can **skip Visual Studio entirely** and use VS Code with MinGW/g++.

---

## Method 1: Use MSYS2 Packages (FASTEST - Recommended)

This is the easiest since you already have g++ working.

### Step 1: Install MSYS2 (if not already installed)

**Check if you have MSYS2:**
```bash
# In your current terminal (Git Bash or CMD)
where gcc
# If it shows C:\msys64\mingw64\bin\gcc.exe - you have MSYS2!
```

**If you don't have MSYS2:**
- Download: https://www.msys2.org/
- Install to: `C:\msys64`
- Add to PATH: `C:\msys64\mingw64\bin`

### Step 2: Install Dependencies with MSYS2

Open **MSYS2 MINGW64** terminal (or Git Bash if g++ is available):

```bash
# Update MSYS2 (if using MSYS2 terminal)
pacman -Syu

# Install all dependencies at once
pacman -S --needed base-devel mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-openssl \
    mingw-w64-x86_64-qt6-base

# This takes 10-15 minutes (much faster than vcpkg!)
```

### Step 3: Build LanShareCPP

```bash
# Navigate to project
cd "/d/ADITYA/LanShare C++/LanShareCPP"

# Create build directory
mkdir -p build && cd build

# Configure with CMake (using MinGW)
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# If successful, run server
./lanshare_server.exe
```

**That's it! No vcpkg, no Visual Studio needed!**

---

## Method 2: Use vcpkg with MinGW (Alternative)

If you want to use vcpkg but with g++ instead of Visual Studio:

### Step 1: Tell vcpkg to use MinGW

```bash
cd "/d/ADITYA/LanShare C++/vcpkg/vcpkg"

# Install for mingw triplet
./vcpkg install boost-asio:x64-mingw-dynamic
./vcpkg install boost-system:x64-mingw-dynamic
./vcpkg install openssl:x64-mingw-dynamic
```

### Step 2: Build with MinGW

```bash
cd "/d/ADITYA/LanShare C++/LanShareCPP"
mkdir -p build && cd build

cmake .. -G "MinGW Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE="/d/ADITYA/LanShare C++/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic

cmake --build .
```

---

## VS Code Setup

### Step 1: Install VS Code Extensions

Open VS Code and install:
- **C/C++** (Microsoft)
- **CMake Tools** (Microsoft)
- **CMake** (twxs)

### Step 2: Configure VS Code

Create `.vscode/settings.json` in your LanShareCPP folder:

```json
{
    "cmake.configureSettings": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_TOOLCHAIN_FILE": "D:/ADITYA/LanShare C++/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake"
    },
    "cmake.generator": "MinGW Makefiles",
    "C_Cpp.default.compilerPath": "C:/msys64/mingw64/bin/g++.exe",
    "C_Cpp.default.intelliSenseMode": "windows-gcc-x64"
}
```

### Step 3: Configure CMake Kit

Press `Ctrl+Shift+P` → Type "CMake: Select a Kit"
- Select your g++ compiler (should auto-detect)
- Or create custom kit if needed

### Step 4: Build in VS Code

1. Press `Ctrl+Shift+P`
2. Type "CMake: Configure"
3. Type "CMake: Build"

Or use bottom toolbar buttons in VS Code!

---

## Quick Setup Script for MSYS2

Save this as `setup-msys2.sh` and run in Git Bash:

```bash
#!/bin/bash

echo "========================================"
echo "LanShareCPP - MSYS2 Setup"
echo "========================================"

# Check if MSYS2 is installed
if ! command -v pacman &> /dev/null; then
    echo "ERROR: MSYS2 not found"
    echo "Please install from: https://www.msys2.org/"
    exit 1
fi

echo "Installing dependencies..."

# Update MSYS2
pacman -Sy --noconfirm

# Install dependencies
pacman -S --needed --noconfirm \
    base-devel \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-openssl

echo ""
echo "Installation complete!"
echo ""
echo "Next steps:"
echo "1. cd '/d/ADITYA/LanShare C++/LanShareCPP'"
echo "2. mkdir -p build && cd build"
echo "3. cmake .. -G 'MinGW Makefiles'"
echo "4. cmake --build ."
echo ""
```

---

## Verify Your g++ Setup

Before building, verify your setup:

```bash
# Check g++ version
g++ --version
# Should show: g++ (Rev...) 13.x.x or newer

# Check CMake
cmake --version
# Should show: cmake version 3.x.x

# Check if Boost is available (if using MSYS2)
ls /mingw64/include/boost
# Should show boost headers

# Check OpenSSL
ls /mingw64/include/openssl
# Should show openssl headers
```

---

## Build Commands Reference

```bash
# Full build process
cd "/d/ADITYA/LanShare C++/LanShareCPP"
mkdir -p build && cd build

# Configure
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build everything
cmake --build . -j4

# Build specific target
cmake --build . --target lanshare_server

# Clean build
cmake --build . --target clean

# Rebuild
rm -rf * && cmake .. -G "MinGW Makefiles" && cmake --build .
```

---

## VS Code Tasks (Optional)

Create `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build LanShare",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "${workspaceFolder}/build"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "Configure CMake",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-G", "MinGW Makefiles",
                "-B", "${workspaceFolder}/build",
                "-S", "${workspaceFolder}"
            ]
        },
        {
            "label": "Run Server",
            "type": "shell",
            "command": "${workspaceFolder}/build/lanshare_server.exe",
            "dependsOn": ["Build LanShare"]
        }
    ]
}
```

Now you can:
- Press `Ctrl+Shift+B` to build
- Press `Ctrl+Shift+P` → "Run Task" → "Run Server"

---

## Advantages of Using g++ Instead of Visual Studio

✅ **Much faster installation** (15 min vs 1 hour)
✅ **Smaller download** (~500 MB vs 6+ GB)
✅ **Cross-platform code** (same compiler on Linux/Windows)
✅ **Works with VS Code** (lightweight editor)
✅ **Package manager** (pacman vs manual vcpkg)
✅ **Familiar for Linux users**

---

## Recommended Path for You

Since you already have g++ working:

1. **Install MSYS2** (if not already installed)
2. **Use pacman** to install Boost and OpenSSL
3. **Build with CMake + MinGW Makefiles**
4. **Use VS Code** for editing and debugging

**Skip vcpkg and Visual Studio entirely!**

---

## Complete Setup Timeline

| Step | Time | Notes |
|------|------|-------|
| Install MSYS2 | 5 min | If needed |
| Install dependencies (pacman) | 10-15 min | Much faster than vcpkg |
| Configure CMake | 1 min | |
| Build project | 2-5 min | First build |
| **Total** | **~20 min** | vs 3+ hours with VS |

---

## Troubleshooting

### "cmake: command not found"
```bash
pacman -S mingw-w64-x86_64-cmake
```

### "Cannot find boost"
```bash
pacman -S mingw-w64-x86_64-boost
```

### "OpenSSL not found"
```bash
pacman -S mingw-w64-x86_64-openssl
```

### Build errors with threads
```bash
# Add to CMakeLists.txt
find_package(Threads REQUIRED)
target_link_libraries(LanShareServer Threads::Threads)
```

---

## What You DON'T Need

❌ Visual Studio (6+ GB)
❌ vcpkg compilation (hours of waiting)
❌ Windows-specific tools

## What You DO Need

✅ MSYS2 (~500 MB)
✅ g++ (you have this!)
✅ CMake
✅ Boost, OpenSSL (via pacman)
✅ VS Code (optional, any editor works)

---

**Start here:** Install MSYS2, then run the pacman commands above. You'll be building in 20 minutes!
