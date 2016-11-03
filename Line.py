import Point
from Pixel import *
from Color import *

class Line(object):
    def __init__(self, x1, y1 , x2, y2, color):
        self.a = Point(x1,y1)
        self.b = Point(x2,y2)
        self.color = Color(color)

    def draw(self, screen):
        pixels = self.getPixels()
        for p in pixels:
            p.draw(screen)


    def getPixels(self):
        color = self.color
        r = color.r
        g = color.g
        b = color.b

        x1 = self.a.x
        y1 = self.a.y
        x2 = self.b.x
        y2 = self.b.y

        points = []
        issteep = abs(y2-y1) > abs(x2-x1)
        if issteep:
            x1, y1 = y1, x1
            x2, y2 = y2, x2
        rev = False
        if x1 > x2:
            x1, x2 = x2, x1
            y1, y2 = y2, y1
            rev = True
        deltax = x2 - x1
        deltay = abs(y2-y1)
        error = int(deltax / 2)
        y = y1
        ystep = None
        if y1 < y2:
            ystep = 1
        else:
            ystep = -1
        for x in range(x1, x2 + 1):
            if issteep:
                points.append(Pixel(y, x, r, g, b))
            else:
                points.append(Pixel(x, y, r, g, b))
            error -= deltay
            if error < 0:
                y += ystep
                error += deltax
        # Reverse the list if the coordinates were reversed
        if rev:
            points.reverse()
        return points


    def __del__(self):
        pass
