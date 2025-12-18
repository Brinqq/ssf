#pragma once

#include "vkdefines.h"
#include "vkentry.h"

#include <vector>
#include <array>
#include <list>

#include <bcl/containers/vector.h>
#include <bcl/containers/span.h>
#include <bcl/containers/bucket.h>

//tmp
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

//index fdata-fsize, rdata-rsize.... nullptr is valid we just 
//wont write that image layer.
struct CubeMapWriteDescription{
  void* data[6];
  size_t bytes[6];
};

typedef void* ResourceHandle;

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

  struct AttachmentResources{
    VkImage image;
    VkImageView view;
    VkFormat format;
    VkDeviceMemory memory;
  };

  struct AttachmentDescription{
    VkFormat format;
    VkImageLayout gpuRefLayout;
    VkImageLayout finalLayout;
    RenderAttachmentUsageType usage[kMaxSubpasses];
    int preserveDepth[kMaxSubpasses];
  };

  struct RenderCommandEntry{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkDescriptorSetLayout* DSL;
    bcl::small_vector<VkDescriptorSet*, 5> sets;
    void* push;
    void(*Exectute)();
  };

  struct CommandStream{
    RenderCommandEntry* states;
     uint32_t count;
  };

  struct RPassState{
    VkRenderPass handle;
    uint32_t numSubpasses;
    VkFramebuffer* framebuffers[kMaxFrameQueue];
  };

  struct ImageResource{
    VkImage image;
    VkDeviceMemory memory;
    VkExtent3D extent;
    VkImageSubresourceRange range;
  };

  struct DepthBuffer{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkFormat format;
  };

struct GpuCubeMap{
  VkImageView view;
  ImageResource* resource;
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
  static constexpr uint64_t _stagingBufferSize = 800000000;
  static constexpr int _macosDeviceLocalFlag = 0;
  static constexpr int _macosHostAccessFlag = 1;

    const VkComponentMapping defaultTextureCMapping{ 
      VK_COMPONENT_SWIZZLE_R,
      VK_COMPONENT_SWIZZLE_G,
      VK_COMPONENT_SWIZZLE_B,
      VK_COMPONENT_SWIZZLE_A
    };


  //Builtin state
  std::array<VkSampler, Sampler::Count> fiSamplers;
  std::array<VkSemaphore, Semaphore::Count> semaphores;
  std::array<VkFence, Fence::Count> fences;
  std::array<VkBuffer, juye::driver::BuiltinUniformCount> BuiltinUniforms;


  bk::bucket<VkDescriptorSetLayout, 3> descriptorSetLayoutLut;


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

  std::vector<DescriptorPool> globalPool;

  GeometryPassPush DefaultGPassStub{};


  //dyn state
  std::unordered_map<VkFramebuffer, AttachmentResources> attachmentMemories;

  bk::bucket<VkFramebuffer, 10> framebufferss;
  
  //TODO: these are so so bad replace with bcl::dense_table or bcl::pool when done verifying completeness.
  std::list<GpuBuffer> bufferList;
  std::list<GpuTexel> texelList;
  std::list<GBufEntry> geometryList;
  std::vector<GBufEntry> drawList;

  //entries
  juye::driver::LightEntryListBundle lightBundle;

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
  VkDescriptorPool geoPassDescriptorPool;

  std::array<VkDescriptorSet, 2> textureDescSet;

  VkCommandPool graphicsPool;
  VkCommandPool transferPool;

  VkCommandBuffer mainCommandBuffer;
  VkFence mainFence;

  VkQueue graphicQueue;
  VkQueue transferQueue;

  std::vector<QueueFamily> queueFamilies;
  std::pair<VkBuffer, VkDeviceMemory> stagingBuffers[7];

  //NOTE: All this is super tmp, for now we basically thow everything i dont
  // know how to structure in here and hope we figure it out when the system makes more sense.
  //------------------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------------------
  
  //geometry pass stuff
  VkPipeline mainPipeline;
  VkPipelineLayout geoPassPipelineLayout;
  VkDescriptorSetLayout* geoPassDescriptorLayout;

  // skybox pass stuff
  VkPipeline skyboxPipeline;
  VkPipelineLayout skyboxPipelineLayout;
  VkDescriptorSetLayout* skyboxDescriptorLayout;;
  VkDescriptorSet skyboxDescriptorSet;

  //uniform: projection-view matrix
  juye::driver::DrawFrustum frustum;
  VkDescriptorSetLayout* frustumDescriptorLayout;
  VkDescriptorSet frustumDescriptorSet;
  VkBuffer projectionViewBuffer;
  VkDeviceMemory projectionViewMemory;
  VkDescriptorSet projectionViewSet;
  const int kProjectionMatrixSize = 128;

  
  bk::bucket<ResourceHandle, 10> resourceLUT;
  bk::bucket<ImageResource, 10> imageLUT;
  //------------------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------------------

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
  void GpuUploadImageData(VkCommandBuffer cmdBuf,VkExtent3D extent, VkBufferImageCopy& copy, VkDeviceMemory stage, VkBuffer srcBuf, VkImage dstBuf, const void* const pData);
  int CreateRenderPass(const bk::span<AttachmentDescription>& attachments, uint32_t numSubpasses, const RenderPassCreateFlags flags, VkRenderPass* pRenderpass);

  void DestroyGraphicPipeline();

public:


  int Init();
  void Destroy();

  int CreateComputeState();
  int DestroyComputeState();

  GeoHandle CreateGeometry(const GeometryData& geo);
  void DestroyGeometry(GeoHandle& geometry);

  ResourceHandle CreateCubeMap(uint32_t size);
  void WriteCubeMap(ResourceHandle handle, const CubeMapWriteDescription& desc);
  void DestroyCubeMap(ResourceHandle handle);

  ResourceHandle CreateLightSource(const juye::Color3& col, const juye::Vector3f& pos, 
                const juye::driver::LightEntryType type);

  void WriteLightSource(ResourceHandle h);
  void DestroyLightSources(ResourceHandle* h, int count);

  void MapGeometryPassPushBuf(GeoHandle& handle, void* pData);
  void UnmapGeometryPassPushBuf(GeoHandle& handle);

  void AddToDrawList(GeoHandle& geometry);
  void RemoveToDrawList(GeoHandle& geometry);

  int CreateGraphicsState(Device& device);
  int DestroyGraphicsState();

  void SetSkyBox(ResourceHandle cubmap);

  void Draw();
  void TestTriangle();
  void WriteFrustum(float* vp);

};//class VK
