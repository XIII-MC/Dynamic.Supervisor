BUILD_DIR="./build"
BUILD_X86_64_DIR="${BUILD_DIR}/x86_64"
BUILD_ARM64_DIR="${BUILD_DIR}/arm64"
BUILD_ARM32_DIR="${BUILD_DIR}/arm32"

rm -rf "${BUILD_DIR}"

mkdir -p "${BUILD_X86_64_DIR}"
mkdir -p "${BUILD_ARM64_DIR}"
mkdir -p "${BUILD_ARM32_DIR}"

echo "Building for x86_64..."
cmake -S . -B "${BUILD_X86_64_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=x86_64
cmake --build "${BUILD_X86_64_DIR}"

echo "Building for ARM64..."
cmake -S . -B "${BUILD_ARM64_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64
cmake --build "${BUILD_ARM64_DIR}"

echo "Building for ARM32..."
cmake -S . -B "${BUILD_ARM32_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armv7l
cmake --build "${BUILD_ARM32_DIR}"

echo "Builds completed!"
