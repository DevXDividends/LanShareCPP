# LanShareCPP - Windows Installation Troubleshooting

## Current Issue: "Unable to find a valid Visual Studio instance"

### What This Means
vcpkg needs a C++ compiler to build packages. On Windows, it expects Visual Studio.

### Solution Options (Pick One)

---

## ✅ RECOMMENDED: Option 1 - Install Visual Studio 2022 Community

### Quick Steps:
1. **Download**: https://visualstudio.microsoft.com/downloads/
2. **Install** → Select "Desktop development with C++"
3. **Restart** command prompt
4. **Continue** with vcpkg installation

### Detailed Steps:

**Step 1: Download Visual Studio**
- Visit: https://visualstudio.microsoft.com/downloads/
- Under "Community" edition, click "Free download"
- File size: ~3 MB (installer), then downloads ~6 GB during install

**Step 2: Run Installer**
- Run `VisualStudioSetup.exe`
- When asked to select workloads, check:
  - ✅ **Desktop development with C++** (REQUIRED)
  
  Optional but helpful:
  - ✅ C++ CMake tools for Windows
  - ✅ Windows 10/11 SDK (latest version)
  - ✅ C++ Clang tools for Windows

**Step 3: Wait for Installation**
- Installation takes 30-60 minutes depending on internet speed
- Requires ~10-15 GB disk space

**Step 4: Verify Installation**
```cmd
# Open NEW command prompt (important!)
where cl
# Should show: C:\Program Files\Microsoft Visual Studio\...\cl.exe
```

**Step 5: Continue vcpkg Installation**
```cmd
cd "D:\ADITYA\LanShare C++\vcpkg\vcpkg"

# This should work now
.\vcpkg install boost-asio:x64-windows
.\vcpkg install boost-system:x64-windows
.\vcpkg install boost-thread:x64-windows
.\vcpkg install openssl:x64-windows

# Optional: Qt6 (takes 1-2 hours)
.\vcpkg install qt6-base:x64-windows
```

---

## 🔧 Option 2 - Build Tools Only (Lighter)

If you don't want the full Visual Studio IDE:

**Step 1: Download Build Tools**
- Visit: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
- Download "Build Tools for Visual Studio 2022"
- File size: Smaller than full Visual Studio

**Step 2: Install Required Components**
- Run installer
- Select:
  - ✅ **Desktop development with C++**
  - ✅ MSVC v143 - VS 2022 C++ x64/x86 build tools
  - ✅ Windows 10/11 SDK

**Step 3: Continue with vcpkg**
- Same as Option 1, Step 5

---

## 🐧 Option 3 - Use MinGW (Linux-style)

For those who prefer GCC over MSVC:

**Step 1: Install MSYS2**
- Download: https://www.msys2.org/
- Install to default location: `C:\msys64`

**Step 2: Open MSYS2 MINGW64 Terminal**
```bash
# Update system
pacman -Syu
# Close and reopen terminal
pacman -Su

# Install tools
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-boost
pacman -S mingw-w64-x86_64-openssl
```

**Step 3: Build Without vcpkg**
```bash
cd "/d/ADITYA/LanShare C++/LanShareCPP"
mkdir -p build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

See `MINGW_SETUP.md` for full details.

---

## 📋 After Installing Visual Studio

### Verify Installation
```cmd
# Open NEW Command Prompt
cd "D:\ADITYA\LanShare C++\vcpkg\vcpkg"

# Test vcpkg can find Visual Studio
.\vcpkg install boost-asio:x64-windows --dry-run

# If it works, you'll see package list without errors
```

### Install All Dependencies
```cmd
# Minimum (server only) - ~30 minutes
.\vcpkg install boost-asio:x64-windows
.\vcpkg install boost-system:x64-windows
.\vcpkg install boost-thread:x64-windows
.\vcpkg install openssl:x64-windows

# Full (with GUI) - ~2 hours
.\vcpkg install boost-asio:x64-windows
.\vcpkg install boost-system:x64-windows
.\vcpkg install boost-thread:x64-windows
.\vcpkg install openssl:x64-windows
.\vcpkg install qt6-base:x64-windows
.\vcpkg install qt6-tools:x64-windows
```

### Integrate with Visual Studio
```cmd
.\vcpkg integrate install
```

This makes packages available to all Visual Studio projects.

---

## 🏗️ Build LanShareCPP

Once dependencies are installed:

```cmd
cd "D:\ADITYA\LanShare C++\LanShareCPP"
mkdir build
cd build

# Configure
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:\ADITYA\LanShare C++\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake"

# Build
cmake --build . --config Release

# Run server
.\Release\lanshare_server.exe
```

---

## 🔍 Common Issues After Installing Visual Studio

### Issue 1: "vcpkg still can't find Visual Studio"

**Solution:**
```cmd
# Close ALL command prompts and PowerShell windows
# Open NEW command prompt
# Visual Studio sets environment variables that need fresh shell
```

### Issue 2: "Wrong Visual Studio version"

**Solution:**
```cmd
# Tell vcpkg which version to use
set VCPKG_PLATFORM_TOOLSET=v143
.\vcpkg install boost-asio:x64-windows
```

### Issue 3: "CMake can't find compiler"

**Solution:**
```cmd
# Use Visual Studio Developer Command Prompt
# Start Menu → Visual Studio 2022 → Developer Command Prompt
cd "D:\ADITYA\LanShare C++\LanShareCPP\build"
cmake .. -G "Visual Studio 17 2022"
```

### Issue 4: vcpkg packages install but CMake still fails

**Solution:**
```cmd
# Ensure you specify toolchain file
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:\ADITYA\LanShare C++\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake"

# Or set environment variable
set CMAKE_TOOLCHAIN_FILE=D:\ADITYA\LanShare C++\vcpkg\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake ..
```

---

## 📊 Installation Time Estimates

| Component | Download | Install | Total |
|-----------|----------|---------|-------|
| Visual Studio 2022 | 10 min | 30-45 min | ~1 hour |
| vcpkg bootstrap | 1 min | 1 min | 2 min |
| Boost packages | 5 min | 15-25 min | ~30 min |
| OpenSSL | 2 min | 5-10 min | ~12 min |
| Qt6 (optional) | 20 min | 60-90 min | ~2 hours |
| **Total (no Qt)** | - | - | **~1.5 hours** |
| **Total (with Qt)** | - | - | **~3.5 hours** |

---

## 💡 Tips While Waiting

While Visual Studio and packages install:

1. **Read Documentation**
   - `START_HERE.md` - Project overview
   - `docs/ROADMAP.md` - Implementation plan
   - `docs/DEVELOPMENT.md` - Architecture guide

2. **Study the Code**
   - Review header files in `common/`, `server/`, `client/`
   - Understand the protocol in `common/Protocol.h`
   - Check encryption in `common/AESGCM.h`

3. **Plan Your Implementation**
   - Decide which components to implement first
   - Read through example code in ROADMAP.md
   - Think about test cases

---

## 🎯 Quick Decision Tree

**Do you have Visual Studio installed?**
- ❌ No → Install Visual Studio 2022 Community (Option 1)
- ✅ Yes, but old version → Install VS 2022 or Build Tools
- ✅ Yes, VS 2022 → Skip to "Install Dependencies"

**Do you want GUI support?**
- ✅ Yes → Install Qt6 (adds 2 hours)
- ❌ No → Skip Qt6, install later if needed

**How much disk space do you have?**
- ✅ 20+ GB → Install everything
- ⚠️ 10-20 GB → Install VS + minimal deps (no Qt)
- ❌ <10 GB → Use MinGW/MSYS2 method

**What's your experience level?**
- 🟢 Beginner → Use Visual Studio (most common)
- 🟡 Intermediate → VS or Build Tools
- 🔵 Advanced → MinGW if you prefer GCC

---

## ✅ Next Steps Checklist

After successfully installing dependencies:

- [ ] Visual Studio 2022 installed
- [ ] vcpkg bootstrap complete
- [ ] boost-asio installed
- [ ] boost-system installed
- [ ] boost-thread installed
- [ ] openssl installed
- [ ] (Optional) Qt6 installed
- [ ] vcpkg integrate install run
- [ ] LanShareCPP built successfully
- [ ] Server runs without errors

---

## 📞 Still Having Issues?

### Check These:

1. **Antivirus**: Disable temporarily during install
2. **Disk Space**: Ensure 20+ GB free
3. **Internet**: Stable connection required
4. **Windows Version**: Windows 10/11 required
5. **Admin Rights**: Run installer as administrator

### Debug Commands:

```cmd
# Check Visual Studio installation
where cl

# Check vcpkg can see Visual Studio
.\vcpkg version --debug

# List installed packages
.\vcpkg list

# Remove and reinstall package
.\vcpkg remove boost-asio:x64-windows
.\vcpkg install boost-asio:x64-windows --recurse
```

---

**Recommendation**: Install Visual Studio 2022 Community. It's the most reliable method for Windows C++ development and has the best tooling support.

**Estimated Time to Working Build**: 2-4 hours (mostly waiting for downloads/installs)
