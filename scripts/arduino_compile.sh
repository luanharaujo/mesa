#!/bin/bash



if [ $# -lt 1 ]; then
	echo "Missing arguments, try: ./arduino_compile.sh folder_code [IP address]"

else
	if [ $# -lt 2 ]; then
		echo "Warning: IP Address as default (192.168.200.1)!"
		ip_rasp="192.168.200.1"
	else
		ip_rasp=${2}
	fi

	folder_code=${1} 
	login_rasp="pi"
	password="atitudes"


	sshpass -p ${password} scp -r src/arduino/${folder_code} ${login_rasp}@${ip_rasp}:/home/pi/src/arduino
	sshpass -p ${password} ssh ${login_rasp}@${ip_rasp} 'cd /home/pi/src/arduino && arduino --upload '${folder_code}'/'${folder_code}'.ino --port /dev/ttyACM0'
fi


#sshpass -p ${password} scp src/jstick.c ${login_rasp}@${ip_rasp}:/home/pi/src
#sshpass -p ${password} scp src/comunicacao_arduino.c ${login_rasp}@${ip_rasp}:/home/pi/src
#sshpass -p ${password} scp src/motor.c ${login_rasp}@${ip_rasp}:/home/pi/src


 #dmesg | grep -n Arduino | 
#dmesg | awk '/Product: Arduino/  {print "Board: Arduino "$7" " $8}'|tail -1
