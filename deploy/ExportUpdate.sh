rm -rf export/

mkdir -p export

zip -r export/web.zip web/

bash deploy/SRV-CrossBuild.sh

cp server/build/arm32/Dynamic.Supervisor-SRV_ARM32 export/Dynamic.Supervisor-SRV_ARM32
cp server/build/arm64/Dynamic.Supervisor-SRV_ARM64 export/Dynamic.Supervisor-SRV_ARM64
cp server/build/x86_64/Dynamic.Supervisor-SRV_x86_64 export/Dynamic.Supervisor-SRV_x86_64

cp installer/install.sh export/install.sh