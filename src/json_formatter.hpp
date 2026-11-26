#ifndef JSON_FORMATTER_HPP_
#define JSON_FORMATTER_HPP_

#include <fstream>
#include <string>
#include <vector>

#include "decree.hpp"
#include "json.hpp"
#include "configuration_format_concept.hpp"

template<HasConfigurationType ConfigurationFormat>
struct JsonFormatter {
    [[nodiscard]] auto parseConfiguration(std::ifstream& streamHandle) -> ReturnType<ConfigurationFormat::ConfigurationType>;
    [[nodiscard]] auto parseConfiguration(std::istream& streamHandle) -> ReturnType<ConfigurationFormat::ConfigurationType>;
    [[nodiscard]] auto writeConfiguration(std::ofstream& out, const ConfigurationFormat::ConfigurationType& configuration) -> ReturnType<void>;
}

template<HasConfigurationType ConfigurationFormat>
auto JsonFormatter<ConfigurationFormat>::parseConfiguration(std::ifstream& streamHandle) -> ReturnType<ConfigurationFormat::ConfigurationType> {
    return parseConfiguration(static_cast<std::istream&>(streamHandle));
}

template<HasConfigurationType ConfigurationFormat>
auto JsonFormatter<ConfigurationFormat>::parseConfiguration(std::istream& streamHandle) -> ReturnType<ConfigurationFormat::ConfigurationType> {
    nlohmann::json jsonFile;

    try {
        streamHandle >> jsonFile;

        return jsonFile.get<ConfigurationFormat::ConfigurationType>();
    } catch (const std::exception& exception) {
        return Decree::makeError(EError::eParseFailed, "Exception occurred during configuration parsing: " + std::string{ exception.what() });
    }
}

template<HasConfigurationType ConfigurationFormat>
auto JsonFormatter<ConfigurationFormat>::writeConfiguration(std::ofstream& out, const ConfigurationFormat::ConfigurationType& configuration) -> ReturnType<void> {
    nlohmann::json j;

    try {
        j = configuration;

        static constexpr auto prettyPrintStepWidth = 4;
        out << std::setw(prettyPrintStepWidth) << j;

        return {};
    } catch (const std::exception& exception) {
        return Decree::makeError(EError::eWriteFailed, "Exception occurred during writing configuration: " + std::string{ exception.what() });
    }
}

#endif /* JSON_FORMATTER_HPP_ */