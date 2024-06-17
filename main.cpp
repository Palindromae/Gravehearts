/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */


// ImGui - standalone example application for Glfw + Vulkan, using programmable
// pipeline If you are new to ImGui, see examples/README.txt and documentation
// at the top of imgui.cpp.

#include <array>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "imgui/imgui_helper.h"


#include "hello_vulkan.h"
#include "imgui/imgui_camera_widget.h"
#include "nvh/cameramanipulator.hpp"
#include "nvh/fileoperations.hpp"
#include "nvpsystem.hpp"
#include "nvvk/commands_vk.hpp"
#include "nvvk/context_vk.hpp"
#include "src/DisjointCommandDispatcher.h"
#include "src/QueueFamilyIndices.h"
#include "src/LoadComputeShaders.h"
#include "src/EntityManager.h"
#include "src/Entity.h"
#include "src/ModelManager.h"
#include "src/ChunkInterface.h"
#include "src/Physics_Const.h"
#include "src/CurrentEntityTLSData.h"
#include <iostream>
#include "src/PhysicsInterface.h"
#include "shaders/EntityConst.h"

//////////////////////////////////////////////////////////////////////////
#define UNUSED(x) (void)(x)
//////////////////////////////////////////////////////////////////////////

// Default search path for shaders
std::vector<std::string> defaultSearchPaths;

// GLFW Callback functions
static void onErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Extra UI
void renderUI(HelloVulkan& helloVk)
{
  ImGuiH::CameraWidget();
  if(ImGui::CollapsingHeader("Light"))
  {
    ImGui::RadioButton("Point", &helloVk.m_pcRaster.lightType, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Infinite", &helloVk.m_pcRaster.lightType, 1);

    ImGui::SliderFloat3("Position", &helloVk.m_pcRaster.lightPosition.x, -20.f, 20.f);
    ImGui::SliderFloat("Intensity", &helloVk.m_pcRaster.lightIntensity, 0.f, 150.f);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int const SAMPLE_WIDTH  = 1280;
static int const SAMPLE_HEIGHT = 720;
HelloVulkan helloVk;
glm::vec4 clearColor = glm::vec4(1, 1, 1, 1.00f);

bool useRaytracer = true;

void Render()
{
    // Start rendering the scene
    helloVk.prepareFrame();

    // Start command buffer of this frame
    auto                   curFrame = helloVk.getCurFrame();
    const VkCommandBuffer& cmdBuf = helloVk.getCommandBuffers()[curFrame];

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);


    // Updating camera buffer
    helloVk.updateUniformBuffer(cmdBuf);
    helloVk.raytracer.updateRtDescriptorSetPerFrame();
    // Clearing screen
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {clearColor[0], clearColor[1], clearColor[2], clearColor[3]} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    // Offscreen render pass
    {
        VkRenderPassBeginInfo offscreenRenderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        offscreenRenderPassBeginInfo.clearValueCount = 2;
        offscreenRenderPassBeginInfo.pClearValues = clearValues.data();
        offscreenRenderPassBeginInfo.renderPass = helloVk.m_offscreenRenderPass;
        offscreenRenderPassBeginInfo.framebuffer = helloVk.m_offscreenFramebuffer;
        offscreenRenderPassBeginInfo.renderArea = { {0, 0}, helloVk.getSize() };

        if (useRaytracer)
        {
            helloVk.raytrace(cmdBuf, clearColor);
        }
        else
        {
            // Rendering Scene
            vkCmdBeginRenderPass(cmdBuf, &offscreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            helloVk.rasterize(cmdBuf);
            vkCmdEndRenderPass(cmdBuf);
        }
    }


    // 2nd rendering pass: tone mapper, UI
    {
        VkRenderPassBeginInfo postRenderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        postRenderPassBeginInfo.clearValueCount = 2;
        postRenderPassBeginInfo.pClearValues = clearValues.data();
        postRenderPassBeginInfo.renderPass = helloVk.getRenderPass();
        postRenderPassBeginInfo.framebuffer = helloVk.getFramebuffers()[curFrame];
        postRenderPassBeginInfo.renderArea = { {0, 0}, helloVk.getSize() };

        // Rendering tonemapper
        vkCmdBeginRenderPass(cmdBuf, &postRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        helloVk.drawPost(cmdBuf);
        // Rendering UI
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
        vkCmdEndRenderPass(cmdBuf);
    }

    // Submit for display
    vkEndCommandBuffer(cmdBuf);
    helloVk.submitFrame();
}



//--------------------------------------------------------------------------------------------------
// Application Entry
//
int main(int argc, char** argv)
{
  UNUSED(argc);

  // Setup GLFW window
  glfwSetErrorCallback(onErrorCallback);
  if(!glfwInit())
  {
    return 1;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(SAMPLE_WIDTH, SAMPLE_HEIGHT, PROJECT_NAME, nullptr, nullptr);


  // Setup camera
  CameraManip.setWindowSize(SAMPLE_WIDTH, SAMPLE_HEIGHT);
  CameraManip.setLookat(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

  // Setup Vulkan
  if(!glfwVulkanSupported())
  {
    printf("GLFW: Vulkan Not Supported\n");
    return 1;
  }

  // setup some basic things for the sample, logging file for example
  NVPSystem system(PROJECT_NAME);

  // Search path for shaders and other media
  defaultSearchPaths = {
      NVPSystem::exePath() + PROJECT_RELDIRECTORY,
      NVPSystem::exePath() + PROJECT_RELDIRECTORY "..",
      std::string(PROJECT_NAME),
  };

  // Vulkan required extensions
  assert(glfwVulkanSupported() == 1);
  uint32_t count{0};
  auto     reqExtensions = glfwGetRequiredInstanceExtensions(&count);

  // Requesting Vulkan extensions and layers
  nvvk::ContextCreateInfo contextInfo;
  contextInfo.setVersion(1, 3);                       // Using Vulkan 1.3
  for(uint32_t ext_id = 0; ext_id < count; ext_id++)  // Adding required extensions (surface, win32, linux, ..)
    contextInfo.addInstanceExtension(reqExtensions[ext_id]);
  contextInfo.addInstanceLayer("VK_LAYER_LUNARG_monitor", true);              // FPS in titlebar
  contextInfo.addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true);  // Allow debug names
  contextInfo.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);            // Enabling ability to present rendering

  // #VKRay: Activate the ray tracing extension
  VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
  contextInfo.addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, false, &accelFeature);  // To build acceleration structures
  VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
  contextInfo.addDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, false, &rtPipelineFeature);  // To use vkCmdTraceRaysKHR
  contextInfo.addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);  // Required by ray tracing pipeline
  contextInfo.addDeviceExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);  
  contextInfo.addDeviceExtension(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);  
  VkPhysicalDeviceRayQueryFeaturesKHR rtQueryfeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
  contextInfo.addDeviceExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, false, &rtQueryfeature);
  // Creating Vulkan base application
  nvvk::Context vkctx{};
  vkctx.initInstance(contextInfo);
  // Find all compatible devices
  auto compatibleDevices = vkctx.getCompatibleDevices(contextInfo);
  assert(!compatibleDevices.empty());
  // Use a compatible device
  vkctx.initDevice(compatibleDevices[0], contextInfo);



  // Window need to be opened to get the surface on which to draw
  VkSurfaceKHR surface = helloVk.getVkSurface(vkctx.m_instance, window);
  vkctx.setGCTQueueWithPresent(surface);

  helloVk.setup(vkctx.m_instance, vkctx.m_device, vkctx.m_physicalDevice, vkctx.m_queueGCT.familyIndex);
  helloVk.createSwapchain(surface, SAMPLE_WIDTH, SAMPLE_HEIGHT);
  helloVk.createDepthBuffer();
  helloVk.createRenderPass();
  helloVk.createFrameBuffers();

  // Setup Imgui
  helloVk.initGUI(0);  // Using sub-pass 0

  // Creation of the example
  helloVk.loadModel(nvh::findFile("media/scenes/cube_multi.obj", defaultSearchPaths, true));
  helloVk.raytracer.createImplictBuffers();

  helloVk.createOffscreenRender();
  helloVk.createDescriptorSetLayout();
  helloVk.createGraphicsPipeline();
  helloVk.createUniformBuffer();
  helloVk.createObjDescriptionBuffer();

  


  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vkctx.m_physicalDevice, surface);
  VkCommandPool commandPool{};

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(vkctx.m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
  }

  // Initiate Nve

  new DisjointCommandDispatcher(&vkctx.m_device, &commandPool, &vkctx.m_queueGCT.queue, &vkctx.m_queueT.queue, &vkctx.m_queueC.queue, &vkctx.m_physicalDevice, &surface, &helloVk.m_alloc);
  new ChunkInterface();

  Shaders::LoadComputeShaders::LoadComputeShaders(&vkctx.m_device);


  new PhysicsInterface();
  new ModelManager();
  new EntityManager();
  Entity* entity2 = new Entity(glm::vec3(0), 0);
  Entity* entity = new Entity(glm::vec3(5), 0);
  Entity* entity3 = new Entity(glm::vec3(1,2,3), 0);
  Entity* entity4 = new Entity(glm::vec3(5,-3,1), 0);

  ChunkVolume* chunkVolume = ChunkInterface::instance->CreateChunk(ChunkID(0));
  ChunkVolume* chunkVolumeB = ChunkInterface::instance->CreateChunk(ChunkID(NoChunksPerAxii,0, NoChunksPerAxii));

  ChunkInterface::instance->CheckChunkArr();


  for (size_t x = 0; x < NoChunksPerAxii; x++)
  {
      for (size_t z = 0; z < NoChunksPerAxii; z++)
      {
          ChunkInterface::instance->TestGeneration(chunkVolume, ChunkID(x,0,z));

      }
  }

  for (size_t x = 0; x < NoChunksPerAxii; x++)
  {
      for (size_t z = 0; z < NoChunksPerAxii; z++)
      {
          ChunkInterface::instance->TestGeneration(chunkVolumeB, ChunkID(x + NoChunksPerAxii,0,z + NoChunksPerAxii));

      }
  }

  helloVk.updateDescriptorSet();
  helloVk.createPostDescriptor();
  helloVk.createPostPipeline();
  helloVk.updatePostDescriptorSet();

  // #VKRay
  helloVk.initRayTracing();

  helloVk.setupGlfwCallbacks(window);
  ImGui_ImplGlfw_InitForVulkan(window, true);

  bool interpolate = true;

  float time = 0;

  auto current = std::chrono::high_resolution_clock::now();
  double accumulatedTime = 0;
  int frame = 0;
  while (!glfwWindowShouldClose(window))
  {
      glfwPollEvents();
      if (helloVk.isMinimized())
          continue;
      frame++;

      // Start the Dear ImGui frame
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();


      // Show UI window.
      if (helloVk.showGui())
      {
          bool changed = false;

          changed |= ImGui::ColorEdit3("Clear color", reinterpret_cast<float*>(&clearColor));
          changed |= ImGui::Checkbox("Ray Tracer mode", &useRaytracer);  // Switch between raster and ray tracing
          changed |= ImGui::Checkbox("Interpolate", &interpolate);  // Allow physics interpolation
          //  if(changed)
             // helloVk.resetFrame();

          ImGuiH::Panel::Begin();
          ImGui::ColorEdit3("Clear color", reinterpret_cast<float*>(&clearColor));
          renderUI(helloVk);
          ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
          ImGuiH::Control::Info("", "", "(F10) Toggle Pane", ImGuiH::Control::Flags::Disabled);
          ImGuiH::Panel::End();
      }


      if (frame == 1) {
          current = std::chrono::high_resolution_clock::now();
      }

      const auto now = std::chrono::high_resolution_clock::now();
      double frameTime = std::chrono::duration_cast<std::chrono::seconds>(now - current).count();

      current = now;
      accumulatedTime += frameTime;

      while (accumulatedTime >= FixedDeltaTime) {

          // How long did previous step take?

          // Expect the previous frame to have finished

          // Do a physics step
          NVEPhysics->InitiateNewPhysicsUpdate(true);

          accumulatedTime -= FixedDeltaTime;
          if (accumulatedTime >= FixedDeltaTime)
          {
              std::cout << "PHYSICS EXPECTING TO BE RAN TWICE";
          }
      }


      // INTERPOLATE between current time and previous time
      //if (interpolate)
      //    TLS::InterpolatedFrame->Interpolate(TLS::PreviousInterpolationFrame, NVEPhysics->GetCurrentDefinedPhysicsFrame(), accumulatedTime / FixedDeltaTime);
    //  else
       //   TLS::InterpolatedFrame->Interpolate(TLS::PreviousInterpolationFrame, NVEPhysics->GetCurrentDefinedPhysicsFrame(), 1);

      /// GAME LOGIC

      // ENTITY LOGIC --- Before physics updates to buffer

      // ENTITY LOGIC --- After buffer
      EntityManager::instance->RebuildTLS();
      EntityManager::instance->UpdateBuffer();

      // Rebuild TLAS --- Can be done before gamelogic of async until now
      EntityManager::instance->BuildTlas();
      ChunkInterface::instance->RebuildGPUStructures();
      // Render
        Render();
  }


  // Cleanup
  vkDeviceWaitIdle(helloVk.getDevice());

  helloVk.destroyResources();
  helloVk.destroy();
  vkctx.deinit();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
