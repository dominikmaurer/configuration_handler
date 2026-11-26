#ifndef CONFIGURATION_FORMAT_CONCEPT_HPP_
#define CONFIGURATION_FORMAT_CONCEPT_HPP_

#include <concepts>
#include <fstream>
#include <type_traits>

#include "decree.hpp"

template<typename T>
concept HasConfigurationType = requires {
    typename T::ConfigurationType;
};

template<typename Formatter, HasConfigurationType Format>
concept CConfigurationFormatter = requires (
    Formatter formatter,
    std::ifstream& ifstream,
    std::istream& istream,
    std::ofstream& writeStream,
    const typename Format::ConfigurationType& configuration
) {
    typename Formatter::EError;

    { formatter.parseConfiguration(ifstream) } -> std::same_as<Decree::ErrorResult<typename Format::ConfigurationType, typename Formatter::EError>>;
    { formatter.parseConfiguration(istream) } -> std::same_as<Decree::ErrorResult<typename Format::ConfigurationType, typename Formatter::EError>>;
    { formatter.writeConfiguration(writeStream, configuration) } -> std::same_as<Decree::ErrorResult<void, typename Formatter::EError>>;
};

#endif /* CONFIGURATION_FORMAT_CONCEPT_HPP_ */
