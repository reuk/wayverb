#include "raytracer_config.h"

int RaytracerConfig::get_rays() const {
    return rays;
}

int RaytracerConfig::get_impulses() const {
    return impulses;
}

float RaytracerConfig::get_ray_hipass() const {
    return ray_hipass;
}

bool RaytracerConfig::get_do_normalize() const {
    return do_normalize;
}

bool RaytracerConfig::get_trim_predelay() const {
    return trim_predelay;
}

bool RaytracerConfig::get_trim_tail() const {
    return trim_tail;
}

bool RaytracerConfig::get_remove_direct() const {
    return remove_direct;
}

float RaytracerConfig::get_volume_scale() const {
    return volume_scale;
}

int& RaytracerConfig::get_rays() {
    return rays;
}
int& RaytracerConfig::get_impulses() {
    return impulses;
}
float& RaytracerConfig::get_ray_hipass() {
    return ray_hipass;
}
bool& RaytracerConfig::get_do_normalize() {
    return do_normalize;
}
bool& RaytracerConfig::get_trim_predelay() {
    return trim_predelay;
}
bool& RaytracerConfig::get_trim_tail() {
    return trim_tail;
}
bool& RaytracerConfig::get_remove_direct() {
    return remove_direct;
}
float& RaytracerConfig::get_volume_scale() {
    return volume_scale;
}
