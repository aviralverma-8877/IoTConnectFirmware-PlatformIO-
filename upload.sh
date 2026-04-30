#!/bin/bash
# PlatformIO Upload Script with SSL Certificate Fix

# Get the correct certificate path
CERT_PATH=$(python -c "import certifi; print(certifi.where())")

# Set environment variables for SSL
export SSL_CERT_FILE="$CERT_PATH"
export REQUESTS_CA_BUNDLE="$CERT_PATH"

# Upload firmware to device
echo "Uploading firmware to ESP-12E..."
pio run -e esp12e -t upload

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Upload Successful!"
    echo ""
    echo "To monitor serial output:"
    echo "  pio device monitor -b 115200"
else
    echo ""
    echo "❌ Upload Failed!"
    exit 1
fi
