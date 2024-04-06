#include <algorithm>
#include <cstdlib>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <pvk/device_context.hh>
#include <pvk/instance_context.hh>

#include <pvk/vk_loader.hh>

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

    return "UNHANDLED";
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

    size_t devices_count = m_vk_context->get_device_count();

    std::cout << std::format("Found {} devices\n", devices_count);

    for (size_t device_idx = 0; device_idx < devices_count; device_idx++) {
        std::optional<DeviceContext> device_ctx =
            m_vk_context->get_device(device_idx);

        std::cout << std::format(
            "Device #{} - {} ({})\n",
            device_idx,
            device_ctx->get_name(),
            device_type_to_str(device_ctx->get_device_type()));
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
