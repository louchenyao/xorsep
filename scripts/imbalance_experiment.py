#! /usr/bin/env python3

import random

def max_load(bins_n, balls_n):
    """ Simulates throwing balls_n balls into bins_n bins, and returns maximum load.
    """
    bins_n = int(bins_n)

    bins = [0] * bins_n
    for i in range(balls_n):
        x = random.randint(0, bins_n-1)
        bins[x] += 1

    return max(bins)

if __name__ == "__main__":
    n = 10**6
    epxected_per_group = 200
    loads = [max_load(n/epxected_per_group, n) for _ in range(5)]
    print(f"bins: {n/epxected_per_group}, balls: {n}, loads: ", loads)
    print(f"overhead rate: {max(loads)/epxected_per_group}")