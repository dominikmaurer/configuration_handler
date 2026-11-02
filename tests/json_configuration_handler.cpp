#include "configuration_handler.hpp"
#include "configuration_format_concept.hpp"
#include "decree.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <expected>

enum class ETestErrorCode {
    eOpenConfigurationFailed,
    eParseError,
    eWriteError
};

using TestError = ETestErrorCode;

struct PlainTextFormatter {
    struct ConfigurationType {
        std::string content;
    };

    Decree::ErrorResult<ConfigurationType, TestError> parseConfiguration(std::ifstream& stream) {
        return ConfigurationType{ std::string{ std::istreambuf_iterator<char>{ stream }, std::istreambuf_iterator<char>{} } };
    }

    Decree::ErrorResult<void, TestError> writeConfiguration(std::ofstream& stream, const ConfigurationType& config) {
        stream << config.content;

        return {};
    }
};

static_assert(CConfigurationFormat<PlainTextFormatter, TestError>);

struct FailingParseFormatter {
    using ConfigurationType = PlainTextFormatter::ConfigurationType;

    Decree::ErrorResult<ConfigurationType, TestError> parseConfiguration(std::ifstream&) {
        return Decree::makeError(TestError::eParseError, "forced parse error");
    }

    Decree::ErrorResult<void, TestError> writeConfiguration(std::ofstream& stream, const ConfigurationType& config) {
        stream << config.content;

        return {};
    }
};

struct FailingWriteFormatter {
    using ConfigurationType = PlainTextFormatter::ConfigurationType;

    Decree::ErrorResult<ConfigurationType, TestError> parseConfiguration(std::ifstream& stream) {
        return ConfigurationType{ std::string{ std::istreambuf_iterator<char>{ stream }, std::istreambuf_iterator<char>{} } };
    }

    Decree::ErrorResult<void, TestError> writeConfiguration(std::ofstream&, const ConfigurationType&) {
        return Decree::makeError(TestError::eWriteError, "forced write error");
    }
};

struct SimpleJsonFormatter {
    struct ConfigurationType {
        std::string name;
        std::string version;
        std::string outputPath;
    };

    Decree::ErrorResult<ConfigurationType, TestError> parseConfiguration(std::ifstream& stream) {
        const std::string json{ std::istreambuf_iterator<char>{ stream }, std::istreambuf_iterator<char>{} };

        ConfigurationType config;
        config.name = extractStringValue(json, "name");
        config.version = extractStringValue(json, "version");
        config.outputPath = extractStringValue(json, "outputPath");

        if(config.name.empty()) {
            return Decree::makeError(TestError::eParseError, "missing 'name' field");
        }

        return config;
    }

    Decree::ErrorResult<void, TestError> writeConfiguration(std::ofstream& stream, const ConfigurationType& config) {
        stream << "{\n"
               << "    \"name\": \"" << config.name << "\",\n"
               << "    \"version\": \"" << config.version << "\",\n"
               << "    \"outputPath\": \"" << config.outputPath << "\"\n"
               << "}\n";

        return {};
    }

private:
    static std::string extractStringValue(const std::string& json, const std::string& key) {
        const auto keyPos = json.find("\"" + key + "\"");
        if(keyPos == std::string::npos) {
            return {};
        }

        const auto colonPos = json.find(':', keyPos);
        if(colonPos == std::string::npos) {
            return {};
        }

        const auto openQuote = json.find('"', colonPos + 1);
        if(openQuote == std::string::npos) {
            return {};
        }

        const auto closeQuote = json.find('"', openQuote + 1);
        if(closeQuote == std::string::npos) {
            return {};
        }

        return json.substr(openQuote + 1, closeQuote - openQuote - 1);
    }
};

static_assert(CConfigurationFormat<SimpleJsonFormatter, TestError>);

class LoadTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_path = std::filesystem::temp_directory_path() / "json_config_handler_load_test.txt";
        std::filesystem::remove(m_path);
    }

    void TearDown() override {
        std::filesystem::remove(m_path);
    }

    std::filesystem::path m_path;
};

TEST_F(LoadTest, ReturnsValueWhenFileExists) {
    static constexpr const char* s_content = "hello world";
    { std::ofstream f{ m_path }; f << s_content; }

    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    const auto result = handler.load(m_path);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->content, s_content);
}

TEST_F(LoadTest, ReturnsErrorWhenFileNotFound) {
    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    const auto result = handler.load(m_path);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().getErrorCode(), TestError::eOpenConfigurationFailed);
}

TEST_F(LoadTest, ErrorMessageContainsMissingPath) {
    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    const auto result = handler.load(m_path);

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().getErrorMessage().find(m_path.string()), std::string_view::npos);
}

TEST_F(LoadTest, PropagatesFormatterParseError) {
    { std::ofstream f{ m_path }; f << "some content"; }

    ConfigurationHandler<FailingParseFormatter, TestError> handler;
    const auto result = handler.load(m_path);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().getErrorCode(), TestError::eParseError);
}

TEST_F(LoadTest, ReturnsEmptyContentForEmptyFile) {
    { std::ofstream f{ m_path }; }

    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    const auto result = handler.load(m_path);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->content.empty());
}

class UpdateTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_path = std::filesystem::temp_directory_path() / "json_config_handler_update_test.txt";
        std::filesystem::remove(m_path);
    }

    void TearDown() override {
        std::filesystem::remove(m_path);
    }

    std::filesystem::path m_path;
};

TEST_F(UpdateTest, CreatesFileWithContent) {
    static constexpr const char* s_content = "written content";

    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    const auto result = handler.update(m_path, PlainTextFormatter::ConfigurationType{ s_content });

    ASSERT_TRUE(result.has_value());

    std::ifstream file{ m_path };
    const std::string readBack{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
    EXPECT_EQ(readBack, s_content);
}

TEST_F(UpdateTest, OverwritesExistingFile) {
    { std::ofstream f{ m_path }; f << "old content"; }

    static constexpr const char* s_newContent = "new content";
    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    const auto result = handler.update(m_path, PlainTextFormatter::ConfigurationType{ s_newContent });

    ASSERT_TRUE(result.has_value());

    std::ifstream file{ m_path };
    const std::string readBack{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
    EXPECT_EQ(readBack, s_newContent);
}

TEST_F(UpdateTest, CreatesParentDirectories) {
    const auto deepPath = std::filesystem::temp_directory_path() / "json_test_nested_dir" / "sub" / "config.txt";

    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    const auto result = handler.update(deepPath, PlainTextFormatter::ConfigurationType{ "data" });

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(std::filesystem::exists(deepPath));

    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "json_test_nested_dir");
}

TEST_F(UpdateTest, PropagatesFormatterWriteError) {
    ConfigurationHandler<FailingWriteFormatter, TestError> handler;
    const auto result = handler.update(m_path, FailingWriteFormatter::ConfigurationType{ "data" });

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().getErrorCode(), TestError::eWriteError);
}

class RoundTripTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_path = std::filesystem::temp_directory_path() / "json_config_handler_roundtrip_test.txt";
        std::filesystem::remove(m_path);
    }

    void TearDown() override {
        std::filesystem::remove(m_path);
    }

    std::filesystem::path m_path;
};

TEST_F(RoundTripTest, LoadAfterUpdateReturnsOriginalContent) {
    static constexpr const char* s_content = "round trip content";

    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    ASSERT_TRUE(handler.update(m_path, PlainTextFormatter::ConfigurationType{ s_content }).has_value());

    const auto result = handler.load(m_path);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->content, s_content);
}

TEST_F(RoundTripTest, MultipleUpdatesRetainLastContent) {
    ConfigurationHandler<PlainTextFormatter, TestError> handler;
    ASSERT_TRUE(handler.update(m_path, PlainTextFormatter::ConfigurationType{ "first" }).has_value());
    ASSERT_TRUE(handler.update(m_path, PlainTextFormatter::ConfigurationType{ "second" }).has_value());

    const auto result = handler.load(m_path);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->content, "second");
}

TEST(CConfigurationFormatConceptTest, PlainTextFormatterSatisfiesConcept) {
    static_assert(CConfigurationFormat<PlainTextFormatter, TestError>);
    SUCCEED();
}

TEST(CConfigurationFormatConceptTest, TypeWithoutConfigurationTypeMemberDoesNotSatisfy) {
    struct NoConfigType {
        Decree::ErrorResult<int, TestError> parseConfiguration(std::ifstream&);
        Decree::ErrorResult<void, TestError> writeConfiguration(std::ofstream&, const int&);
    };
    static_assert(!CConfigurationFormat<NoConfigType, TestError>);
    SUCCEED();
}

TEST(CConfigurationFormatConceptTest, TypeWithNonEnumErrorCodeDoesNotSatisfy) {
    struct BadErrorCode {};
    static_assert(!CConfigurationFormat<PlainTextFormatter, BadErrorCode>);
    SUCCEED();
}

class JsonFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_sourcePath = std::filesystem::path{ TEST_RESOURCES_DIR } / "test_configuration.json";
        m_outputPath = std::filesystem::temp_directory_path() / "json_config_handler_json_test.json";
        std::filesystem::remove(m_outputPath);
    }

    void TearDown() override {
        std::filesystem::remove(m_outputPath);
    }

    std::filesystem::path m_sourcePath;
    std::filesystem::path m_outputPath;
};

TEST_F(JsonFileTest, LoadsNameField) {
    ConfigurationHandler<SimpleJsonFormatter, TestError> handler;
    const auto result = handler.load(m_sourcePath);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "test_project");
}

TEST_F(JsonFileTest, LoadsVersionField) {
    ConfigurationHandler<SimpleJsonFormatter, TestError> handler;
    const auto result = handler.load(m_sourcePath);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->version, "1.0.0");
}

TEST_F(JsonFileTest, LoadsOutputPathField) {
    ConfigurationHandler<SimpleJsonFormatter, TestError> handler;
    const auto result = handler.load(m_sourcePath);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->outputPath, "build/output");
}

TEST_F(JsonFileTest, RoundTripPreservesAllFields) {
    ConfigurationHandler<SimpleJsonFormatter, TestError> handler;

    const auto loaded = handler.load(m_sourcePath);
    ASSERT_TRUE(loaded.has_value());

    ASSERT_TRUE(handler.update(m_outputPath, *loaded).has_value());

    const auto reloaded = handler.load(m_outputPath);
    ASSERT_TRUE(reloaded.has_value());
    EXPECT_EQ(reloaded->name, loaded->name);
    EXPECT_EQ(reloaded->version, loaded->version);
    EXPECT_EQ(reloaded->outputPath, loaded->outputPath);
}
