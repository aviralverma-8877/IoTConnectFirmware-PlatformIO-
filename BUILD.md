# Build Instructions

This document provides detailed instructions for building and flashing the IoTConnect firmware.

## 🛠️ Prerequisites

### Required Software
- **Python 3.7+** - [Download Python](https://www.python.org/downloads/)
- **PlatformIO Core** - Will be installed in setup
- **Git** - For version control

### Hardware Requirements
- ESP8266 development board (ESP-12E, NodeMCU, etc.)
- USB cable for programming
- 3.3V power supply (500mA+ recommended)

## 📦 Installation

### 1. Install PlatformIO

```bash
# Using pip (recommended)
pip install --trusted-host pypi.org --trusted-host files.pythonhosted.org platformio

# Verify installation
pio --version
```

**Note**: If you encounter SSL certificate errors, use the `--trusted-host` flags as shown above.

### 2. Clone Repository

```bash
git clone https://github.com/aviralverma-8877/IoTConnectFirmware-PlatformIO-.git
cd IoTConnectFirmware-PlatformIO-
```

### 3. Configure Credentials

⚠️ **Important**: Set up MQTT credentials before building!

```bash
# Copy example file
cp src/iotconnect_mqtt_cred.h.example src/iotconnect_mqtt_cred.h

# Edit with your credentials
nano src/iotconnect_mqtt_cred.h  # or use any text editor
```

Update the following in `src/iotconnect_mqtt_cred.h`:
```cpp
#define MQTT_HOST "your-mqtt-broker.com"
#define MQTT_PORT 1883
#define MQTT_UNAME "your_username"
#define MQTT_PASS "your_password"
```

## 🔨 Building the Firmware

### Option 1: Using Build Script (Recommended)

The repository includes convenience scripts that handle SSL certificate configuration:

```bash
# Build firmware
./build.sh

# Upload to device
./upload.sh
```

### Option 2: Using PlatformIO Directly

If you encounter SSL certificate errors, set the certificate path first:

```bash
# Set certificate path (Linux/Mac)
export SSL_CERT_FILE=$(python -c "import certifi; print(certifi.where())")
export REQUESTS_CA_BUNDLE=$(python -c "import certifi; print(certifi.where())")

# Windows (Git Bash)
export SSL_CERT_FILE="$(python -c "import certifi; print(certifi.where())")"
export REQUESTS_CA_BUNDLE="$(python -c "import certifi; print(certifi.where())")"

# Build for ESP-12E (default)
pio run -e esp12e

# Build for other boards
pio run -e nodemcuv2    # NodeMCU v2
pio run -e esp01_1m     # ESP-01 (1MB)
pio run -e huzzah       # Adafruit Huzzah
pio run -e thing        # SparkFun Thing
```

### Option 3: Using PlatformIO IDE

1. Open project in VS Code with PlatformIO extension
2. Select environment (esp12e, nodemcuv2, etc.)
3. Click "Build" button in PlatformIO toolbar

## 📤 Flashing the Firmware

### Upload Firmware

```bash
# Using script
./upload.sh

# Using PlatformIO
pio run -e esp12e -t upload

# Specify port manually
pio run -e esp12e -t upload --upload-port COM3     # Windows
pio run -e esp12e -t upload --upload-port /dev/ttyUSB0  # Linux
```

### Upload Filesystem (Web Interface)

The filesystem contains the web interface files (HTML, CSS, JavaScript):

```bash
# Upload filesystem data
pio run -e esp12e -t uploadfs
```

**⚠️ Important**: Upload filesystem on first flash or after modifying files in `data/` directory.

## 📊 Build Output

### Successful Build
```
RAM:   [=====     ]  47.3% (used 38756 bytes from 81920 bytes)
Flash: [=====     ]  47.7% (used 498693 bytes from 1044464 bytes)
========================= [SUCCESS] Took 5.13 seconds =========================
```

### Build Artifacts Location
```
.pio/build/esp12e/
├── firmware.bin     # Flash this to device (~492KB)
└── firmware.elf     # Debug symbols (~1.9MB)
```

## 🔍 Monitoring & Debugging

### Serial Monitor

```bash
# Monitor serial output
pio device monitor -b 115200

# Exit monitor: Ctrl+C
```

### View Debug Output

The firmware outputs JSON-formatted debug messages:
```json
{"action":"display","head":"Setup","body":"LittleFS initialized."}
{"action":"display","head":"onWifiConnect","body":"WiFi Connected"}
```

## 🐛 Troubleshooting

### SSL Certificate Errors

**Error**: `Could not find a suitable TLS CA certificate bundle`

**Solution**:
```bash
# Find certificate path
python -c "import certifi; print(certifi.where())"

# Set environment variables
export SSL_CERT_FILE="<path-from-above>"
export REQUESTS_CA_BUNDLE="<path-from-above>"

# Or use build.sh script which handles this automatically
./build.sh
```

### Compilation Errors

```bash
# Clean build directory
pio run -t clean

# Update libraries
pio lib update

# Rebuild
pio run -e esp12e
```

### Upload Fails

1. **Check USB cable**: Use a data cable, not charge-only
2. **Check driver**: Install CH340/CP2102 USB-to-Serial driver
3. **Check port**: 
   ```bash
   pio device list  # List available ports
   ```
4. **Enter flash mode**: 
   - Hold BOOT/FLASH button
   - Press RESET button
   - Release RESET
   - Release BOOT

### Out of Memory Errors

If you get compilation errors about memory:

1. **Reduce firmware features**: Comment out unused modules
2. **Optimize for size**: Add to `platformio.ini`:
   ```ini
   build_flags = -Os  # Optimize for size instead of speed
   ```

### Missing Dependencies

```bash
# Install missing dependencies
pio lib install

# Or force reinstall
pio lib install --force
```

## 📈 Build Statistics

| Board | Flash Used | RAM Used | Status |
|-------|------------|----------|--------|
| ESP-12E | 47.7% (498KB) | 47.3% (38KB) | ✅ Tested |
| NodeMCU v2 | ~47% | ~47% | ✅ Tested |
| ESP-01 (1MB) | ~95% | ~47% | ⚠️ Limited |
| Huzzah | ~47% | ~47% | ✅ Tested |
| Thing | ~47% | ~47% | ✅ Tested |

## 🚀 Advanced Options

### Verbose Build

```bash
pio run -e esp12e -v
```

### Clean Build

```bash
# Clean specific environment
pio run -e esp12e -t clean

# Clean all
pio run -t clean
```

### Build All Environments

```bash
# Build for all configured boards
pio run
```

### Custom Build Flags

Edit `platformio.ini`:
```ini
[env:esp12e]
build_flags = 
    -DDEBUG_ESP_PORT=Serial
    -DDEBUG_ESP_SSL
    -Os  # Optimize for size
```

## 📝 Environment Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `SSL_CERT_FILE` | SSL certificate bundle path | `/path/to/cacert.pem` |
| `REQUESTS_CA_BUNDLE` | Requests library cert path | `/path/to/cacert.pem` |
| `PLATFORMIO_UPLOAD_PORT` | Default upload port | `COM3` or `/dev/ttyUSB0` |

## 🔄 Continuous Integration

The project includes a Jenkinsfile for CI/CD:

```groovy
// Builds all board variants
- esp01_1m
- esp12e
- nodemcuv2
- huzzah
- thing
```

Output artifacts are placed in `build/` directory.

## 📚 Additional Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP8266 Arduino Core](https://arduino-esp8266.readthedocs.io/)
- [Project README](README.md)
- [Improvements Log](IMPROVEMENTS.md)

## 💡 Tips

1. **First build takes longer**: PlatformIO downloads tools and libraries
2. **Incremental builds are fast**: Subsequent builds only compile changed files
3. **Use build.sh script**: Handles SSL certificates automatically
4. **Monitor serial output**: Helps debug issues during development
5. **Upload filesystem first**: Ensures web interface works correctly

## 🆘 Getting Help

If you encounter issues:

1. Check this troubleshooting guide
2. Review [GitHub Issues](https://github.com/aviralverma-8877/IoTConnectFirmware-PlatformIO-/issues)
3. Enable verbose mode: `pio run -v`
4. Check PlatformIO forums: [community.platformio.org](https://community.platformio.org/)

---

**Last Updated**: 2026-04-30  
**PlatformIO Version**: 6.1.19  
**Platform**: ESP8266 @ 4.2.1
