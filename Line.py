import Point

class Line(object):
    def __init__(self, x1, y1 , x2, y2):
        self.a = Point(x1,y1)
        self.b = Point(x2,y2)

    def __del__(self):
        pass
