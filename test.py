#!/usr/bin/env python

from Screen import *
from PPM import *

def main():
    s = Screen(1000, 1000)
    p = PPM(s)
    p.writeToFile('image.ppm')

main()
