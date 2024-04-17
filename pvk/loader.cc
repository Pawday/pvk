#include <algorithm>
#include <format>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <utility>

#include <cstddef>
#include <cstring>

#include "pvk/dso_loader.hh"
#include "pvk/vk_api.hh"
#include "pvk/vk_loader.hh"

#include "pvk/log.hh"

namespace {
bool glad_version_is_not_sane(int ver_maj, int ver_min)
{
    if (ver_maj < 0 || ver_min < 0) {
        pvk::fatal(
            "{}",
            "GLAD loader returned negative vulkan version, maybe its time to "
            "write a proper loader");
        return true;
    }
    return false;
}

static GLADapiproc load_vk_proc(void *library, const char *vk_proc_name)
{
    pvk::SymLoader *vk_library = reinterpret_cast<pvk::SymLoader *>(library);

    auto proc_addres = vk_library->load_sym(vk_proc_name);
    if (!proc_addres) {
        pvk::error("Fail to load Vulkan function {}\n", vk_proc_name);
        return NULL;
    }

    if (*proc_addres == NULL) {
        pvk::error(
            "Well, Vulkan vendor by whatever reason desided "
            "to put !!valid!! Vulkan function \"{0}\" at addres 0 "
            "(!!ZERO!!) "
            "current loader is not able to handle that so we assume there "
            "is "
            "no \"{0}\" symbol (FUCK YOU VENDOR)",
            vk_proc_name);
        return NULL;
    }
    return reinterpret_cast<GLADapiproc>(*proc_addres);
}
} // namespace

namespace pvk {

std::optional<Loader> Loader::load(const std::string &library) noexcept
{
    Loader output;

    std::optional<SymLoader> vk_library = SymLoader::load(library);
    if (!vk_library) {
        pvk::warning("Load shared Vulkan library \"{}\" failed", library);
        return std::nullopt;
    }

    int vulkan_version = gladLoadVulkanUserPtr(NULL, load_vk_proc, &vk_library);
    if (vulkan_version == 0) {
        pvk::error("Loading Vulkan from {} failed\n", library);
        return std::nullopt;
    }

    int vulkan_version_maj = GLAD_VERSION_MAJOR(vulkan_version);
    int vulkan_version_min = GLAD_VERSION_MINOR(vulkan_version);
    if (glad_version_is_not_sane(vulkan_version_maj, vulkan_version_min)) {
        return std::nullopt;
    }

    VKVersion version;

    if (std::numeric_limits<VKVersion::Major_t>::max() < vulkan_version_maj) {
        pvk::warning(
            "Actual major version of vulkan is {}", vulkan_version_maj);
        version.major = std::numeric_limits<VKVersion::Major_t>::max();
    } else {
        version.major = vulkan_version_maj;
    }

    if (std::numeric_limits<VKVersion::Minor_t>::max() < vulkan_version_min) {
        pvk::warning(
            "Actual minor version of vulkan is {}", vulkan_version_min);
        version.minor = std::numeric_limits<VKVersion::Minor_t>::max();
    } else {
        version.minor = vulkan_version_min;
    }

    output.m_vk_library = std::move(vk_library);
    output.m_version = version;
    return output;
}

Loader::~Loader() noexcept
{
    /*
     * gladUnloadVulkanUserPtr or something
     */
}
} // namespace pvk
