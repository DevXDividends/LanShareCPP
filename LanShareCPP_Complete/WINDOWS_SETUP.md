# LanShareCPP - Windows Setup Guide

## Prerequisites

- **Windows 10/11** (64-bit)
- **Visual Studio 2019 or 2022** (Community Edition is free)
  - Install "Desktop development with C++" workload
- **Git** (install from https://git-scm.com/)
- **CMake** (install from https://cmake.org/download/)

## Method 1: Using PowerShell (Recommended)

### Step 1: Install vcpkg

Open **PowerShell** (not Git Bash):

```powershell
# Navigate to your project location
cd "D:\ADITYA\LanShare C++\"

# Clone vcpkg (if not already done)
git clone https://github.com/Microsoft/vcpkg.git

# Navigate to vcpkg directory
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# This creates vcpkg.exe
```

### Step 2: Install Dependencies

```powershell
# Install Boost (this takes time - 20-30 minutes)
.\vcpkg install boost-asio:x64-windows boost-system:x64-windows boost-thread:x64-windows

# Install OpenSSL
.\vcpkg install openssl:x64-windows

# Install Qt6 (this takes VERY long - 1-2 hours)
.\vcpkg install qt6-base:x64-windows qt6-tools:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

### Step 3: Build LanShareCPP

```powershell
# Navigate to LanShareCPP directory
cd "D:\ADITYA\LanShare C++\LanShareCPP"

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:\ADITYA\LanShare C++\vcpkg\scripts\buildsystems\vcpkg.cmake"

# Build
cmake --build . --config Release

# Or open the .sln file in Visual Studio
```

## Method 2: Using Git Bash

If you prefer Git Bash:

### Step 1: Bootstrap vcpkg

```bash
# Navigate to vcpkg directory
cd "/d/ADITYA/LanShare C++/vcpkg"

# Use the shell script version
./bootstrap-vcpkg.sh

# OR call the batch file through cmd
cmd //c bootstrap-vcpkg.bat
```

### Step 2: Install Dependencies

```bash
# Use forward slashes in Git Bash
./vcpkg.exe install boost-asio:x64-windows
./vcpkg.exe install boost-system:x64-windows
./vcpkg.exe install boost-thread:x64-windows
./vcpkg.exe install openssl:x64-windows
./vcpkg.exe install qt6-base:x64-windows
```

### Step 3: Build Project

```bash
cd "/d/ADITYA/LanShare C++/LanShareCPP"
mkdir -p build && cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE="/d/ADITYA/LanShare C++/vcpkg/scripts/buildsystems/vcpkg.cmake" -G "Visual Studio 17 2022"

cmake --build . --config Release
```

## Method 3: Using CMake GUI (Easiest for Beginners)

### Step 1: Install Dependencies with PowerShell

```powershell
cd "D:\ADITYA\LanShare C++\vcpkg"
.\bootstrap-vcpkg.bat
.\vcpkg install boost-asio:x64-windows boost-system:x64-windows openssl:x64-windows qt6-base:x64-windows
.\vcpkg integrate install
```

### Step 2: Use CMake GUI

1. Open **CMake GUI**
2. **Where is the source code**: `D:\ADITYA\LanShare C++\LanShareCPP`
3. **Where to build binaries**: `D:\ADITYA\LanShare C++\LanShareCPP\build`
4. Click **Configure**
5. Select **Visual Studio 17 2022** (or your version)
6. Add entry: 
   - Name: `CMAKE_TOOLCHAIN_FILE`
   - Type: `FILEPATH`
   - Value: `D:\ADITYA\LanShare C++\vcpkg\scripts\buildsystems\vcpkg.cmake`
7. Click **Generate**
8. Click **Open Project** (opens Visual Studio)
9. Build in Visual Studio (Build → Build Solution)

## Quick Setup Script

Save this as `setup-windows.bat` and run in PowerShell:

```batch
@echo off
echo ========================================
echo LanShareCPP Windows Setup
echo ========================================

REM Set paths (MODIFY THESE TO MATCH YOUR SYSTEM)
set PROJECT_ROOT=D:\ADITYA\LanShare C++
set VCPKG_ROOT=%PROJECT_ROOT%\vcpkg

echo Checking vcpkg...
if not exist "%VCPKG_ROOT%" (
    echo Cloning vcpkg...
    cd "%PROJECT_ROOT%"
    git clone https://github.com/Microsoft/vcpkg.git
)

echo Bootstrapping vcpkg...
cd "%VCPKG_ROOT%"
call bootstrap-vcpkg.bat

echo Installing dependencies...
echo This will take 30-60 minutes depending on your internet speed
vcpkg install boost-asio:x64-windows
vcpkg install boost-system:x64-windows  
vcpkg install boost-thread:x64-windows
vcpkg install openssl:x64-windows
vcpkg install qt6-base:x64-windows

echo Integrating with Visual Studio...
vcpkg integrate install

echo.
echo ========================================
echo Setup Complete!
echo ========================================
echo.
echo Next steps:
echo 1. Open PowerShell or CMD
echo 2. cd "%PROJECT_ROOT%\LanShareCPP"
echo 3. mkdir build
echo 4. cd build
echo 5. cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
echo 6. cmake --build . --config Release
echo.
pause
```

## Minimal Dependencies (Faster Install)

If Qt6 takes too long, you can start with just the server:

```powershell
# Install only what's needed for server and console client
.\vcpkg install boost-asio:x64-windows
.\vcpkg install boost-system:x64-windows
.\vcpkg install openssl:x64-windows

# Build without GUI components
cmake .. -DCMAKE_TOOLCHAIN_FILE="path\to\vcpkg\scripts\buildsystems\vcpkg.cmake" -DBUILD_GUI=OFF
```

## Common Issues & Solutions

### Issue 1: "bootstrap-vcpkg.bat: command not found"
**Solution**: You're in Git Bash. Use PowerShell/CMD or run:
```bash
cmd //c bootstrap-vcpkg.bat
```

### Issue 2: "CMake could not find Boost"
**Solution**: Make sure you specified the toolchain file:
```powershell
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

### Issue 3: vcpkg install hangs or is very slow
**Solution**: 
- Check your internet connection
- Qt6 installation takes 1-2 hours - this is normal
- You can install dependencies one at a time

### Issue 4: "LINK : fatal error LNK1104: cannot open file 'boost_system-vc142-mt-x64-1_XX.lib'"
**Solution**: 
```powershell
# Reinstall boost
.\vcpkg remove boost-asio:x64-windows
.\vcpkg install boost-asio:x64-windows --recurse
```

### Issue 5: CMake can't find Qt6
**Solution**:
```powershell
# Set Qt path manually
cmake .. -DCMAKE_TOOLCHAIN_FILE="path\to\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_PREFIX_PATH="path\to\vcpkg\installed\x64-windows"
```

## Verify Installation

After installation, verify dependencies:

```powershell
# Check vcpkg packages
.\vcpkg list

# Should show:
# boost-asio:x64-windows
# boost-system:x64-windows
# openssl:x64-windows
# qt6-base:x64-windows (if installed)
```

## Building for Different Configurations

### Debug Build
```powershell
cmake --build . --config Debug
```

### Release Build (Optimized)
```powershell
cmake --build . --config Release
```

### Both
```powershell
cmake --build . --config Debug
cmake --build . --config Release
```

## Running the Applications

After successful build:

```powershell
# Navigate to build directory
cd build

# Run server (Debug)
.\Debug\lanshare_server.exe

# Run server (Release)
.\Release\lanshare_server.exe

# Run GUI client (if built)
.\Release\lanshare_gui.exe
```

## Visual Studio Development

If you want to develop in Visual Studio:

1. Build once using CMake to generate .sln file
2. Open `build\LanShareCPP.sln` in Visual Studio
3. Set startup project (right-click on project → Set as Startup Project)
4. Press F5 to run with debugging

## Recommended: Use Visual Studio Installer

Alternative to vcpkg - use Visual Studio installer:

1. Open **Visual Studio Installer**
2. Modify your installation
3. Go to **Individual Components**
4. Search and install:
   - C++ CMake tools for Windows
   - C++ Boost libraries
   - C++ OpenSSL support
   - Qt (if available)

This is simpler but less flexible than vcpkg.

## Environment Variables

Add to PATH for easier access:

```powershell
# Add vcpkg to PATH (optional)
$env:Path += ";D:\ADITYA\LanShare C++\vcpkg"

# Or permanently in System Settings:
# Settings → System → About → Advanced system settings → Environment Variables
```

## Next Steps

Once dependencies are installed:

1. Read `START_HERE.md` in the project root
2. Follow `docs\ROADMAP.md` for implementation steps
3. Start with server implementation (Week 1)

## Need Help?

- **CMake Issues**: Check `CMakeCache.txt` in build directory
- **Linking Issues**: Ensure you're using the same architecture (x64)
- **Runtime Issues**: Copy DLLs from vcpkg\installed\x64-windows\bin to your exe directory

---

**Estimated Setup Time:**
- vcpkg bootstrap: 5 minutes
- Boost + OpenSSL: 20-30 minutes
- Qt6: 60-120 minutes
- Total: ~2 hours

**Recommendation**: Start the installation and work on reading the documentation while it installs!
