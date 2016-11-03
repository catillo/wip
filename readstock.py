#!/usr/bin/env python

from Screen import *
from PPM import *
from Line import *
from Graph import *
from pprint import pformat
from Pixel import *
import sys

def getHeader(data):
    header = {}
    token = ''
    header['ppm_type'] = ''
    header['width'] = ''
    header['height'] = ''
    header['maxColor'] = ''

    dataOffset = 0

    for d in data:
        dataOffset += 1
        if d == '\n':
            if '#' in token:
                pass
            elif header['ppm_type'] == '':
                header['ppm_type'] = token
            elif header['width'] == '':
                header['width'] = token.split(' ')[0]
                header['height'] = token.split(' ')[1]
            elif header['maxColor'] == '':
                header['maxColor'] = token
                break

            token = ''
            continue

        token += d


    return header, dataOffset

def findPoints(header, data, dataOffset, sampleWidth):
    width = int(header['width'])
    height = int(header['height'])

    imgSize = width * height * 3
    r = -1
    g = -1
    b = -1

    points = []

    x = 0
    y = height - 1

    for i in range(0, imgSize):
        if x >= width:
            x = 0
            y -= 1

        pos = i + dataOffset
        val = ord(data[pos])
        if r == -1:
            r = val
        elif g == -1:
            g = val
        elif b == -1:
            b = val

            if (x % sampleWidth) == 0:
                c = Color('blue')

                if r == c.r and g == c.g and b == c.b:
                    bluePixel = Pixel(x, y, r, g, b)
                    points.append(bluePixel)

            r = -1
            g = -1
            b = -1

            x += 1

    return points

def main():
    args = sys.argv

    if len(args) < 2:
        print 'Usage: %s [ppm file]' % args[0]
        return -1

    in_ppm = args[1]

    with open(in_ppm, 'rb') as file:
        data = file.read()

        header, dataOffset = getHeader(data)

        print 'Header = %s' % (pformat(header))
        print 'Dataoffset = %d' % (dataOffset)

        points = findPoints(header, data, dataOffset, 10)

        s = Screen(2000, 1000)
        g = Graph(1999, 999, 10, 10, 'grey')
        g.draw(s)
        blue = Color('blue')
        prevPoint = None

        for p in points:
            if False:
                if prevPoint == None:
                    prevPoint = p
                else:
                    print 'Making line %d,%d - %d,%d' % (prevPoint.point.x, prevPoint.point.y, 
                                                         p.point.x, p.point.y)
                    l = Line(prevPoint.point.x, prevPoint.point.y, 
                         p.point.x, p.point.y, "blue")
                    l.draw(s)

                    prevPoint = p
            else:
                print 'Point (%d,%d) ' % (p.point.x, p.point.y)
                p.draw(s)

        p = PPM(s)
        p.writeToFile('newimage.ppm')
main()
