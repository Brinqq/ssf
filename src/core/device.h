#pragma once

#if _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#include "windows.h"
#include "keyboard.h"
#endif

#if __APPLE__
#include "GLFW/glfw3.h"
#endif


class Device{
private:
  typedef void* PlatformWindowHandle;


public:
   //windowing
  PlatformWindowHandle GraphicsWindow;
   void CloseGraphicWindow();
   int CreateGraphicWindow(float width, float height, const char* name);
   bool IsGraphicWindowRunning();
   void Tick();

  //input


};


