#!/bin/sh
# For Ubuntu 20.04/22.04/24.04

# uninstall current cmake
sudo apt remove --purge --auto-remove cmake

# prepare for installation
sudo apt update
sudo apt install -y software-properties-common lsb-release
sudo apt clean all

# obtain a copy of kitware signing key
test -f /usr/share/doc/kitware-archive-keyring/copyright ||
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

# add the kitware repository to  sources list
# For Ubuntu 24.04:
#echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
# For Ubuntu 22.04:
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
# For Ubuntu 20.04:
#echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
sudo apt update

# remove the manually obtained signed key
test -f /usr/share/doc/kitware-archive-keyring/copyright ||
sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg

# install the keyring package
sudo apt install kitware-archive-keyring

# install cmake
sudo apt update
sudo apt install cmake