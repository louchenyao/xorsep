#! /usr/bin/env python3

import argparse
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import os
import pandas as pd
import subprocess
import tempfile

# parse the benchmark output and return the datafram
def parse_bench_csv(csv_fn):
    # skim off the debug information
    with open(csv_fn) as f:
        c = ""
        for l in f.readlines():
            # there are at least 7 commas for each line
            if l.count(",") < 7:
                continue
            c += l
        
        tf = tempfile.NamedTemporaryFile(delete = False)
        tf.write(c.encode())
        tf.close()
    d = pd.read_csv(tf.name)
    os.unlink(tf.name)
    return d

def extract_benchmark(df, contains_str):
    b = df[df['name'].str.contains(contains_str)]
    # extract the number of keys for each run
    b['keys'] = b['name'].apply(lambda x: int(x.split('/')[-1]))
    return b

# plot the figure of query performances
def plot_query(log):
    xorsep_query = extract_benchmark(log, "/xorsep_query/")
    xorsep_query_batch = extract_benchmark(log, "/xorsep_query_batch/")
    xorsepdyn_query = extract_benchmark(log, "/xorsepdyn_query/")
    xorsepdyn_query_batch = extract_benchmark(log, "/xorsepdyn_query_batch/")
    setsep_query = extract_benchmark(log, "/setsep_query/")
    setsep_query_batch = extract_benchmark(log, "/setsep_query_batch/")
    othello_query = extract_benchmark(log, "/othello_query/")

    matplotlib.rcParams.update({'font.size': 20}) 
    fig, axs = plt.subplots(1, 2, figsize=(13, 4))

    # plot query
    ax = axs[0]
    ax.grid(True, axis='both')
    ax.set_xlabel("# Keys")
    ax.set_ylabel("Query Time (ns)")
    ax.plot(xorsep_query['keys'], xorsep_query['cpu_time'], label='XorSep', marker='v', color='teal')
    ax.plot(xorsep_query_batch['keys'], xorsep_query_batch['cpu_time'], label='XorSep+batch', marker='^', color='rosybrown')
    ax.plot(xorsepdyn_query['keys'], xorsepdyn_query['cpu_time'], label='XorSepDyn', marker='<', color='bisque')
    ax.plot(xorsepdyn_query_batch['keys'], xorsepdyn_query_batch['cpu_time'], label='XorSepDyn+batch', marker='>', color='greenyellow')
    ax.plot(setsep_query['keys'], setsep_query['cpu_time'], label='SetSep', marker='1', color='orangered')
    ax.plot(setsep_query_batch['keys'], setsep_query_batch['cpu_time'], label='SetSep+batch', marker='2', color='fuchsia')
    ax.plot(othello_query['keys'], othello_query['cpu_time'], label='Othello', marker='3', color='navy')

    # plot llc-misses
    if "LLC-misses" in xorsep_query.columns.values:
        ax = axs[1]
        ax.grid(True, axis='both')
        ax.set_xlabel("# Keys")
        ax.set_ylabel("LLC-misses")
        ax.plot(xorsep_query['keys'], xorsep_query['LLC-misses'], label='XorSep', marker='v', color='teal')
        ax.plot(xorsep_query_batch['keys'], xorsep_query_batch['LLC-misses'], label='XorSep+batch', marker='^', color='rosybrown')
        ax.plot(xorsepdyn_query['keys'], xorsepdyn_query['LLC-misses'], label='XorSepDyn', marker='<', color='bisque')
        ax.plot(xorsepdyn_query_batch['keys'], xorsepdyn_query_batch['LLC-misses'], label='XorSepDyn+batch', marker='>', color='greenyellow')
        ax.plot(setsep_query['keys'], setsep_query['LLC-misses'], label='SetSep', marker='1', color='orangered')
        ax.plot(setsep_query_batch['keys'], setsep_query_batch['LLC-misses'], label='SetSep+batch', marker='2', color='fuchsia')
        ax.plot(othello_query['keys'], othello_query['LLC-misses'], label='Othello', marker='3', color='navy')
 
    handles, labels = ax.get_legend_handles_labels()
    fig.legend(handles, labels, loc='upper center', bbox_to_anchor=(0.45, 1.25), ncol=4, shadow=True, fontsize=15)

    # save
    fig.savefig("query.pdf", bbox_inches='tight')

# plot gaussian elimination
def plot_gaussian_elimination(df):
    def extract_performance(df, name):
        r = df[df['name'].str.contains(name)]
        return r['items_per_second'].values[0]

    naive = extract_performance(df, 'BM_build_n')
    bitset = extract_performance(df, 'BM_build_bitset_')
    bitset_col = extract_performance(df, 'BM_build_bitset_2_')

    matplotlib.rcParams.update({'font.size': 20}) 
    fig, ax = plt.subplots(1, 1)
    y_pos = np.arange(3)
    ax.bar(y_pos, [naive, bitset, bitset_col], align='center', alpha=0.5)
    ax.set_xticks(y_pos)
    ax.set_xticklabels(['naive', 'bitset', 'bitset+col'])
    ax.set_ylabel('keys/sec')
    ax.set_title('Gauss Elimination Performance\n#ows(keys)=182, #cols=256\n')
    ax.grid(True, axis='y')

    # save
    fig.savefig("gauss_elimination.pdf", bbox_inches='tight')

def plot_cancels():
    p = subprocess.run(["bazel", "run", "//:group_exp_cancles"], stdout=subprocess.PIPE)
    assert(p.returncode == 0)
    n_x = []
    cancles_y = []

    # parse the output
    # sample output:
    # n = 232, m = 256, #cancles in Gaussian Elimination = 1215.910
    # n = 233, m = 256, #cancles in Gaussian Elimination = 1165.770
    for l in p.stdout.decode().splitlines():
        if l.startswith("n = "):
            n, m, cancles = l.split(",")
            n = int(n.split()[-1])
            m = int(m.split()[-1])
            cancles = float(cancles.split()[-1])

            n_x.append(n)
            cancles_y.append(cancles)
            assert(m == 256)
    

    # plot
    matplotlib.rcParams.update({'font.size': 20}) 
    fig, ax = plt.subplots(1, 1)
    ax.grid(True, axis='both')
    ax.set_xlabel("# Keys (Rows)")
    ax.set_ylabel("# Cancels")
    ax.set_title("Expected number of cancels in\nGauss Elimination\n#cols = 256")
    ax.plot(n_x, cancles_y, color='teal')
    # save
    fig.savefig("cancles.pdf", bbox_inches='tight')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--csv', type=str, help='The benchmark output file path.')
    parser.add_argument('--filter', type=str, default="", help='Run only the plots whose name contains the filter')
    args = parser.parse_args()

    if args.filter in  plot_query.__name__:
        log = parse_bench_csv(args.csv)
        plot_query(log)

    if args.filter in plot_gaussian_elimination.__name__:
        log = parse_bench_csv(args.csv)
        plot_gaussian_elimination(log)

    if args.filter in plot_cancels.__name__:
        plot_cancels()