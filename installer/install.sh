echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
echo "     Dynamic.Supervisor Installer (b0001/R)"
echo "     sudo privileges will be asked."
echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

echo ""
echo "=-= Installing JSON, PHP, unzip and wget... =-="
echo ""

sudo apt install nlohmann-json3-dev php unzip wget -y

echo ""
echo "=-= JSON, PHP, unzip and wget installed. =-="
echo ""

echo ""
echo "=-= Setting up Supervisor Server as a service... =-="
echo ""

sudo mkdir /etc/dynamic/supervisor/ -p

sudo wget -O /etc/dynamic/supervisor/Dynamic.Supervisor-SRV https://github.com/XIII-MC/Dynamic.Supervisor/releases/download/b0001%2FR/Dynamic.Supervisor-SRV

sudo bash -c 'cat <<EOF > /etc/systemd/system/Dynamic.Supervisor-SRV.service
[Unit]
Description=A network/machine supervisor.
After=network.target

[Service]
ExecStart=/etc/dynamic/supervisor/Dynamic.Supervisor-SRV
WorkingDirectory=/etc/dynamic/supervisor/
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
echo "=-= Setting up web UI... =-="
echo ""

sudo wget -O /etc/dynamic/supervisor/web.zip https://github.com/XIII-MC/Dynamic.Supervisor/releases/download/b0001%2FR/web.zip

unzip /etc/dynamic/supervisor/web.zip -d /var/www/html/supervisor/

echo ""
echo "=-= Web UI online. =-="
echo ""

echo ""
echo "=-= Setup done! Thank you for using Dynamic.Supervisor! =-="
echo ""