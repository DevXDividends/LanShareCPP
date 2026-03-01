# LanShareCPP - MinGW Setup (Alternative to Visual Studio)

## Option: Using MinGW-w64 Instead of Visual Studio

If you don't want to install Visual Studio, you can use MinGW-w64 with MSYS2.

### Step 1: Install MSYS2

1. Download MSYS2 installer:
   - https://www.msys2.org/
   - Download: `msys2-x86_64-latest.exe`

2. Run the installer (install to default location: `C:\msys64`)

3. Open **MSYS2 MINGW64** terminal (from Start Menu)

### Step 2: Install Development Tools

```bash
# Update package database
pacman -Syu

# Close and reopen MSYS2 MINGW64 terminal, then:
pacman -Su

# Install development tools
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-boost
pacman -S mingw-w64-x86_64-openssl
pacman -S mingw-w64-x86_64-qt6-base
pacman -S git
```

### Step 3: Build LanShareCPP

```bash
# Navigate to project (adjust path)
cd "/d/ADITYA/LanShare C++/LanShareCPP"

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Run
./lanshare_server.exe
```

### Advantages of MinGW Method

✅ Faster installation (no large Visual Studio download)
✅ Simpler package management with pacman
✅ All dependencies available via pacman
✅ More Unix-like development experience

### Disadvantages

❌ Less common on Windows
❌ May have compatibility issues with some libraries
❌ Smaller Windows development community

## Comparison

| Method | Download Size | Install Time | Ease of Use |
|--------|--------------|--------------|-------------|
| Visual Studio 2022 | ~2-6 GB | 30-60 min | Easiest (official) |
| VS Build Tools | ~1-2 GB | 20-30 min | Medium |
| MinGW/MSYS2 | ~500 MB | 10-15 min | Medium (Linux-like) |

## Recommended: Visual Studio for Windows Development

While MinGW works, **Visual Studio is recommended** because:
- Official Microsoft toolchain
- Better Windows compatibility
- vcpkg is designed for it
- Most Windows C++ developers use it
- Better debugging tools
- Future-proof for Windows development

## Quick Decision Guide

**Choose Visual Studio if:**
- You're new to C++ on Windows
- You want the best compatibility
- You have decent internet speed
- You have ~10GB free disk space

**Choose MinGW if:**
- You're comfortable with Linux tools
- You have limited disk space
- You want faster setup
- You're experienced with makefiles

