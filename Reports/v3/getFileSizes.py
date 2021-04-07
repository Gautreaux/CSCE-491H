import os

TLD = "/mnt/r/"

files = [f for f in os.listdir(TLD) if f[-len(".gcode"):] == ".gcode"]

print(f"Found {len(files)} files")

with open("fileSizes.csv", 'w') as outFile:
    outFile.write("name, file_size\n")
    for f in files:
        name = f.split('.')[0]
        size = os.stat(TLD + f).st_size
        outFile.write(f"{name}, {size}\n")