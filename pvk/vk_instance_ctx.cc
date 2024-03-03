#include <optional>
#include <utility>

#include <pvk/vk_instance_ctx.hh>

#include "vk_instance_ctx_impl.hh"

namespace pvk {

std::optional<InstanceContext> InstanceContext::create() noexcept
{
    std::optional<InstanceContext::Impl> impl = InstanceContext::Impl::create();
    if (!impl) {
        return std::nullopt;
    }

    InstanceContext output;
    new (output.impl) InstanceContext::Impl(std::move(*impl));
    return output;
}

InstanceContext::InstanceContext(InstanceContext &&other) noexcept
{
    auto &other_impl = InstanceContext::Impl::cast_from(other.impl);
    new (impl) InstanceContext::Impl(std::move(other_impl));
}

InstanceContext &InstanceContext::operator=(InstanceContext &&other) noexcept
{
    std::swap(this->impl, other.impl);
    return *this;
}

InstanceContext::~InstanceContext()
{
    Impl::cast_from(impl).~Impl();
}

} // namespace pvk
