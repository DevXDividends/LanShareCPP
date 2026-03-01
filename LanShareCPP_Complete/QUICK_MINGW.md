# Quick MinGW Setup (Avoid Visual Studio)

## If You Want to Skip Visual Studio Entirely

### Install MSYS2 (Includes MinGW-w64)

**Step 1: Download MSYS2**
- https://www.msys2.org/
- Download: `msys2-x86_64-latest.exe`
- Install to: `C:\msys64` (default)

**Step 2: Open MSYS2 MINGW64 Terminal**
- From Start Menu, search "MSYS2 MINGW64"
- NOT the "MSYS2 MSYS" - use "MINGW64"!

**Step 3: Install Everything**
```bash
# Update package database
pacman -Syu
# Say 'Y' to proceed
# Terminal will close - reopen it

# Update again
pacman -Su

# Install all development tools and dependencies
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-boost
pacman -S mingw-w64-x86_64-openssl
pacman -S mingw-w64-x86_64-qt6-base  # Optional, for GUI
pacman -S git
```

**Step 4: Build LanShareCPP**
```bash
# Navigate to project (note: /d/ instead of D:\)
cd "/d/ADITYA/LanShare C++/LanShareCPP"

# Create build directory
mkdir -p build && cd build

# Configure with MinGW
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build
mingw32-make -j4

# Run
./lanshare_server.exe
```

## Comparison

| Method | Size | Time | Difficulty |
|--------|------|------|------------|
| Visual Studio | 6-10 GB | 1 hour | Easy |
| VS Build Tools | 2-3 GB | 30 min | Easy |
| MinGW (MSYS2) | 500 MB | 15 min | Medium |

## Which Should You Choose?

**Choose Visual Studio/Build Tools if:**
- ✅ You want standard Windows C++ development
- ✅ You have good internet and disk space
- ✅ You want best compatibility
- ✅ You're new to C++ on Windows

**Choose MinGW if:**
- ✅ You're comfortable with Linux-style tools
- ✅ You have limited disk space
- ✅ You want faster installation
- ✅ You don't want Microsoft tools

## Current Recommendation

For LanShareCPP project, **Visual Studio is easier** because:
1. vcpkg is designed for it
2. Better Windows compatibility
3. Easier to follow standard tutorials
4. Most Windows C++ developers use it

But MinGW works perfectly fine if you prefer it!
