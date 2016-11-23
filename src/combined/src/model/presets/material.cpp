#include "combined/model/presets/material.h"

namespace wayverb {
namespace combined {
namespace model {
namespace presets {

/// From vorlander2007

/// TODO scattering coefficients

namespace {

constexpr auto s = 0.1;

}  // namespace

// clang-format off
const std::vector<material> materials{
        material{"hard surface - average",                                      {{{0.02, 0.02, 0.02, 0.03, 0.03, 0.04, 0.05, 0.05}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - rendered brickwork",                           {{{0.01, 0.01, 0.02, 0.02, 0.03, 0.03, 0.04, 0.04}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - rough concrete",                               {{{0.02, 0.02, 0.03, 0.03, 0.03, 0.04, 0.07, 0.07}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - smooth unpainted concrete",                    {{{0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.05, 0.05}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - rough lime wash",                              {{{0.02, 0.02, 0.03, 0.04, 0.05, 0.04, 0.03, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - smooth brickwork, flush pointing, painted",    {{{0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - smooth brickwork, 10mm deep pointing, rough",  {{{0.08, 0.08, 0.09, 0.12, 0.16, 0.22, 0.24, 0.24}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - brick wall, rough finish",                     {{{0.03, 0.03, 0.03, 0.03, 0.04, 0.05, 0.07, 0.07}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - ceramic tiles, smooth finish",                 {{{0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - limestone",                                    {{{0.02, 0.02, 0.02, 0.03, 0.04, 0.05, 0.05, 0.05}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - reverb chamber",                               {{{0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.04, 0.04}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - concrete floor",                               {{{0.01, 0.01, 0.03, 0.05, 0.02, 0.02, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"hard surface - marble floor",                                 {{{0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},

        material{"lightweight - wool-backed plasterboard",                      {{{0.15, 0.15, 0.10, 0.06, 0.04, 0.04, 0.05, 0.05}}, {{s, s, s, s, s, s, s, s}}}},
        material{"lightweight - wooden lining",                                 {{{0.27, 0.27, 0.23, 0.22, 0.15, 0.10, 0.07, 0.06}}, {{s, s, s, s, s, s, s, s}}}},

        material{"glazing - single 3mm glass pane",                             {{{0.08, 0.08, 0.04, 0.03, 0.03, 0.02, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"glazing - normal glass window",                               {{{0.10, 0.10, 0.05, 0.04, 0.03, 0.03, 0.03, 0.03}}, {{s, s, s, s, s, s, s, s}}}},
        material{"glazing - lead glazing",                                      {{{0.30, 0.30, 0.20, 0.14, 0.10, 0.05, 0.05, 0.05}}, {{s, s, s, s, s, s, s, s}}}},
        material{"glazing - double glazing, large gap",                         {{{0.15, 0.15, 0.05, 0.03, 0.03, 0.02, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"glazing - double glazing, small gap",                         {{{0.10, 0.10, 0.07, 0.05, 0.03, 0.02, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"glazing - double glazing, leaded",                            {{{0.15, 0.15, 0.30, 0.18, 0.10, 0.05, 0.05, 0.05}}, {{s, s, s, s, s, s, s, s}}}},

        material{"wood - 16mm, on planks",                                      {{{0.18, 0.18, 0.12, 0.10, 0.09, 0.08, 0.07, 0.07}}, {{s, s, s, s, s, s, s, s}}}},
        material{"wood - thin plywood",                                         {{{0.42, 0.42, 0.21, 0.10, 0.08, 0.06, 0.06, 0.06}}, {{s, s, s, s, s, s, s, s}}}},
        material{"wood - 16mm, on studs",                                       {{{0.18, 0.18, 0.12, 0.10, 0.09, 0.08, 0.07, 0.07}}, {{s, s, s, s, s, s, s, s}}}},
        material{"wood - audience floor",                                       {{{0.09, 0.09, 0.06, 0.05, 0.05, 0.05, 0.04, 0.04}}, {{s, s, s, s, s, s, s, s}}}},
        material{"wood - stage floor",                                          {{{0.10, 0.10, 0.07, 0.06, 0.06, 0.06, 0.06, 0.06}}, {{s, s, s, s, s, s, s, s}}}},
        material{"wood - solid door",                                           {{{0.14, 0.14, 0.10, 0.06, 0.08, 0.10, 0.10, 0.10}}, {{s, s, s, s, s, s, s, s}}}},
        
        material{"floor - standard reflective",                                 {{{0.02, 0.02, 0.03, 0.03, 0.03, 0.03, 0.02, 0.02}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - cotton carpet",                                       {{{0.07, 0.07, 0.31, 0.49, 0.81, 0.66, 0.54, 0.48}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - thick carpet",                                        {{{0.10, 0.10, 0.40, 0.62, 0.70, 0.63, 0.88, 0.88}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - thin carpet",                                         {{{0.02, 0.02, 0.04, 0.08, 0.20, 0.35, 0.40, 0.40}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - open foam-backed carpet",                             {{{0.03, 0.03, 0.09, 0.25, 0.31, 0.33, 0.44, 0.44}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - closed foam-backed carpet",                           {{{0.03, 0.03, 0.09, 0.20, 0.54, 0.70, 0.72, 0.72}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - tufted felt-backed carpet",                           {{{0.08, 0.08, 0.08, 0.30, 0.60, 0.75, 0.80, 0.80}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - needle felt",                                         {{{0.02, 0.02, 0.02, 0.05, 0.15, 0.30, 0.40, 0.40}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - soft carpet",                                         {{{0.09, 0.09, 0.08, 0.21, 0.26, 0.27, 0.37, 0.37}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - hairy carpet",                                        {{{0.11, 0.11, 0.14, 0.37, 0.43, 0.27, 0.25, 0.25}}, {{s, s, s, s, s, s, s, s}}}},
        material{"floor - rubber carpet",                                       {{{0.04, 0.04, 0.04, 0.08, 0.12, 0.10, 0.10, 0.10}}, {{s, s, s, s, s, s, s, s}}}},
        
        material{"curtains - heavy cotton",                                     {{{0.30, 0.30, 0.45, 0.65, 0.56, 0.59, 0.71, 0.71}}, {{s, s, s, s, s, s, s, s}}}},
        material{"curtains - light",                                            {{{0.05, 0.05, 0.06, 0.39, 0.63, 0.70, 0.73, 0.73}}, {{s, s, s, s, s, s, s, s}}}},
        material{"curtains - studio",                                           {{{0.36, 0.36, 0.26, 0.51, 0.45, 0.62, 0.76, 0.76}}, {{s, s, s, s, s, s, s, s}}}},
};
// clang-format on

}  // namespace presets
}  // namespace model
}  // namespace combined
}  // namespace wayverb
