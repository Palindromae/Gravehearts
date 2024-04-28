#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX

#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <stddef.h>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <set>
#include <array>
#include <chrono>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

