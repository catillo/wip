from Pixel import WhitePixel
from Color import *

class Screen(object):
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.rows = {}

    def setPixel(self, pixel):
        x = pixel.point.x
        y = pixel.point.y

        p = self.getPixel(x, y)
        p.r = pixel.r
        p.g = pixel.g
        p.b = pixel.b

    def getPixel(self, x, y):
        assert x < self.width, 'Out of bounds!'

        row = self.getRow(y)
        pixel = row.get(x, None)
        if pixel == None:
            pixel = WhitePixel(x, y)
            row[x] = pixel
        return pixel


    def getRow(self, row):
        assert(row < self.height)
        rowNum = self.height -1 - row

        row = self.rows.get(rowNum, None)
        if row == None:
            row = {}
            self.rows[rowNum] = row

        return row


    def __del__(self):
        pass
