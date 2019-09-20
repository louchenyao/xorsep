#! /usr/bin/env python3

import matplotlib.pyplot as plt
import random

def simulate(bins_n, balls_n):
    """ Simulates throwing balls_n balls into bins_n bins, and returns #balls in bins.
    """
    bins_n = int(bins_n)

    bins = [0] * bins_n
    for i in range(balls_n):
        x = random.randint(0, bins_n-1)
        bins[x] += 1

    bins.sort()
    return bins

def nth_percentile(a, n):
    l = len(a)
    idx = int(n/100*l) - 1
    assert(idx >= 0)
    return a[idx]

def analyze(bins):
    bins_n = len(bins)
    balls_n = sum(bins)
    avg = balls_n / bins_n
    print(f"bins: {bins_n}, balls: {balls_n} ({balls_n/8/1024/1024} Mb), avg: {avg}")

    bits_for_indexes = 0
    bits_for_indexes += 16 * bins_n # bins index
    bits_for_indexes += 6 * bins_n # bin's hash family
    print("bits for indexes: %.4f" % (bits_for_indexes/balls_n))

    for i in [60, 65, 70, 75, 80, 85, 90, 95, 100]:
        ith_load = nth_percentile(bins, i)
        
        bits = 0
        
        bits_for_arrays = 0
        bits_for_arrays += i/100 * bins_n * 1.1 * ith_load # bitarrys for bins which have less than ith percentile load
        bits_for_arrays += (100-i)/100 * bins_n * 1.1 * bins[-1] # bitarrys for remaining bins

        bits = bits_for_indexes + bits_for_arrays

        print(f"%3dth percentile load: {ith_load}, bits: %.4f, bits for arrays: %.4f" % (i, bits/balls_n, bits_for_arrays/balls_n))

def plot_load_distribution(bins):
    mx = max(bins)
    balls_n = sum(bins)
    avg = int(balls_n / len(bins))

    counts = [0]*800
    for b in bins:
        counts[b] += 1

    fig, ax = plt.subplots(1, 1)
    ax.plot(counts)
    fig.savefig(f"dist_{balls_n}_{avg}.png")
    

if __name__ == "__main__":
    n = 1*10**7

    for epxected_per_group in [200, 250, 300, 350, 400]:
        print("********************************")
        print(f"* balls = {n}, avg_per_bin = {epxected_per_group}")
        print("********************************")
        bins = simulate(n/epxected_per_group, n)
        plot_load_distribution(bins)
        for i in range(3):
            print(f"run {i} ...")
            bins = simulate(n/epxected_per_group, n)
            analyze(bins)
