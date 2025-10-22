#!/usr/bin/env bash
# configure.sh
# Usage: ./configure.sh [download_url] [out_dir] [--force]
# Downloads the traderapi zip from GitHub raw URL and extracts it into ./res

set -euo pipefail

URL_DEFAULT="https://raw.githubusercontent.com/hanjingo/livermore-res/main/v6.7.11_20250714_traderapi.zip"
URL="${1:-$URL_DEFAULT}"
OUT_DIR="${2:-$(pwd)/res}"
FORCE=false
if [[ "${3:-}" == "--force" ]]; then
    FORCE=true
fi

echo "[INFO] Download URL: $URL"
echo "[INFO] Output dir: $OUT_DIR"

mkdir -p "$OUT_DIR"
ZIP_PATH="$OUT_DIR/ctp_download.zip"

if [[ -f "$ZIP_PATH" ]]; then
    if [[ "$FORCE" == "true" ]]; then
        rm -f "$ZIP_PATH"
        echo "[INFO] Removed existing $ZIP_PATH due to --force"
    else
        echo "[INFO] $ZIP_PATH already exists. Use --force to overwrite or remove it manually." >&2
        exit 0
    fi
fi

# Download
if command -v curl >/dev/null 2>&1; then
    echo "[INFO] Downloading with curl..."
    curl -L "$URL" -o "$ZIP_PATH"
elif command -v wget >/dev/null 2>&1; then
    echo "[INFO] Downloading with wget..."
    wget -O "$ZIP_PATH" "$URL"
else
    echo "[ERROR] Neither curl nor wget is available." >&2
    exit 1
fi

# Extract
if command -v unzip >/dev/null 2>&1; then
    echo "[INFO] Extracting with unzip..."
    unzip -o "$ZIP_PATH" -d "$OUT_DIR"
elif command -v bsdtar >/dev/null 2>&1; then
    echo "[INFO] Extracting with bsdtar..."
    bsdtar -xf "$ZIP_PATH" -C "$OUT_DIR"
else
    echo "[ERROR] No unzip or bsdtar found to extract ZIP file." >&2
    exit 1
fi

# cleanup
rm -f "$ZIP_PATH"

echo "[INFO] Extraction complete. Files are in: $OUT_DIR"
echo "[INFO] If using CMake, remove the build/ directory and re-run configuration to pick up headers."

exit 0
