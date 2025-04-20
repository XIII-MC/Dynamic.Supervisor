# Make sure your up to date
sudo apt update && sudo apt upgrade -y

# Needed for CMake to compile
sudo apt install build-essential

# Needed for JSON parsing
sudo apt install rapidjson-dev

# Needed for Cown (backend)
sudo apt install libasio-dev

# Download Cown and install it
# !! MAKE SURE TO USE LATEST !!
wget -O crow.deb https://github.com/CrowCpp/Crow/releases/download/v1.2.1.2/Crow-1.2.1-Linux.deb
sudo dpkg -i crow.deb