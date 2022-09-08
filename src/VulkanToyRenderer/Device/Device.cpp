#include <VulkanToyRenderer/Device/Device.h>

#include <set>
#include <vector>
#include <cstring>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/Swapchain/SwapchainManager.h>
#include <VulkanToyRenderer/Settings/vLayersConfig.h>

Device::Device() {}

Device::~Device() {}

void Device::createLogicalDevice(
      QueueFamilyIndices& requiredQueueFamiliesIndices
) {
   // - Specifies which QUEUES we want to create.
   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   // We use a set because we need to not repeat them to create the new one
   // for the logical device.
   std::set<uint32_t> uniqueQueueFamilies = {
         requiredQueueFamiliesIndices.graphicsFamily.value(),
         requiredQueueFamiliesIndices.presentFamily.value()
   };

   float queuePriority = 1.0f;
   for (uint32_t queueFamily : uniqueQueueFamilies)
   {
      VkDeviceQueueCreateInfo queueCreateInfo{};

      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      queueCreateInfos.push_back(queueCreateInfo);
   }

   // - Specifices which device FEATURES we want to use.
   // For now, this will be empty.
   VkPhysicalDeviceFeatures deviceFeatures{};


   // Now we can create the logical device.
   VkDeviceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   createInfo.queueCreateInfoCount = static_cast<uint32_t>(
         queueCreateInfos.size()
   );
   createInfo.pQueueCreateInfos = queueCreateInfos.data();
   createInfo.pEnabledFeatures = &deviceFeatures;

   // - Specifies which device EXTENSIONS we want to use.
   createInfo.enabledExtensionCount = static_cast<uint32_t>(
         m_requiredExtensions.size()
   );

   createInfo.ppEnabledExtensionNames = m_requiredExtensions.data();

   // Previous implementations of Vulkan made a distinction between instance 
   // and device specific validation layers, but this is no longer the 
   // case. That means that the enabledLayerCount and ppEnabledLayerNames 
   // fields of VkDeviceCreateInfo are ignored by up-to-date 
   // implementations. However, it is still a good idea to set them anyway to 
   // be compatible with older implementations:

   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {
      createInfo.enabledLayerCount = static_cast<uint32_t>(
            vLayersConfig::VALIDATION_LAYERS.size()
      );
      createInfo.ppEnabledLayerNames = (
            vLayersConfig::VALIDATION_LAYERS.data()
      );
   } else
      createInfo.enabledLayerCount = 0;

   auto status = vkCreateDevice(
         m_physicalDevice,
         &createInfo,
         nullptr,
         &m_logicalDevice
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create logical device!");

}
void Device::pickPhysicalDevice(
      VkInstance& vkInstance,
      QueueFamilyIndices& requiredQueueFamiliesIndices,
      const VkSurfaceKHR& windowSurface,
      SwapchainManager& swapchain
) {
   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

   if (deviceCount == 0)
      throw std::runtime_error("Failed to find GPUs with Vulkan support!");

   std::vector<VkPhysicalDevice> devices(deviceCount);
   vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
 
   for (const auto& device : devices)
   {
      if (isPhysicalDeviceSuitable(
               requiredQueueFamiliesIndices,
               windowSurface,
               swapchain,
               device
         )
      ) {
         m_physicalDevice = device;
         break;
      }
   }

   if (m_physicalDevice == VK_NULL_HANDLE)
      throw std::runtime_error("Failed to find a suitable GPU!");
}

bool Device::isPhysicalDeviceSuitable(
      QueueFamilyIndices& requiredQueueFamiliesIndices,
      const VkSurfaceKHR& windowSurface,
      SwapchainManager& swapchain,
      const VkPhysicalDevice& possiblePhysicalDevice
) {

   // - Queue-Families
   // Verifies if the device has the Queue families that we need.
   requiredQueueFamiliesIndices.getIndicesOfRequiredQueueFamilies(
         possiblePhysicalDevice,
         windowSurface
   );

   if (requiredQueueFamiliesIndices.areAllQueueFamiliesSupported == false)
      return false;
   
   // - Device Properties
   // Verifies if the device has the properties we want.
   VkPhysicalDeviceProperties deviceProperties;
   // Gives us basic device properties like the name, type and supported
   // Vulkan version.
   vkGetPhysicalDeviceProperties(possiblePhysicalDevice, &deviceProperties);

   // - Device Features
   // Verifies if the device has the features we want.
   VkPhysicalDeviceFeatures deviceFeatures;
   // Tells us if features like texture compression, 64 bit floats, multi
   // vieport renderending and so on, are compatbile with this device.
   vkGetPhysicalDeviceFeatures(possiblePhysicalDevice, &deviceFeatures);

   // Here we can score the gpu(so later select the best one to use) or just
   // verify if it has the features that we need.

   // For now, we will just return the dedicated one.
   if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      return false;

   // - Device Extensions
   if (areAllExtensionsSupported(possiblePhysicalDevice) == false)
      return false;

   // - Swapchain support
   if (swapchain.isSwapchainAdequated(
            possiblePhysicalDevice, windowSurface
      ) == false
   ) {
      return false;
   }

   return true;
}

bool Device::areAllExtensionsSupported(
      const VkPhysicalDevice& possiblePhysicalDevice
) {
   uint32_t extensionCount;
   vkEnumerateDeviceExtensionProperties(
         possiblePhysicalDevice,
         nullptr,
         &extensionCount,
         nullptr
   );

   std::vector<VkExtensionProperties> availableExtensions(extensionCount);
   vkEnumerateDeviceExtensionProperties(
         possiblePhysicalDevice,
         nullptr,
         &extensionCount,
         availableExtensions.data()
   );

   // Verifies if all required extensions are available in the device.
   // (it can be improved)
   for (const auto& requiredExtension : m_requiredExtensions)
   {
      bool extensionFound = false;

      for (const auto& availableExtension : availableExtensions)
      {
         const char* extensionName = availableExtension.extensionName;
         if (std::strcmp(requiredExtension, extensionName) == 0)
         {
            extensionFound = true;
            break;
         }
      }

      if (extensionFound == false)
         return false;
   }

   return true;
}

const VkDevice& Device::getLogicalDevice()
{
   return m_logicalDevice;
}

const VkPhysicalDevice& Device::getPhysicalDevice()
{
   return m_physicalDevice;
}