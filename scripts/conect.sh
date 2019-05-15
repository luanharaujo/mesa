#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Warning: IP Address as default (192.168.200.1)!"
	ip_rasp="192.168.200.1"
else
	ip_rasp=${1}
fi

login_rasp="pi"
password="atitudes"

sshpass -p ${password} ssh ${login_rasp}@${ip_rasp}

