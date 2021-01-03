
from copy import deepcopy

class BitMap():
    def __init__(self, sizeInBits, startFull=False) -> None:
        self.bytes = (sizeInBits + (0 if sizeInBits % 8 == 0 else 8)) // 8
        self.byteArray = bytearray(self.bytes)
        self.size = sizeInBits

        if(startFull):
            for i in range(self.bytes):
                self.byteArray[i] = 255

    def __getitem__(self, key) -> int:
        if key < 0 or key >= self.size:
            raise KeyError(f"Illegal key {key}")
        b = key // 8
        r = key % 8

        v = self.byteArray[b]
        
        return (1 if ((v & (1 << r)) != 0) else 0)

    def __setitem__(self, key, value) -> None:
        if key < 0 or key >= self.size:
            raise KeyError(f"Illegal key {key}")

        if value not in [1,0]:
            raise ValueError(f"{value} ({type(value)}) could not be converted into 0,1")

        # for the conversion implied above
        #   this shouldn't be necessary?
        if value == 1:
            value = 1
        else:
            value = 0

        b = key // 8
        r = key % 8

        if value == 1:
            self.byteArray[b] |= (value << r)
        else:
            self.byteArray[b] &= (255 ^ (1 << r))

    def __len__(self) -> int:
        return self.size

    def __str__(self) -> str:
        return f"ByteArray[{self.size}]"

    def __eq__(self, other) -> bool:
        if len(self) != len(other):
            return False
        for i in range(self.bytes):
            if self.byteArray[i] != other.byteArray[i]:
                return False
        return True

    def isFull(self) -> bool:
        """Return True iff this all maps are set"""
        # TODO - why won't this one work?
        # i = 0
        # while(i+8 <= self.size):
        #     if self.byteArray[i//8] != 255:
        #         return False
        #     i += 8
        # for j in range(i,self.size):
        #     if self[j] == 0:
        #         return False
        for i in range(self.size):
            if self[i] == 0:
                return False
        return True

    def getCount(self) -> int:
        """"Return the number of unset items in the bitmap"""
        count = 0
        for i in range(self.size):
            if self[i] == 1:
                count += 1
        return count

    def __hash__(self) -> int:
        # return a 32 bit integer that is the logical XOR of the 32 bit subcomponents
        # should this be memorized until next insert?
        # TODO - not the best hash function:
        #   really only checking the index%32 parity
        #   pretty easy to design collisions
        myHash = 0
        for i in range(0,self.bytes,4):
            subSeg = 0
            for offset in range(0, 4):
                if i+offset < self.bytes:
                    v = self.byteArray[i+offset]
                    subSeg |= (v << (8 * offset))
            myHash ^= subSeg
        return myHash



    def dump(self) -> str:
        l = len(self.byteArray)
        s = ""
        charSpacing = len(str(self.size))
        formatString = f"[{{: >{charSpacing}}}, {{: >{charSpacing}}}] "
        nextStart = 0
        for i in range(l):
            if(i % 8 == 0):
                # row starters
                s += formatString.format(nextStart,min(nextStart+63, self.size))
                nextStart += 64
            isLast = (i == (l-1))
            # TODO - put into list and join list is more efficient?
            s += format(self.byteArray[i], f'08b')[:][::-1][:(self.size % 8) if isLast else 8]
            s += ("" if isLast else " ")
            if((i+1) % 8 == 0) or isLast:
                # row enders
                s += '\n'
        return s

# TODO - formal unit testing

if __name__ == "__main__":
    b = BitMap(5)
    b[3] = 1
    print(b.dump())
    assert(b[0] == 0)
    assert(b[3] == 1)

    bb = BitMap(101)
    bb[100] = 1
    print(bb.dump())

    bbb = BitMap(101, True)
    # print(bbb.isFull())
    bbb[100] = 0
    t = bbb[100]
    assert(t == 0)
    bbb[63] = 0
    t = bbb[63]
    assert(t == 0)
    t = bbb[62]
    assert(t == 1)
    print(bbb.dump())

    assert(bbb.isFull() == False)
    assert(BitMap(200, True).isFull() == True)

    bbbb = deepcopy(bbb)

    assert(bbb == bbbb)
    assert(hash(bbb) == hash(bbbb))
    bbb[99] = 0
    bbbb[4] = 0
    assert(bbb != bbbb)
    # print(hash(bbb))
    # print(hash(bbbb)) 
    
    # print(bbb.dump())
    # print(bbbb.dump())
    
    assert(hash(bbb) != hash(bbbb))

   

    print("All test cases passed successfully")
