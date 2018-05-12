FROM ubuntu:16.04
MAINTAINER Irina Goltsman <irina-goltsman@yandex.ru>

ENV HOME /root

RUN apt-get update -y
RUN apt-get install -y python3-pip python3-dev build-essential git cmake
RUN pip3 install --upgrade pip

RUN git clone https://github.com/dmylzenova/celebrity_faces.git

RUN pip3 install -r celebrity_faces/requirements.txt

CMD celebrity_faces/viewer/run.py --port=2018

# docker build -t flask-server:latest .
# docker run -d -p 2018:2018 flask-server
# docker ps
# docker stop <conteiner-id>
