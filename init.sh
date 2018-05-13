#!/usr/bin/env bash
sudo apt-get update -y && sudo apt-get upgrade -y
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
sudo apt-add-repository 'deb https://apt.dockerproject.org/repo ubuntu-xenial main'
sudo apt-get update -y && apt-cache policy docker-engine
sudo apt-get install -y docker-engine
sudo usermod -aG docker $(whoami)
sudo docker run hello-world

mkdir -p dockerfiles
cd dockerfiles
wget 'https://github.com/dmylzenova/celebrity_faces/blob/master/Dockerfile' -O Dockerfile

sudo docker build --no-cache -t flask-server:latest .
sudo docker run -i -t -p 2018:2018 flask-server

# docker ps
# docker stop <conteiner-id>