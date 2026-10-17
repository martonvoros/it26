#!/usr/bin/env bash
# ==============================================================================
# IT26 - Linux Build & Packaging Script
# Native 64-bit Workstation based on Jeffrey Lim's Impulse Tracker
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

echo "======================================================================"
echo " Building IT26 for Linux (x86_64)"
echo "======================================================================"

# Determine generator
GENERATOR="Unix Makefiles"
if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
fi

BUILD_DIR="build-linux"
mkdir -p "${BUILD_DIR}"

echo ">> Configuring with CMake (${GENERATOR})..."
cmake -B "${BUILD_DIR}" -G "${GENERATOR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DIT_FETCH_SDL=ON

echo ">> Compiling IT26..."
cmake --build "${BUILD_DIR}" --config Release -j"$(nproc 2>/dev/null || echo 4)"

echo ">> Staging Linux distribution bundle..."
DIST_DIR="dist/IT26-linux-x86_64"
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}/assets" "${DIST_DIR}/res"

# Copy binary
if [ -f "${BUILD_DIR}/IT26" ]; then
    cp "${BUILD_DIR}/IT26" "${DIST_DIR}/IT26"
elif [ -f "${BUILD_DIR}/impulse-tracker" ]; then
    cp "${BUILD_DIR}/impulse-tracker" "${DIST_DIR}/IT26"
fi
chmod +x "${DIST_DIR}/IT26"

# Copy assets & documentation
cp head.jpg "${DIST_DIR}/"
cp -r assets/* "${DIST_DIR}/assets/"
cp LICENSE README.md "${DIST_DIR}/"
if [ -f res/it26.desktop ]; then
    cp res/it26.desktop "${DIST_DIR}/res/"
fi
if [ -f res/impulse.ico ]; then
    cp res/impulse.ico "${DIST_DIR}/res/"
fi

# Create launcher script
cat << 'EOF' > "${DIST_DIR}/run.sh"
#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"
exec ./IT26 "$@"
EOF
chmod +x "${DIST_DIR}/run.sh"

echo ">> Creating tarball..."
mkdir -p dist
tar -czf "dist/IT26-linux-x86_64.tar.gz" -C dist IT26-linux-x86_64

echo "======================================================================"
echo " IT26 Linux build successful!"
echo " Binary:  ${DIST_DIR}/IT26"
echo " Package: dist/IT26-linux-x86_64.tar.gz"
echo "======================================================================"
