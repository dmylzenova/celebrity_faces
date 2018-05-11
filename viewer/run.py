#!/usr/bin/env python3
import argparse
from app import app
from app import views


def parse_args():
    parser = argparse.ArgumentParser(description="Celebrity faces viewer",
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--host", type=str, default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2018)
    parser.add_argument("--debug", action="store_true", default=False)

    return parser.parse_args()


def main():
    args = parse_args()

    app.run(host=args.host, port=args.port, debug=args.debug)


if __name__ == "__main__":
    main()
