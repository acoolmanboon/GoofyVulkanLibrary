/*
GoofyVulkanLibrary. A vulkan wrapper, designed to allow users to code Vulkan applications without high boilerplate.
Copyright (C) 2026 acoolmanboon

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <GFVL.hpp>
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

using namespace GFVL;
// USER-DEFINED STUFF
namespace GFVL {
KeyState InputState::getKeycodeState(Keycode keycode) {
  return keycodeStates.at(static_cast<size_t>(keycode));
}
bool InputState::isKeyDown(Keycode keycode) {
  return keycodeStates.at(static_cast<size_t>(keycode)).event == KeyEvent::Down;
}
bool InputState::isKeyUp(Keycode keycode) {
  if (keycodeStates.at(static_cast<size_t>(keycode)).event == KeyEvent::Up) {
    keycodeStates.at(static_cast<size_t>(keycode)).event = KeyEvent::None;
    return true;
  }
  return false;
}
bool InputState::isKeyRepeated(Keycode keycode) {
  return keycodeStates.at(static_cast<size_t>(keycode)).isRepeated;
}

MouseButtonState InputState::getMouseButtonState(MouseButton button) {
  return mouseButtonStates.at(static_cast<size_t>(button));
}
bool InputState::isMouseButtonDown(MouseButton button) {
  return mouseButtonStates.at(static_cast<size_t>(button)).event == KeyEvent::Down;
}
bool InputState::isMouseButtonUp(MouseButton button) {
  return mouseButtonStates.at(static_cast<size_t>(button)).event == KeyEvent::Up;
}
uint8_t InputState::getMouseButtonClicks(MouseButton button) {
  return mouseButtonStates.at(static_cast<size_t>(button)).clicks;
}

MouseState InputState::getMouseState() {
  return mouseState;
}
bool InputState::isMouseMoved() {
  return mouseState.moved;
}
bool InputState::isRunning() {
  return running;
}
bool InputState::framebufferResizedCallBack() {
  if (framebufferResized == true) {
    framebufferResized = false;
    return true;
  } else {
    return false;
  }
}

void InputState::pollInputs() {

  mouseState.xDelta = 0;
  mouseState.yDelta = 0;
  mouseState.moved = false;
  SDL_Event event;
  for (KeyState &state : keycodeStates) {
    state.isRepeated = true;
  }
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT)
      running = false;

    if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
      framebufferResized = true;

    if (event.type == SDL_EVENT_KEY_DOWN) {
      keycodeStates[static_cast<size_t>(event.key.scancode)] = {.event = KeyEvent::Down, .isRepeated = false};
    }

    if (event.type == SDL_EVENT_KEY_UP) {
      keycodeStates[static_cast<size_t>(event.key.scancode)] = {.event = KeyEvent::Up, .isRepeated = false};
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      mouseButtonStates[static_cast<size_t>(event.button.button)] = {.event = KeyEvent::Down, .clicks = event.button.clicks};
    }
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
      mouseButtonStates[static_cast<size_t>(event.button.button)] = {.event = KeyEvent::Up, .clicks = event.button.clicks};
    }

    if (event.type == SDL_EVENT_MOUSE_MOTION) {
      mouseState = {.x = event.motion.x, .y = event.motion.y, .xDelta = event.motion.xrel, .yDelta = event.motion.yrel, .moved = true};
    }
  }
}
} // namespace GFVL
