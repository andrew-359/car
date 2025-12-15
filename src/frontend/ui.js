// ui.js - Handles all DOM interactions

const statusEl = document.getElementById("status");
const joystickContainer = document.getElementById("joystick-container");
const joystickHandle = document.getElementById("joystick-handle");

function updateStatus(text, color) {
  statusEl.textContent = text;
  if (color) {
    statusEl.style.color = color;
  }
}

function updateJoystickPosition(x, y) {
  joystickHandle.style.transform = `translate(${x}px, ${y}px)`;
}

export const UI = {
  joystickContainer,
  updateStatus,
  updateJoystickPosition,
};
