from Point import Point

class Pixel(object):
    def __init__(self, x, y, r, g, b):
        self.r = r
        self.g = g
        self.b = b
        self.point = Point(int(x),int(y))

    def draw(self, screen):
        screen.setPixel(self)


    def __del__(self):
        pass

class WhitePixel(Pixel):
    def __init__(self, x, y):
        super(WhitePixel, self).__init__(x, y, 255, 255, 255)

    def __del__(self):
        pass

