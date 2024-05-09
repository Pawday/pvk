#include <memory>

#include <cstddef>

#include <new>
#include <optional>
#include <pvk/pipeline.hh>
#include <utility>
#include <vector>

#include <pvk/device.hh>
#include <pvk/pipeline.hh>

#include "pvk/log.hh"
#include "pvk/logger.hh"

#include "pvk/internal/vk_allocator.hh"
#include "pvk/internal/vk_api.hh"

namespace pvk {

struct Pipeline::Impl
{
    Impl(std::shared_ptr<Device> device_context) noexcept
        : m_device_context(device_context)
    {
        l.set_name("Pipeline");
        m_allocator = std::make_unique<Allocator>();
    }

    Impl(Impl &&o)
        : m_shaders(std::move(o.m_shaders)),
          m_device_context(std::move(o.m_device_context)),
          m_pipeline(o.m_pipeline), m_allocator(std::move(o.m_allocator)),
          l(std::move(o.l))
    {
    }

    ~Impl() noexcept
    {
        if (m_pipeline == VK_NULL_HANDLE) {
            l.debug("Destroy unconfigured");
            return;
        }
    }

    bool configure()
    {
        if (m_shaders.size() == 0) {
            l.warning("No single shader attached");
            return false;
        }

        return true;
    }

    static Impl &cast_from(std::byte *data)
    {
        return *std::launder(reinterpret_cast<Impl *>(data));
    }

    static Impl const &cast_from(std::byte const *data)
    {
        return *std::launder(reinterpret_cast<Impl const *>(data));
    }

    static bool assert_size()
    {
        static_assert(sizeof(Impl) < Pipeline::impl_size);
        return true;
    }

  private:
    std::vector<VkShaderModule> m_shaders;
    std::shared_ptr<Device> m_device_context;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    std::unique_ptr<Allocator> m_allocator = nullptr;
    Logger l;
};

Pipeline::Pipeline(Impl &&impl_obj) noexcept
{
    new (impl) Impl(std::move(impl_obj));
}

std::optional<Pipeline>
    Pipeline::create(std::shared_ptr<Device> device_context) noexcept
{
    if (device_context == nullptr) {
        pvk::error("Pipeline::create called with nullptr");
        return std::nullopt;
    }

    if (!device_context->connected()) {
        pvk::error("Pipeline::create called with unconnected deviceContext");
        return std::nullopt;
    }

    return Pipeline(Impl(device_context));
}

Pipeline::Pipeline(Pipeline &&o) noexcept
{
    new (impl) Impl(std::move(Impl::cast_from(o.impl)));
}

Pipeline::~Pipeline() noexcept
{
    Impl::cast_from(impl).~Impl();
}

} // namespace pvk
