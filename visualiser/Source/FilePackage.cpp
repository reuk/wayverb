#include "FilePackage.hpp"

FilePackage::FilePackage(const File& object,
                         const File& material,
                         const File& config)
        : object{object}
        , material{material}
        , config{config} {
}

File FilePackage::get_object() const {
    return object;
}
File FilePackage::get_material() const {
    return material;
}
File FilePackage::get_config() const {
    return config;
}