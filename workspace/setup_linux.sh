# Make sure your up to date
sudo apt update && sudo apt upgrade -y

# Needed for CMake to compile
sudo apt install build-essential

# Needed for JSON parsing
sudo apt install rapidjson-dev

# Needed for Cown (backend)
sudo apt install libboost-all-dev libssl-dev

# Install Crow
mkdir -p include
git submodule add -f https://github.com/CrowCpp/Crow.git include/Crow
git submodule update --init --recursive