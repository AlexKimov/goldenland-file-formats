class DatFileParser {
    constructor(path) {
        this.groups = new Map();
        this.reader = new TextFile(path);        
    }

    read() { 
        let line = this.reader.readLine();
        if (line.includes("nGroup")) {
            let num = getValueFromStr(line);

        }
            
        for (let i = 0; i < num; i++) {
            line = this.reader.readLine();
            
            if (line.includes("name")) {
                let name = getValueFromStr(line);
            }
            
            line = this.reader.readLine();
            if (line.includes("size"))  {
                let size = getValueFromStr(line);
            }
            let index = getIndexFromStr(line);  
            
            this.groups.set(name.toLowerCase(), [index, size]);
        }           
    }
}

export default DatFileParser;