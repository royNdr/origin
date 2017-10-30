#!/bin/bash
RED='\033[0;31m'
NC='\033[0m' # No Color
runDir='/home/dima/Documents/C++/Last_C++/FinalProject/solutions/10_Integration_Master_Slave_Kamikaze'
nbdN='0'

modprobe nbd
echo 4 | sudo tee /sys/class/block/nbd0/queue/max_sectors_kb
rm -f /tmp/my*
rm -f /tmp/log*

sleep 1
echo -e "${RED}starting test${NC}"
echo -e "${RED}Run The Slave Proccess${NC}"
cd ${runDir}/Slave
gnome-terminal -e ./t1.sh &
sleep 1 
echo -e "${RED}Run The Master Proccess${NC}"
cd ${runDir}/Master
gnome-terminal -e ./t1.sh  &
echo -e "${RED}Format nbd0 to ext4${NC}"
sleep 1
mkfs.ext4 /dev/nbd${nbdN}
echo -e "${RED}Mount ~/mnt to nbd0${NC}"
sleep 1
mount /dev/nbd${nbdN} ~/mnt
cd ~/mnt
sleep 1
echo -e "${RED}Create 1.txt and put there ~/Documents/C++/Last_C++/${NC}"
touch 1.txt
tree ~/Documents/C++/Last_C++/ >1.txt
echo -e "${RED}Print file 1.txt${NC}"
sleep 3
cat 1.txt 
cd ..
sleep 3
echo -e "${RED}unmount the dir ~/mnt${NC}"
umount ~/mnt
sleep 1 
echo -e "${RED}mount dir ~/mnt1${NC}"
mount /dev/nbd${nbdN} ~/mnt1
echo -e "${RED}Print 1.txt from ~/mnt1${NC}"
cat ~/mnt1/1.txt
sleep 1
echo -e "${RED}unmount the dir ~/mnt1${NC}"
umount ~/mnt1 
echo -e "${RED}Disconect and stop NBD device${NC}"
sleep 5
nbd-client -d /dev/nbd${nbdN}





