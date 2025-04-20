echo "[...] Preparing..."
BUILD_DIR="./build"
BUILD_X86_64_DIR="${BUILD_DIR}/x86_64"
BUILD_ARM64_DIR="${BUILD_DIR}/arm64"
BUILD_ARM32_DIR="${BUILD_DIR}/arm32"

rm -rf "${BUILD_DIR}"

mkdir -p "${BUILD_X86_64_DIR}"
mkdir -p "${BUILD_ARM64_DIR}"
mkdir -p "${BUILD_ARM32_DIR}"

echo "[...] Building for x86_64..."
cmake -S . -B "${BUILD_X86_64_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=x86_64
cmake --build "${BUILD_X86_64_DIR}"
echo "[OK!] Build for x86_64 successful!"

echo "[...] Building for ARM64..."
cmake -S . -B "${BUILD_ARM64_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
cmake --build "${BUILD_ARM64_DIR}"
echo "[OK!] Build for ARM64 successful!"

echo "[...] Building for ARM32..."
cmake -S . -B "${BUILD_ARM32_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=armv7l \
    -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
    -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++
cmake --build "${BUILD_ARM32_DIR}"
echo "[OK!] Build for ARM32 successful!"

echo "[OK!] All builds completed!"