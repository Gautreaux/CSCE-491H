import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

class Ellipse():
    def __init__(self, a, b):
        self.a = a
        self.b = b
    
    def getXYCoordFromRadians(self, rad):
        r = ((math.cos(rad)/self.a)**2 + (math.sin(rad)/self.b)**2)**(-0.5)

        return (r*math.cos(rad), r*math.sin(rad))

    def getYCoordsFromX(self, x):
        k = ((self.b**2) - (((x*b)/(a))**2))
        assert(k >= 0)
        v = k**.5
        return (v, -v)
    pass

if __name__ == "__main__":

    a = 5
    b = .5
    ellipse = Ellipse(a, b)

    # Normal Ellipse

    x = list(np.linspace(-a, a, 2*a*2))

    yPairsList = list(map(lambda x: ellipse.getYCoordsFromX(x), x))
    
    forListPos = list(map(lambda x,yPair: (x, yPair[0]), x, yPairsList))
    forListNeg = list(map(lambda x,yPair: (x, yPair[1]), x, yPairsList))

    forListNeg.reverse()

    pairsList = forListPos + forListNeg

    plotList = [None]*(len(pairsList)-1)
    
    for i in range(len(plotList)):
        plotList[i] = ((pairsList[i][0], pairsList[i+1][0]), (pairsList[i][1], pairsList[i+1][1]))


    xList = list(map(lambda x: x[0], plotList))
    yList = list(map(lambda x: x[1], plotList))
    cList = list(map(lambda x: cm.jet(x), np.linspace(0,1,len(plotList))))

    for i in range(len(plotList)):
        plt.plot(xList[i], yList[i], color=cList[i])

    plt.gcf().gca().set_aspect('equal')
    plt.gcf().gca().add_artist(plt.Circle([0,0], radius=.5, color=(0,0,1,.2)))

    plt.title(f"Ellipse, printed blue to red, {len(plotList)} segments")

    plt.show()

    # odonnell

    plt.figure()

    cListAdj = list(map(lambda x: 'b' if x < .5 else 'r', np.linspace(0,1,len(plotList))))

    for i in range(len(plotList)):
        plt.plot(xList[i], yList[i], color=cListAdj[i])

    plt.gcf().gca().set_aspect('equal')
    plt.gcf().gca().add_artist(plt.Circle([0,0], radius=.5, color=(0,0,1,.2)))

    plt.title(f"Ellipse, odonnell split, {len(plotList)} segments")

    plt.show()

    # desired

    plt.figure()

    cListAdj2 = list(map(lambda x: 'b' if x < .25 or x > .75 else 'r', np.linspace(0,1,len(plotList))))

    for i in range(len(plotList)):
        plt.plot(xList[i], yList[i], color=cListAdj2[i])
    plt.plot([.2,.2],[b,-b], color='g')

    plt.gcf().gca().set_aspect('equal')
    plt.gcf().gca().add_artist(plt.Circle([0,0], radius=.5, color=(0,0,1,.2)))

    plt.title(f"Ellipse, with added segments, {len(plotList)} segments")

    plt.show()

    # Interior Segments

    plt.figure()

    cnt = 0

    for i in range(len(pairsList)):
        for j in range(i+1, len(pairsList)):
            if abs(i-j) == 1:
                continue

            p1 = pairsList[i]
            p2 = pairsList[j]
            cnt += 1

            plt.plot([p1[0], p2[0]], [p1[1], p2[1]], color='r')

    for i in range(len(plotList)):
            plt.plot(xList[i], yList[i], color='b')

    plt.gcf().gca().set_aspect('equal')
    plt.gcf().gca().add_artist(plt.Circle([0,0], radius=.5, color=(0,0,1,.2)))

    plt.title(f"Ellipse, with added segments, {cnt} segments")

    plt.show()

    pass