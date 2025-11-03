#include "device.h"
#include "core/debug.h"

#if __APPLE__
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"


int Device::CreateGraphicWindow(float width, float height, const char* name){
  if(glfwInit() == GLFW_FALSE){
    ssf_runtime_error()
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GraphicsWindow = glfwCreateWindow(width, height, name, nullptr, nullptr);

  if(GraphicsWindow == nullptr){
    ssf_runtime_error();
  }

  return 0;
}

void Device::CloseGraphicWindow(){
  glfwDestroyWindow(static_cast<GLFWwindow*>(GraphicsWindow));
  GraphicsWindow = nullptr;
  glfwTerminate();
}

bool Device::IsGraphicWindowRunning(){
  return GraphicsWindow != nullptr;
}

void Device::Tick(){
  if(IsGraphicWindowRunning()){
    glfwPollEvents();

    if(glfwWindowShouldClose(static_cast<GLFWwindow*>(GraphicsWindow)) == GLFW_TRUE){
      CloseGraphicWindow();
    }

  }
}

#endif


#if _WIN32

#endif
