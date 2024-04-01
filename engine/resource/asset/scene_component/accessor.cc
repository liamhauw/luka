// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "resource/asset/scene_component/accessor.h"

#include "core/log.h"
#include "resource/asset/scene_component/buffer_view.h"

namespace luka {

namespace ast::sc {

Accessor::Accessor(const BufferView* buffer_view, u64 byte_offset,
                   bool normalized, u32 component_type, u64 count, u32 type,
                   const std::string& name)
    : Component{name},
      buffer_view_{buffer_view},
      byte_offset_{byte_offset},
      normalized_{normalized},
      component_type_{component_type},
      count_{count},
      type_{type} {
  CalculateBufferData();
}

Accessor::Accessor(const std::vector<BufferView*>& buffer_view_components,
                   const tinygltf::Accessor& tinygltf_accessor)
    : Component{tinygltf_accessor.name},
      buffer_view_{buffer_view_components[tinygltf_accessor.bufferView]},
      byte_offset_{tinygltf_accessor.byteOffset},
      normalized_{tinygltf_accessor.normalized},
      component_type_{static_cast<u32>(tinygltf_accessor.componentType)},
      count_{tinygltf_accessor.count},
      type_{static_cast<u32>(tinygltf_accessor.type)} {
  CalculateBufferData();
}

std::type_index Accessor::GetType() { return typeid(Accessor); }

u64 Accessor::GetCount() const { return count_; }

std::pair<const u8*, u64> Accessor::GetBuffer() const {
  return std::make_pair(buffer_data_, buffer_size_);
}

u32 Accessor::GetStride() const { return buffer_stride_; }

vk::Format Accessor::GetFormat() const { return format_; }

void Accessor::CalculateBufferData() {
  const u8* whole_data_begin{buffer_view_->GetBuffer()->GetData()->data()};
  buffer_stride_ = GetByteStride(buffer_view_->GetByteStride());
  buffer_data_ =
      whole_data_begin + byte_offset_ + buffer_view_->GetByteOffset();
  buffer_size_ = count_ * buffer_stride_;
  format_ = ParseFormat();
}

u32 Accessor::GetByteStride(u32 buffer_view_byte_stride) {
  if (buffer_view_byte_stride == 0) {
    i32 component_size_in_byte{
        tinygltf::GetComponentSizeInBytes(component_type_)};
    if (component_size_in_byte <= 0) {
      THROW("Unsupport component type");
    }

    i32 component_count{tinygltf::GetNumComponentsInType(type_)};
    if (component_count <= 0) {
      THROW("Unsupport type");
    }

    return static_cast<u32>(component_size_in_byte * component_count);
  } else {
    i32 component_size_in_byte{
        tinygltf::GetComponentSizeInBytes(component_type_)};
    if (component_size_in_byte <= 0) {
      THROW("Unsupport component type");
    }

    if (buffer_view_byte_stride % static_cast<u32>(component_size_in_byte) !=
        0) {
      THROW("Unsupport component type");
    }
    return static_cast<u32>(buffer_view_byte_stride);
  }
}

vk::Format Accessor::ParseFormat() {
  vk::Format format;

  switch (component_type_) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      if (!normalized_) {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR8Sint;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR8G8Sint;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR8G8B8Sint;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR8G8B8A8Sint;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      } else {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR8Snorm;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR8G8Snorm;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR8G8B8Snorm;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR8G8B8A8Snorm;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      }
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      if (!normalized_) {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR8Uint;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR8G8Uint;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR8G8B8Uint;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR8G8B8A8Uint;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      } else {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR8Unorm;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR8G8Unorm;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR8G8B8Unorm;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR8G8B8A8Unorm;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      }
      break;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      if (!normalized_) {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR16Sint;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR16G16Sint;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR16G16B16Sint;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR16G16B16A16Sint;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      } else {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR16Snorm;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR16G16Snorm;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR16G16B16Snorm;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR16G16B16A16Snorm;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      }
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      if (!normalized_) {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR16Uint;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR16G16Uint;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR16G16B16Uint;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR16G16B16A16Uint;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      } else {
        switch (type_) {
          case TINYGLTF_TYPE_SCALAR:
            format = vk::Format::eR16Unorm;
            break;
          case TINYGLTF_TYPE_VEC2:
            format = vk::Format::eR16G16Unorm;
            break;
          case TINYGLTF_TYPE_VEC3:
            format = vk::Format::eR16G16B16Unorm;
            break;
          case TINYGLTF_TYPE_VEC4:
            format = vk::Format::eR16G16B16A16Unorm;
            break;
          default:
            THROW("Unsupport type");
            break;
        }
      }
      break;
    case TINYGLTF_COMPONENT_TYPE_INT:
      switch (type_) {
        case TINYGLTF_TYPE_SCALAR:
          format = vk::Format::eR32Sint;
          break;
        case TINYGLTF_TYPE_VEC2:
          format = vk::Format::eR32G32Sint;
          break;
        case TINYGLTF_TYPE_VEC3:
          format = vk::Format::eR32G32B32Sint;
          break;
        case TINYGLTF_TYPE_VEC4:
          format = vk::Format::eR32G32B32A32Sint;
          break;
        default:
          THROW("Unsupport type");
          break;
      }
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      switch (type_) {
        case TINYGLTF_TYPE_SCALAR:
          format = vk::Format::eR32Uint;
          break;
        case TINYGLTF_TYPE_VEC2:
          format = vk::Format::eR32G32Uint;
          break;
        case TINYGLTF_TYPE_VEC3:
          format = vk::Format::eR32G32B32Uint;
          break;
        case TINYGLTF_TYPE_VEC4:
          format = vk::Format::eR32G32B32A32Uint;
          break;
        default:
          THROW("Unsupport type");
          break;
      }
      break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      switch (type_) {
        case TINYGLTF_TYPE_SCALAR:
          format = vk::Format::eR32Sfloat;
          break;
        case TINYGLTF_TYPE_VEC2:
          format = vk::Format::eR32G32Sfloat;
          break;
        case TINYGLTF_TYPE_VEC3:
          format = vk::Format::eR32G32B32Sfloat;
          break;
        case TINYGLTF_TYPE_VEC4:
          format = vk::Format::eR32G32B32A32Sfloat;
          break;
        default:
          THROW("Unsupport type");
          break;
      }
      break;
    default:
      THROW("Unsupport component type");
      break;
  }

  return format;
}

}  // namespace ast::sc

}  // namespace luka