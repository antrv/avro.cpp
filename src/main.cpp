#include <fstream>
#include "avro_writer.h"

// sample
struct VAApplication
{
    std::string app_identifier;
    std::string app_version;
    std::string app_display_name;
    std::optional<int64_t> install_time;
    std::optional<int64_t> last_update_time;
    std::string app_publisher;
    std::optional<bool> is_system_app;
    std::unordered_map<std::string, std::string> undefined_metadata;

    void write_to(const avro::binary_writer writer) const
    {
        writer.write(app_identifier);
        writer.write(app_version);
        writer.write(app_display_name);
        writer.write(install_time);
        writer.write(last_update_time);
        writer.write(app_publisher);
        writer.write(is_system_app);
        writer.write(undefined_metadata);
    }
};

int __cdecl main()
{
    using value_t = std::map<std::string, int32_t>;
    using schema_t = std::vector<value_t>;

    std::string schema{"{\"type\":\"VAApplication\",\"name\":\"VAApplication\",\"namespace\":\"com.vertoanalytics.core.meters.datatypes\",\"fields\":[{\"name\":\"appIdentifier\",\"type\":\"string\"},{\"name\":\"appVersion\",\"type\":\"string\"},{\"name\":\"appDisplayName\",\"type\":[\"null\",\"string\"]},{\"name\":\"installTime\",\"type\":[\"null\",\"long\"]},{\"name\":\"lastUpdateTime\",\"type\":[\"null\",\"long\"]},{\"name\":\"appPublisher\",\"type\":[\"null\",\"string\"]},{\"name\":\"isSystemApp\",\"type\":[\"null\",\"boolean\"]},{\"name\":\"undefinedMetadata\",\"type\":[\"null\",{\"type\":\"map\",\"values\":\"string\"}]}]}"};

    std::ofstream file{"example.avro", std::ios::binary};
    avro::avro_writer<schema_t> writer{file, std::move(schema)};

    value_t v1{{"hello", 123}, {"test", 1000}};
    value_t v2{{"aaa", -100}, {"bbb", -1}};
    schema_t v{std::move(v1), std::move(v2)};

    writer.write(v);
    writer.flush();
}
