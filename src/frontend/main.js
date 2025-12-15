// main.js - The application entry point

import { UI } from "./ui.js";
import { createInputController } from "./inputController.js";
import { WebSocketClient } from "./webSocketClient.js";

// --- Configuration ---
const SEND_INTERVAL_MS = 50; // Send commands 20 times per second

// --- Application State ---
let controlState = { throttle: 0, steer: 0 };

// --- Initialization ---
function main() {
  UI.updateStatus("Connecting...", "#FFA500");

  const wsClient = new WebSocketClient(`ws://${window.location.hostname}/ws`);

  wsClient.onOpen = () => {
    UI.updateStatus("Connected", "#4CAF50");
    startSendingCommands();
  };

  wsClient.onClose = () => {
    UI.updateStatus("Disconnected. Retrying...", "#F44336");
    stopSendingCommands();
  };

  wsClient.onError = () => {
    UI.updateStatus("Connection Error", "#F44336");
  };

  wsClient.onMessage = (message) => {
    if (message.type === "status") {
      UI.updateStatus(
        `State: ${message.value}`,
        message.value === "FAILSAFE" ? "#F44336" : "#4CAF50"
      );
    }
  };

  createInputController((newState) => {
    controlState = newState;
  });

  let sendInterval;

  function startSendingCommands() {
    if (sendInterval) clearInterval(sendInterval);
    sendInterval = setInterval(() => {
      const command = {
        t: Math.round(controlState.throttle),
        s: Math.round(controlState.steer),
      };
      wsClient.send(command);
    }, SEND_INTERVAL_MS);
  }

  function stopSendingCommands() {
    if (sendInterval) clearInterval(sendInterval);
    sendInterval = null;
  }
}

// --- Run the application ---
document.addEventListener("DOMContentLoaded", main);
