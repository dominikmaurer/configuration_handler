# Configuration Handler

A header-only C++23 library for loading and updating configuration files with a configurable formatter.

## Main headers

- `configuration_handler.hpp` provides `ConfigurationHandler<ConfigurationFormatter, ConfigurationFormat>`.
- `configuration_format_concept.hpp` provides `HasConfigurationType` and `CConfigurationFormatter`.
- `formatters/json_formatter/json_formatter.hpp` provides `JsonFormatter<ConfigurationFormat>` with the bundled single-header nlohmann/json implementation.

The CMake target is `json::configuration_handler`. It exposes the `src` and `formatters/json_formatter` directories and links `decree::decree` transitively.

## Formatter concept

```cpp
template<typename T>
concept HasConfigurationType = requires {
    typename T::ConfigurationType;
};

template<typename Formatter, typename Format>
concept CConfigurationFormatter = HasConfigurationType<Format> && requires (
    Formatter formatter,
    std::ifstream& ifstream,
    std::istream& istream,
    std::ofstream& writeStream,
    const typename Format::ConfigurationType& configuration
) {
    typename Formatter::EError;

    { formatter.parseConfiguration(ifstream) }
        -> std::same_as<Decree::ErrorResult<typename Format::ConfigurationType, typename Formatter::EError>>;
    { formatter.parseConfiguration(istream) }
        -> std::same_as<Decree::ErrorResult<typename Format::ConfigurationType, typename Formatter::EError>>;
    { formatter.writeConfiguration(writeStream, configuration) }
        -> std::same_as<Decree::ErrorResult<void, typename Formatter::EError>>;
};
```

A formatter must define `EError` and a nested `ConfigurationType` on the format type:

```cpp
struct AppFormat {
    struct ConfigurationType {
        std::string host;
        int port = 0;
    };
};
```

## ConfigurationHandler

`ConfigurationHandler` opens files and delegates parsing and writing to the formatter:

```cpp
#include "configuration_handler.hpp"
#include "configuration_format_concept.hpp"
#include "json_formatter.hpp"

ConfigurationHandler<JsonFormatter<AppFormat>, AppFormat> handler;

const auto config = handler.load("app.json");
if(!config) {
    // Handle config.error().
}

const auto writeResult = handler.update(
    "app.json",
    AppFormat::ConfigurationType{ "localhost", 8080 }
);
```

`load` returns `Decree::ErrorResult<ConfigurationType, ConfigurationHandler::EError>`. `update` returns `Decree::ErrorResult<void, ConfigurationHandler::EError>`.

## JSON formatter

The JSON formatter returns `JsonFormatter<ConfigurationFormat>::EError` with two possible values:

- `eParseFailed`
- `eWriteFailed`

The configuration type must provide nlohmann/json conversion support, for example with `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE`:

```cpp
#include "json.hpp"

struct AppFormat {
    struct ConfigurationType {
        std::string host;
        int port = 0;
    };
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    AppFormat::ConfigurationType,
    host,
    port
);

static_assert(
    CConfigurationFormatter<
        JsonFormatter<AppFormat>,
        AppFormat
    >
);
```

## Custom formatter

```cpp
struct IniFormatter {
    enum class EError {
        eParseFailed,
        eWriteFailed
    };

    using ConfigurationType = AppFormat::ConfigurationType;

    Decree::ErrorResult<ConfigurationType, EError>
    parseConfiguration(std::ifstream& stream) {
        // Parse the stream.
        return ConfigurationType{};
    }

    Decree::ErrorResult<ConfigurationType, EError>
    parseConfiguration(std::istream& stream) {
        // Parse the stream.
        return ConfigurationType{};
    }

    Decree::ErrorResult<void, EError>
    writeConfiguration(std::ofstream& stream, const ConfigurationType& configuration) {
        // Write the stream.
        return {};
    }
};

static_assert(CConfigurationFormatter<IniFormatter, AppFormat>);
```

## Build

Prerequisites:

- CMake 3.28 or newer
- A C++23 compiler
- Ninja or another CMake-supported build tool

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

With tests:

```sh
cmake -S . -B build -G Ninja -DCONFIGURATION_HANDLER_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Consume the library through CMake:

```cmake
target_link_libraries(my_target PRIVATE json::configuration_handler)
```
