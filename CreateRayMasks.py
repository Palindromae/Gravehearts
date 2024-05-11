class vec3:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z
    
dim = vec3(4,4,4)

pos = vec3(1,1,1)
dir = vec3(1,1,1)

def getPosIndex(vec):
    index = vec.x
    index |= vec.y<<2
    index |= vec.z<<(4)
    return index
def setBit(x,y,z):
    index = x
    index |= y<<2
    index |= z<<(4)
    
    return 1 << index
    

def TestRayAxii(n, p_n, n_dir, n_max):
    if n_dir == 0:
        return p_n == n 
    
    if n_dir < 0:
        return 0<= n <= p_n
    return  p_n<= n < n_max


def CouldRayHit(x,y,z, pos, dir):
    return TestRayAxii(x,pos.x,dir.x,dim.x) and TestRayAxii(y,pos.y,dir.y,dim.y) and TestRayAxii(z,pos.z,dir.z,dim.z) 


def calculateHitMask(position, dir):
    mask = 0
    for x in range(dim.x):
        for y in range(dim.y):
            for z in range(dim.z):
                if not CouldRayHit(x,y,z,position,dir):
                    continue
                mask |= setBit(x,y,z)
    return mask


masks = []
for iz in range(dim.z):
    for iy in range(dim.y):
        for ix in range(dim.x):
            for izd in range(-1,2):
                for iyd in range(-1,2):
                    for ixd in range(-1,2):
                        mask = calculateHitMask(vec3(ix,iy,iz),vec3(ixd,iyd,izd))
                        masks.append(mask)

# print(maskEnd)
# print(bin(maskEnd))
# print(hex(maskEnd))


def GetPosDirIndex(pos,dir):
    index = getPosIndex(pos)

    dir.x += 1
    dir.y += 1
    dir.z += 1

    dindex = dir.x + dir.y* 3 + dir.z *3 * 3
    maxSize = 3*3*3

    return index*maxSize + dindex


with open('RayMasks.txt', 'w') as f:
    f.write(f"{len(masks)},\n")
    for line in masks:
        a = format(line, '016x') 
        f.write(f"0x{a}UL,\n")

# TESTING
tempPos = vec3(2,1,3)
tempDir = vec3(-1,-1,-1)

tempPosIndex =GetPosDirIndex(tempPos,tempDir)



#print(masks[tempPosIndex])

#1110111000000000111011100000000011101110000000001110111

value = 15 
hex_string = format(value, '016x') 
print(hex_string) 