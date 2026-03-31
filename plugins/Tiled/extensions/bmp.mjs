export let BMP = {};

const IS_LITTLE_ENDIAN = true
const BMP_HEADER_SIZE = 138
const BMP_HEADER_SIZE1 = 54
const PERSON_IMAGE_SPRITE_FRAMES_NUM = 8;

function concatArrayBuffers (buffer1, buffer2) {
    let tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
    tmp.set(new Uint8Array(buffer1), 0);
    tmp.set(new Uint8Array(buffer2), buffer1.byteLength);
    return tmp.buffer;
};

function setBMPImageHeader(bmpBuffer, width, height, size) {
    const dataView = new DataView(bmpBuffer);
       
    dataView.setUint16(0, 19778, IS_LITTLE_ENDIAN); // magic
    dataView.setUint32(2, BMP_HEADER_SIZE + width * height * 4, IS_LITTLE_ENDIAN); // bfSize
    dataView.setUint32(6, 0, IS_LITTLE_ENDIAN); // bfReserved
    dataView.setUint32(10, BMP_HEADER_SIZE, IS_LITTLE_ENDIAN); // bfOffBits
        
    dataView.setUint32(14, 124, IS_LITTLE_ENDIAN); // bfSize 
    dataView.setUint32(18, width, IS_LITTLE_ENDIAN); // bfWidth
    dataView.setUint32(22, height, IS_LITTLE_ENDIAN); // bfHeight
    dataView.setUint16(26, 1, IS_LITTLE_ENDIAN); // biPlanes
    dataView.setUint16(28, 32, IS_LITTLE_ENDIAN); // biBitCount
    dataView.setUint32(30, 3, IS_LITTLE_ENDIAN); // biCompression
    
    dataView.setUint32(34, size); // biSizeImage
    dataView.setUint32(38, 2835, IS_LITTLE_ENDIAN); // biXPelsPerMeter
    dataView.setUint32(42, 2835, IS_LITTLE_ENDIAN); // biYPelsPerMeter
    dataView.setUint32(46, 0, IS_LITTLE_ENDIAN); // biCrlUsed
    dataView.setUint32(50, 0, IS_LITTLE_ENDIAN); // biClrImportant 

    //ColorTable
    dataView.setUint32(54, 0x0000ff00, IS_LITTLE_ENDIAN); // r     
    dataView.setUint32(58, 0x00ff0000, IS_LITTLE_ENDIAN); // g
    dataView.setUint32(62, 0xff000000, IS_LITTLE_ENDIAN); // b
    dataView.setUint32(66, 0x000000ff, IS_LITTLE_ENDIAN); // a  
}

BMP.convert8BitImageTo32Bit = function (imageData) {
    const dataView = new DataView(imageData);
    let biCrlUsed = dataView.getUint32(46, IS_LITTLE_ENDIAN);
    if (!biCrlUsed) biCrlUsed = 256;       
    const bfWidth = dataView.getUint32(18, IS_LITTLE_ENDIAN); 
    const bfHeight = dataView.getUint32(22, IS_LITTLE_ENDIAN); 

    const size = bfWidth * bfHeight;    
    
    let data = new ArrayBuffer(size * 4 + BMP_HEADER_SIZE);       
    const dataView2 = new DataView(data);
    
    setBMPImageHeader(data, bfWidth, bfHeight, size * 4); 
    
    let colors = [];
    let offset = BMP_HEADER_SIZE1;
    let offset2 = BMP_HEADER_SIZE;

    for (let i = 0; i < biCrlUsed; i++) {
        colors.push(dataView.getUint32(offset, true));
        offset += 4;
    }

    let padding = (4 - (bfWidth % 4)) % 4;
    let index = 0;
    let alpha = 0;
    
    const idx = dataView.getUint8(offset, true);
    const transparentColor = colors[idx];
    
    for (let i = 1; i <= size; i++) {
        index = dataView.getUint8(offset, true);
         
        let color = colors[index];        
        if (color === transparentColor)
            alpha = 0;
        else
            alpha = 0xFF;

        let pixel = (alpha) | (((colors[index] >> 16) & 0xFF) << 8) | (((colors[index] >> 8) & 0xFF) << 16) | (((colors[index]) & 0xFF) << 24)        
        dataView2.setUint32(offset2, pixel, IS_LITTLE_ENDIAN);

        if ((i % bfWidth) === 0) {
            offset += padding;             
        };
        
        offset2 += 4;        
        offset += 1;        
    }

    return data;
}

function concatBuffersToBuffer (buffers) {
    const totalLength = buffers.reduce((acc, buf) => acc + buf.byteLength, 0);

    const result = new Uint8Array(totalLength);

    let offset = 0;
    for (const buf of buffers) {
    result.set(new Uint8Array(buf), offset);
    offset += buf.byteLength;
    }

    return result.buffer; 
}

BMP.splitImage = function (data, spriteNum = 0) {
    let dataView = new DataView(data);
    const bfOffBits = dataView.getUint32(10, IS_LITTLE_ENDIAN); 
    const bfWidth = dataView.getUint32(18, IS_LITTLE_ENDIAN); 
    const bfHeight = dataView.getUint32(22, IS_LITTLE_ENDIAN);
    
    let offset = bfOffBits;
    let bmpHeader = data.slice(0, offset);
    
    let tileNum;
    let tiles = [];
   
    if  (spriteNum === 0) {
        spriteNum = PERSON_IMAGE_SPRITE_FRAMES_NUM;
        const width = bfWidth / spriteNum; 
        const height = bfHeight / spriteNum;
                  
        const rowLength = (bfWidth + (4 - (bfWidth % 4)) % 4);
        const rowsLength = rowLength * height;
        let tileOffset;
        
        const paddingSize = (4 - (width % 4)) % 4;
        const tileSize = (width + paddingSize) * height;            
        const tilePadding = new Uint8Array(paddingSize);
        
        dataView = new DataView(bmpHeader);
        dataView.setUint32(2, bfOffBits + tileSize, IS_LITTLE_ENDIAN);       
        dataView.setUint32(18, width, IS_LITTLE_ENDIAN);            
        dataView.setUint32(22, height, IS_LITTLE_ENDIAN);            
        dataView.setUint32(34, tileSize, IS_LITTLE_ENDIAN); 
        
        for (let k = 1; k <= spriteNum; k++) {
            const rows = data.slice(offset, offset + rowsLength);   
            
            tileOffset = offset;
            for (let z = 0; z < spriteNum; z++) {
                let dataBuffers = [bmpHeader];                    
                for (let i = 0; i < height; i++) {
                    const rowData = data.slice(tileOffset + i * rowLength, tileOffset + width + i * rowLength);
                    dataBuffers.push(rowData);
                    dataBuffers.push(tilePadding);
                }; 

                const tileData = concatBuffersToBuffer(dataBuffers);

                tiles.push(tileData);
                tileOffset += width;                         
            }
            
            offset += rowsLength;
        }         
    } else {
        const ImageHeight = bfHeight / spriteNum;
        
        const size = (bfWidth + (4 - (bfWidth % 4)) % 4) * ImageHeight;
               
        dataView = new DataView(bmpHeader);
        dataView.setUint32(2, bfOffBits + size, IS_LITTLE_ENDIAN);       
        dataView.setUint32(22, ImageHeight, IS_LITTLE_ENDIAN);            
        dataView.setUint32(34, size, IS_LITTLE_ENDIAN);            
        
        for (let i = 1; i <= spriteNum; i++) {       
            const tileData = data.slice(offset, offset + size);                    
            tiles.push(concatArrayBuffers(bmpHeader, tileData));
                
            offset += size;
        }                           
    }
    
    return tiles;     
} 
    
BMP.splitImageByRows = function (image) {
        const dataView = new DataView(image.data);
        const bfOffBits = dataView.getUint32(10, IS_LITTLE_ENDIAN);
        const bfWidth = dataView.getUint32(18, IS_LITTLE_ENDIAN);
        const bfHeight = dataView.getUint32(22, IS_LITTLE_ENDIAN);
        const rowSize = bfWidth * 4; // 4 байта на пиксель
        const rows = [];

        for (let i = 0; i < bfHeight; i++) {
            const offset = bfOffBits + i * rowSize;
            rows.push(image.data.slice(offset, offset + rowSize));
        }
        return rows;    
    }
