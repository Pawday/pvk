#include <algorithm>
#include <cstdlib>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <pvk/device_context.hh>
#include <pvk/instance_context.hh>

#include "pvk/physical_device.hh"
#include "pvk/vk_loader.hh"

using namespace pvk;

namespace {
static std::optional<Loader> load_vulkan(int argc, char const *const *argv)
{

#if defined(USE_WINDOWS_VULKAN)
    constexpr std::string_view vulkan_dso_default("vulkan-1");
#else
    constexpr std::string_view vulkan_dso_default("libvulkan.so");
#endif

    std::string vk_dso_location;
    if (argc < 2) {
        vk_dso_location = vulkan_dso_default;
    } else {
        vk_dso_location = argv[1];
        std::cout << std::format(
            "[INFO]: Loading alternative Vulkan library from \"{}\"\n",
            vk_dso_location);
    }
    return Loader::load(vk_dso_location);
}

static std::string device_type_to_str(pvk::DeviceType t)
{
    switch (t) {
    case DeviceType::GPU:
        return "GPU";
    case DeviceType::CPU:
        return "CPU";
    case DeviceType::UNKNOWN:
        return "UNKNOWN";
    }
}

} // namespace

struct Application
{
    Application(const Application &) = delete;
    Application(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application &operator=(Application &&) = delete;

    Application();

    operator bool() const
    {
        if (!m_vk_context) {
            return false;
        }
        return true;
    }

    void log_devices()
    {
        auto log_device = [](DeviceContext &device) {
            std::cout << std::format(
                "Found {} device \"{}\"\n",
                device_type_to_str(device.get_device_type()),
                device.get_name());
        };
        std::ranges::for_each(devices, log_device);
    }

  private:
    std::optional<InstanceContext> m_vk_context;

    std::vector<DeviceContext> devices;
};

Application::Application()
{
    m_vk_context = InstanceContext::create();
    if (!m_vk_context) {
        return;
    }

    std::vector<PhysicalDevice> dev_handles = m_vk_context->get_devices();
    for (PhysicalDevice &device : dev_handles) {
        auto device_context = DeviceContext::create(device);
        if (!device_context) {
            continue;
        }
        this->devices.emplace_back(std::move(*device_context));
    }

    for (auto &device : devices) {
        device.connect();
    }
}

int main(int argc, char **argv)
{
    std::optional<Loader> vk_loader_ctx = load_vulkan(argc, argv);
    if (!vk_loader_ctx) {
        std::cerr << "Can not use Vulkan\n";
        return EXIT_FAILURE;
    }

    Loader::VKVersion version = vk_loader_ctx->get_version();
    std::cout << std::format(
        "Loaded Vulkan {}.{}\n", version.major, version.minor);

    Application app;
    if (!app) {
        std::cerr << "App init failue\n";
        return EXIT_FAILURE;
    }

    app.log_devices();

    return EXIT_SUCCESS;
}
