#!/usr/bin/env python

from Screen import *
from PPM import *
from Line import *
from Graph import *
from pprint import pformat
from Pixel import *
import sys

def getHeader(data):
    header = {}
    token = ''
    header['ppm_type'] = ''
    header['width'] = ''
    header['height'] = ''
    header['maxColor'] = ''

    dataOffset = 0

    for d in data:
        dataOffset += 1
        if d == '\n':
            if '#' in token:
                pass
            elif header['ppm_type'] == '':
                header['ppm_type'] = token
            elif header['width'] == '':
                header['width'] = token.split(' ')[0]
                header['height'] = token.split(' ')[1]
            elif header['maxColor'] == '':
                header['maxColor'] = token
                break

            token = ''
            continue

        token += d


    return header, dataOffset

def duplicateXinList(p, l):
    x = p.point.x
    for i in l:
        if i.point.x == x:
            return True
    return False


def findPoints(header, data, dataOffset, sampleWidth):
    width = int(header['width'])
    height = int(header['height'])

    imgSize = width * height * 3
    r = -1
    g = -1
    b = -1

    points = []

    x = 0
    y = height - 1

    for i in range(0, imgSize):
        if x >= width:
            x = 0
            y -= 1

        pos = i + dataOffset
        val = ord(data[pos])
        if r == -1:
            r = val
        elif g == -1:
            g = val
        elif b == -1:
            b = val

            if (x % sampleWidth) == 0:
                c = Color('blue')

                if r == c.r and g == c.g and b == c.b:
                    bluePixel = Pixel(x, y, r, g, b)
                    points.append(bluePixel)

            r = -1
            g = -1
            b = -1

            x += 1

    assert None not in points
    points.sort()

    filtered = []
    for p in points:
        if not duplicateXinList(p, filtered):
            filtered.append(p)

    return filtered

def drawPoints(points):
    s = Screen(2000, 1000)
    g = Graph(1999, 999, 10, 10, 'grey')
    g.draw(s)
    prevPoint = None

    for p in points:
        if True:
            if prevPoint == None:
                prevPoint = p
            else:
                '''print 'Making line %d,%d - %d,%d' % (prevPoint.point.x, prevPoint.point.y,
                                                     p.point.x, p.point.y)'''
                l = Line(prevPoint.point.x, prevPoint.point.y,
                     p.point.x, p.point.y, "blue")
                l.draw(s)

                prevPoint = p
        else:
            print 'Point (%d,%d) ' % (p.point.x, p.point.y)
            p.draw(s)

    p = PPM(s)
    p.writeToFile('newimage.ppm')

class SPAction_BuyNoSell (object):
    def __init__(self):
        self.initialPoint = None
        self.isp = 100.00                # initial stock price
        self.investment = 1000.00
        self.numStocks = 0.00
        self.totalInvestment = 0.00
        self.stockPrice = None

    def run(self, point):
        if self.initialPoint == None:
            self.initialPoint = point
            self.iy  = self.initialPoint.point.y  # initial y


        self.stockPrice = (self.isp * point.point.y) / self.iy

        stocks_bought = (self.investment / self.stockPrice)
        self.numStocks += stocks_bought

        self.totalInvestment += self.investment

        print "New stock price = %f" % self.stockPrice

    def finish(self):
        print '\n'
        print 'totalInvestment = %f' % self.totalInvestment
        print 'Last stock price = %f' % self.stockPrice
        print 'numStocks = %f' % self.numStocks
        print 'total cash value = %f' % (self.stockPrice * self.numStocks)

    def __del__(self):
        pass

class SPAction_BuyAndSellAtThreshold (object):
    def __init__(self):
        self.initialPoint = None
        self.initialStockPrice = 100.00                # initial stock price
        self.investmentPerCycle = 1000.00
        self.totalNumStocks = 0
        self.totalInvestment = 0.00
        self.stockPrice = None
        self.cashOnHand = 0.00

        self.buyBelow = 140.00
        self.sellAbove = 147.00

    def buy(self, stockPrice):
        stocks_bought = int(self.cashOnHand / stockPrice)

        if stocks_bought <= 0:
            print """Can't buy. Not enough cash!"""
            return

        total_bill = stockPrice * stocks_bought 

        remainder = self.cashOnHand % stockPrice

        self.cashOnHand -= total_bill
        assert self.cashOnHand >= 0
        self.cashOnHand += remainder

        self.totalNumStocks += stocks_bought

        print 'Bought stock at %f per share' % stockPrice

    def sell(self, stockPrice):
        if self.totalNumStocks <= 0:
            print 'Nothing to sell!!!'
            return

        self.cashOnHand += (stockPrice * self.totalNumStocks)
        stocksSold = self.totalNumStocks
        self.totalNumStocks = 0

        print 'Sold %d stock(s) at %f per share' % (stocksSold, self. stockPrice)


    def run(self, point):
        if self.initialPoint == None:
            self.initialPoint = point
            self.iy  = self.initialPoint.point.y  # initial y
            print 'Stock multiplier = %f' % (self.initialStockPrice  / self.iy)


        self.cashOnHand += self.investmentPerCycle
        self.totalInvestment += self.investmentPerCycle

        self.stockPrice = (self.initialStockPrice * point.point.y) / self.iy

        print 'Stock Price at %f' % (self.stockPrice)

        if (self.stockPrice >= self.sellAbove):
            self.sell(self.stockPrice)
        elif (self.stockPrice <= self.buyBelow):
            self.buy(self.stockPrice)



    def finish(self):
        print '\n'
        print 'totalInvestment = %f' % self.totalInvestment
        print 'Last stock price = %f' % self.stockPrice
        print 'numStocks = %f' % self.totalNumStocks

        totalStock_CashValue = (self.stockPrice * self.totalNumStocks)
        print 'total cash value of stocks = %f' % totalStock_CashValue

        print 'total cash on hand = %f' % self.cashOnHand

        totalCashValue = (self.cashOnHand + ((self.stockPrice * self.totalNumStocks)))
        print 'Total Cash Value = %f' % totalCashValue

        growth = (((totalCashValue - self.totalInvestment) / self.totalInvestment) * 100.00)
        print 'Growth = %0.2f %%' % growth

    def __del__(self):
        pass


def calculateSavings(points, action):
    for p in points:
        action.run(p)
        

    action.finish()
    # every point, we add 1K phpe

def main():
    args = sys.argv

    if len(args) < 2:
        print 'Usage: %s [ppm file]' % args[0]
        return -1

    in_ppm = args[1]

    points = None

    with open(in_ppm, 'rb') as file:
        data = file.read()

        header, dataOffset = getHeader(data)

        print 'Header = %s' % (pformat(header))
        print 'Dataoffset = %d' % (dataOffset)

        points = findPoints(header, data, dataOffset, 40)

    #drawPoints(points)

    #savings = calculateSavings(points, SPAction_BuyNoSell())
    savings = calculateSavings(points, SPAction_BuyAndSellAtThreshold())


main()
