#! /usr/bin/env python3

import argparse
import matplotlib
import matplotlib.pyplot as plt
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
    sepset_query = extract_benchmark(log, "/sepset_query/")
    sepset_query_batch = extract_benchmark(log, "/sepset_query_batch/")
    othello_query = extract_benchmark(log, "/othello_query/")

    # plot
    matplotlib.rcParams.update({'font.size': 20}) 
    fig, ax = plt.subplots(1, 1)
    ax.grid(True, axis='both')
    ax.set_xlabel("# Keys")
    ax.set_ylabel("Query Time (ns)")
    ax.plot(ssfe_query['keys'], ssfe_query['cpu_time'], label='SSFE', marker='v', color='teal')
    ax.plot(ssfe_query_batch['keys'], ssfe_query_batch['cpu_time'], label='SSFE+batch', marker='^', color='rosybrown')
    ax.plot(ssfe_dyn_query['keys'], ssfe_dyn_query['cpu_time'], label='SSFE_DYN', marker='<', color='bisque')
    ax.plot(ssfe_dyn_query_batch['keys'], ssfe_dyn_query_batch['cpu_time'], label='SSFE_DYN+batch', marker='>', color='greenyellow')
    ax.plot(sepset_query['keys'], sepset_query['cpu_time'], label='SetSep', marker='1', color='orangered')
    ax.plot(sepset_query_batch['keys'], sepset_query_batch['cpu_time'], label='SetSep+batch', marker='2', color='fuchsia')
    ax.plot(othello_query['keys'], othello_query['cpu_time'], label='Othello', marker='3', color='navy')
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.4), ncol=2, shadow=True, fontsize=15)

    # save
    fig.savefig("query.pdf", bbox_inches='tight')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--csv', type=str, help='The benchmark output file path.')
    args = parser.parse_args()

    log = parse_bench_csv(args.csv)
    plot_query(log)