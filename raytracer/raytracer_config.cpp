#include "raytracer_config.h"

int RayverbConfig::get_rays() const {
    return rays;
}

int RayverbConfig::get_impulses() const {
    return impulses;
}

float RayverbConfig::get_ray_hipass() const {
    return ray_hipass;
}

bool RayverbConfig::get_do_normalize() const {
    return do_normalize;
}

bool RayverbConfig::get_trim_predelay() const {
    return trim_predelay;
}

bool RayverbConfig::get_trim_tail() const {
    return trim_tail;
}

bool RayverbConfig::get_remove_direct() const {
    return remove_direct;
}

float RayverbConfig::get_volume_scale() const {
    return volume_scale;
}

int& RayverbConfig::get_rays() {
    return rays;
}
int& RayverbConfig::get_impulses() {
    return impulses;
}
float& RayverbConfig::get_ray_hipass() {
    return ray_hipass;
}
bool& RayverbConfig::get_do_normalize() {
    return do_normalize;
}
bool& RayverbConfig::get_trim_predelay() {
    return trim_predelay;
}
bool& RayverbConfig::get_trim_tail() {
    return trim_tail;
}
bool& RayverbConfig::get_remove_direct() {
    return remove_direct;
}
float& RayverbConfig::get_volume_scale() {
    return volume_scale;
}
