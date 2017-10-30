#!/bin/bash
RED='\033[0;31m'
NC='\033[0m' # No Color
runDir='/home/royn/svn/Proj/src/project/buse/buse_proj'
nbdN='0'

modprobe nbd
sleep 1

echo -e "${RED}starting test${NC}"
echo -e "${RED}Run The Slave Proccess${NC}"
cd ${runDir}
sleep 1 

gnome-terminal -e ./buse_test &
sleep 1 

echo -e "${RED}Format nbd0 to ext4${NC}"
sleep 1
mkfs.ext4 /dev/nbd${nbdN}
echo -e "${RED}Mount ~/mnt to nbd0${NC}"
sleep 1
mount /dev/nbd${nbdN} ~/mnt
cd ~/mnt
sleep 1
echo -e "${RED}Create 1.txt and put there ~/Documents/C++/Last_C++/${NC}"
#touch 1.txt
#tree ~/Documents/C++/Last_C++/ >1.txt
#echo -e "${RED}Print file 1.txt${NC}"
#sleep 3
#cat 1.txt 
#cd ..
#sleep 3
#echo -e "${RED}unmount the dir ~/mnt${NC}"
#umount ~/mnt
#sleep 1 
#echo -e "${RED}mount dir ~/mnt1${NC}"
#mount /dev/nbd${nbdN} ~/mnt1
#echo -e "${RED}Print 1.txt from ~/mnt1${NC}"
#cat ~/mnt1/1.txt
#sleep 1
#echo -e "${RED}unmount the dir ~/mnt1${NC}"
#umount ~/mnt1 
#echo -e "${RED}Disconect and stop NBD device${NC}"
#sleep 5
#nbd-client -d /dev/nbd${nbdN}





