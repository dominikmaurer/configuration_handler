#ifndef CONFIGURATION_FORMAT_CONCEPT_HPP_
#define CONFIGURATION_FORMAT_CONCEPT_HPP_

#include <concepts>
#include <fstream>
#include <type_traits>

#include "decree.hpp"

template<typename EErrorCodes>
concept CConfigurationErrorCode = std::is_enum_v<EErrorCodes> && requires {
    { EErrorCodes::eOpenConfigurationFailed };
};

template<typename T>
concept HasConfigurationType = requires {
    typename T::ConfigurationType;
};

template<typename Formatter, HasConfigurationType Format, typename EErrorCodes>
concept CConfigurationFormatter = CConfigurationErrorCode<EErrorCodes> && requires (
    Formatter formatter,
    std::ifstream& ifstream,
    std::istream& istream,
    std::ofstream& writeStream,
    const typename Format::ConfigurationType& configuration
) {
    { formatter.parseConfiguration(ifstream) } -> std::same_as<Decree::ErrorResult<typename Format::ConfigurationType, EErrorCodes>>;
    { formatter.parseConfiguration(istream) } -> std::same_as<Decree::ErrorResult<typename Format::ConfigurationType, EErrorCodes>>;
    { formatter.writeConfiguration(writeStream, configuration) } -> std::same_as<Decree::ErrorResult<void, EErrorCodes>>;
};

#endif /* CONFIGURATION_FORMAT_CONCEPT_HPP_ */
