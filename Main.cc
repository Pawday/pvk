#include <cstdlib>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "pvk/vk_instance_ctx.hh"
#include "pvk/vk_loader.hh"

using namespace pvk;

std::optional<Loader> load_vulkan_linux(int argc, char const *const *argv)
{
    std::string_view vk_so_linux_default_path("/usr/lib/libvulkan.so");
    std::string vk_so_linux_path;
    if (argc < 2) {
        vk_so_linux_path = vk_so_linux_default_path;
    } else {
        vk_so_linux_path = argv[1];
        std::cout << std::format(
            "[INFO]: Loading alternative Vulkan library from \"{}\"\n",
            vk_so_linux_path
        );
    }
    return Loader::load(vk_so_linux_path);
}

std::optional<Loader> load_vulkan_win32(int argc, char const *const *argv)
{
    (void)argc;
    (void)argv;
    return Loader::load("vulkan-1");
}

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

    bool is_not_valid() const
    {
        if (!*this) {
            return true;
        }
        return false;
    }

  private:
    std::optional<InstanceContext> m_vk_context;
};

Application::Application()
{
    m_vk_context = InstanceContext::create();
    if (!m_vk_context) {
        return;
    }
}

int main(int argc, char **argv)
{
    std::optional<Loader> vk_loader_ctx = load_vulkan_linux(argc, argv);
    // std::optional<Loader> vk_loader_ctx = load_vulkan_win32(argc, argv);
    if (!vk_loader_ctx) {
        std::cerr << "Can not use Vulkan\n";
        return EXIT_FAILURE;
    }

    Loader::VKVersion version = vk_loader_ctx->get_version();
    std::cout << std::format(
        "Loaded Vulkan {}.{}\n", version.major, version.minor
    );

    Application app;
    if (!app) {
        std::cerr << "App init failue\n";
        return EXIT_FAILURE;
    }

    return 0;
}
