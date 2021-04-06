


filesLists = [
    ["failedFiles.0.csv", "failedFiles.1.csv", "failedFiles.3.csv"],
    ["recomputeRes.0.csv", "recomputeRes.1.csv", "recomputeRes.3.csv"]
]

def processType1(path):
    with open(path, 'r') as inFile:
        next(inFile)
        for line in inFile:
            yield line.split(",")[0].split(".")[0].strip()

def processType2(path):
    with open(path, 'r') as inFile:
        next(inFile)
        for line in inFile:
            t = line.split(",")
            if(t[1].strip() == "True" or t[2].strip() == "True"):
                yield t[0].split(".")[0].replace("'","").strip()

generatorTypes = [processType1, processType2]

assert(len(generatorTypes) == len(filesLists))

sets = []

for i in range(len(filesLists)):
    for path in filesLists[i]:
        print(f"Starting: {path}")
        sets.append(set(generatorTypes[i](path)))

for i in range(len(sets)):
    for j in range(i+1, len(sets)):
        assert(len(sets[i].symmetric_difference(sets[j])) == 0)

print("Checks pased: errors consistent in all files")
