from testModuleTwo import doItemTwo

def doItem():
    print("ITEM-1")
    doItemTwo()

class CACHE_DATE:
    def __init__(self):
        #get last cached date:
        import sys
        cachePath = sys.modules[self.__module__].__cached__

        from os.path import getmtime

        modifiedTime = getmtime(cachePath)

        from time import time

        print(f"current time is {time()} and modified time is {modifiedTime}")

        print(self.__module__)
