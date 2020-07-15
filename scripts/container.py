#!/usr/bin/env python3

import os
import sys
import signal
import argparse
import subprocess
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


def build_image(target, jobs=1):
    cmd = ["docker", "build", "-m8g"]
    cmd += ["--build-arg", "SOURCE=" + Container.dirs["root"]]
    cmd += ["--build-arg", "BUILD=" + Container.dirs["build"]]
    cmd += ["--build-arg", "TARGET=" + target.triplet]
    cmd += ["--build-arg", "JOBS=" + str(jobs)]
    cmd += ["-t", Container.image_tag]
    cmd += [Container.dirs["root"]]

    subprocess.run(cmd)


def run_cmd(command):
    cmd = ["docker", "run"]
    cmd += ["-it", "--rm"]
    cmd += ["-v", Container.dirs["root"] + ":" + Container.dirs["root"]]
    cmd += ["-v", Container.dirs["build"] + ":" + Container.dirs["build"]]
    for int_port, ext_addr_port in Container.ports.items():
        cmd += ["-p", ext_addr_port[0] + ":" + str(ext_addr_port[1]) + ":" + str(int_port)]

    cmd += [Container.image_tag]
    cmd += command

    subprocess.run(cmd)


def main():
    targets_str = ", ".join(map(lambda t: t.target, TARGETS))
    cwd = os.getcwd()
    ncpu = os.cpu_count()

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
    a.add_argument("-j", dest="jobs", help="Number of jobs", default=ncpu)

    args = a.parse_args()
    target = [t for t in TARGETS if t.target == args.arch][0]

    global Container
    Container = Container(image_tag="yaeos-{}".format(target.triplet),
                          ports={"1234": ("127.0.0.1", args.debug)},
                          dirs={"root": args.project_root, "build": args.output})

    if args.action == "exec":
        run_cmd(
            command=args.cmd,
        )
    elif args.action == "make":
        run_cmd(
            command=["make"] + args.cmd,
        )
    elif args.action == "build_image":
        build_image(
            target=target,
            jobs=args.jobs
        )
    elif args.action == "gdbserver":
        run_cmd(
            command=["gdbserver", "0.0.0.0:1234"] + args.cmd,
        )
    elif args.action == "tests":
        run_cmd(
            command=["make", "tests"],
        )
        run_cmd(
            command=["./scripts/run_tests.py"] + args.cmd,
        )
    else:
        a.print_help()


main()
