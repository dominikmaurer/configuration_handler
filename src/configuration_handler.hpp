#ifndef CONFIGURATION_HANDLER_HPP_
#define CONFIGURATION_HANDLER_HPP_

#include <fstream>
#include <filesystem>

#include "configuration_format_concept.hpp"
#include "decree.hpp"

template<typename ConfigurationFormatter, typename ConfigurationFormat>
requires CConfigurationFormatter<ConfigurationFormatter, ConfigurationFormat>
class ConfigurationHandler {
public:
    enum class EError {
        eOpenConfigurationFailed,
        eFormatterParseFailed,
        eFormatterWriteFailed
    };
    template<typename T>
    using ReturnType = Decree::ErrorResult<T, EError>;

    ConfigurationHandler() = default;

    [[nodiscard]] auto load(const std::filesystem::path& configurationPath) -> ReturnType<typename ConfigurationFormat::ConfigurationType>;
    [[nodiscard]] auto update(const std::filesystem::path& configurationPath, const typename ConfigurationFormat::ConfigurationType& configuration) -> ReturnType<void>;
    [[nodiscard]] auto getFormatter() const -> ConfigurationFormatter&;

private:
    ConfigurationFormatter m_formatter{};
};

template<typename ConfigurationFormatter, typename ConfigurationFormat>
requires CConfigurationFormatter<ConfigurationFormatter, ConfigurationFormat>
auto ConfigurationHandler<ConfigurationFormatter, ConfigurationFormat>::load(const std::filesystem::path& configurationPath) -> ReturnType<typename ConfigurationFormat::ConfigurationType> {
    std::ifstream configurationFile{ configurationPath };
    if (!configurationFile.is_open()) {
        return Decree::makeError(EError::eOpenConfigurationFailed, "Failed to open config file: " + configurationPath.string());
    }

    const auto formatterResult = m_formatter.parseConfiguration(configurationFile);
    if(!formatterResult.has_value()) {
        return Decree::makeError(EError::eFormatterParseFailed, formatterResult.error().getErrorMessage());
    }

    return formatterResult.value();
}

template<typename ConfigurationFormatter, typename ConfigurationFormat>
requires CConfigurationFormatter<ConfigurationFormatter, ConfigurationFormat>
auto ConfigurationHandler<ConfigurationFormatter, ConfigurationFormat>::update(const std::filesystem::path& configurationPath, const typename ConfigurationFormat::ConfigurationType& configuration) -> ReturnType<void> {
    if (!configurationPath.parent_path().empty()) {
        std::filesystem::create_directories(configurationPath.parent_path());
    }

    std::ofstream configurationFile{ configurationPath };
    if (!configurationFile.is_open()) {
        return Decree::makeError(EError::eOpenConfigurationFailed, "Failed to open config file: " + configurationPath.string());
    }

    if(const auto formatterResult = m_formatter.writeConfiguration(configurationFile, configuration); !formatterResult.has_value()) {
        return Decree::makeError(EError::eFormatterWriteFailed, formatterResult.error().getErrorMessage());
    }

    return {};
}

template<typename ConfigurationFormatter, typename ConfigurationFormat>
requires CConfigurationFormatter<ConfigurationFormatter, ConfigurationFormat>
auto ConfigurationHandler<ConfigurationFormatter, ConfigurationFormat>::getFormatter() const -> ConfigurationFormatter& {
    return m_formatter;
}

#endif /* CONFIGURATION_HANDLER_HPP_ */
