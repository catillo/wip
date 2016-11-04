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
        runner = 0
        for x in xpoints:
            if (runner % 15) == 0:
                lineColor = 'red'
            elif (runner % 5) == 0:
                lineColor = 'green'
            else:
                lineColor = 'grey'

            self.lines.append(Line(x, 0, x, height, lineColor))
            runner += 1

        runner = 0
        for y in ypoints:
            if (runner % 15) == 0:
                lineColor = 'red'
            elif (runner % 5) == 0:
                lineColor = 'green'
            else:
                lineColor = 'grey'

            self.lines.append(Line(0,y, width,y, lineColor))
            runner += 1

    def draw(self, screen):
        for l in self.lines:
            pixels = l.getPixels()
            for p in pixels:
                p.draw(screen)

    def __del__(self):
        pass
