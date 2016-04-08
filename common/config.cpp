#include "config.h"

#include "vec.h"

#include <fstream>

void config::attempt_json_parse(const std::string& fname,
                                rapidjson::Document& doc) {
    std::ifstream in(fname);
    std::string file((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());
    doc.Parse(file.c_str());
}

void config::JsonGetter<Vec3f>::get(const rapidjson::Value& value) const {
    t.x = value[0].GetDouble();
    t.y = value[1].GetDouble();
    t.z = value[2].GetDouble();
}
