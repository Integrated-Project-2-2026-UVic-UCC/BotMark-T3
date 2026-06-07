document.addEventListener('DOMContentLoaded', () => {
    // DOM Elements
    const ipInput = document.getElementById('ip-input');
    const connectBtn = document.getElementById('connect-btn');
    const statusDot = document.getElementById('status-dot');
    const statusText = document.getElementById('status-text');
    
    const stopBtn = document.getElementById('stop-btn');
    const startBtn = document.getElementById('start-btn');
    const pauseResumeBtn = document.getElementById('pause-resume-btn');

    // State Variables
    let currentIP = localStorage.getItem('esp32_ip') || '';
    let isConnected = false;
    let isPaused = false;
    let checkStatusInterval = null;

    // Initialize UI
    if (currentIP) {
        ipInput.value = currentIP;
        startStatusCheck();
    }

    // Event Listeners
    connectBtn.addEventListener('click', () => {
        const ip = ipInput.value.trim();
        if (ip) {
            currentIP = ip;
            localStorage.setItem('esp32_ip', currentIP);
            startStatusCheck();
            // Optional: Give visual feedback on button
            connectBtn.textContent = 'Saved';
            setTimeout(() => { connectBtn.textContent = 'Set'; }, 1500);
        } else {
            alert('Please enter a valid IP address.');
        }
    });

    stopBtn.addEventListener('click', () => {
        sendCommand('stop');
    });

    startBtn.addEventListener('click', () => {
        sendCommand('start');
        isPaused = false;
        updatePauseResumeButton();
    });

    pauseResumeBtn.addEventListener('click', () => {
        if (isPaused) {
            sendCommand('resume');
            isPaused = false;
        } else {
            sendCommand('pause');
            isPaused = true;
        }
        updatePauseResumeButton();
    });

    // Helper Functions
    function updatePauseResumeButton() {
        if (isPaused) {
            pauseResumeBtn.textContent = 'RESUME';
            pauseResumeBtn.classList.remove('btn-pause');
            pauseResumeBtn.classList.add('btn-resume');
        } else {
            pauseResumeBtn.textContent = 'PAUSE';
            pauseResumeBtn.classList.remove('btn-resume');
            pauseResumeBtn.classList.add('btn-pause');
        }
    }

    function updateConnectionStatus(connected) {
        isConnected = connected;
        if (connected) {
            statusDot.className = 'dot connected';
            statusText.textContent = 'Connected';
            statusText.style.color = 'var(--text-primary)';
        } else {
            statusDot.className = 'dot disconnected';
            statusText.textContent = 'Disconnected';
            statusText.style.color = 'var(--text-secondary)';
        }
    }

    // Communication API
    async function sendCommand(command) {
        if (!currentIP) {
            alert('Please set the ESP32 IP address first.');
            return;
        }
        
        const url = `http://${currentIP}/${command}`;
        try {
            // We use mode: 'no-cors' if the ESP32 doesn't send CORS headers,
            // but to read the response or status properly, cors is needed.
            // Using a simple POST request. ESP32 should ideally handle it.
            const response = await fetch(url, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded'
                },
                // Add a small timeout using AbortController
                signal: AbortSignal.timeout(3000)
            });
            
            // If we get here and it didn't throw a network error, assume success
            console.log(`Command ${command} sent successfully.`);
            updateConnectionStatus(true);
        } catch (error) {
            console.error(`Failed to send command ${command}:`, error);
            updateConnectionStatus(false);
        }
    }

    function startStatusCheck() {
        if (checkStatusInterval) {
            clearInterval(checkStatusInterval);
        }
        // Check immediately
        checkStatus();
        // Then poll every 3 seconds
        checkStatusInterval = setInterval(checkStatus, 3000);
    }

    async function checkStatus() {
        if (!currentIP) return;
        const url = `http://${currentIP}/status`;
        try {
            const response = await fetch(url, {
                method: 'GET',
                signal: AbortSignal.timeout(2000)
            });
            if (response.ok) {
                updateConnectionStatus(true);
            } else {
                updateConnectionStatus(false);
            }
        } catch (error) {
            // Network error or timeout means disconnected
            updateConnectionStatus(false);
        }
    }
});
