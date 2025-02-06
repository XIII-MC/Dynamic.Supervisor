rm -rf export/

mkdir -p export

zip -r export/web.zip web/

bash deploy/CrossArchBuild.sh

cp build/arm32/Dynamic.Supervisor-SRV_ARM32 export/Dynamic.Supervisor-SRV_ARM32
cp build/arm64/Dynamic.Supervisor-SRV_ARM64 export/Dynamic.Supervisor-SRV_ARM64
cp build/x86_64/Dynamic.Supervisor-SRV_x86_64 export/Dynamic.Supervisor-SRV_x86_64

cp build/arm32/Dynamic.Supervisor-CLT_ARM32 export/Dynamic.Supervisor-CLT_ARM32
cp build/arm64/Dynamic.Supervisor-CLT_ARM64 export/Dynamic.Supervisor-CLT_ARM64
cp build/x86_64/Dynamic.Supervisor-CLT_x86_64 export/Dynamic.Supervisor-CLT_x86_64


cp installer/install.sh export/install.sh