echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
echo "     Dynamic.Supervisor Installer (b0001/R)"
echo "     sudo privileges will be asked."
echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

echo ""
echo "=-= Installing JSON and PHP... =-="
echo ""

sudo apt install nlohmann-json3-dev php -y

echo ""
echo "=-= JSON and PHP installed. =-="
echo ""

echo ""
echo "=-= Setting up Supervisor Server as a service... =-="
echo ""

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


