#!/usr/bin/env bash
sudo apt-get update -y && sudo apt-get upgrade -y
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
sudo apt-add-repository 'deb https://apt.dockerproject.org/repo ubuntu-xenial main'
sudo apt-get update -y && apt-cache policy docker-engine
sudo apt-get install -y docker-engine
sudo usermod -aG docker $(whoami)
sudo docker run hello-world

sudo apt-get install -y git unzip
if [ ! -d "celebrity_faces" ]; then
    git clone https://github.com/dmylzenova/celebrity_faces.git
else
    git pull
fi

curl -fsSL https://raw.githubusercontent.com/CWSpear/local-persist/master/scripts/install.sh | sudo bash
docker volume create -d local-persist -o mountpoint=$HOME/celebrity_faces --name=celebrity_faces

# Upload data
cd celebrity_faces/viewer/app/static
if [ ! -f model.zip ]; then
    wget 'https://downloader.disk.yandex.ru/disk/c2e34c3c49a64b55f37be8123aa23b8a0ab2ff7cfe95895dc8be2a433227e144/5af7ddcc/ddZ6cPttF3My8iD24FAVMGih0Y48jpk0L8HqWhH8T8uJd3oy42om7dvGdDthoDm11gjlbipE8-KemsJlt_0WsA%3D%3D?uid=0&filename=model.zip&disposition=attachment&hash=g1kEsVlquYHXVsRHYxcZqBD6M/zUVQn%2B%2BVPQwis107Y%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=192225146&hid=3194ee851e6f825d5717ee58e886c511&media_type=compressed&tknv=v2' \
        -O model.zip
    rm -rf model
    unzip model.zip
fi

if [ ! -f index.zip ]; then
    wget 'https://downloader.disk.yandex.ru/disk/56429913bf621d0fecddaffb07f496253228e91bb4f846d71ccbae7ef240d68c/5af7de83/ddZ6cPttF3My8iD24FAVMLWMAimOr1IH7kjEXqvIlMc_o218Slwcl6_M90Vr188_ME4XXrbkjFcbHAQMNG27zQ%3D%3D?uid=0&filename=data.zip&disposition=attachment&hash=id3IdAvIzwSJCyzBrF3/Omv8HNaptwDTxnwRW29BLZY%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=254814383&hid=6ce4cf160312f8a880ba38d671985212&media_type=compressed&tknv=v2' \
        -O index.zip
    rm -rf index
    unzip index.zip
fi

if [ ! -f cropped_photos.zip ]; then
    wget 'https://downloader.disk.yandex.ru/disk/1e1a11d600fcc976238f6ea9f9a4330216f12cca314dda9e6158b48a53efec71/5af7dc47/ddZ6cPttF3My8iD24FAVMNMVwbHXYPOtYUfz0SZYfxH4mFRft67jMJo_GH5EjeEpmnhqvnutlhOthPxqre2UjA%3D%3D?uid=0&filename=cropped_photos.zip&disposition=attachment&hash=feWHaLxtk1%2B8dmL0kDELoAtGd9yXP8yu5rSTpmx4ucM%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=743778718&hid=43983205d7b1bae83ea31cf278897abd&media_type=compressed&tknv=v2' \
        -O cropped_photos.zip
    unzip cropped_photos.zip
fi

cd $HOME
cp celebrity_faces/Dockerfile Dockerfile
sudo docker build --no-cache -t flask-server:latest .
sudo docker run -v celebrity_faces:/root/celebrity_faces -d -p 2018:2018 flask-server

# docker ps
# docker stop <conteiner-id>
