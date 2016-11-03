#!/usr/bin/env python

from Screen import *
from PPM import *
from Line import *
from Graph import *

def main():
    s = Screen(2000, 1000)
    g = Graph(1999, 999, 10, 10, 'grey')
    g.draw(s)

    p = PPM(s)
    p.writeToFile('image.ppm')

main()
