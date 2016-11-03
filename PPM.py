from Screen import Screen

class PPM(object):
    def __init__(self, screen):
        self.screen = screen

    def writeToFile(self, filename):
        screen = self.screen
        f = open(filename, 'wb')
        header = 'P6\n%d\n %d\n255\n' % (screen.width, screen.height)
        f.write(header)

        for h in range(0, screen.height):
            r = screen.getRow(screen.height - 1 - h)
            for p in r:
                ba = bytearray([p.r, p.g, p.b])
                f.write(ba)

        f.close()

    def __del__(self):
        pass
