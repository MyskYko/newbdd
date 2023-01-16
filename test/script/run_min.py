import subprocess

def getsize(f):
    sp = subprocess.run(f"abc -q \"read {f}; ps;\" | grep -Po \"and =[ ]*\K[0-9]*\"", shell = True, capture_output = True, encoding="utf-8", check=True)
    return int(sp.stdout.strip())

def getio(f):
    sp = subprocess.run(f"abc -q \"read {f}; ps;\" | grep -Po \"i/o =[ ]*\K[0-9/ ]*\"", shell = True, capture_output = True, encoding="utf-8", check=True)
    io = sp.stdout.split('/')
    return [int(s.strip()) for s in io]

def tra1(f, g):
    tra1.seed += 1
    subprocess.run(f"../../build/tra -r {tra1.seed} {f} {g}", shell = True, check=True)

def tra2(f, g, sorttype, pishuffle, parameter):
    subprocess.run(f"../../build/tra -s {sorttype} -i {pishuffle} -p {parameter} {f} {g}", shell = True, check=True)

def abc1(f, g):
    subprocess.run(f"abc -q \"read {f}; {abc1.opt1}; write {g}\"", shell = True, check=True)
abc1.dc2 = "dc2; dc2; dc2; dc2; dc2"
abc1.opt1 = f"compress2rs; {abc1.dc2}; resyn; {abc1.dc2}; resyn2; {abc1.dc2}; resyn3; {abc1.dc2}; resub –l -N 2 -K 16; {abc1.dc2}; iresyn –l; {abc1.dc2}; resyn2rs; {abc1.dc2}; &get; &fraig –x; &put; {abc1.dc2}"

def abc2(f, g):
    subprocess.run(f"abc -q \"read {f}; dch; if -a -m; mfs2 -e; st; write {g}\"", shell = True, check=True)

def abc3(f, g):
    subprocess.run(f"abc -q \"read {f}; clp; st; write {g}\"", shell = True, check=True)

def abc4(f, g):
    io = getio(f)
    subprocess.run(f"abc -q \"&r {f}; &ttopt -I {io[0]} -O {io[1]} -X 100; &w {g}\"", shell = True, check=True)

import shutil
import os

def trabest(f):
    n = getsize(f)
    g = f + ".tmp.aig"
    shutil.copyfile(f, g)
    for st in range(4):
        for pi in range(5):
            for pa in range(16):
                h = g + ".tmp.aig"
                tra2(g, h, st, pi, pa)
                abc1(h, h)
                m = getsize(h)
                #print(f"\t\t\t\t\ttrabest ({st}, {pi}, {pa}) {m}")
                if m < n:
                    n = m
                    shutil.copyfile(h, f)
                os.remove(h)
    os.remove(g)

def opt1(f):
    n = getsize(f)
    while True:
        if not opt1.abc_only:
            tra1(f, f)
        abc1(f, f)
        m = getsize(f)
        print(f"\t\t\t\topt1 {m}")
        if m < n:
            n = m
        else:
            break

def opt2(f):
    opt1(f)
    n = getsize(f)
    print(f"\t\t\topt2 {n}")
    g = f + ".tmp.aig"
    shutil.copyfile(f, g)
    for i in range(opt2.n):
        abc2(g, g)
        opt1(g)
        m = getsize(g)
        print(f"\t\t\topt2 {m}")
        if m < n:
            n = m
            shutil.copyfile(g, f)
    os.remove(g)

def opt3(f):
    n = getsize(f)
    g = f + ".tmp.aig"
    shutil.copyfile(f, g)
    for i in range(opt3.n):
        tra1.seed = 1234 * i
        h = g + ".tmp.aig"
        shutil.copyfile(f, h)
        opt2(h)
        m = getsize(h)
        print(f"\t\topt3 {m}")
        if m < n:
            n = m
            shutil.copyfile(h, g)
        os.remove(h)
    shutil.copyfile(g, f)
    os.remove(g)

def opt4(f):
    n = getsize(f)
    a = f + ".tmp.aig"
    shutil.copyfile(f, a)
    b = f + ".clp.aig"
    abc3(f, b)
    c = f + ".ttopt.aig"
    abc4(f, c)
    for g in [a, b, c]:
        opt3(g)
        m = getsize(g)
        print(f"\topt4 {m}")
        if m < n:
            n = m
            shutil.copyfile(g, f)
        os.remove(g)


def run(f):
    g = run.output_dir + '/' + os.path.basename(f)
    shutil.copyfile(f, g)
    opt4(g)


import sys
import argparse
import multiprocessing

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('input_list', type=str)
    parser.add_argument('output_dir', type=str)
    parser.add_argument('-n', '--num_restarts', type=int, default=20)
    parser.add_argument('-m', '--num_hops', type=int, default=10)
    parser.add_argument('-a', '--abc_only', action='store_true')
    parser.add_argument('-p', '--parallel', action='store_true')
    parser.add_argument('-v', '--verbose', action='store_true')
    args = parser.parse_args()

    opt3.n = args.num_restarts + 1
    opt2.n = args.num_hops
    opt1.abc_only = args.abc_only

    if os.path.exists(args.output_dir):
        inp = input(f"rm {args.output_dir}? [y/n] : ")
        if inp != 'y':
            print("terminated")
            exit(0)
        if os.path.isfile(args.output_dir):
            os.remove(args.output_dir)
        else:
            shutil.rmtree(args.output_dir)

    if not args.verbose:
        sys.stdout = open(os.devnull, 'w')

    os.mkdir(args.output_dir)
    run.output_dir = args.output_dir

    f = open(args.input_list)
    lines = f.readlines()
    lines = [line.strip() for line in lines]
    lines = [line for line in lines if line.endswith('.aig')]
    
    if args.parallel:
        p = multiprocessing.Pool()
        p.map(run, lines)
    else:
        for line in lines:
            run(line)

    sys.stdout = sys.__stdout__
    for line in lines:
        s = getsize(args.output_dir + "/" + os.path.basename(line))
        print(f"{os.path.basename(line)} {s}")
