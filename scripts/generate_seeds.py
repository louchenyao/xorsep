#! /usr/bin/env python3

import random

def generate(n):
    s = ""
    for i in range(n):
        s1 = random.randint(0, 2**64 - 1)
        s2 = random.randint(0, 2**64 - 1)
        s3 = random.randint(0, 2**64 - 1)
        s += "{%du, %du, %du}, " % (s1, s2, s3)
    print(s)

if __name__ == "__main__":
    generate(256)