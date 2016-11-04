import Point
from Pixel import *

class Color(object):
    def __init__(self, color):
        r = 0
        g = 0
        b = 0

        if color == 'black':
            r = 0
            g = 0
            b = 0
        elif color == 'white':
            r = 255
            g = 255
            b = 255
        elif color == 'yellow':
            r = 255
            g = 255
            b = 0
        elif color == 'blue':
            r = 0
            g = 0
            b = 255
        elif color == 'grey':
            r = 225
            g = 225
            b = 225

        elif color == 'green':
            r = 0
            g = 225
            b = 0
        elif color == 'red':
            r = 255
            g = 0
            b = 0

        else:
            assert False, 'Unsupported color!'

        self.r = r
        self.g = g
        self.b = b


    def __del__(self):
        pass
