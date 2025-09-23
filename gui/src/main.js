const { app, BrowserWindow } = require('electron');
const path = require('path');

function createWindow() {
  const win = new BrowserWindow({
    width: 800,
    height: 480,
    webPreferences: {
      nodeIntegration: true, // now Node works in renderer
      contextIsolation: false
    },
    fullscreen: true,
  });

  win.loadFile('index.html');
}

app.whenReady().then(createWindow);

