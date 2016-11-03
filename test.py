#!/usr/bin/env python

from Screen import *
from PPM import *
from Line import *

def main():
    s = Screen(2000, 1000)
    l = Line(0,0, 1999, 999, 'black')
    l.draw(s)

    p = PPM(s)
    p.writeToFile('image.ppm')

main()
