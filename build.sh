#!/bin/bash
# PlatformIO Build Script with SSL Certificate Fix
# This script sets the correct SSL certificate path and builds the firmware

# Get the correct certificate path
CERT_PATH=$(python -c "import certifi; print(certifi.where())")

# Set environment variables for SSL
export SSL_CERT_FILE="$CERT_PATH"
export REQUESTS_CA_BUNDLE="$CERT_PATH"

# Build for ESP-12E (default)
echo "Building firmware for ESP-12E..."
pio run -e esp12e

# Show build results
if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "✅ Build Successful!"
    echo "========================================="
    echo ""
    echo "Firmware Location: .pio/build/esp12e/firmware.bin"
    echo "Size: $(du -h .pio/build/esp12e/firmware.bin | cut -f1)"
    echo ""
    echo "To upload to device:"
    echo "  ./upload.sh"
    echo ""
    echo "To upload filesystem:"
    echo "  pio run -e esp12e -t uploadfs"
else
    echo ""
    echo "❌ Build Failed!"
    exit 1
fi
