// SPDX license identifier: MIT.
// Copyright (C) 2023 Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "function/gpu/image.h"

#include "core/log.h"

namespace luka {

Image::Image(const vk::raii::Device& device, const VmaAllocator& allocator,
             const vk::ImageCreateInfo& image_ci, const std::string& name)
    : allocator_{allocator} {
  VkImageCreateInfo vk_image_ci{static_cast<VkImageCreateInfo>(image_ci)};
  VmaAllocationCreateInfo allocation_ci{.usage = VMA_MEMORY_USAGE_AUTO};
  VkImage image;
  vmaCreateImage(allocator_, &vk_image_ci, &allocation_ci, &image, &allocation_,
                 nullptr);
  image_ = image;

  vk::ImageViewType image_view_type;
  switch (image_ci.imageType) {
    case vk::ImageType::e1D:
      image_view_type = vk::ImageViewType::e1D;
      break;
    case vk::ImageType::e2D:
      image_view_type = vk::ImageViewType::e2D;
      break;
    case vk::ImageType::e3D:
      image_view_type = vk::ImageViewType::e3D;
      break;
    default:
      THROW("Unknown image type");
  }

  vk::ImageViewCreateInfo image_view_ci{
      {},
      image_,
      image_view_type,
      image_ci.format,
      {},
      {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0,
       VK_REMAINING_ARRAY_LAYERS}};
  image_view_ = vk::raii::ImageView{device, image_view_ci};

  vk::SamplerCreateInfo sampler_ci;
  sampler_ = vk::raii::Sampler{device, sampler_ci};

  descriptor_image_info_ =
      vk::DescriptorImageInfo{*sampler_, *image_view_, image_ci.initialLayout};

#ifndef NDEBUG
  if (!name.empty()) {
    vk::DebugUtilsObjectNameInfoEXT image_name_info{
        vk::ObjectType::eImage,
        reinterpret_cast<uint64_t>(static_cast<VkImage>(image_)),
        (name + "_image").c_str()};
    device.setDebugUtilsObjectNameEXT(image_name_info);

    vk::DebugUtilsObjectNameInfoEXT image_view_name_info{
        vk::ObjectType::eImageView,
        reinterpret_cast<uint64_t>(static_cast<VkImageView>(*image_view_)),
        (name + "_image_view").c_str()};
    device.setDebugUtilsObjectNameEXT(image_view_name_info);

    vk::DebugUtilsObjectNameInfoEXT sampler_name_info{
        vk::ObjectType::eSampler,
        reinterpret_cast<uint64_t>(static_cast<VkSampler>(*sampler_)),
        (name + "_image_sampler").c_str()};
    device.setDebugUtilsObjectNameEXT(sampler_name_info);
  }

#endif
}

Image::~Image() {
  vmaDestroyImage(allocator_, static_cast<VkImage>(image_), allocation_);
}

Image::Image(Image&& rhs) noexcept
    : allocator_{rhs.allocator_},
      image_{std::exchange(rhs.image_, {})},
      allocation_{std::exchange(rhs.allocation_, {})},
      image_view_{std::exchange(rhs.image_view_, {nullptr})},
      sampler_{std::exchange(rhs.sampler_, {nullptr})},
      descriptor_image_info_{std::exchange(rhs.descriptor_image_info_, {})} {}

Image& Image::operator=(Image&& rhs) noexcept {
  if (this != &rhs) {
    allocator_ = rhs.allocator_;
    std::swap(image_, rhs.image_);
    std::swap(allocation_, rhs.allocation_);
    std::swap(image_view_, rhs.image_view_);
    std::swap(sampler_, rhs.sampler_);
    std::swap(descriptor_image_info_, rhs.descriptor_image_info_);
  }
  return *this;
}

const vk::Image& Image::operator*() const noexcept { return image_; }

const vk::DescriptorImageInfo& Image::GetDescriptorImageInfo() const {
  return descriptor_image_info_;
}

}  // namespace luka
