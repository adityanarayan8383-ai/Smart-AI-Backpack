let port;
let reader;
let writer;

let registeredItems = [];
let isReading = false;

// Connect Arduino
async function connectArduino() {
    try {
        port = await navigator.serial.requestPort();
        await port.open({ baudRate: 9600 });

        writer = port.writable.getWriter();

        document.getElementById("connectionStatus").innerText = "Connected";

        readArduino();

    } catch (error) {
        alert("Arduino connection failed: " + error);
    }
}

// Read Arduino data
async function readArduino() {
    const decoder = new TextDecoder();

    reader = port.readable.getReader();
    isReading = true;

    let buffer = "";

    try {
        while (true) {
            const { value, done } = await reader.read();

            if (done) break;

            buffer += decoder.decode(value, { stream: true });

            let lines = buffer.split("\n");
            buffer = lines.pop();

            for (let line of lines) {
                handleArduinoData(line.trim());
            }
        }
    } catch (error) {
        console.log(error);
    }
}

// Send commands to Arduino
async function sendCommand(command) {
    if (!writer) {
        alert("Connect Arduino first!");
        return;
    }

    const encoder = new TextEncoder();

    await writer.write(encoder.encode(command + "\n"));
}

// Register item
function registerItem() {
    sendCommand("r");
}

// Check registered items
function checkItems() {
    sendCommand("c");
}

// Scan backpack
function scanItems() {
    sendCommand("s");
}

// Remove all items
function removeAllItems() {
    if (confirm("Are you sure you want to remove all registered items?")) {
        sendCommand("x");
    }
}

// Handle Arduino response
function handleArduinoData(data) {

    console.log(data);

    // Successful registration
    if (data.includes("REGISTERED SUCCESSFULLY!")) {

        let itemName = data.replace(" REGISTERED SUCCESSFULLY!", "").trim();

        if (!registeredItems.includes(itemName)) {
            registeredItems.push(itemName);
            updateItemList();
        }
    }

    // Registered items list
    if (data.startsWith("Item ") && data.includes(":")) {

        let itemName = data.split(":").slice(1).join(":").trim();

        if (itemName && !registeredItems.includes(itemName)) {
            registeredItems.push(itemName);
            updateItemList();
        }
    }

    // Remove all
    if (data.includes("All Items Removed!")) {
        registeredItems = [];
        updateItemList();
    }

    // Scan result
    if (data.includes("FOUND!")) {
        showMessage(data, "success");
    }

    if (data.includes("Item Not Registered!")) {
        showMessage(data, "error");
    }

    if (data.includes("Item Already Registered!")) {
        showMessage(data, "error");
    }

    if (data.includes("No Items Registered.")) {
        registeredItems = [];
        updateItemList();
    }

    // Unknown item registration
    if (data.includes("Enter Item Name:")) {
        let name = prompt("Enter the name of this item:");

        if (name && writer) {
            sendCommand(name);
        }
    }

    // Display Arduino messages
    showMessage(data);
}

// Update registered item count and list
function updateItemList() {

    const countElement = document.getElementById("registeredCount");

    if (countElement) {
        countElement.innerText = registeredItems.length;
    }

    const itemsGrid = document.getElementById("itemsGrid");

    if (!itemsGrid) return;

    itemsGrid.innerHTML = "";

    registeredItems.forEach((item, index) => {

        const card = document.createElement("div");

        card.className = "item-card found";

        card.innerHTML = `
            <h3>${index + 1}. ${escapeHTML(item)}</h3>
            <p>RFID Registered</p>
            <span>Registered</span>
        `;

        itemsGrid.appendChild(card);
    });
}

// Display messages
function showMessage(message, type = "") {

    const output = document.getElementById("output");

    if (output) {
        output.innerText = message;
        output.className = type;
    }
}

// Prevent HTML injection in item names
function escapeHTML(text) {
    const div = document.createElement("div");
    div.textContent = text;
    return div.innerHTML;
}