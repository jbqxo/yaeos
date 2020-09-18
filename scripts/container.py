#!/usr/bin/env python3

import os
import os.path
import sys
import re
import argparse
import subprocess
from collections import namedtuple
from dataclasses import dataclass

Target = namedtuple("Target", ["target", "triplet"])


@dataclass
class container:
    image_tag: str
    ports: dict
    dirs: dict


TARGETS = [
    Target(target="i686", triplet="i686-elf"),
    Target(target="aarch32", triplet="arm-none-eabi"),
]
DEFAULT_TARGET = TARGETS[0]


def find_target(arch):
    return [t for t in TARGETS if t.target == arch][0]


def build_image(target, container, jobs=1):
    cmd = ["docker", "build", "-m8g"]
    cmd += ["--build-arg", "SOURCE=" + container.dirs["root"]]
    cmd += ["--build-arg", "BUILD=" + container.dirs["build"]]
    cmd += ["--build-arg", "TRIPLET=" + target.triplet]
    cmd += ["--build-arg", "ARCH=" + target.target]
    cmd += ["--build-arg", "JOBS=" + str(jobs)]
    cmd += ["-t", container.image_tag]
    cmd += [container.dirs["root"]]

    subprocess.run(cmd)


# Stolen from: https://stackoverflow.com/a/38662876
def escape_ansi(line):
    ansi_escape = re.compile(r'(?:\x1B[@-_]|[\x80-\x9F])[0-?]*[ -/]*[@-~]')
    return ansi_escape.sub('', line)


def run_cmd(command, container, args):
    cmd = ["docker", "run"]
    cmd += ["-i", "--rm"]
    cmd += ["-v", container.dirs["root"] + ":" + container.dirs["root"]]
    cmd += ["-v", container.dirs["build"] + ":" + container.dirs["build"]]
    for int_port, ext_addr_port in container.ports.items():
        cmd += ["-p", ext_addr_port[0] + ":" +
                str(ext_addr_port[1]) + ":" + str(int_port)]

    if sys.stdin.isatty():
        cmd += ["-t"]

    cmd += [container.image_tag]
    cmd += command

    if args.clear_escapes:
        p = subprocess.run(cmd, capture_output=True, text=True)
        sys.stdout.write(escape_ansi(p.stdout))
        sys.stderr.write(escape_ansi(p.stderr))
    else:
        p = subprocess.run(cmd)
    return p.returncode == 0


def run_make(targets, container, args, variables={}, jobs=1):
    mvars = map(lambda kv: kv[0] + "=" + kv[1], variables.items())
    jobs_str = "-j{0}".format(jobs)

    return run_cmd(container=container,
                   command=["make"] + list(mvars) + [jobs_str] + targets,
                   args=args)


def do_exec(container, args):
    """Execute given comand"""
    run_cmd(container=container, command=args.cmd, args=args)


def do_make(container, args):
    """Run make with given target"""
    run_make(args.cmd, container, args, jobs=args.jobs)


def do_build_image(container, args):
    """Build docker image"""
    build_image(container=container,
                target=find_target(args.arch), jobs=args.jobs)


def do_gdb(container, args):
    """Run gdb server against given file"""
    run_cmd(container=container,
            command=["gdbserver", "0.0.0.0:1234"] + args.cmd,
            args=args)


def do_tests(container, args):
    """Run unit tests"""
    if not run_make(["tests"], container, args,
                    variables={"TARGET_ARCH": "tests"},
                    jobs=args.jobs):
        return
    run_cmd(container=container,
            command=["./scripts/run_tests.py"] + args.cmd,
            args=args)


def find_sanitizers(args):
    san_prefix = "tests_san_"
    confmk_dir = args.project_root + "/confmk"
    san_targets = []
    for f in os.listdir(confmk_dir):
        if not os.path.isfile(confmk_dir + "/" + f):
            continue
        if not f.startswith(san_prefix):
            continue
        san_targets.append(f.replace(".mk", ""))
    return san_targets


def do_sanitizers(container, args):
    """Run sanitizers"""
    if not run_make(["tests"], container, args,
                    variables={"TARGET_ARCH": "tests"},
                    jobs=args.jobs):
        return

    for t in find_sanitizers(args):
        if not run_make(["tests"], container, args,
                        variables={"TARGET_ARCH": t},
                        jobs=args.jobs):
            return

    run_cmd(container=container,
            command=["./scripts/run_sanitizers.py"] + args.cmd)


def do_clean(container, args):
    """Clean the build directory"""
    for t in find_sanitizers(args):
        run_make(["clean"], container, args,
                 variables={"TARGET_ARCH": t},
                 jobs=args.jobs)

    run_make(["clean"], container, args, jobs=args.jobs)
    run_make(["clean"], container, args, jobs=args.jobs,
             variables={"TARGET_ARCH": "tests"})


ACTIONS = {"exec": do_exec,
           "make": do_make,
           "image": do_build_image,
           "gdb": do_gdb,
           "test": do_tests,
           "san": do_sanitizers,
           "clean": do_clean, }


def parse_args():
    targets_str = ", ".join(map(lambda t: t.target, TARGETS))
    actions_str = ", ".join(
        map(lambda kv: "{0} ({1})".format(kv[0], kv[1].__doc__), ACTIONS.items()))
    cwd = os.getcwd()
    ncpu = os.cpu_count()

    a = argparse.ArgumentParser(description="Run isolated build environment")
    a.add_argument(
        "-a",
        dest="arch",
        help="Target architecture ({})".format(targets_str),
        default=DEFAULT_TARGET.target,
    )
    a.add_argument("-C",
                   dest="project_root",
                   help="Project root",
                   default=cwd)
    a.add_argument("-b",
                   dest="output",
                   help="Output directory",
                   default=cwd + "/build")
    a.add_argument("action",
                   help="Action to perform: " + actions_str,
                   choices=ACTIONS)
    a.add_argument("-e", dest="clear_escapes", action="store_true",
                   help="Remove escape sequences from the output")
    a.add_argument("-d", dest="debug", help="Debug port", default=1234)
    a.add_argument("cmd", help="Command to execute.", nargs="*")
    a.add_argument("-j", dest="jobs", help="Number of jobs", default=ncpu)
    return a, a.parse_args()


def main():
    raw_args, args = parse_args()

    c = container(image_tag="yaeos-{}"
                  .format(find_target(args.arch).triplet),
                  ports={"1234": ("127.0.0.1", args.debug)},
                  dirs={"root": args.project_root, "build": args.output})

    action = ACTIONS.get(args.action)
    if action is None:
        raw_args.print_help()
        return

    action(c, args)


main()
