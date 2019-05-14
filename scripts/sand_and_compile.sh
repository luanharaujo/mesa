#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Warning: IP Address as default (192.168.200.1)!"
	ip_rasp="192.168.200.1"
else
	ip_rasp=${1}
fi

login_rasp="pi"
password="atitudes"

sshpass -p ${password} scp src/main.c ${login_rasp}@${ip_rasp}:/home/pi/src
sshpass -p ${password} scp src/jstick.c ${login_rasp}@${ip_rasp}:/home/pi/src
#sshpass -p ${password} scp src/comunicacao_arduino.c ${login_rasp}@${ip_rasp}:/home/pi/src
sshpass -p ${password} scp src/motor.c ${login_rasp}@${ip_rasp}:/home/pi/src

sshpass -p ${password} ssh ${login_rasp}@${ip_rasp} 'cd /home/pi/src && gcc main.c -o ../prog/mesa -lwiringPi'

