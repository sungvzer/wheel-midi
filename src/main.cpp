#include "rtmidi/RtMidi.h"
#include <SDL2/SDL.h>
#include <SDL_joystick.h>
#include <SDL_timer.h>
#include <chrono>
#include <cstring>
#include <iostream>

const int G29_PEDAL_ACCEL = 2; // Accelerate pedal axis
// const int G29_PEDAL_BRAKE = 3;  // Brake pedal axis
// const int G29_PEDAL_CLUTCH = 1; // Clutch pedal axis (if supported)

std::vector<unsigned char> create_midi_message(unsigned char pressure) {
  std::vector<unsigned char> message;
  message.push_back(0xB0); // Control Change
  message.push_back(0x04); // Accelerator pedal
  message.push_back(pressure);
  return message;
}

unsigned char get_pedal_midi_value(SDL_Joystick *joystick, int pedal) {
  int value = SDL_JoystickGetAxis(joystick, pedal);
  return (1 - (value + 32768) / 65535.0) * 127;
}

int main() {
  RtMidiOut *midiout = new RtMidiOut();
  SDL_Joystick *joystick;
  bool quit = false;
  SDL_Event e;

  try {
    midiout->openPort();
  } catch (RtMidiError &error) {
    error.printMessage();
    exit(EXIT_FAILURE);
  }

  if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    return -1;
  }

  // Check if any joysticks are connected
  if (SDL_NumJoysticks() < 1) {
    std::cerr << "No joysticks connected!" << std::endl;
    SDL_Quit();
    return -1;
  }

  joystick = SDL_JoystickOpen(0);
  // Open the first joystick
  if (joystick == nullptr) {
    std::cerr << "Unable to open joystick! SDL Error: " << SDL_GetError()
              << std::endl;
    SDL_Quit();
    return -1;
  }

  int last_accel_pressure = 0;

  while (!quit) {
    auto start = std::chrono::high_resolution_clock::now();
    // Poll for events (such as quitting)
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    double accel_pressure = get_pedal_midi_value(joystick, G29_PEDAL_ACCEL);
    if (accel_pressure == last_accel_pressure) {
      continue;
    }
    last_accel_pressure = accel_pressure;

    auto message = create_midi_message(accel_pressure);
    midiout->sendMessage(&message);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    // poll every 50ms
    auto sleep_time = 50 - elapsed_seconds.count() * 1000;
    SDL_Delay(sleep_time);
  }

  // Clean up and close the joystick
  SDL_JoystickClose(joystick);
  joystick = nullptr;

  // Quit SDL subsystems
  delete midiout;
  SDL_Quit();
  return 0;
}
