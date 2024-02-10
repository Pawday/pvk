#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <cstddef>
#include <cstring>

#include "pvk/dso_loader.hh"
#include "pvk/vk_api.hh"
#include "pvk/vk_loader.hh"

namespace {
bool glad_version_is_not_sane(int ver_maj, int ver_min)
{
    if (ver_maj < 0 || ver_min < 0) {
        char8_t fuck_msg_u8[] = u8"Ебаный GLAD, какого хуя блять?\n";
        std::vector<char> fuck_message(sizeof(fuck_msg_u8));
        std::memcpy(fuck_message.data(), fuck_msg_u8, sizeof(fuck_msg_u8));
        std::cerr << fuck_message.data();
        return true;
    }
    return false;
}

static GLADapiproc load_vk_proc(void *library, const char *vk_proc_name)
{
    SymLoader *vk_library = reinterpret_cast<SymLoader *>(library);

    auto proc_addres = vk_library->load_sym(vk_proc_name);
    if (!proc_addres) {
        const std::string err_msg =
            std::format("Fail to load Vulkan function {}\n", vk_proc_name);
        std::cerr << err_msg;
        return NULL;
    }

    if (*proc_addres == NULL) {
        std::cerr << std::format(
            "Well, Vulkan vendor by whatever fucking reason desided "
            "to put !!valid!! Vulkan function \"{0}\" at addres 0 "
            "(!!ZERO!!) "
            "current loader is not able to handle that so we assume there "
            "is "
            "no \"{0}\" symbol (FUCK YOU VENDOR)\n",
            vk_proc_name
        );
        return NULL;
    }
    return reinterpret_cast<GLADapiproc>(*proc_addres);
}
} // namespace

std::optional<VKLoader> VKLoader::load(const std::string &library) noexcept
{
    VKLoader output;

    std::optional<SymLoader> vk_library = SymLoader::load(library);
    if (!vk_library) {
        std::cerr << std::format(
            "Load shared Vulkan library \"{}\" failed\n", library
        );
        return std::nullopt;
    }

    int vulkan_version = gladLoadVulkanUserPtr(NULL, load_vk_proc, &vk_library);
    if (vulkan_version == 0) {
        std::cerr << std::format("Loading Vulkan from {} failed\n", library);
        return std::nullopt;
    }

    int vulkan_version_maj = GLAD_VERSION_MAJOR(vulkan_version);
    int vulkan_version_min = GLAD_VERSION_MINOR(vulkan_version);
    if (glad_version_is_not_sane(vulkan_version_maj, vulkan_version_min)) {
        return std::nullopt;
    }

    VKVersion version;

    if (std::numeric_limits<VKVersion::Major_t>::max() < vulkan_version_maj) {
        std::cerr << std::format(
            "Actual major version of vulkan is {}\n", vulkan_version_maj
        );
        version.major = std::numeric_limits<VKVersion::Major_t>::max();
    } else {
        version.major = vulkan_version_maj;
    }

    if (std::numeric_limits<VKVersion::Minor_t>::max() < vulkan_version_min) {
        std::cerr << std::format(
            "Actual major version of vulkan is {}\n", vulkan_version_maj
        );
        version.minor = std::numeric_limits<VKVersion::Minor_t>::max();
    } else {
        version.major = vulkan_version_min;
    }

    output.m_vk_library = std::move(vk_library);
    output.m_version = version;
    return output;
}

VKLoader::~VKLoader() noexcept
{
    /*
     * gladUnloadVulkanUserPtr or something
     */
}
