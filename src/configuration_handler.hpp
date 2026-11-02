#ifndef CONFIGURATION_HANDLER_HPP_
#define CONFIGURATION_HANDLER_HPP_

#include <fstream>
#include <filesystem>

#include "configuration_format_concept.hpp"
#include "decree.hpp"

template<typename ConfigurationFormatter, typename EErrorCodes>
requires CConfigurationFormat<ConfigurationFormatter, EErrorCodes>
class ConfigurationHandler {
public:
    template<typename T>
    using ReturnType = Decree::ErrorResult<T, EErrorCodes>;

    ConfigurationHandler() = default;

    [[nodiscard]] ReturnType<typename ConfigurationFormatter::ConfigurationType> load(const std::filesystem::path& configurationPath);
    [[nodiscard]] ReturnType<void> update(const std::filesystem::path& configurationPath, const typename ConfigurationFormatter::ConfigurationType& configuration);

private:
    ConfigurationFormatter m_formatter{};
};

template<typename ConfigurationFormatter, typename EErrorCodes>
requires CConfigurationFormat<ConfigurationFormatter, EErrorCodes>
auto ConfigurationHandler<ConfigurationFormatter, EErrorCodes>::load(const std::filesystem::path& configurationPath) -> ReturnType<typename ConfigurationFormatter::ConfigurationType> {
    std::ifstream configurationFile{ configurationPath };
    if (!configurationFile.is_open()) {
        return Decree::makeError(EErrorCodes::eOpenConfigurationFailed, "Failed to open config file: " + configurationPath.string());
    }

    return m_formatter.parseConfiguration(configurationFile);
}

template<typename ConfigurationFormatter, typename EErrorCodes>
requires CConfigurationFormat<ConfigurationFormatter, EErrorCodes>
auto ConfigurationHandler<ConfigurationFormatter, EErrorCodes>::update(const std::filesystem::path& configurationPath, const typename ConfigurationFormatter::ConfigurationType& configuration) -> ReturnType<void> {
    if (!configurationPath.parent_path().empty()) {
        std::filesystem::create_directories(configurationPath.parent_path());
    }

    std::ofstream configurationFile{ configurationPath };
    if (!configurationFile.is_open()) {
        return Decree::makeError(EErrorCodes::eOpenConfigurationFailed, "Failed to open config file: " + configurationPath.string());
    }

    return m_formatter.writeConfiguration(configurationFile, configuration);
}

#endif /* CONFIGURATION_HANDLER_HPP_ */
