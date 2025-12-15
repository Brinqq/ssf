#include "device.h"
#include "core/global.h"

#if __APPLE__
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

std::unordered_map<int, std::atomic<int>*> PhysicalKeyMap;

void glfwKeyboardCallback(GLFWwindow* h, int key, int scan, int action, int mods){
  if(action == GLFW_PRESS){
    auto val = PhysicalKeyMap.find(key);
    if (val != PhysicalKeyMap.end()){
      val->second->store(1, std::memory_order_release);
      return;
    }
  }

  if(action == GLFW_RELEASE){
    auto val = PhysicalKeyMap.find(key);
    if (val != PhysicalKeyMap.end()){
      val->second->store(0, std::memory_order_release);
    }

  };
}

int Device::CreateGraphicWindow(float width, float height, const char* name){
  if(glfwInit() == GLFW_FALSE){
    juye_runtime_error();
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GraphicsWindow = glfwCreateWindow(width, height, name, nullptr, nullptr);

  if(GraphicsWindow == nullptr){
    juye_runtime_error();
  }

  glfwGetWindowSize((GLFWwindow*)GraphicsWindow, &windowW, &windowH);
  glfwSetKeyCallback((GLFWwindow*)GraphicsWindow, &glfwKeyboardCallback);

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
    // FrameInputBuf.clear();

    if(glfwWindowShouldClose(static_cast<GLFWwindow*>(GraphicsWindow)) == GLFW_TRUE){
      CloseGraphicWindow();
    }

  //input
  }
}

#endif


#if _WIN32

#endif
