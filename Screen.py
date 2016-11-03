from Pixel import WhitePixel

class Screen(object):
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.rows = []
        for h in range(0, height):
            row = []
            self.rows.append(row)
            for w in range(0, width):
                row.append(WhitePixel(w,h))

    def getRow(self, row):
        assert(row < self.height)
        return self.rows[self.height -1 - row]

    def __del__(self):
        pass
