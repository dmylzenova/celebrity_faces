FROM ubuntu:16.04
MAINTAINER Irina Goltsman <irina-goltsman@yandex.ru>

ENV HOME /root

RUN apt-get update -y
RUN apt-get install -y python3-pip python3-dev build-essential git cmake
RUN pip3 install --upgrade pip

RUN git clone https://github.com/dmylzenova/celebrity_faces.git

RUN pip3 install -r celebrity_faces/requirements.txt

RUN ls
RUN cd celebrity_faces && git fetch && git checkout ira_docker && cd ..

CMD celebrity_faces/viewer/run.py \
    --port=2018 \
    --cropped-photos-dir=$HOME/data/cropped_photos \
    --model-dir-path=$HOME/data/model \
    --index-path=$HOME/data/index/150k_50t_index.ann \
    --int-mapping-path=$HOME/data/index/int_indices_mapping.csv \
    --uploaded-img-path=$HOME/data/uploaded

# docker build -t flask-server:latest .
# docker run -d -p 2018:2018 flask-server
# docker ps
# docker stop <conteiner-id>
