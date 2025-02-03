echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
echo "     Dynamic.Supervisor Installer (b0003/R)"
echo "     sudo privileges will be asked."
echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

ARCH=$(uname -m)

if [[ "$ARCH" != "x86_64" && "$ARCH" != "aarch64" && "$ARCH" != "armv7l" ]]; then

    echo "Unsupported architecture: $ARCH"

    exit 1

fi

echo ""
echo "=-= Installing JSON, PHP, unzip and wget... =-="
echo ""

sudo apt install nlohmann-json3-dev php unzip wget -y

echo ""
echo "=-= JSON, PHP, unzip and wget installed. =-="
echo ""

echo ""
echo "=-= Setting up web UI... =-="
echo ""

sudo mkdir -p /var/www/html/supervisor/
sudo mkdir -p /etc/gteam/dynamic/supervisor/config /etc/gteam/dynamic/supervisor/results

if [ ! -f /etc/gteam/dynamic/supervisor/config/hosts.json ]; then

  sudo touch /etc/gteam/dynamic/supervisor/config/hosts.json

  sudo bash -c 'cat <<EOF > /etc/gteam/dynamic/supervisor/config/hosts.json
[
    {
        "name": "Local",
        "ip": "127.0.0.1"
    }
]
EOF'

fi

sudo touch /etc/gteam/dynamic/supervisor/results/ping_results.json

sudo wget -O /etc/gteam/dynamic/supervisor/web.zip https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/web.zip

sudo unzip /etc/gteam/dynamic/supervisor/web.zip -d /var/www/html/supervisor/

sudo rm /etc/gteam/dynamic/supervisor/web.zip

echo ""
echo "=-= Web UI online. =-="
echo ""

echo ""
echo "=-= Setting up Supervisor Server as a service... =-="
echo ""

case "$ARCH" in
    x86_64)
        sudo wget -O /etc/gteam/dynamic/supervisor/Dynamic.Supervisor-SRV https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-SRV_x86_64
        ;;
    aarch64)
        sudo wget -O /etc/gteam/dynamic/supervisor/Dynamic.Supervisor-SRV https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-SRV_ARM64
        ;;
    armv7l)
        sudo wget -O /etc/gteam/dynamic/supervisor/Dynamic.Supervisor-SRV https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-SRV_ARM32
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

sudo chown www-data -R /etc/gteam/dynamic/supervisor/

sudo chmod +x /etc/gteam/dynamic/supervisor/Dynamic.Supervisor-SRV

sudo bash -c 'cat <<EOF > /etc/systemd/system/Dynamic.Supervisor-SRV.service
[Unit]
Description=A network/machine supervisor.
After=network.target

[Service]
ExecStart=/etc/gteam/dynamic/supervisor/Dynamic.Supervisor-SRV
WorkingDirectory=/etc/gteam/dynamic/supervisor/
Restart=always
User=www-data
Group=www-data
Environment=PATH=/usr/bin:/usr/local/bin

[Install]
WantedBy=multi-user.target
EOF'

sudo systemctl daemon-reload

sudo systemctl enable Dynamic.Supervisor-SRV.service

sudo systemctl start Dynamic.Supervisor-SRV.service

sudo systemctl status Dynamic.Supervisor-SRV.service

echo ""
echo "=-= Supervisor Server is now a service. =-="
echo ""

echo ""
echo "=-= Setup done! Thank you for using Dynamic.Supervisor! =-="
echo ""