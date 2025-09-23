// script.js
const fs = require('fs');
const path = require('path');

// liveTotal.json is in the same folder as script.js
const livePath = path.join(__dirname, 'liveTotal.json');

function updateStats() {
    try {
        const data = fs.readFileSync(livePath, 'utf8');
        const stats = JSON.parse(data);

        document.getElementById('peopleCount').innerText = stats.refreshCount || 0;
        document.getElementById('voltage').innerText = "220 V";
        document.getElementById('energy').innerText = stats.energy;
        document.getElementById('current').innerText = stats.current*1000 + " mA";
        document.getElementById('power').innerText = stats.power + " W";
    } catch (err) {
        console.warn("Could not read liveTotal.json:", err);
    }
}

// initial call
updateStats();

// update every second
setInterval(updateStats, 1000);

