#!/usr/bin/env python3

from collections import namedtuple
import os
import sys
import argparse
import docker

Target = namedtuple("Target", ["target", "triplet"])
TARGETS = [
    Target(target="i686", triplet="i686-elf"),
    Target(target="aarch32", triplet="arm-none-eabi"),
]
DEFAULT_TARGET = TARGETS[0]


def build_image(dclient, dir_root, dir_out, image_name, target):
    build_args = {
        "SOURCE": dir_root,
        "BUILD": dir_out,
        "TARGET": target.triplet,
    }

    print("Trying to build the image. It will take a while...")
    _, logs = dclient.images.build(
        path=dir_root, tag=image_name, buildargs=build_args,)
    for entry in logs:
        if "stream" in entry:
            for line in entry["stream"].splitlines():
                print(line)


def run_cmd(dclient, dir_root, dir_out, verbose_lvl, image_name, command):
    volumes = {
        dir_root: {"bind": dir_root, "mode": "rw"},
        dir_out: {"bind": dir_out, "mode": "rw"},
    }
    envvars = {}
    if verbose_lvl > 0:
        envvars["STEPS"] = "1"

    container = dclient.containers.run(
        image=image_name,
        command=command,
        volumes=volumes,
        stdout=True,
        stderr=True,
        detach=True,
        environment=envvars,
    )
    container.wait()
    sys.stdout.buffer.write(container.logs())
    container.remove()


def gen_cc(dclient, dir_root, dir_out, verbose_lvl, image_name):
    targets_todo = ["clean", "test", "clean", "kernel", "clean", "libc"]
    for target in targets_todo:
        run_cmd(
            dclient,
            dir_root,
            dir_out,
            verbose_lvl,
            image_name,
            "compiledb -o {}/compile_commands.json ".format(
                dir_root) + "make " + target,
        )


def build_argparser():
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
        "action", help="Action to perform (exec, build_image, gen_compile_commands)"
    )
    a.add_argument("cmd", help="Command to execute.", nargs="*")
    a.add_argument(
        "-v", dest="verbose", action="count", default=0, help="Print build steps"
    )
    return a


def main():
    argsp = build_argparser()
    args = argsp.parse_args()
    target = [t for t in TARGETS if t.target == args.arch][0]
    image_name = "yaeos-{}".format(target.triplet)
    dclient = docker.from_env()

    if args.action == "exec":
        run_cmd(
            dclient=dclient,
            dir_root=args.project_root,
            dir_out=args.output,
            verbose_lvl=args.verbose,
            image_name=image_name,
            command=" ".join(args.cmd),
        )
    elif args.action == "build_image":
        build_image(
            dclient=dclient,
            dir_root=args.project_root,
            dir_out=args.output,
            image_name=image_name,
            target=target,
        )
    elif args.action == "gen_compile_commands":
        gen_cc(
            dclient=dclient,
            dir_root=args.project_root,
            dir_out=args.output,
            verbose_lvl=args.verbose,
            image_name=image_name,
        )
    else:
        argsp.print_help()


main()
