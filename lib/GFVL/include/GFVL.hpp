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
/**
 * @file GFVL.hpp
 * @brief Exposes GFVL.
 * @details This is probably the file you wanna include.
 */
#ifndef GFVL_CPP
#define GFVL_CPP
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "../lib/GFVL_core.hpp"
#include "../lib/GFVL_definition.hpp"
namespace GFVL {

/**
 * @class Mesh
 * @brief Represents a single renderable object.
 * @details Meshes contain all of the data required to draw a single object. A mesh is not an object, it is simply used for graphical rendering, so you can render the same mesh over and over again.
 */
class Mesh { 
public:
  /**
   * @struct CreateInfo
   * @brief Configuration for creating Mesh class.
   */
  struct CreateInfo {
    size_t size;                                                                       ///< Size of the vertice data in bytes
    void *data;                                                                        ///< Pointer to the raw mesh data.
    VertexBuffer::MemoryAllocation memoryAllocation = VertexBuffer::MemoryAllocation::HostVisible; ///< How this mesh will be allocated in memory
  };

  size_t size() const noexcept;
  VertexBuffer::MemoryAllocation memoryAllocation() const noexcept;

  /**
   * @brief Creates a mesh buffer.
   * @param device A reference to your Device.
   * @param createinfo Mesh creation info.
   */
  Mesh(DEVICE &device, const CreateInfo &createInfo); ///< Creates a mesh.

  Mesh(const Mesh &other) = delete;            ///< Meshes may not be copied since meshes cannot share the same Vulkan objects. It is recommended to just recreate a mesh with the same vertex data.
  Mesh &operator=(const Mesh &other) = delete; ///< Meshes may not be copied since meshes cannot share the same Vulkan objects. It is recommended to just recreate a mesh with the same vertex data.

  Mesh(Mesh &&other) = default;  ///< Move constructor, allowed but it will destroy the other object.
  Mesh &operator=(Mesh &&other); ///< Move assignment operator, allowed but it will destroy the other object.

  ~Mesh(); ///< Destroys mesh and associated info

  friend class INSTANCE;
private:
  const DEVICE &device_;            ///< Stores the device reference.
  VertexBuffer vertexBuffer_; ///< The buffer containing the actual memory
};

enum Keycode : uint32_t {
  UNKNOWN = 0,

  A = 4,
  B = 5,
  C = 6,
  D = 7,
  E = 8,
  F = 9,
  G = 10,
  H = 11,
  I = 12,
  J = 13,
  K = 14,
  L = 15,
  M = 16,
  N = 17,
  O = 18,
  P = 19,
  Q = 20,
  R = 21,
  S = 22,
  T = 23,
  U = 24,
  V = 25,
  W = 26,
  X = 27,
  Y = 28,
  Z = 29,

  Num_1 = 30,
  Num_2 = 31,
  Num_3 = 32,
  Num_4 = 33,
  Num_5 = 34,
  Num_6 = 35,
  Num_7 = 36,
  Num_8 = 37,
  Num_9 = 38,
  Num_0 = 39,

  RETURN = 40,
  ESCAPE = 41,
  BACKSPACE = 42,
  TAB = 43,
  SPACE = 44,

  MINUS = 45,
  EQUALS = 46,
  LEFTBRACKET = 47,
  RIGHTBRACKET = 48,
  BACKSLASH = 49,
  NONUSHASH = 50,
  SEMICOLON = 51,
  APOSTROPHE = 52,
  GRAVE = 53,
  COMMA = 54,
  PERIOD = 55,
  SLASH = 56,

  CAPSLOCK = 57,

  F1 = 58,
  F2 = 59,
  F3 = 60,
  F4 = 61,
  F5 = 62,
  F6 = 63,
  F7 = 64,
  F8 = 65,
  F9 = 66,
  F10 = 67,
  F11 = 68,
  F12 = 69,

  PRINTSCREEN = 70,
  SCROLLLOCK = 71,
  PAUSE = 72,
  INSERT = 73,
  HOME = 74,
  PAGEUP = 75,
  DELETE = 76,
  END = 77,
  PAGEDOWN = 78,
  RIGHT = 79,
  LEFT = 80,
  DOWN = 81,
  UP = 82,

  NUMLOCKCLEAR = 83,
  KP_DIVIDE = 84,
  KP_MULTIPLY = 85,
  KP_MINUS = 86,
  KP_PLUS = 87,
  KP_ENTER = 88,
  KP_1 = 89,
  KP_2 = 90,
  KP_3 = 91,
  KP_4 = 92,
  KP_5 = 93,
  KP_6 = 94,
  KP_7 = 95,
  KP_8 = 96,
  KP_9 = 97,
  KP_0 = 98,
  KP_PERIOD = 99,

  NONUSBACKSLASH = 100,
  APPLICATION = 101,
  POWER = 102,
  KP_EQUALS = 103,
  F13 = 104,
  F14 = 105,
  F15 = 106,
  F16 = 107,
  F17 = 108,
  F18 = 109,
  F19 = 110,
  F20 = 111,
  F21 = 112,
  F22 = 113,
  F23 = 114,
  F24 = 115,
  EXECUTE = 116,
  HELP = 117, /**< AL Integrated Help Center */
  MENU = 118, /**< Menu (show menu) */
  SELECT = 119,
  STOP = 120,  /**< AC Stop */
  AGAIN = 121, /**< AC Redo/Repeat */
  UNDO = 122,  /**< AC Undo */
  CUT = 123,   /**< AC Cut */
  COPY = 124,  /**< AC Copy */
  PASTE = 125, /**< AC Paste */
  FIND = 126,  /**< AC Find */
  MUTE = 127,
  VOLUMEUP = 128,
  VOLUMEDOWN = 129,
  KP_COMMA = 133,
  KP_EQUALSAS400 = 134,
  INTERNATIONAL1 = 135,
  INTERNATIONAL2 = 136,
  INTERNATIONAL3 = 137, /**< Yen */
  INTERNATIONAL4 = 138,
  INTERNATIONAL5 = 139,
  INTERNATIONAL6 = 140,
  INTERNATIONAL7 = 141,
  INTERNATIONAL8 = 142,
  INTERNATIONAL9 = 143,
  LANG1 = 144, /**< Hangul/English toggle */
  LANG2 = 145, /**< Hanja conversion */
  LANG3 = 146, /**< Katakana */
  LANG4 = 147, /**< Hiragana */
  LANG5 = 148, /**< Zenkaku/Hankaku */
  LANG6 = 149, /**< reserved */
  LANG7 = 150, /**< reserved */
  LANG8 = 151, /**< reserved */
  LANG9 = 152, /**< reserved */

  ALTERASE = 153, /**< Erase-Eaze */
  SYSREQ = 154,
  CANCEL = 155, /**< AC Cancel */
  CLEAR = 156,
  PRIOR = 157,
  RETURN2 = 158,
  SEPARATOR = 159,
  OUT = 160,
  OPER = 161,
  CLEARAGAIN = 162,
  CRSEL = 163,
  EXSEL = 164,

  KP_00 = 176,
  KP_000 = 177,
  THOUSANDSSEPARATOR = 178,
  DECIMALSEPARATOR = 179,
  CURRENCYUNIT = 180,
  CURRENCYSUBUNIT = 181,
  KP_LEFTPAREN = 182,
  KP_RIGHTPAREN = 183,
  KP_LEFTBRACE = 184,
  KP_RIGHTBRACE = 185,
  KP_TAB = 186,
  KP_BACKSPACE = 187,
  KP_A = 188,
  KP_B = 189,
  KP_C = 190,
  KP_D = 191,
  KP_E = 192,
  KP_F = 193,
  KP_XOR = 194,
  KP_POWER = 195,
  KP_PERCENT = 196,
  KP_LESS = 197,
  KP_GREATER = 198,
  KP_AMPERSAND = 199,
  KP_DBLAMPERSAND = 200,
  KP_VERTICALBAR = 201,
  KP_DBLVERTICALBAR = 202,
  KP_COLON = 203,
  KP_HASH = 204,
  KP_SPACE = 205,
  KP_AT = 206,
  KP_EXCLAM = 207,
  KP_MEMSTORE = 208,
  KP_MEMRECALL = 209,
  KP_MEMCLEAR = 210,
  KP_MEMADD = 211,
  KP_MEMSUBTRACT = 212,
  KP_MEMMULTIPLY = 213,
  KP_MEMDIVIDE = 214,
  KP_PLUSMINUS = 215,
  KP_CLEAR = 216,
  KP_CLEARENTRY = 217,
  KP_BINARY = 218,
  KP_OCTAL = 219,
  KP_DECIMAL = 220,
  KP_HEXADECIMAL = 221,

  LCTRL = 224,
  LSHIFT = 225,
  LALT = 226, /**< alt, option */
  LGUI = 227, /**< windows, command (apple), meta */
  RCTRL = 228,
  RSHIFT = 229,
  RALT = 230, /**< alt gr, option */
  RGUI = 231, /**< windows, command (apple), meta */
  MODE = 257,
  SLEEP = 258, /**< Sleep */
  WAKE = 259,  /**< Wake */

  CHANNEL_INCREMENT = 260, /**< Channel Increment */
  CHANNEL_DECREMENT = 261, /**< Channel Decrement */

  MEDIA_PLAY = 262,           /**< Play */
  MEDIA_PAUSE = 263,          /**< Pause */
  MEDIA_RECORD = 264,         /**< Record */
  MEDIA_FAST_FORWARD = 265,   /**< Fast Forward */
  MEDIA_REWIND = 266,         /**< Rewind */
  MEDIA_NEXT_TRACK = 267,     /**< Next Track */
  MEDIA_PREVIOUS_TRACK = 268, /**< Previous Track */
  MEDIA_STOP = 269,           /**< Stop */
  MEDIA_EJECT = 270,          /**< Eject */
  MEDIA_PLAY_PAUSE = 271,     /**< Play / Pause */
  MEDIA_SELECT = 272,         /* Media Select */

  AC_NEW = 273,        /**< AC New */
  AC_OPEN = 274,       /**< AC Open */
  AC_CLOSE = 275,      /**< AC Close */
  AC_EXIT = 276,       /**< AC Exit */
  AC_SAVE = 277,       /**< AC Save */
  AC_PRINT = 278,      /**< AC Print */
  AC_PROPERTIES = 279, /**< AC Properties */

  AC_SEARCH = 280,    /**< AC Search */
  AC_HOME = 281,      /**< AC Home */
  AC_BACK = 282,      /**< AC Back */
  AC_FORWARD = 283,   /**< AC Forward */
  AC_STOP = 284,      /**< AC Stop */
  AC_REFRESH = 285,   /**< AC Refresh */
  AC_BOOKMARKS = 286, /**< AC Bookmarks */

  SOFTLEFT = 287,
  SOFTRIGHT = 288,
  CALL = 289,
  ENDCALL = 290,

  RESERVED = 400, /**< 400-500 reserved for dynamic keycodes */

  COUNT = 512, /**< not a key, just marks the number of scancodes for array bounds */

};
enum MouseButton : uint8_t {
  Left = 0,
  Middle = 1,
  Right = 2,
  Thumb1 = 3,
  Thumb2 = 4,
  Count = 5
};
enum KeyEvent : uint8_t {
  None = 0,
  Down = 1,
  Up = 2,
};
struct MouseButtonState {
  KeyEvent event;
  uint8_t clicks;
};
struct MouseState {
  float x;
  float y;
  float xDelta;
  float yDelta;
  bool moved;
};

struct KeyState {
  KeyEvent event;
  bool isRepeated;
};

class InputState {
public:
  KeyState getKeycodeState(Keycode keycode) {
    return this->keycodeStates.at(static_cast<size_t>(keycode));
  }
  bool isKeyDown(Keycode keycode) {
    return this->keycodeStates.at(static_cast<size_t>(keycode)).event == KeyEvent::Down;
  }
  bool isKeyUp(Keycode keycode) {
    if (this->keycodeStates.at(static_cast<size_t>(keycode)).event == KeyEvent::Up) {
      this->keycodeStates.at(static_cast<size_t>(keycode)).event = KeyEvent::None;
      return true;
    }
    return false;
  }
  bool isKeyRepeated(Keycode keycode) {
    return this->keycodeStates.at(static_cast<size_t>(keycode)).isRepeated;
  }

  MouseButtonState getMouseButtonState(MouseButton button) {
    return this->mouseButtonStates.at(static_cast<size_t>(button));
  }
  bool isMouseButtonDown(MouseButton button) {
    return this->mouseButtonStates.at(static_cast<size_t>(button)).event == KeyEvent::Down;
  }
  bool isMouseButtonUp(MouseButton button) {
    return this->mouseButtonStates.at(static_cast<size_t>(button)).event == KeyEvent::Up;
  }
  uint8_t getMouseButtonClicks(MouseButton button) {
    return this->mouseButtonStates.at(static_cast<size_t>(button)).clicks;
  }

  MouseState getMouseState() {
    return this->mouseState;
  }
  bool isMouseMoved() {
    return this->mouseState.moved;
  }

  InputState() : keycodeStates(Keycode::COUNT, KeyState{.event = KeyEvent::None, .isRepeated = false}), mouseButtonStates(MouseButton::Count, MouseButtonState{.event = KeyEvent::None, .clicks = 0}) {

  }
  friend class INSTANCE;
private:
  MouseState mouseState;
  std::vector<KeyState> keycodeStates;
  std::vector<MouseButtonState> mouseButtonStates;
};

class INSTANCE {
private:
  VkInstance instance;

public:
  SDL_Window *window;

private:
  VkSurfaceKHR surface;

public:
  DEVICE device;

private:
  Swapchain swapchain;
  RENDERPASS renderPass;

public:
  UNIFORM_BUFFER uniformBuffer;
  std::vector<SHADER> shaderStages;
  PIPELINE pipeline;

private:

  Framebuffer framebuffer;
  CommandPool commandPool;

  uint32_t currentFrame = 0;
  uint32_t maxFramesInFlight;

  std::vector<GFVL::Semaphore> imageAvailableSemaphore;
  std::vector<GFVL::Semaphore> renderFinishedSemaphore;
  std::vector<GFVL::Fence> inFlightFence;
  std::vector<VkFence> imagesInFlightFence;

public:
  std::vector<GFVL::Mesh> meshesToRender;

  bool running = true;
  int w = 0;
  int h = 0;
  float aspectRatio = 0.0f;
  bool framebufferResized = false;

  VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
  InputState inputState;

  void pollInputs();
  void frame();

  INSTANCE(
      APPLICATION_INFO applicationInfo,
      VERTEX_LAYOUT &layout,
      std::vector<UNIFORM_BUFFER_BINDING> &bindings,
      std::vector<SHADER_STAGE> &stages);

  ~INSTANCE();

  INSTANCE(const INSTANCE &) = delete;
  INSTANCE &operator=(const INSTANCE &) = delete;

  INSTANCE(INSTANCE &&) = delete;
  INSTANCE &operator=(INSTANCE &&) = delete;
};

} // namespace GFVL

#endif