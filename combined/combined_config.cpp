#include "combined_config.h"

#include "scene_data.h"

#include <iostream>

CombinedConfig read_config(const std::string & file) {
    rapidjson::Document document;
    attemptJsonParse(file, document);

    if (document.HasParseError()) {
        std::cerr << "Encountered error while parsing config file:"
                  << std::endl;
        std::cerr << GetParseError_En(document.GetParseError()) << std::endl;
        exit(1);
    }

    if (!document.IsObject()) {
        std::cerr << "Rayverb config must be stored in a JSON object"
                  << std::endl;
        exit(1);
    }

    CombinedConfig cc;
    auto json_getter = get_json_getter(cc);

    if (json_getter.check(document)) {
        json_getter.get(document);
    } else {
        throw std::runtime_error("json getter check failed");
    }

    return cc;
}
