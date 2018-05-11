FROM ubuntu:16.04
MAINTAINER Irina Goltsman <irina-goltsman@yandex.ru>

ENV HOME /root

RUN apt-get update -y
RUN apt-get install -y python3-pip python3-dev build-essential git
RUN pip3 install --upgrade pip

RUN git clone https://github.com/dmylzenova/celebrity_faces.git
RUN cd celebrity_faces && git checkout ira_docker && cd ..

RUN apt-get install -y cmake
RUN pip3 install -r celebrity_faces/requirements.txt

CMD celebrity_faces/viewer/run.py --port=2018 --host='0.0.0.0'
