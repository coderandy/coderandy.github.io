#!/usr/local/bin/bash
grey="\e[1;30m"
blue="\e[1;32m"
white="\e[1;37m"
nc="\e[0m"
vhosts=0

echo "  _    _ _     _  _____  _______ _______ _______"
echo "   \  /  |_____| |     | |______    |    |______"
echo "    \/   |     | |_____| ______|    |    ______|"
echo " "
echo " "
echo -e "                    for.. The ${blue} ${HOSTNAME}${white} Box."
echo " "
echo -e "${grey}1${blue}. ${nc}display ${blue}ipv4 ${nc}vhosts"
echo -e "${grey}2${blue}. ${nc}display ${blue}ipv6 ${nc}vhosts"
echo -e "Select your choice ${grey}[1 ${blue}or ${grey}2]? "
read vhosts

if [ $vhosts -eq 1 ] ; then
     /usr/local/bin/vh

else #### nested if i.e. if within if ######

       if [ $vhosts -eq 2 ] ; then
        /usr/local/bin/vh6
        echo "ipv6 vhosts"
       fi
fi
