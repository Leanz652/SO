#!/bin/bash
setearIPSKernel () {
	sed -i 's/^IP_ESCUCHA=.*/IP_ESCUCHA='"$IP_KERNEL"'/' $1
	sed -i 's/^IP_MEMORIA=.*/IP_MEMORIA='"$IP_MEMORIA"'/' $1
	sed -i 's/^IP_CPU=.*/IP_CPU='"$IP_CPU"'/' $1
	cat $1
	read -p "Continuar? " -n 1 -r
	echo
}

setearIPSCPU () {
	sed -i 's/^IP_ESCUCHA=.*/IP_ESCUCHA='"$IP_CPU"'/' $1
	sed -i 's/^IP_MEMORIA=.*/IP_MEMORIA='"$IP_MEMORIA"'/' $1
	cat $1
	read -p "Continuar? " -n 1 -r
	echo
}

setearIPSMemoria () {
	sed -i 's/^IP_ESCUCHA=.*/IP_ESCUCHA='"$IP_MEMORIA"'/' $1
	cat $1
	read -p "Continuar? " -n 1 -r
	echo
}

setearIPSConsola () {
	sed -i 's/^IP_KERNEL=.*/IP_KERNEL='"$IP_KERNEL"'/' $1
	cat $1
	read -p "Continuar? " -n 1 -r
	echo
}



echo -e "\nCONFIGURANDO IPs DE MODULOS...\n"

echo -e "\nINGRESE IP MEMORIA...\n"
read IP_MEMORIA

echo -e "\nINGRESE IP CPU...\n"
read IP_CPU

echo -e "\nINGRESE IP KERNEL...\n"
read IP_KERNEL

echo -e "\nVALORES INGRESADOS: \n"
echo -e "\nIP MEMORIA: "$IP_MEMORIA"\n"
echo -e "\nIP CPU: "$IP_CPU"\n"
echo -e "\nIP KERNEL: "$IP_KERNEL"\n"

IPLOCAL=$(ifconfig enp0s3 | grep 'inet addr' | cut -d: -f2 | awk '{print $1}')

read -p "¿Estan bien las ips? " -n 1 -r
echo    # (optional) move to a new line

echo -e "\n\nSETEADO MEMORIA: \n\n"

setearIPSMemoria /home/utnso/tp-2022-1c-Grupo-999/Memoria/cfg/memoria.config

setearIPSMemoria /home/utnso/tp-2022-1c-Grupo-999/Memoria/cfg/memoria_INTEGRAL.config

setearIPSMemoria /home/utnso/tp-2022-1c-Grupo-999/Memoria/cfg/memoria_MEMORIA_1.config

setearIPSMemoria /home/utnso/tp-2022-1c-Grupo-999/Memoria/cfg/memoria_TLB.config

echo -e "\n\nSETEADO CPU: \n\n"

setearIPSCPU /home/utnso/tp-2022-1c-Grupo-999/CPU/cfg/cpu.config

setearIPSCPU /home/utnso/tp-2022-1c-Grupo-999/CPU/cfg/cpu_MEMORIA_1.config

setearIPSCPU /home/utnso/tp-2022-1c-Grupo-999/CPU/cfg/cpu_TLB.config

setearIPSCPU /home/utnso/tp-2022-1c-Grupo-999/CPU/cfg/cpu_INTEGRAL.config

echo -e "\n\nSETEADO KERNEL: \n\n"

setearIPSKernel /home/utnso/tp-2022-1c-Grupo-999/Kernel/cfg/kernel.config

setearIPSKernel /home/utnso/tp-2022-1c-Grupo-999/Kernel/cfg/kernel.config.plani

setearIPSKernel /home/utnso/tp-2022-1c-Grupo-999/Kernel/cfg/kernel.config.susp

setearIPSKernel /home/utnso/tp-2022-1c-Grupo-999/Kernel/cfg/kernel_INTEGRAL.config

setearIPSKernel /home/utnso/tp-2022-1c-Grupo-999/Kernel/cfg/kernel_MEMORIA_1.config

setearIPSKernel /home/utnso/tp-2022-1c-Grupo-999/Kernel/cfg/kernel_TLB.config

echo -e "\n\nSETEADO CONSOLA: \n\n"

setearIPSConsola /home/utnso/tp-2022-1c-Grupo-999/Consola/cfg/consola.config