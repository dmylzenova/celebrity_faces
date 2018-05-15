#!/usr/bin/env bash
set -xue -o pipefail -o posix
sudo apt-get update -y && sudo apt-get upgrade -y
sudo apt-get install -y python3-pip python3-dev build-essential cmake libgtk2.0-dev git wget unzip
if [ ! -d "celebrity_faces" ]; then
    git clone https://github.com/dmylzenova/celebrity_faces.git
else
    cd $HOME/celebrity_faces && git pull
fi
sudo pip3 install --upgrade pip
sudo pip3 install -r $HOME/celebrity_faces/requirements.txt
cd $HOME/celebrity_faces/index && python3 setup.py build_ext --inplace && cp pylsh*.so $HOME/celebrity_faces/viewer

# Upload data
cd $HOME/celebrity_faces/viewer/app/static
if [ ! -f model.zip ]; then
    wget 'https://downloader.disk.yandex.ru/disk/40221263c2967f50f18e912cec990f6ad1646fd553d07e1127f35c84f3b04f83/5afb7c4c/ddZ6cPttF3My8iD24FAVMGih0Y48jpk0L8HqWhH8T8uJd3oy42om7dvGdDthoDm11gjlbipE8-KemsJlt_0WsA==?uid=0&filename=model.zip&disposition=attachment&hash=g1kEsVlquYHXVsRHYxcZqBD6M/zUVQn++VPQwis107Y=&limit=0&content_type=application/x-zip-compressed&fsize=192225146&hid=3194ee851e6f825d5717ee58e886c511&media_type=compressed&tknv=v2' \
        -O model.zip
    rm -rf model
    unzip model.zip
fi

if [ ! -f index.tar.gz ]; then
    wget 'https://downloader.disk.yandex.ru/disk/1fe230e012562745e33f1a971a09f32eae01a355fa4f7cb7d08aa2e64bae6cc1/5afb7c66/hcn9DwIWb0_PkfTJnWMiaHGFEdVHxVXvMyxC7I_XlwJqJIawRyXEMfWFDKs3l2CMQ4Z9dkWJSL9HIjcL6bDV9Q==?uid=0&filename=data.tar.gz&disposition=attachment&hash=++6fwQGYQcslFHy7dzj3e+3J/uyYEquUqxV8CKH4W3U=&limit=0&content_type=application/gzip&fsize=139173022&hid=cea20f2b9a830f0b67b8044389b769a5&media_type=compressed&tknv=v2' \
        -O index.tar.gz
    rm -rf index
    tar -xzvf index.tar.gz
fi

if [ ! -f cropped_photos.zip ]; then
    wget 'https://downloader.disk.yandex.ru/disk/4d6554eac43fcce5986cf5cc4bb1c74f2de050d4fd87a21104360925acadeb96/5afb7c77/ddZ6cPttF3My8iD24FAVMNMVwbHXYPOtYUfz0SZYfxH4mFRft67jMJo_GH5EjeEpmnhqvnutlhOthPxqre2UjA==?uid=0&filename=cropped_photos.zip&disposition=attachment&hash=feWHaLxtk1+8dmL0kDELoAtGd9yXP8yu5rSTpmx4ucM=&limit=0&content_type=application/x-zip-compressed&fsize=743778718&hid=43983205d7b1bae83ea31cf278897abd&media_type=compressed&tknv=v2' \
        -O cropped_photos.zip
    unzip cropped_photos.zip
fi

cd $HOME/celebrity_faces/viewer
chmod +x ./run.py
./run.py
