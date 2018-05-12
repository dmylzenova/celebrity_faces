#!/usr/bin/env python3
import argparse
import os
import socket
import logging
from logging.handlers import RotatingFileHandler
from multiprocessing import Process

import gevent.pywsgi
import gevent.server

import app


def parse_args():
    parser = argparse.ArgumentParser(description="Celebrity faces viewer",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--host", type=str, default="")
    parser.add_argument("--port", type=int, default=2018)
    parser.add_argument("--debug", action="store_true", default=False)
    parser.add_argument("--uploaded-img-path", type=str,
                        default=os.path.dirname(__file__) + '/app/static/uploaded_img')
    parser.add_argument("--model-dir-path", type=str, default=os.path.dirname(__file__) + '/app/static/model')
    parser.add_argument("--index-path", type=str,
                        default=os.path.dirname(__file__) + '/app/static/index/150k_50t_index.ann')
    parser.add_argument("--int-mapping-path", type=str,
                        default=os.path.dirname(__file__) + '/app/static/index/int_indices_mapping.csv')
    parser.add_argument("--model-name", type=str, default="model-20170512-110547")
    parser.add_argument("--checkpoint", type=str, default="model-20170512-110547.ckpt-250000")
    parser.add_argument("--cropped-photos-dir", type=str,
                        default=os.path.dirname(__file__) + "/app/static/cropped_photos")
    parser.add_argument("--haarcascade-xml-path", type=str,
                        default=os.path.dirname(__file__) + '/app/static/haarcascade_frontalface_default.xml')
    parser.add_argument("--log-file-path", default=None)
    parser.add_argument("--process-count", type=int, default=1)

    return parser.parse_args()


def serve_forever(listener, application, environ):
    gevent.pywsgi.WSGIServer(listener, application, environ=environ).serve_forever()


def main():
    args = parse_args()
    app.init_app(args.uploaded_img_path, args.model_dir_path, args.model_name, args.checkpoint,
                 args.cropped_photos_dir, args.index_path, args.int_mapping_path, args.haarcascade_xml_path)
    app.app.debug = args.debug

    if args.log_file_path:
        handler = RotatingFileHandler(args.log_file_path, maxBytes=1024**3, backupCount=1)
        handler.setLevel(logging.INFO)
        formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
        handler.setFormatter(formatter)
        app.app.logger.addHandler(handler)

    listener = gevent.server._tcp_listener((args.host, args.port), reuse_addr=1, family=socket.AF_INET6)
    environ = {
        "wsgi.multiprocess": True,
    }

    for i in range(args.process_count):
        Process(target=serve_forever,
                kwargs={"listener": listener, "application": app.app.wsgi_app, "environ": environ}).start()
    serve_forever(listener, app.app.wsgi_app, environ)


if __name__ == "__main__":
    main()
