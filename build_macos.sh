#!/usr/bin/env bash
# ==============================================================================
# IT26 - macOS Build & Bundle Packaging Script
# Native 64-bit Workstation based on Jeffrey Lim's Impulse Tracker
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

echo "======================================================================"
echo " Building IT26 for macOS (Universal / Apple Silicon & Intel)"
echo "======================================================================"

GENERATOR="Unix Makefiles"
if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
fi

BUILD_DIR="build-macos"
mkdir -p "${BUILD_DIR}"

# Detect architectures
ARCH_FLAG=""
if [ "$(uname -s)" = "Darwin" ]; then
    ARCH_FLAG="-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
fi

echo ">> Configuring with CMake (${GENERATOR})..."
cmake -B "${BUILD_DIR}" -G "${GENERATOR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DIT_FETCH_SDL=ON \
    ${ARCH_FLAG}

echo ">> Compiling IT26.app bundle..."
cmake --build "${BUILD_DIR}" --config Release -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ">> Bundling resources into IT26.app..."
APP_BUNDLE="${BUILD_DIR}/IT26.app"
if [ ! -d "${APP_BUNDLE}" ]; then
    APP_BUNDLE="${BUILD_DIR}/impulse-tracker.app"
fi

if [ -d "${APP_BUNDLE}" ]; then
    RESOURCES_DIR="${APP_BUNDLE}/Contents/Resources"
    mkdir -p "${RESOURCES_DIR}/assets"
    cp head.jpg "${RESOURCES_DIR}/" || true
    cp -r assets/* "${RESOURCES_DIR}/assets/" || true
    cp LICENSE README.md "${RESOURCES_DIR}/" || true
fi

echo ">> Staging macOS distribution package..."
DIST_DIR="dist/macos"
mkdir -p "${DIST_DIR}"
rm -rf "${DIST_DIR}/IT26.app"
cp -R "${APP_BUNDLE}" "${DIST_DIR}/IT26.app"
cp LICENSE README.md "${DIST_DIR}/"

echo ">> Creating macOS archive..."
mkdir -p dist
(cd "${DIST_DIR}" && zip -r -y -q "../../dist/IT26-macos-universal.zip" IT26.app LICENSE README.md)

echo "======================================================================"
echo " IT26 macOS build successful!"
echo " Bundle:  ${DIST_DIR}/IT26.app"
echo " Archive: dist/IT26-macos-universal.zip"
echo "======================================================================"
