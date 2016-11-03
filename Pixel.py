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

    def __cmp__ (self, a):
        #print '__cmp__ called.'
        if a == None:
            return 1
        elif self.point.x < a.point.x:
            return -1
        elif self.point.x == a.point.x:
            if self.point.y < a.point.y:
                return -1
            elif self.point.y > a.point.y:
                return 1
            else:
                return 0
        else:
            return 1

class WhitePixel(Pixel):
    def __init__(self, x, y):
        super(WhitePixel, self).__init__(x, y, 255, 255, 255)

    def __del__(self):
        pass

