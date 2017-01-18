#! /bin/bash
sudo ipfw add pipe 1 src-ip 192.168.6.0/24 in
sudo ipfw pipe 1 config bw 100kbit/s queue 20 delay 35ms
echo "IPFW RULES APPLIED"
