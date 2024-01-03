STR_MAGIC = "PakkaByRCL^DPL2000"
STR_MAGIC_LENGTH = 18
BOOL_IS_RLE_COMPRESSED_DATA = 105
BOOL_IS_UNCOMPRESSED_DATA = 106

from inc_noesis import *


def registerNoesisTypes():
    handle = noesis.register("Heath The Unchosen path game archive file", ".nrm")
    noesis.setHandlerExtractArc(handle, hthExtractNRMFile)
    
    return 1
        
        
class hthArchiveFileHeader():
    def __init__(self):
        self.filename = "" 
        self.offset = 0        
        self.size = 0
        self.unpackedSize = 0
        
    def read(self, filereader):
        filereader.seek(5, NOESEEK_REL)  #skip fourcc code
        
        self.size = filereader.readUInt()
        self.unpackedSize = filereader.readUInt()
        
        length = filereader.readUInt()  
        self.size -=  length       
        self.filename = noeAsciiFromBytes(filereader.readBytes(length))

        self.offset = filereader.tell()
        
  
  
class hthFile():    
    def __init__(self, reader):
        self.reader = reader
        self.header = None
        self.data = None
   
    def getUnpackedFileData(self):
        packedData = self.reader.readBytes(self.header.size)

        currentPos = 0
        currentPos2 = 0
    
        # unpacked data - output buffer
        self.data = bytearray()
        self.dataPos = 0
    
        while currentPos < self.header.size:
            controlByte = packedData[currentPos]
            byte = packedData[currentPos:currentPos + 1]  
            currentPos += 1
            
            # first byte control, 105 - rle compression
            if controlByte == BOOL_IS_RLE_COMPRESSED_DATA:
                barray = packedData[currentPos + 1: currentPos + 3]                    
                length = noeUnpack('H'*(len(barray)//2), barray)[0]
                
                if length != 0:
                    # repeat 4 same bytes for length number
                    value = bytearray([packedData[currentPos]] * 4)
                    
                    for i in range(length >> 2):
                        self.data += value
                        self.dataPos += 4
                    
                    # image alignment by 4                        
                    for i in range(length & 3):                               
                        self.data += packedData[currentPos:currentPos + 1] 
                        self.dataPos += 1

                currentPos += 3                                                      
                
            elif controlByte == BOOL_IS_UNCOMPRESSED_DATA: 
                self.data += packedData[currentPos:currentPos + 1]
                self.dataPos += 1
                currentPos += 1                   
                
            else: # copy raw data as is               
                self.data += byte
                self.dataPos += 1
   
    def readHeader(self):           
        self.header = hthArchiveFileHeader()
        self.header.read(self.reader)          
                 
    def read(self):
        self.readHeader()
        self.getUnpackedFileData()
        
  
class hthArchiveFile():  
    def __init__(self, filereader):
        self.reader = filereader
        self.magic = ""
    
    def readHeader(self):
        self.magic = noeAsciiFromBytes(self.reader.readBytes(STR_MAGIC_LENGTH))
        
        #if self.magic == STR_MAGIC:
        #print(self.magic)  

    def getUnpackedFiles(self):
        self.reader.seek(STR_MAGIC_LENGTH, NOESEEK_ABS)  #skip fourcc code
        
        while not self.reader.checkEOF():
            file = hthFile(self.reader)
            file.read()
                        
            yield file         
    
    def read(self):
        self.readHeader()
        
    
def hthExtractNRMFile(fileName, fileLen, justChecking):
    with open(fileName, "rb") as f:
        if justChecking: #it's valid
            return 1       

        hth = hthArchiveFile(NoeBitStream(f.read()))
        hth.read()

        for f in hth.getUnpackedFiles():              
            rapi.exportArchiveFile(f.header.filename, f.data)
                        
    return 1