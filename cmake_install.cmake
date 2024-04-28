# Install script for directory: D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/Projects/_install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin_x64" TYPE EXECUTABLE FILES "D:/Projects/bin_x64/Release/vk_ray_tracing__before_KHR.exe")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin_x64_debug" TYPE EXECUTABLE FILES "D:/Projects/bin_x64/Debug/vk_ray_tracing__before_KHR.exe")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin_x64/vk_ray_tracing__before_KHR/spv" TYPE FILE FILES
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/frag_shader.frag.spv"
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/passthrough.vert.spv"
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/post.frag.spv"
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/vert_shader.vert.spv"
      )
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin_x64_debug/vk_ray_tracing__before_KHR/spv" TYPE FILE FILES
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/frag_shader.frag.spv"
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/passthrough.vert.spv"
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/post.frag.spv"
      "D:/Projects/vk_raytracing_tutorial_KHR/ray_tracing__before/spv/vert_shader.vert.spv"
      )
  endif()
endif()

