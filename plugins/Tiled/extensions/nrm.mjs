import { Helpers } from 'utils.mjs';

const BOOL_IS_RLE_COMPRESSED_DATA = 0x69
const BOOL_IS_UNCOMPRESSED_DATA = 0x6A
const MAGIC = 'PakkaByRCL^DPL2000'

function unpackImageDataFromBuffer(buffer, size) {
    let packedBufferPos = 0;
    let outDataBufferPos = 0;
    const dataView = new DataView(buffer);
    const data = new ArrayBuffer(size);
    const dataView1 = new DataView(data);
                       
    while (packedBufferPos < buffer.byteLength) {
        const controlByte = dataView.getUint8(packedBufferPos, true);
        packedBufferPos += 1;

        // first byte is control one, where 105 means data is rle compressed
        switch (controlByte) {
            case BOOL_IS_RLE_COMPRESSED_DATA:  
           
                let dataByte = dataView.getUint8(packedBufferPos, true); 
                const length = dataView.getUint16(packedBufferPos + 1, true);

                if (length) {
                    // copy byte 4 times to out buffer for length times

                    for (let i = 0; i < (length >> 2); ++i) { 
                        dataView1.setUint32(outDataBufferPos, Helpers.unpack8bitTo32Bit(dataByte) );
                        outDataBufferPos += 4;
                    }
                                                                
                    for (let i = 0; i < (length & 3); ++i) {
                        dataView1.setUint8(outDataBufferPos, dataByte);
                         
                       outDataBufferPos += 1;
                    }                              
                };

                packedBufferPos += 3; 
                break;                
            case BOOL_IS_UNCOMPRESSED_DATA:
                const db = dataView.getUint8(packedBufferPos);            
                dataView1.setUint8(outDataBufferPos, db);
                outDataBufferPos += 1;
                packedBufferPos += 1;
                break;
            default:              
                dataView1.setUint8(outDataBufferPos, controlByte);
                outDataBufferPos += 1;
        }
    }    
    
    return data;
};

class NRM8bitImage {
    constructor(size = 0, unpackedSize = 0, filepath = "", offset = 0) {
        this.size = size;
        this.unpackedSize = unpackedSize;
        this.filepath = filepath;
        this.offset = offset;
        this.data = null;
    }
    
    getHeader() {
        let dataView = new DataView(this.data);
        const bfOffBits = dataView.getUint32(10, IS_LITTLE_ENDIAN); 
        
        return this.data.slice(0, bfOffBits);
    }   
    getImageDimensions() {
        const dataView = new DataView(this.data);
        const imageWidth = dataView.getUint32(18, IS_LITTLE_ENDIAN);
        const imageHeight = dataView.getUint32(22, IS_LITTLE_ENDIAN);
        return {width: imageWidth, height: imageHeight};
    }  
}

class NRMFileImageParser {
    constructor(filename) {
        this.bmpImages = [];
        this.filename = filename;  
        this.file = new BinaryFile(filename, BinaryFile.ReadOnly);  
        this.filebuffer = null;        
    }
    
    destructor() {  
        this.file.close();
    }
    
    _getImageDataFromBuffer(img) {
        const buffer = this.filebuffer.slice(img.offset, img.offset + img.size);        
        return unpackImageDataFromBuffer(buffer, img.unpackedSize);
    }

    getFilenames() {
        let imageNames = []
        
        for (let img of this.bmpImages) {
            imageNames.push(img.filepath);
        }
        
        return imageNames;
    }
    
    getFilenamesByFilePath(path, relative = true) {
        let files = [];
        let f;
        const filenames = this.getFilenames();
        for (let  [indx, filename] of filenames.entries())                    
            if (filename.includes(path)) {
                if (relative) {
                    const offs = filename.indexOf(path) + path.length; 
                    f = filename.substring(offs + 1, filename.length);
                } else                    
                    f = filename; 
                
                files.push({name: f, index: indx, spriteIndex: 0});
            }

        return files;          
    }
    
    getImageByIndex(index) {
        const img = this.bmpImages[index];
        img.data = this._getImageDataFromBuffer(img);
        return img;
    }     
    
    getImageByName(name, includes = true) {
        for (let img of this.bmpImages) {
            if (includes) {
                if (img.filepath.includes(name)) {
                    img.data = this._getImageDataFromBuffer(img);
                    return img;
                }                    
            } else {
                const filename = Helpers.getFilenameFromPath(img.filepath);
                if (filename === name) {
                    img.data = this._getImageDataFromBuffer(img);
                    return img;     
                }                       
            }
        }
        
        return null;
    } 

    getFilteredImagesByFilePath(dir) {      
        let files = [];
        const filenames = this.getFilenames();
        for (let filename of filenames)                    
            if (filename.includes(dir))     
                files.push(this.getImageByIndex(filenames.indexOf(filename)));
            

        return files;            
    }
    
    getRootDirs(dir) {
        let paths = this.getFilenames();
        
        return Helpers.getFoldersFromRelativePaths(dir, paths)
    }    

    *getImages() {
        for (let img of this.bmpImages) {
            img.data = this._getImageDataFromBuffer(img);     
            yield img;
        }        
    }    
    
    read() { 
        this.filebuffer = this.file.readAll();
        let dataView = new DataView(this.filebuffer);
        
        let pos = 18; // skip header

        while (pos < this.filebuffer.byteLength) {
            pos += 5; // skip fourcc
            let size = dataView.getUint32(pos, true);
            const unpackedSize = dataView.getUint32(pos + 4, true);
            const strlen = dataView.getUint32(pos + 8, true);  
            
            if (!size) {
                pos += 12;
                continue;
            }
        
            let headerEndPos = pos + 12 + strlen;
        
            const filepath = this.filebuffer.slice(pos + 12, pos + 12 + strlen).toString();
            
            if (filepath.includes("bmp") || filepath.includes("msc")) {
                const img = new NRM8bitImage(size - strlen, unpackedSize, filepath, headerEndPos);
                this.bmpImages.push(img);
               
            }
            pos += 12 + size;
        }                    
    }
}

export default NRM8bitImage;
export default NRMFileImageParser;