#!/usr/bin/env bash
sudo apt-get update -y && sudo apt-get upgrade -y
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
sudo apt-add-repository 'deb https://apt.dockerproject.org/repo ubuntu-xenial main'
sudo apt-get update -y && apt-cache policy docker-engine
sudo apt-get install -y docker-engine
sudo usermod -aG docker $(whoami)
sudo docker run hello-world

sudo apt-get install -y git
if [ ! -d "celebrity_faces" ]; then
    git clone https://github.com/dmylzenova/celebrity_faces.git
else
    git pull
fi

curl -fsSL https://raw.githubusercontent.com/CWSpear/local-persist/master/scripts/install.sh | sudo bash
docker volume create -d local-persist -o mountpoint=$HOME/celebrity_faces --name=celebrity_faces

# TODO: Upload data
#cd celebrity_faces/viewer/app/static
#cd wget ..

cd $HOME/celebrity_faces/dockerfiles/viewer
sudo docker build --no-cache -t flask-server:latest .
# TODO: не видно изнутри папку celebrity_faces - грусть печаль
sudo docker run -v celebrity_faces:/root/celebrity_faces -d -p 2018:2018 flask-server

# docker ps
# docker stop <conteiner-id>
