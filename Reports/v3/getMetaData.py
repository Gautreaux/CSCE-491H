import statistics as stat
import matplotlib.pyplot as plt
from numpy import ScalarType

files = [
"performanceStats.0.csv",
"performanceStats.1.csv",
"performanceStats.3.csv"
]

recomputeFiles = [
    "recomputeRes.0.csv",
    "recomputeRes.1.csv",
    "recomputeRes.3.csv"
]

def getLastLine(path):
    l = None
    with open(path, 'r') as inFile:
        for line in inFile:
            l = line
    return l.strip()

print("Seconds, qty_done, qty_remaining, mb_processed")

for f in files:
    print(getLastLine(f))

def getAllPerFiles(path):
    l = []
    with open(path, 'r') as inFile:
        next(inFile)
        for line in inFile:
            t = line.split(",")
            ll = []
            ll.append(t[0].split(".")[0].strip()[1:])
            ll.append(t[1].strip())
            ll.append(t[2].strip())

            if(ll[-1] == "True"):
                while(len(ll) < 9):
                    ll.append(None)
                l.append(ll)
                continue

            ll.append(float(t[3].strip()))
            ll.append(int(t[4].strip()))
            ll.append(int(t[5].strip()))
            ll.append(int(t[6].strip()))
            ll.append(int(t[7].strip()))
            ll.append(int(t[8].strip()))

            l.append(ll)
        return l

def getPrintEfficiency(f):
    return (f[-2]/f[-1])

def getBaseEfficiency(f):
    return (f[-3]/f[-4])

def getFileSizeMap(path):
    m = {}
    with open(path, 'r') as inFile:
        next(inFile)
        for line in inFile:
            n,v = line.replace(", "," ").split(" ")
            v = int(v)
            assert(n not in m)
            m[n] = v
    return m

allFiles = list(map(getAllPerFiles, recomputeFiles))

for f in allFiles:
    print("CPU Time (s): ", end="")
    print(round(sum(map(lambda x : x[3], filter(lambda x : x[2] == "False", f)))))

printEfficiencyList = list(map(lambda f : list(map(getPrintEfficiency, filter(lambda x : x[2] == 'False', f))), allFiles))
baseEfficiencyList  = list(map(lambda f : list(map(getBaseEfficiency,  filter(lambda x : x[2] == 'False', f))), allFiles))
printTimeList       = list(map(lambda f : list(map(lambda x : x[-1],   filter(lambda x : x[2] == 'False', f))), allFiles))
rawTimeList         = list(map(lambda f : list(map(lambda x : x[-2],   filter(lambda x : x[2] == 'False', f))), allFiles))
baseTimeList        = list(map(lambda f : list(map(lambda x : x[-3],   filter(lambda x : x[2] == 'False', f))), allFiles))
newBaseTimeList     = list(map(lambda f : list(map(lambda x : x[-4],   filter(lambda x : x[2] == 'False', f))), allFiles))

assert(rawTimeList[0] == rawTimeList[1])
assert(rawTimeList[0] == rawTimeList[2])
assert(baseTimeList[0] == baseTimeList[1])
assert(baseTimeList[0] == baseTimeList[2])

for f in printEfficiencyList:
    print("AVG Print Efficiency: ", end="")
    print(stat.mean(f))

for i in range(3):
    print("MAX Print Efficiency: ", end="")
    m = (max(printEfficiencyList[i]))
    print(m, end=" ")
    print(list(map(lambda x : x[0], filter(lambda x : x[2] == "False" and getPrintEfficiency(x) == m, allFiles[i]))))

for f in printEfficiencyList:
    print("STDDEV print Efficiency: ", end="")
    print(stat.stdev(f))

for f in baseEfficiencyList:
    print("AVG Base Efficiency: ", end="")
    print(stat.mean(f))

for i in range(3):
    print("MAX Base Efficiency: ", end="")
    m = (max(baseEfficiencyList[i]))
    print(list(map(lambda x : x[0], filter(lambda x : x[2] == "False" and getBaseEfficiency(x) == m, allFiles[i]))), end=" ")
    print(m)

for f in baseEfficiencyList:
    print("STDDEV base Efficiency: ", end ="")
    print(stat.stdev(f))

names = ["Theoretical Model", "CODEX Model", "Current Model"]

for p,w in zip(printEfficiencyList, rawTimeList):
    n = sum([x*y for x,y in zip(p,w)])
    d = sum(w)
    print("Print Efficiency weighted by raw Print Time: ", end= "")
    print(n/d)

for p,w in zip(baseEfficiencyList, baseTimeList):
    n = sum([x*y for x,y in zip(p,w)])
    d = sum(w)
    print("Base efficiency weighted raw base time: ", end = "")
    print(n/d)

for p,b in zip(rawTimeList, baseTimeList):
    print("Time % printing in raw file: ", end="")
    print(stat.mean(map(lambda x : x[0] / x[1], zip(p,b))))
    print(sum(p) / sum(b))

for p,b in zip(printTimeList, newBaseTimeList):
    print("Time % printing in new file: ", end="")
    print(stat.mean(map(lambda x : x[0] / x[1], zip(p,b))))
    print(sum(p) / sum(b))

print("Skipping plot generation")

exit(0)

for i in range(3):
    plt.figure()
    plt.scatter(rawTimeList[i], printEfficiencyList[i])
    plt.title(f"Print Efficiency vs Raw Print Time, {names[i]}")
    plt.xlabel("Raw Print Time")
    plt.ylabel("Print Efficiency")
    plt.gca().ticklabel_format(axis='both', style='plain')
    plt.show()

for i in range(3):
    plt.figure()
    plt.scatter(baseTimeList[i], baseEfficiencyList[i])
    plt.title(f"Base Efficiency vs Raw Base Time, {names[i]}")
    plt.xlabel("Raw Base Time")
    plt.ylabel("Base Efficiency")
    plt.gca().ticklabel_format(axis='both', style='plain')
    plt.show()

sizesMap = getFileSizeMap("fileSizes.csv")

sizesList = list(map(lambda x : sizesMap[x[0]], filter(lambda x : x[2] == "False", allFiles[0])))

plt.figure()
plt.hist(list(map(lambda x : x / (1024), sizesList)))
plt.title("Histogram of file sizes (KB)")
plt.yscale("log")
plt.ylabel("Quantity")
plt.xlabel("Size (KB)")
plt.ticklabel_format(axis='x', style='plain')
plt.show()

plt.figure()
plt.hist(baseTimeList[0])
plt.title("Histogram of base time")
plt.ticklabel_format(axis="both", style="plain")
plt.yscale("log")
plt.xlabel("Base Time")
plt.ylabel("Quantity")
plt.show()