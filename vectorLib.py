# file for vector functions:
#this is certainly not the most efficient

#get the unit vector from start point to end point
def getUnitVector(startPoint, endPoint) -> tuple:
    return unitize(tuple(map(lambda s,e: e-s, startPoint, endPoint)))

def vectorMagnitude(vector):
    return sum(map(lambda x: x**2, vector))**.5

# unitize a n-dimensional vector to its unit representation
def unitize(vector) -> tuple:
    s = vectorMagnitude(vector)
    if(s == 0):
        return vector
    return tuple(map(lambda x: round(x/s, 8), vector))

def reverseVector(vector):
    return tuple(map(lambda x: -x, vector))

def vectorDot(vA, vB) -> float:
    return sum(map(lambda x,y: x*y, vA, vB))

def vectorCross(vA, vB) -> tuple:
    if(len(vA) == 3):
        return (
            vA[1]*vB[2]-vA[2]*vB[1],
            vA[2]*vB[0]-vA[0]*vB[2],
            vA[0]*vB[1]-vA[1]*vB[0]
        )
    raise NotImplementedError(f"Cross product for vector length {len(vA)} not implemented")

#supports vector + vector, vector + scalar
def vectorAdd(vA, vB):
    try:
        return tuple(map(lambda x,y: x+y, vA,vB))
    except TypeError:
        return tuple(map(lambda x: x+vB, vA))


#supports vector - vector, vector - scalar
def vectorSub(vA, vB):
    try:
        return tuple(map(lambda x,y: x-y, vA,vB))
    except TypeError:
        return tuple(map(lambda x: x-vB, vA))

#supports vector * scalar
#TODO URGENT - is this correct
#   its used in get projection point
def vectorMult(v, s):
    return tuple(map(lambda x: x-s, v))

#lol
def pointDistance(pA, pB):
    return vectorMagnitude(vectorSub(pA, pB))

#TODO - unit testing