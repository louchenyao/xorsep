#! /usr/bin/env python3

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
    print("================================")
    print(f"bins: {bins_n}, balls: {balls_n} ({balls_n/8/1024/1024} Mb), avg: {avg}")
    for i in [60, 65, 70, 75, 80, 85, 90, 95, 100]:
        ith_load = nth_percentile(bins, i)
        
        bits = 0
        bits += 16 * bins_n # bins index
        bits += 6 * bins_n # bin's hash family
        bits += i/100 * bins_n * 1.1 * ith_load # bitarrys for bins which have less than ith percentile load
        bits += (100-i)/100 * bins_n * 1.1 * bins[-1] # bitarrys for remaining bins

        print(f"{i}th percentile load: {ith_load}, bits: {bits/balls_n}, bits for index and hash family: {24*bins_n/balls_n}")

if __name__ == "__main__":
    n = 1*10**7
    epxected_per_group = 300
    
    for _ in range(5):
        bins = simulate(n/epxected_per_group, n)
        analyze(bins)
