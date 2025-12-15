// inputController.js - Manages joystick and keyboard inputs

import { UI } from "./ui.js";

export function createInputController(onStateChange) {
  const joystickElement = UI.joystickContainer;
  const maxDiff = joystickElement.offsetWidth / 2;
  let isDragging = false;
  const DEADZONE_RADIUS = 0.1 * maxDiff; // Мертвая зона - 10% от радиуса

  // --- State ---
  const keyState = { w: false, a: false, s: false, d: false };
  const controlState = { throttle: 0, steer: 0 };
  const KEY_SPEED = 200;

  // --- Private Methods ---
  function notifyStateChange() {
    if (onStateChange) {
      onStateChange({ ...controlState });
    }
  }

  function updateFromKeys() {
    let keyThrottle = 0;
    let keySteer = 0;
    if (keyState.w) keyThrottle += KEY_SPEED;
    if (keyState.s) keyThrottle -= KEY_SPEED;
    if (keyState.a) keySteer -= KEY_SPEED;
    if (keyState.d) keySteer += KEY_SPEED;

    controlState.throttle = keyThrottle;
    controlState.steer = keySteer;
    notifyStateChange();
  }

  function handleJoystickMove(clientX, clientY) {
    if (!isDragging) return;
    const rect = joystickElement.getBoundingClientRect();
    let x = clientX - rect.left - joystickElement.offsetWidth / 2;
    let y = clientY - rect.top - joystickElement.offsetHeight / 2;

    const distance = Math.min(maxDiff, Math.sqrt(x * x + y * y));

    // Применяем мертвую зону
    if (distance < DEADZONE_RADIUS) {
      resetJoystick();
      return;
    }

    const angle = Math.atan2(y, x);

    x = distance * Math.cos(angle);
    y = distance * Math.sin(angle);

    controlState.throttle = (-y / maxDiff) * 255;
    controlState.steer = (x / maxDiff) * 255;

    UI.updateJoystickPosition(x, y);
    notifyStateChange();
  }

  function resetJoystick() {
    isDragging = false;
    // Reset only if no keys are pressed
    if (!Object.values(keyState).some((v) => v)) {
      controlState.throttle = 0;
      controlState.steer = 0;
      notifyStateChange();
    }
    UI.updateJoystickPosition(0, 0);
  }

  // --- Event Listeners ---
  joystickElement.addEventListener("mousedown", () => {
    isDragging = true;
  });
  document.addEventListener("mouseup", resetJoystick);
  document.addEventListener("mousemove", (e) =>
    handleJoystickMove(e.clientX, e.clientY)
  );

  joystickElement.addEventListener(
    "touchstart",
    (e) => {
      isDragging = true;
      e.preventDefault();
    },
    { passive: false }
  );
  document.addEventListener("touchend", resetJoystick);
  document.addEventListener(
    "touchmove",
    (e) => {
      if (e.touches.length > 0) {
        handleJoystickMove(e.touches[0].clientX, e.touches[0].clientY);
      }
      e.preventDefault();
    },
    { passive: false }
  );

  document.addEventListener("keydown", (e) => {
    const key = e.key.toLowerCase();
    if (key in keyState && !keyState[key]) {
      keyState[key] = true;
      updateFromKeys();
    }
  });
  document.addEventListener("keyup", (e) => {
    const key = e.key.toLowerCase();
    if (key in keyState) {
      keyState[key] = false;
      // If all keys are up, reset to 0 unless dragging joystick
      if (!isDragging) {
        updateFromKeys();
      }
    }
  });
}
