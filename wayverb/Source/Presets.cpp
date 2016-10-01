#include "Presets.hpp"

#include "common/stl_wrappers.h"

namespace model {

aligned::vector<scene_data::material> get_presets() {
    aligned::vector<scene_data::material> ret{
            scene_data::material{
                    "concrete_floor",
                    surface{{0.99, 0.97, 0.95, 0.98, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "brickwork",
                    surface{{0.99, 0.98, 0.98, 0.97, 0.97, 0.96, 0.96, 0.96},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "rough_lime_wash",
                    surface{{0.98, 0.97, 0.96, 0.95, 0.96, 0.97, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "limestone",
                    surface{{0.98, 0.98, 0.97, 0.96, 0.95, 0.95, 0.95, 0.95},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "wooden_door",
                    surface{{0.86, 0.9, 0.94, 0.92, 0.9, 0.9, 0.9, 0.9},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "rough_concrete",
                    surface{{0.98, 0.97, 0.97, 0.97, 0.96, 0.93, 0.93, 0.93},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "smooth_floor",
                    surface{{0.98, 0.97, 0.97, 0.97, 0.97, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "lead_glazing",
                    surface{{0.7, 0.8, 0.86, 0.9, 0.95, 0.95, 0.95, 0.95},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "leaded_glazing",
                    surface{{0.85, 0.7, 0.82, 0.9, 0.95, 0.95, 0.95, 0.95},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "chairs__2",
                    surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "takapaaty_flat__2",
                    surface{{0.85, 0.9, 0.94, 0.96, 0.96, 0.95, 0.95, 0.95},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "double_glazing",
                    surface{{0.85, 0.95, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "audience_floor",
                    surface{{0.91, 0.94, 0.95, 0.95, 0.95, 0.96, 0.96, 0.96},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "katto_flat__2",
                    surface{{0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "verb_chamber",
                    surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.96, 0.96, 0.96},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "plywood_panels",
                    surface{{0.58, 0.79, 0.9, 0.92, 0.94, 0.94, 0.94, 0.94},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "smooth_concrete",
                    surface{{0.6, 0.8, 0.9, 0.95, 0.97, 0.97, 0.96, 0.95},
                            {0.99, 0.99, 0.98, 0.98, 0.98, 0.95, 0.95, 0.95}}},
            scene_data::material{
                    "glass_window",
                    surface{{0.9, 0.95, 0.96, 0.97, 0.97, 0.97, 0.97, 0.97},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "ceramic_tiles",
                    surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "pile_carpet",
                    surface{{0.97, 0.91, 0.75, 0.69, 0.6, 0.56, 0.56, 0.56},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "4cm_planks",
                    surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "point_brickwork",
                    surface{{0.92, 0.91, 0.88, 0.84, 0.78, 0.76, 0.76, 0.76},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "marble_floor",
                    surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "wooden_lining",
                    surface{{0.73, 0.77, 0.78, 0.85, 0.9, 0.93, 0.94, 0.94},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "hull__2",
                    surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.96, 0.96, 0.96},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "stucco_brick",
                    surface{{0.97, 0.97, 0.97, 0.96, 0.95, 0.93, 0.93, 0.93},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "vasen_flat__2",
                    surface{{0.73, 0.77, 0.78, 0.85, 0.9, 0.93, 0.94, 0.94},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "double_glazing_2",
                    surface{{0.9, 0.93, 0.95, 0.97, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "hard_wall",
                    surface{{0.98, 0.98, 0.97, 0.97, 0.96, 0.95, 0.95, 0.95},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "thin_carpet",
                    surface{{0.98, 0.96, 0.92, 0.8, 0.75, 0.6, 0.6, 0.6},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "etupaaty_flat__2",
                    surface{{0.92, 0.91, 0.88, 0.84, 0.78, 0.76, 0.76, 0.76},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "oikea_flat__2",
                    surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "smooth_brickwork",
                    surface{{0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "wood_40mm_studs",
                    surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "cotton_carpet",
                    surface{{0.93, 0.69, 0.51, 0.19, 0.34, 0.46, 0.52, 0.52},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "3mm_glass",
                    surface{{0.92, 0.96, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "tufted_carpet",
                    surface{{0.9, 0.6, 0.37, 0.3, 0.37, 0.12, 0.12, 0.12},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "plasterboard",
                    surface{{0.85, 0.9, 0.94, 0.96, 0.96, 0.95, 0.95, 0.95},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "stage_floor",
                    surface{{0.9, 0.93, 0.94, 0.94, 0.94, 0.94, 0.94, 0.94},
                            {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
            scene_data::material{
                    "echo chamber",
                    surface{{0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99},
                            {0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99, 0.99}}},
    };

    proc::sort(ret,
               [](const auto& i, const auto& j) { return i.name < j.name; });

    return ret;
}

}  // namespace model