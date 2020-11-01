#pragma once

#include <vulkan/vulkan.hpp>

namespace ddls
{
void createSyncObjects(VkDevice device, std::vector<VkSemaphore>& imageAvailableSemaphores,
                      std::vector<VkSemaphore>& renderFinishedSemaphores,
                       std::vector<VkFence>& inFlightFences, std::vector<VkFence>& imagesInFlight,
                       std::vector<VkImage>& swapChainImages);
}
