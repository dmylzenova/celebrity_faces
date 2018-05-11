#!/usr/bin/env python3
import argparse
import os
import app


def parse_args():
    parser = argparse.ArgumentParser(description="Celebrity faces viewer",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--host", type=str, default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2018)
    parser.add_argument("--debug", action="store_true", default=False)
    parser.add_argument("--uploaded-img-path", type=str,
                        default=os.path.dirname(__file__) + '/app/static/uploaded_img')

    return parser.parse_args()


def main():
    args = parse_args()
    app.init_app(args.uploaded_img_path)
    app.app.run(host=args.host, port=args.port, debug=args.debug)


if __name__ == "__main__":
    main()
