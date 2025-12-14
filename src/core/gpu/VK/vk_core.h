#pragma once

#include "vk_defines.h"

#include <vector>
#include <array>

#include <list>

#include <bcl/containers/vector.h>
#include <bcl/containers/span.h>
#include <bcl/containers/bucket.h>

//tmp
#include "gk/prefabs.h"
#include "glm/glm.hpp"
//---------

// with xxHash and a LUT/Hashmap. 
// - add reference count to these so we know when to remove on deletion of said resource.
// - include functions for getting the cached handle for a optinal optimization for the frontend,
//   this eleminates having to rehash the gpu resource on every draw target creation.
// - figure out how to invalidate a handle if ref count hits 0 and resource is destoryed or perhaps throw a error.
// - make sure to defer deletion of ref counted objects to a state where there are no in-flight frames.
// - Thread safety?

//TODO: MAJOR, Go through a support windows and add dynamic state information
//based on gpu characteristics.

//TODO: Figure out a better descriptor pool allocation/creation stratagey.


struct Device;
struct ShaderContainer;
class VK;


enum DescriptorPoolType{
  DescriptorPoolTexture,
};

enum RenderPassCreateFlags{
  RenderPassNone = 0x0,
  RenderPassDepthBit = 0x1,
};


struct GeometryData{
  void* pVertex;
  void* pIndices;
  void* texture;

  size_t vertexBytes;
  size_t indicesBytes;
  uint32_t textureWidth;
  uint32_t textureHeight;

  uint32_t numIndices;
};


struct CubeMapData{
};


class VK{
private:

  //static pipeline

  struct Semaphore{
   static constexpr int RenderReady = 0;
   static constexpr int RenderDone = 1;
   static constexpr int CopiesDone = 2;
   static constexpr int Count = 3;
  };

  struct Fence{
    static constexpr int FrameInFlight = 0;
    static constexpr int Count = 1;
  };

  struct Sampler{
    static constexpr int ClampTexture = 0;
    static constexpr int Count = 1;

  };

  struct RenderPass{
    static constexpr int Geometry = 0;
    static constexpr int Count = 1;
  };

  enum GpuResourceType{
      GpuResourceBuffer,
      GpuResourceImage,
  };

  enum GpuReadMemoryType{
    GpuReadMemoryLinear,
    GpuReadMemoryUnspecified,
    GpuReadMemoryTransferReady,
    GpuReadMemoryShaderReady
  };

  enum RenderAttachmentUsageType{
    RenderAttachmentUsageWrite,
    RenderAttachmentUsageRead,
    RenderAttachmentUsagePreserve,
  };

  enum RenderPipelineFlags{
    RenderPipelineDepthEnable = 0x0,
    RenderPipelineWireframe = 0x2
  };


  static constexpr int kMaxSubpasses = 4;
  static constexpr int kMaxFrameQueue = 3;

  struct Swapchain{
    struct Data{
      VkImageView view;
      VkSemaphore Semaphore;
    };

    VkSwapchainKHR handle;
    VkSurfaceKHR surface;
    VkExtent2D extent;
    uint8_t currentBuf;
    uint8_t maxFrames;
    VkFormat format;
    bcl::small_vector<Swapchain::Data, 3> buf;
  };

  struct AttachmentMemoryRegistry{
    VkImage image;
    VkImageView view;
    VkFormat format;
    VkDeviceMemory memory;
  };

  struct RenderAttachment{
    VkFormat format;
    VkImageLayout gpuRefLayout;
    VkImageLayout finalLayout;
    RenderAttachmentUsageType usage[kMaxSubpasses];
    int preserveDepth[kMaxSubpasses];
  };

  struct DrawState{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkDescriptorSetLayout* DSL;
    bcl::small_vector<VkDescriptorSet*, 5> sets;
    void* push;
  };

  struct RPassState{
    VkRenderPass handle;
    uint32_t numSubpasses;
    VkFramebuffer* framebuffers[kMaxFrameQueue];
  };

  struct DepthBuffer{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkFormat format;
  };

  struct GpuBuffer{
    VkBuffer handle;
    VkDeviceMemory memory;
  };

  struct GpuTexel{
    VkImage handle;
    VkDeviceMemory memory;

    //TODO: these should not be coupled with the GpuTexel because it duplicates with same textured geomtery.
    VkImageView view;
    VkDescriptorSet descriptor;
  };

  struct GeometryPassPush{
    float transform[16];
    float view[16];
    float projection[16];
  };
  
  //TODO: change from std::list::iterators when bcl::dense_table change.
  //32 bytes per iterator wtf.
  struct GBufEntry{
    std::list<GpuBuffer>::iterator vertex;  
    std::list<GpuBuffer>::iterator index;  
    std::list<GpuTexel>::iterator texture;  
    void* push;
    uint32_t numIndices;
  };


  // compile time state
  static constexpr uint64_t _stagingBufferSize = 80000000;
  static constexpr int _macosDeviceLocalFlag = 0;
  static constexpr int _macosHostAccessFlag = 1;

  // fixed state
    const VkComponentMapping defaultTextureCMapping{ 
      VK_COMPONENT_SWIZZLE_R,
      VK_COMPONENT_SWIZZLE_G,
      VK_COMPONENT_SWIZZLE_B,
      VK_COMPONENT_SWIZZLE_A
    };

  std::array<VkSampler, Sampler::Count> fiSamplers;
  std::array<VkSemaphore, Semaphore::Count> semaphores;
  std::array<VkFence, Fence::Count> fences;

  std::list<VkDescriptorSetLayout> DSLS;

  enum DescriptorResourceFlags{
    Texture = 0x0,
    Storage = 0x2,
    Uniform = 0x4,
  };

  struct DescriptorPool{
    VkDescriptorPool pool;
    uint8_t maxSets;
    uint8_t remainingSets;
  };

  std::list<VkDescriptorSet> GlobalDescriptors;
  std::list<VkDescriptorSet> PipelineDescriptors;
  std::list<VkDescriptorSet> ObjectDescriptors;
  std::vector<DescriptorPool> textureDP;

  GeometryPassPush DefaultGPassStub{};


  //dyn state
  std::unordered_map<VkFramebuffer, AttachmentMemoryRegistry> attachmentMemories;

  bk::bucket<VkFramebuffer, 10> framebufferss;
  
  //TODO: these are so so bad replace with bcl::dense_table or bcl::pool when done verifying completeness.
  std::list<GpuBuffer> bufferList;
  std::list<GpuTexel> texelList;
  std::list<GBufEntry> geometryList;
  std::vector<GBufEntry> drawList;

  public:
    typedef std::list<GBufEntry>::iterator GeoHandle;
  private:
  
  DepthBuffer depthBuffer;

  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice gpu;

  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
  VkExtent2D swapchainExtent;

  uint8_t numBackbuffers = 2;
  uint32_t curBackBuffer = 0;
  VkImageView swapchainViews[2];
  VkFramebuffer framebuffers[2];
  std::array<VkSemaphore, 2> swapSemaphores;

  VkRenderPass mainRenderpass;
  VkPipeline mainPipeline;
  VkDescriptorPool geoPassDescriptorPool;
  VkDescriptorSetLayout geoPassDescriptorLayout;
  VkPipelineLayout geoPassPipelineLayout;
  std::array<VkDescriptorSet, 2> textureDescSet;

  VkCommandPool graphicsPool;
  VkCommandPool transferPool;

  VkCommandBuffer mainCommandBuffer;
  VkFence mainFence;

  VkQueue graphicQueue;
  VkQueue transferQueue;

  std::vector<QueueFamily> queueFamilies;
  std::pair<VkBuffer, VkDeviceMemory> stagingBuffers[2];

  ivk::FeatureSet features;

  struct{
    const ssf::prefabs::TexturedCube<uint16_t> data;
    VkBuffer vbo;
    VkDeviceMemory vHandle;
    uint64_t vDataSize;

    VkBuffer ibo;
    VkDeviceMemory iHandle;
    uint64_t iDataSize;

    VkImage texture;
    VkImageView textureView;
    VkDeviceMemory texHandle;

    std::array<VkDescriptorSet, 2> textureDescSet;
    VkSampler sampler;

    glm::mat4 mvp[3];
    

  }_TmpCube;

private:
  //Dynamic state Initialization
  
  //TMP:
  void tCreateDescriptorPools(DescriptorPoolType type, uint32_t count, DescriptorPool* pMemory);

  
  //fixed State Initialization
  void CreateFixedSamplers(bool rebuild);
  void CreateFixedDescriptors();

  void SwapBackBuffers();

  void SetGpuImageBarriers(VkCommandBuffer cmdBuf, const VkImageMemoryBarrier* const pBarrier, uint32_t count, VkPipelineStageFlags src, VkPipelineStageFlags dst);

  void GpuUploadBufData(VkCommandBuffer cmdBuf, VkDeviceMemory stage, VkBuffer srcBuf, VkBuffer dstBuf, const void* const pData, size_t bytes);
  void GpuUploadImageData(VkCommandBuffer cmdBuf,VkExtent3D extent, VkDeviceMemory stage, VkBuffer srcBuf, VkImage dstBuf, const void* const pData);
  int CreateRenderPass(const bk::span<RenderAttachment>& attachments, uint32_t numSubpasses, const RenderPassCreateFlags flags, VkRenderPass* pRenderpass);

  void DestroyGraphicPipeline();

  VkPipelineLayout CreatePipelineLayoutFromContainer(const ShaderContainer& container);

public:

  typedef int ResourceHandle;

  int Init();
  void Destroy();

  int CreateComputeState();
  int DestroyComputeState();

  GeoHandle CreateGeometry(const GeometryData& geo);
  void DestroyGeometry(GeoHandle& geometry);

  ResourceHandle CreateCubeMap(const CubeMapData& data);
  void DestroyCubeMap(ResourceHandle handle);

  void MapGeometryPassPushBuf(GeoHandle& handle, void* pData);
  void UnmapGeometryPassPushBuf(GeoHandle& handle);

  void AddToDrawList(GeoHandle& geometry);
  void RemoveToDrawList(GeoHandle& geometry);

  int CreateGraphicsState(Device& device);
  int DestroyGraphicsState();

  void SetSkyBox(const void* texture, uint32_t width, uint32_t height);

  void Draw();
  void TestTriangle();

};//class VK
