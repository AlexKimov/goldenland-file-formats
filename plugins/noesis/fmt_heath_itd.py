IMAGE_WIDTH = 83
IMAGE_HEIGHT = 85
PALLETE_SIZE = 256
PIXEL_LENGTH = 4


from inc_noesis import *


def registerNoesisTypes():
    handle = noesis.register("Heath The unchosen path (2001) inventory item images", ".itd")
    noesis.setHandlerLoadRGBA(handle, itdLoadRGBA)
    noesis.setHandlerTypeCheck(handle, itdCheckType)
    
    return 1


class ITDImage:
    def __init__(self, name = ""):
        self.palette = None
        self.data = None
        self.width = IMAGE_WIDTH
        self.height = IMAGE_HEIGHT       
        self.filename = name
        
    def read(self, reader):
        self.palette = reader.readBytes(PALLETE_SIZE * PIXEL_LENGTH)
        size = reader.readUShort()
        colorIndex = reader.readUByte()
        self.data = reader.readBytes(size)    
              
        
class ITDArchive:
    def __init__(self, reader, filename):
        self.reader = reader   
        self.name = filename 
        
        self.table = {
           "amulets.itd": (56, 1),
           "axes.itd": (56, 0),
           "bows.itd": (56, 0),
           "elbarms.itd": (48, 0),
           "helmets.itd": (56, 0),
           "hits.itd": (56, 0),
           "panoplys.itd": (56, 0),
           "potions.itd": (56, 1),
           "protectors.itd": (56, 0),
           "quests.itd": (48, 1),
           "rolls.itd": (64, 1),
           "swords.itd": (64, 0)
        }                
            
    def getImages(self):
        while not self.reader.checkEOF():
            self.skippedBytesCount = self.table[self.name][0] | 0        
            self.reader.seek(self.skippedBytesCount, NOESEEK_REL)
            
            length = self.reader.readUShort()
            name =  noeAsciiFromBytes(self.reader.readBytes(length))   
            
            if self.table[self.name][1]:
                length = self.reader.readUShort()
                self.reader.seek(length * 16, NOESEEK_REL) 
            image = ITDImage(name)       
            image.read(self.reader)

            yield image
  
  
def itdCheckType(data):
    #
    return 1
    
    
def itdLoadRGBA(data, texList):
    #noesis.logPopup()
        
    itd = ITDArchive(NoeBitStream(data), os.path.basename(rapi.getInputName()))       
    for image in itd.getImages():    
        imageData = rapi.imageDecodeRawPal(image.data, image.palette, image.width, \
            image.height, 8, "a8r8g8b8")   
            
        for i in range(0, image.width*image.height):
            imageData[i*4 + 3] = 255
        
        texList.append(NoeTexture(image.filename, image.width, image.height, imageData, noesis.NOESISTEX_RGBA32)) 
            
        
    return 1

