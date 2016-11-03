from Line import *

class Graph(object):
    def __init__(self, width, height, dx, dy, color):
        xpoints = []
        x = 0.0
        while True:
            xpoints.append(round(x))
            x += dx
            if x >= width:
                break


        ypoints = []
        y = 0.0
        while True:
            ypoints.append(round(y))
            y += dy
            if y >= height:
                break


        self.lines = []
        for x in xpoints:
            self.lines.append(Line(x,0, x,height, color))

        for y in ypoints:
            self.lines.append(Line(0,y, width,y, color))

    def draw(self, screen):
        for l in self.lines:
            pixels = l.getPixels()
            for p in pixels:
                p.draw(screen)

    def __del__(self):
        pass
