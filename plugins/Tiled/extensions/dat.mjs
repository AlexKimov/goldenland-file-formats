import { Helpers } from 'utils.mjs';
        
class DatFileParser {
    constructor(path, properties) {
        this.groups = null;
        this.reader = new TextFile(path);
        const filename = Helpers.getFilenameFromPath(path);
        
        this.properties = properties[filename];        
    }

    read() { 
        const lines = this.reader.readAll().split('\n');
        let i = 0;
        while (i < lines.length) {
            const line = lines[i];
            if (line !== "") {
                if (line.includes("nGroup")) {
                    const num = parseInt(Helpers.getValueFromStr(line));  
                    
                    this.groups = Array.from(Array(num), () => { 
                        return {}; 
                    })

                    i += 1; 
                } else {           
                    for (const property of this.properties.names) { 
                        const line = lines[i];
                        const value = Helpers.getValueFromStr(line);
                        const index = Helpers.getIndexFromStr(line);
                            
                        this.groups[index][property] = value;
                        i += 1;                          
                    } 
                                   
                }                
            } else
                i += 1;         
        }
           
    }
}



export default DatFileParser;