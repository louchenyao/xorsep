#! /usr/bin/env python3

import argparse
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import os
import pandas as pd
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
    ssfe_query = extract_benchmark(log, "/ssfe_query/")
    ssfe_query_batch = extract_benchmark(log, "/ssfe_query_batch/")
    ssfe_dyn_query = extract_benchmark(log, "/ssfe_dong_query/")
    ssfe_dyn_query_batch = extract_benchmark(log, "/ssfe_dong_query_batch/")
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
    ax.plot(ssfe_query['keys'], ssfe_query['cpu_time'], label='SSFE', marker='v', color='teal')
    ax.plot(ssfe_query_batch['keys'], ssfe_query_batch['cpu_time'], label='SSFE+batch', marker='^', color='rosybrown')
    ax.plot(ssfe_dyn_query['keys'], ssfe_dyn_query['cpu_time'], label='SSFE_DYN', marker='<', color='bisque')
    ax.plot(ssfe_dyn_query_batch['keys'], ssfe_dyn_query_batch['cpu_time'], label='SSFE_DYN+batch', marker='>', color='greenyellow')
    ax.plot(setsep_query['keys'], setsep_query['cpu_time'], label='SetSep', marker='1', color='orangered')
    ax.plot(setsep_query_batch['keys'], setsep_query_batch['cpu_time'], label='SetSep+batch', marker='2', color='fuchsia')
    ax.plot(othello_query['keys'], othello_query['cpu_time'], label='Othello', marker='3', color='navy')

    # plot llc-misses
    if "LLC-misses" in ssfe_query.columns.values:
        ax = axs[1]
        ax.grid(True, axis='both')
        ax.set_xlabel("# Keys")
        ax.set_ylabel("LLC-misses")
        ax.plot(ssfe_query['keys'], ssfe_query['LLC-misses'], label='SSFE', marker='v', color='teal')
        ax.plot(ssfe_query_batch['keys'], ssfe_query_batch['LLC-misses'], label='SSFE+batch', marker='^', color='rosybrown')
        ax.plot(ssfe_dyn_query['keys'], ssfe_dyn_query['LLC-misses'], label='SSFE_DYN', marker='<', color='bisque')
        ax.plot(ssfe_dyn_query_batch['keys'], ssfe_dyn_query_batch['LLC-misses'], label='SSFE_DYN+batch', marker='>', color='greenyellow')
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
    ax.set_title('Gauss Elimination Performance\nrows/keys=182, cols=256\n')
    ax.grid(True, axis='y')

    # save
    fig.savefig("gauss_elimination.pdf", bbox_inches='tight')


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--csv', type=str, help='The benchmark output file path.')
    args = parser.parse_args()

    log = parse_bench_csv(args.csv)
    plot_query(log)
    plot_gaussian_elimination(log)