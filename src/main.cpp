#include <fstream>
#include <chrono>
#include <iostream>
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
    std::string schema{"{\"type\":\"VAApplication\",\"name\":\"VAApplication\",\"namespace\":\"com.vertoanalytics.core.meters.datatypes\",\"fields\":[{\"name\":\"appIdentifier\",\"type\":\"string\"},{\"name\":\"appVersion\",\"type\":\"string\"},{\"name\":\"appDisplayName\",\"type\":[\"null\",\"string\"]},{\"name\":\"installTime\",\"type\":[\"null\",\"long\"]},{\"name\":\"lastUpdateTime\",\"type\":[\"null\",\"long\"]},{\"name\":\"appPublisher\",\"type\":[\"null\",\"string\"]},{\"name\":\"isSystemApp\",\"type\":[\"null\",\"boolean\"]},{\"name\":\"undefinedMetadata\",\"type\":[\"null\",{\"type\":\"map\",\"values\":\"string\"}]}]}"};

    std::ofstream file{"example.avro", std::ios::binary};

    avro::avro_writer<VAApplication> writer{file, std::move(schema)};

    //avro::binary_writer w{file};
    //std::variant<int32_t, std::string> v{"hello world!"};
    //w.write(v);

    VAApplication app{"app.exe", "1.0.0", "My super app", 1234567890, {}, "My publisher", false, {}};
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < 10000000; i++)
    {
        writer.write(app);
        writer.flush();
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << "ms\n";
}
