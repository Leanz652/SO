#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalling commons libraries...\n\n"
COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD
echo -e "\n\nReemplazando path en Shared...\n\n"
sed -i 's/matias\/Desktop\/utn\/SO/utnso/g' /home/utnso/tp-2022-1c-Grupo-999/Shared/Debug/subdir.mk

echo -e "\n\nBuildeando Shared...\n\n"
make all -C /home/utnso/tp-2022-1c-Grupo-999/Shared/Debug

echo -e "\n\nInstalando Shared...\n\n"
sudo cp -u /home/utnso/tp-2022-1c-Grupo-999/Shared/Debug/libShared.so /usr/lib
sudo cp -u /home/utnso/tp-2022-1c-Grupo-999/Shared/client_utils.h /usr/local/include
sudo cp -u /home/utnso/tp-2022-1c-Grupo-999/Shared/server_utils.h /usr/local/include
sudo cp -u /home/utnso/tp-2022-1c-Grupo-999/Shared/shared_utils.h /usr/local/include

echo -e "\n\nBuildeando Modulos...\n\n"
make all -C /home/utnso/tp-2022-1c-Grupo-999/Memoria/Debug
make all -C /home/utnso/tp-2022-1c-Grupo-999/CPU/Debug
make all -C /home/utnso/tp-2022-1c-Grupo-999/Kernel/Debug
make all -C /home/utnso/tp-2022-1c-Grupo-999/Consola/Debug
echo -e "\n\nDeploy done!\n\n"