export let Helpers = {};

Helpers.getFilenameFromPath = function (path) {
    return path.replace(/^.*[\\/]/, '');
}

Helpers.getFileExtension = function (fileName) {
    return fileName.substring(fileName.lastIndexOf('.') + 1);
}

Helpers.unpack8bitTo32Bit = function (value) {
    return (value | (value << 8) | (value << 16) | (value << 24));
}  

Helpers.splitLastOccurrence = function (str, substring) {
  const lastIndex = str.lastIndexOf(substring);
  
  let before = '';
  let after = '';
  
  if (lastIndex !== -1) {
    before = str.slice(0, lastIndex);
    after = str.slice(lastIndex + 1);
  }
  
  return [before, after];
}

Helpers.create2DArray = function (m, n) {
    return Array.from({ length: m }, () => Array.from({ length: n }, () => 0));
};

Helpers.getKey = function (val) {
  return [...map].find(([key, value]) => val === value)[0];
}

Helpers.getFoldersFromRelativePaths = function (rootDir, filePaths) {
    const folders = new Set();

    for (const filePath of filePaths) {
        if (filePath.startsWith(rootDir)) {                
            const path = filePath.split('\\');
            // const filepath = `${path[0]}\\${path[1]}`;
            const filepath = `${path[1]}`;
            folders.add(filepath);
        }
    }

    return Array.from(folders);
}

Helpers.hexToInteger = function (str) {
    return parseInt(str.replace('#', ''), 16);
}

Helpers.concatArrayBuffers = function (buffer1, buffer2) {
    let tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
    tmp.set(new Uint8Array(buffer1), 0);
    tmp.set(new Uint8Array(buffer2), buffer1.byteLength);
    return tmp.buffer;
};

Helpers.getValueFromStr = function (str, delimetr = ":", pos = 1) {
    return str.split(delimetr)[pos].trim();
}

Helpers.getIndexFromStr = function (str) {
    const [name, value] = str.split(":");
    return parseInt(name.split("_")[1].trim());
}