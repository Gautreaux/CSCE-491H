
import sys 
from GraphicsGenerators.gcodeParser import gcodePathParser
from GraphicsGenerators.layerColorer import *

print("Hello")


if len(sys.argv) < 3:
    print(f"{sys.argv[0]} <mode> <name>")
    print(f"Mode: any value for layer colorer")
    exit(1)



parser = gcodePathParser()
parseResult = parser.parseFile(sys.argv[2])

parseResult.getReport()

print(f"Total print Len: {sum(map(lambda x: 0 if x is None else x, map(lambda x: x.printAmt, parseResult)))}")
layers = parseResult.getZLayers()
print(f"Z layers: {layers}")

name = sys.argv[2].split('/')[-1]

colorLayer(parseResult, name)
# colorChains(parseResult, name)