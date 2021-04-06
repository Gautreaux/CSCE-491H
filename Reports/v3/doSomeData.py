
files = ["recomputeRes.0.csv", "recomputeRes.1.csv", "recomputeRes.3.csv"]

def fileGenerator(path):
    with open(path, 'r') as inFile:
        next(inFile)
        for line in inFile:
            t = line.split(",")
            if(t[1].strip() == "True" or t[2].strip() == "True"):
                continue
            
            name = t[0].split(".")[0].replace("'","").strip()
            yield list(map(int, t[5:]))


for path in files:
    newTimeTotalTotal = 0
    baseTimeTotalTotal = 0
    rawTimeTotalTotal = 0
    printTimeTotalTotal = 0
    for f in fileGenerator(path):
        newTimeTotalTotal += f[0]
        baseTimeTotalTotal += f[1]
        rawTimeTotalTotal += f[2]
        printTimeTotalTotal += f[3]

    print(newTimeTotalTotal)
    print(baseTimeTotalTotal)
    print(rawTimeTotalTotal)
    print(printTimeTotalTotal)

    print(rawTimeTotalTotal / printTimeTotalTotal)
    print(baseTimeTotalTotal / newTimeTotalTotal)
    print()
        