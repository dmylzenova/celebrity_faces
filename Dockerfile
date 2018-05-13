FROM ubuntu:16.04
MAINTAINER Irina Goltsman <irina-goltsman@yandex.ru>

ENV HOME /root

RUN apt-get update -y
RUN apt-get install -y python3-pip python3-dev build-essential cmake libgtk2.0-dev git
RUN pip3 install --upgrade pip

RUN git clone https://github.com/dmylzenova/celebrity_faces.git
RUN cd $HOME/celebrity_faces/viewer/app/static && wget 'https://downloader.disk.yandex.ru/disk/c2e34c3c49a64b55f37be8123aa23b8a0ab2ff7cfe95895dc8be2a433227e144/5af7ddcc/ddZ6cPttF3My8iD24FAVMGih0Y48jpk0L8HqWhH8T8uJd3oy42om7dvGdDthoDm11gjlbipE8-KemsJlt_0WsA%3D%3D?uid=0&filename=model.zip&disposition=attachment&hash=g1kEsVlquYHXVsRHYxcZqBD6M/zUVQn%2B%2BVPQwis107Y%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=192225146&hid=3194ee851e6f825d5717ee58e886c511&media_type=compressed&tknv=v2' \
    -O model.zip && rm -rf model && unzip model.zip

RUN cd $HOME/celebrity_faces/viewer/app/static && wget 'https://downloader.disk.yandex.ru/disk/c2e34c3c49a64b55f37be8123aa23b8a0ab2ff7cfe95895dc8be2a433227e144/5af7ddcc/ddZ6cPttF3My8iD24FAVMGih0Y48jpk0L8HqWhH8T8uJd3oy42om7dvGdDthoDm11gjlbipE8-KemsJlt_0WsA%3D%3D?uid=0&filename=model.zip&disposition=attachment&hash=g1kEsVlquYHXVsRHYxcZqBD6M/zUVQn%2B%2BVPQwis107Y%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=192225146&hid=3194ee851e6f825d5717ee58e886c511&media_type=compressed&tknv=v2' \
    -O model.zip && rm -rf model && unzip model.zip

RUN cd $HOME/celebrity_faces/viewer/app/static && wget 'https://downloader.disk.yandex.ru/disk/c2e34c3c49a64b55f37be8123aa23b8a0ab2ff7cfe95895dc8be2a433227e144/5af7ddcc/ddZ6cPttF3My8iD24FAVMGih0Y48jpk0L8HqWhH8T8uJd3oy42om7dvGdDthoDm11gjlbipE8-KemsJlt_0WsA%3D%3D?uid=0&filename=model.zip&disposition=attachment&hash=g1kEsVlquYHXVsRHYxcZqBD6M/zUVQn%2B%2BVPQwis107Y%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=192225146&hid=3194ee851e6f825d5717ee58e886c511&media_type=compressed&tknv=v2' \
    -O model.zip && rm -rf model && unzip model.zip


RUN pip3 install -r $HOME/celebrity_faces/requirements.txt

CMD $HOME/celebrity_faces/viewer/run.py

# docker build -t flask-server:latest .
# docker run -d -p 2018:2018 flask-server
# docker ps
# docker stop <conteiner-id>