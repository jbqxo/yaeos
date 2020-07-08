#!/usr/bin/env python3

import os
import sys
import signal
import argparse
import docker
from collections import namedtuple
from dataclasses import dataclass

Target = namedtuple("Target", ["target", "triplet"])
TARGETS = [
    Target(target="i686", triplet="i686-elf"),
    Target(target="aarch32", triplet="arm-none-eabi"),
]
DEFAULT_TARGET = TARGETS[0]


@dataclass
class Container:
    image_tag: str
    ports: dict
    dirs: dict
    obj: docker.models.containers.Container = None


def sigint_handler(signum, frame):
    if Container.obj is not None:
        print("Trying to stop the container...")
        Container.obj.stop(timeout=3)
        sys.stdout.buffer.write(Container.obj.logs())
        Container.obj.remove()
    sys.exit(0)


def build_image(dclient, target):
    build_args = {
        "SOURCE": Container.dirs["root"],
        "BUILD": Container.dirs["build"],
        "TARGET": target.triplet,
    }

    print("Trying to build the image. It will take a while...")
    _, logs = dclient.images.build(path=Container.dirs["root"],
                                   tag=Container.image_tag,
                                   buildargs=build_args)
    for entry in logs:
        if "stream" in entry:
            for line in entry["stream"].splitlines():
                print(line)


def run_cmd(dclient, command):
    global Container
    volumes = {
        Container.dirs["root"]: {"bind": Container.dirs["root"],
                                       "mode": "rw"},
        Container.dirs["build"]: {"bind": Container.dirs["build"],
                                        "mode": "rw"}
    }

    Container.obj = dclient.containers.create(
        image=Container.image_tag,
        command=command,
        volumes=volumes,
        ports=Container.ports,
        detach=True)
    try:
        Container.obj.start()
    finally:
        Container.obj.wait()
        sys.stdout.buffer.write(Container.obj.logs())
        Container.obj.remove()


def main():
    targets_str = ", ".join(map(lambda t: t.target, TARGETS))
    cwd = os.getcwd()

    a = argparse.ArgumentParser(description="Run isolated build environment")
    a.add_argument(
        "-a",
        dest="arch",
        help="Target architecture ({})".format(targets_str),
        default=DEFAULT_TARGET.target,
    )
    a.add_argument("-C", dest="project_root", help="Project root", default=cwd)
    a.add_argument(
        "-b", dest="output", help="Output directory", default=cwd + "/build"
    )
    a.add_argument(
        "action", help="Action to perform (exec, build_image, make, gdbserver, tests)"
    )
    a.add_argument("-d", dest="debug", help="Debug port", default=1234)
    a.add_argument("cmd", help="Command to execute.", nargs="*")

    args = a.parse_args()
    target = [t for t in TARGETS if t.target == args.arch][0]
    dclient = docker.from_env()

    global Container
    Container = Container(image_tag="yaeos-{}".format(target.triplet),
                          ports={"1234": ("127.0.0.1", args.debug)},
                          dirs={"root": args.project_root, "build": args.output})

    signal.signal(signal.SIGINT, sigint_handler)
    if args.action == "exec":
        run_cmd(
            dclient=dclient,
            command=" ".join(args.cmd),
        )
    elif args.action == "make":
        run_cmd(
            dclient=dclient,
            command="make " + " ".join(args.cmd),
        )
    elif args.action == "build_image":
        build_image(
            dclient=dclient,
            target=target,
        )
    elif args.action == "gdbserver":
        run_cmd(
            dclient=dclient,
            command="gdbserver 0.0.0.0:1234 " + " ".join(args.cmd),
        )
    elif args.action == "tests":
        run_cmd(
            dclient=dclient,
            command="make tests"
        )
        run_cmd(
            dclient=dclient,
            command="./scripts/run_tests.py " + " ".join(args.cmd),
        )
    else:
        a.print_help()


main()
