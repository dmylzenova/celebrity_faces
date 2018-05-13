FROM ubuntu:16.04
MAINTAINER Irina Goltsman <irina-goltsman@yandex.ru>

ENV HOME /root

RUN apt-get update -y
RUN apt-get install -y python3-pip python3-dev build-essential cmake libgtk2.0-dev git wget unzip
RUN pip3 install --upgrade pip

RUN git clone https://github.com/dmylzenova/celebrity_faces.git
RUN cd celebrity_faces/viewer/app/static && wget 'https://downloader.disk.yandex.ru/disk/c2e34c3c49a64b55f37be8123aa23b8a0ab2ff7cfe95895dc8be2a433227e144/5af7ddcc/ddZ6cPttF3My8iD24FAVMGih0Y48jpk0L8HqWhH8T8uJd3oy42om7dvGdDthoDm11gjlbipE8-KemsJlt_0WsA%3D%3D?uid=0&filename=model.zip&disposition=attachment&hash=g1kEsVlquYHXVsRHYxcZqBD6M/zUVQn%2B%2BVPQwis107Y%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=192225146&hid=3194ee851e6f825d5717ee58e886c511&media_type=compressed&tknv=v2' \
    -O model.zip && rm -rf model && unzip model.zip

RUN cd celebrity_faces/viewer/app/static && wget 'https://downloader.disk.yandex.ru/disk/cf69c74b19f8df69a4a92114eb01c59452552a3e3a2386e7252bffd02ceb150b/5af7f04d/ddZ6cPttF3My8iD24FAVMLWMAimOr1IH7kjEXqvIlMc_o218Slwcl6_M90Vr188_ME4XXrbkjFcbHAQMNG27zQ%3D%3D?uid=0&filename=data.zip&disposition=attachment&hash=id3IdAvIzwSJCyzBrF3/Omv8HNaptwDTxnwRW29BLZY%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=254814383&hid=6ce4cf160312f8a880ba38d671985212&media_type=compressed&tknv=v2' \
    -O index.zip && rm -rf index && unzip index.zip

RUN cd celebrity_faces/viewer/app/static && wget 'https://downloader.disk.yandex.ru/disk/cef427fd5805b06fe7e4fed0880e20f85accec9d4ddf7fedac0f787520a0bb67/5af7f024/ddZ6cPttF3My8iD24FAVMNMVwbHXYPOtYUfz0SZYfxH4mFRft67jMJo_GH5EjeEpmnhqvnutlhOthPxqre2UjA%3D%3D?uid=0&filename=cropped_photos.zip&disposition=attachment&hash=feWHaLxtk1%2B8dmL0kDELoAtGd9yXP8yu5rSTpmx4ucM%3D&limit=0&content_type=application%2Fx-zip-compressed&fsize=743778718&hid=43983205d7b1bae83ea31cf278897abd&media_type=compressed&tknv=v2' \
    -O cropped_photos.zip && unzip cropped_photos.zip

RUN pip3 install -r celebrity_faces/requirements.txt

CMD celebrity_faces/viewer/run.py

# docker build -t flask-server:latest .
# docker run -d -p 2018:2018 flask-server
# docker ps
# docker stop <conteiner-id>