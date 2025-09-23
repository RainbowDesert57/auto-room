const { contextBridge } = require('electron');
const fs = require('fs');
const path = require('path');

// Correct path relative to preload.js
const livePath = path.join(__dirname, 'liveTotal.json');

console.log("Preload loaded. JSON exists?", fs.existsSync(livePath));

contextBridge.exposeInMainWorld('api', {
  readLiveTotal: () => {
    try {
      const data = fs.readFileSync(livePath, 'utf8');
      return JSON.parse(data);
    } catch (err) {
      console.error('Failed to read JSON:', err);
      return null;
    }
  }
});

