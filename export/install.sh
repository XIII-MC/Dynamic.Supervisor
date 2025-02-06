install_server() {

  echo ": Setting up Supervisor-Server as a service..."

  sudo mkdir -p /etc/gteam/dynamic/supervisor/server/config /etc/gteam/dynamic/supervisor/server/results

  case "$ARCH" in
      x86_64)
          sudo wget -O /etc/gteam/dynamic/supervisor/server/Dynamic.Supervisor-SRV https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-SRV_x86_64
          ;;
      aarch64)
          sudo wget -O /etc/gteam/dynamic/supervisor/server/Dynamic.Supervisor-SRV https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-SRV_ARM64
          ;;
      armv7l)
          sudo wget -O /etc/gteam/dynamic/supervisor/server/Dynamic.Supervisor-SRV https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-SRV_ARM32
          ;;
      *)
          echo "! Unsupported architecture: $ARCH (exit code 'arch unsupported/1')"
          exit 1
          ;;
  esac

  if [ ! -f /etc/gteam/dynamic/supervisor/server/config/hosts.json ]; then

    sudo touch /etc/gteam/dynamic/supervisor/server/config/hosts.json

    sudo bash -c 'cat <<EOF > /etc/gteam/dynamic/supervisor/server/config/hosts.json
[
  {
    "name": "Local",
    "ip": "127.0.0.1"
  }
]
EOF'

    fi

  sudo touch /etc/gteam/dynamic/supervisor/server/results/ping_results.json

  sudo chmod +x /etc/gteam/dynamic/supervisor/server/Dynamic.Supervisor-SRV

  sudo chown www-data -R /etc/gteam/dynamic/supervisor/server/

  sudo bash -c 'cat <<EOF > /etc/systemd/system/Dynamic.Supervisor-SRV.service
[Unit]
Description=A network/machine supervisor.
After=network.target

[Service]
ExecStart=/etc/gteam/dynamic/supervisor/server/Dynamic.Supervisor-SRV
WorkingDirectory=/etc/gteam/dynamic/supervisor/server/
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

  echo ": Supervisor-Server is now a service."

}

install_web_ui() {

  echo ": Setting up web UI..."

  echo ": Installing JSON, PHP, unzip and wget..."

  sudo apt install nlohmann-json3-dev php unzip wget -y

  echo ": JSON, PHP, unzip and wget installed."

  sudo mkdir -p /var/www/html/supervisor/

  sudo wget -O /etc/gteam/dynamic/supervisor/web.zip https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/web.zip

  sudo unzip /etc/gteam/dynamic/supervisor/web.zip -d /var/www/html/supervisor/

  sudo chown www-data -R /var/www/html/supervisor/

  sudo rm /etc/gteam/dynamic/supervisor/web.zip

  echo ": Web UI online."

}

install_client() {

  echo ": Setting up Supervisor-Client as a service..."

  sudo mkdir -p /etc/gteam/dynamic/supervisor/client/config /etc/gteam/dynamic/supervisor/client/results

  case "$ARCH" in

    x86_64)
      sudo wget -O /etc/gteam/dynamic/supervisor/client/Dynamic.Supervisor-CLT https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-CLT_x86_64
      ;;
    aarch64)
      sudo wget -O /etc/gteam/dynamic/supervisor/client/Dynamic.Supervisor-CLT https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-CLT_ARM64
      ;;
    armv7l)
      sudo wget -O /etc/gteam/dynamic/supervisor/client/Dynamic.Supervisor-CLT https://github.com/XIII-MC/Dynamic.Supervisor/releases/latest/download/Dynamic.Supervisor-CLT_ARM32
      ;;
    *)

      echo "! Unsupported architecture: $ARCH (exit code 'arch unsupported/2')"
      exit 1
      ;;

  esac

  if [ ! -f /etc/gteam/dynamic/supervisor/client/config/hosts.json ]; then

      sudo touch /etc/gteam/dynamic/supervisor/client/config/hosts.json

      read -r -p "What is the Supervisor-Server IP address ? " supervisorServerIP

      sudo bash -c "cat <<EOF > /etc/gteam/dynamic/supervisor/client/config/hosts.json
{
  \"allowed_ip\": \"${supervisorServerIP}\"
}
EOF"

  fi

  sudo touch /etc/gteam/dynamic/supervisor/client/results/monitor_results.json

  sudo chmod +x /etc/gteam/dynamic/supervisor/client/Dynamic.Supervisor-CLT

  sudo chown root -R /etc/gteam/dynamic/supervisor/client/

  sudo bash -c 'cat <<EOF > /etc/systemd/system/Dynamic.Supervisor-CLT.service
[Unit]
Description=A network/machine supervisor.
After=network.target

[Service]
ExecStart=/etc/gteam/dynamic/supervisor/client/Dynamic.Supervisor-CLT
WorkingDirectory=/etc/gteam/dynamic/supervisor/client/
Restart=always
User=root
Group=root
Environment=PATH=/usr/bin:/usr/local/bin

[Install]
WantedBy=multi-user.target
EOF'

  sudo systemctl daemon-reload

  sudo systemctl enable Dynamic.Supervisor-CLT.service

  sudo systemctl start Dynamic.Supervisor-CLT.service

  sudo systemctl status Dynamic.Supervisor-CLT.service

  echo ": Supervisor-Client is now a service."

}

echo ""
echo "- GTeam's Dynamic.Supervisor Setup Script | Build N°4/NLTS/NHF/RELEASE | Tested & built for Debian/Ubuntu -"
echo "! This is NOT a LTS (Long Term Support) version."
echo "! This is NOT a HF (HotFix) version."
echo "- Thank you for using our script! (https://github.com/GTeamX/Dynamic.Supervisor) -"
echo ""

ARCH=$(uname -m)

if [[ "$ARCH" != "x86_64" && "$ARCH" != "aarch64" && "$ARCH" != "armv7l" ]]; then

    echo "! Unsupported architecture: $ARCH (exit code: 'arch unsupported/0')"

    exit 1

fi

while true; do

    echo ""
    echo "Choose an install/upgrade/uninstall option."
    echo "1 | Install Supervisor-Server."
    echo "2 | Install the Web UI for Supervisor-Server"
    echo "3 | Install Supervisor-Client"
    echo "4 | Exit"
    read -r -p "Enter your choice [1-4]: " choice
    echo ""

    case $choice in

        1)

            install_server

            ;;

        2)

            install_web_ui

            ;;

        3)

            install_client

            ;;

        4)

          echo ": Exiting setup..."
  
          exit 0

          ;;

        *)
            echo "! Invalid choice. Please select 1, 2, or 3. (error code: 'out of bounds choice/0')"
            ;;

    esac

done