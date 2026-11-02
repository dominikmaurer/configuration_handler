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

template<typename Parser, typename EErrorCodes>
concept CConfigurationFormat = CConfigurationErrorCode<EErrorCodes> && requires (Parser parser, std::ifstream& stream, std::ofstream& writeStream, const typename Parser::ConfigurationType& configuration) {
    typename Parser::ConfigurationType;
    { parser.parseConfiguration(stream) } -> std::same_as<Decree::ErrorResult<typename Parser::ConfigurationType, EErrorCodes>>;
    { parser.writeConfiguration(writeStream, configuration) } -> std::same_as<Decree::ErrorResult<void, EErrorCodes>>;
};

#endif /* CONFIGURATION_FORMAT_CONCEPT_HPP_ */
