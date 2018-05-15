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
    wget 'https://downloader.disk.yandex.ru/disk/07dc3096fefad426f42cd0eb3f9b63c8278c5850cdac2d79e8e6fa2fba597876/5afb3b85/ddZ6cPttF3My8iD24FAVMGih0Y48jpk0L8HqWhH8T8uJd3oy42om7dvGdDthoDm11gjlbipE8-KemsJlt_0WsA==?uid=264449331&filename=model.zip&disposition=attachment&hash=&limit=0&content_type=application/x-zip-compressed&fsize=192225146&hid=3194ee851e6f825d5717ee58e886c511&media_type=compressed&tknv=v2&etag=884877488a490e6622acb6ad773ea9e2' \
        -O model.zip
    rm -rf model
    unzip model.zip
fi

if [ ! -f index.tar.gz ]; then
    wget 'https://downloader.disk.yandex.ru/disk/5c1a7576278accfe4e6e8c398515eae357cada739b973d88804ee589206c7dce/5afb76fa/hcn9DwIWb0_PkfTJnWMiaHGFEdVHxVXvMyxC7I_XlwJqJIawRyXEMfWFDKs3l2CMQ4Z9dkWJSL9HIjcL6bDV9Q==?uid=0&filename=data.tar.gz&disposition=attachment&hash=++6fwQGYQcslFHy7dzj3e+3J/uyYEquUqxV8CKH4W3U=&limit=0&content_type=application/gzip&fsize=139173022&hid=cea20f2b9a830f0b67b8044389b769a5&media_type=compressed&tknv=v2' \
        -O index.tar.gz
    rm -rf index
    tar -xzvf index.tar.gz
fi

if [ ! -f cropped_photos.zip ]; then
    wget 'https://downloader.disk.yandex.ru/disk/c6862ec3b05af70542b7bc6b27729fa77b7b99fc9493007d63a4e48009ee4885/5afb78fc/ddZ6cPttF3My8iD24FAVMNMVwbHXYPOtYUfz0SZYfxH4mFRft67jMJo_GH5EjeEpmnhqvnutlhOthPxqre2UjA==?uid=0&filename=cropped_photos.zip&disposition=attachment&hash=feWHaLxtk1+8dmL0kDELoAtGd9yXP8yu5rSTpmx4ucM=&limit=0&content_type=application/x-zip-compressed&fsize=743778718&hid=43983205d7b1bae83ea31cf278897abd&media_type=compressed&tknv=v2' \
        -O cropped_photos.zip
    unzip cropped_photos.zip
fi

cd $HOME/celebrity_faces/viewer
chmod +x ./run.py
./run.py
