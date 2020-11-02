# do analysis on the layer

from fileIterables import allFilesInRecurseByType, getNameFromPath
from statistics import pstdev as stddev
from statistics import mean

Layer_DIR = "layerSampleSet"
LFILE_TYPE = "lyr"

class Stats:
    def __init__(self):
        self.segments = 0
        self.pointsMean = 0
        self.pointsDev = 0

statsList = []
#how many times is the most visited point visited?
mostVisitedPointQty = 0
mostVisitedPointPoint = (None, None)
mostVisitedPointFile = ""

for filePath in allFilesInRecurseByType(Layer_DIR, LFILE_TYPE):
    with open(filePath) as f:
        stats = Stats()

        segCounter = 0

        uniquePoints = 0
        pointsDict = {}

        for line in f:
            tokens = line.strip().split(" ")

            tokens_int = list(map(lambda x: round(float(x)*10000), tokens))

            for t in tokens_int:
                if t%10 != 0:
                    print(tokens)
                    raise ValueError(f"need more rounding precision: {t} {filePath}")

            p1 = (tokens_int[0], tokens_int[1])
            p2 = (tokens_int[2], tokens_int[3])

            for p in [p1,p2]:
                if p not in pointsDict:
                    pointsDict[p] = 1
                    uniquePoints += 1
                else:
                    pointsDict[p] += 1

            segCounter += 1

        for k in pointsDict:
            if(pointsDict[k] > mostVisitedPointQty):
                mostVisitedPointQty = pointsDict[k]
                mostVisitedPointFile = filePath
                mostVisitedPointPoint = k

        stats.segments = segCounter
        stats.pointsMean = mean(map(lambda x: pointsDict[x], pointsDict))
        stats.pointsDev = stddev(map(lambda x: pointsDict[x], pointsDict))
        statsList.append(stats)

segMean = mean(map(lambda x: x.segments, statsList))
segDev = stddev(map(lambda x: x.segments, statsList))
pointsMeanMean = mean(map(lambda x: x.pointsMean, statsList))
pointsMeanDev = stddev(map(lambda x: x.pointsMean, statsList))
pointsDevMean = mean(map(lambda x: x.pointsDev, statsList))
pointsDevDev = stddev(map(lambda x: x.pointsDev, statsList))

print(f"SM : {segMean}")
print(f"SD : {segDev}")
print(f"PMM: {pointsMeanMean}")
print(f"PMD: {pointsMeanDev}")
print(f"PDM: {pointsDevMean}")
print(f"PDD: {pointsDevDev}")
print("")
print(f"MVPQ: {mostVisitedPointQty}")
print(f"MVPP: {mostVisitedPointPoint}")
print(f"MVPF: {mostVisitedPointFile}")