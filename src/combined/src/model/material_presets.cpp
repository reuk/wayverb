#include "combined/model/material_presets.h"
#include "combined/model/material.h"
#include "combined/model/vector.h"

namespace wayverb {
namespace combined {
namespace model {

/// From vorlander2007

/// TODO scattering coefficients

// clang-format off
const std::vector<material> presets{
        material{"hard surface - average",                                      {{{0.02, 0.02, 0.02, 0.03, 0.03, 0.04, 0.05, 0.05}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - rendered brickwork",                           {{{0.01, 0.01, 0.02, 0.02, 0.03, 0.03, 0.04, 0.04}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - rough concrete",                               {{{0.02, 0.02, 0.03, 0.03, 0.03, 0.04, 0.07, 0.07}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - smooth unpainted concrete",                    {{{0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.05, 0.05}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - rough lime wash",                              {{{0.02, 0.02, 0.03, 0.04, 0.05, 0.04, 0.03, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - smooth brickwork, flush pointing, painted",    {{{0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - smooth brickwork, 10mm deep pointing, rough",  {{{0.08, 0.08, 0.09, 0.12, 0.16, 0.22, 0.24, 0.24}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - brick wall, rough finish",                     {{{0.03, 0.03, 0.03, 0.03, 0.04, 0.05, 0.07, 0.07}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - ceramic tiles, smooth finish",                 {{{0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - limestone",                                    {{{0.02, 0.02, 0.02, 0.03, 0.04, 0.05, 0.05, 0.05}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - reverb chamber",                               {{{0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.04, 0.04}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - concrete floor",                               {{{0.01, 0.01, 0.03, 0.05, 0.02, 0.02, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"hard surface - marble floor",                                 {{{0.01, 0.01, 0.01, 0.01, 0.02, 0.02, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},

        material{"lightweight - wool-backed plasterboard",                      {{{0.15, 0.15, 0.10, 0.06, 0.04, 0.04, 0.05, 0.05}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"lightweight - wooden lining",                                 {{{0.27, 0.27, 0.23, 0.22, 0.15, 0.10, 0.07, 0.06}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},

        material{"glazing - single 3mm glass pane",                             {{{0.08, 0.08, 0.04, 0.03, 0.03, 0.02, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"glazing - normal glass window",                               {{{0.10, 0.10, 0.05, 0.04, 0.03, 0.03, 0.03, 0.03}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"glazing - lead glazing",                                      {{{0.30, 0.30, 0.20, 0.14, 0.10, 0.05, 0.05, 0.05}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"glazing - double glazing, large gap",                         {{{0.15, 0.15, 0.05, 0.03, 0.03, 0.02, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"glazing - double glazing, small gap",                         {{{0.10, 0.10, 0.07, 0.05, 0.03, 0.02, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"glazing - double glazing, leaded",                            {{{0.15, 0.15, 0.30, 0.18, 0.10, 0.05, 0.05, 0.05}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},

        material{"wood - 16mm, on planks",                                      {{{0.18, 0.18, 0.12, 0.10, 0.09, 0.08, 0.07, 0.07}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"wood - thin plywood",                                         {{{0.42, 0.42, 0.21, 0.10, 0.08, 0.06, 0.06, 0.06}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"wood - 16mm, on studs",                                       {{{0.18, 0.18, 0.12, 0.10, 0.09, 0.08, 0.07, 0.07}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"wood - audience floor",                                       {{{0.09, 0.09, 0.06, 0.05, 0.05, 0.05, 0.04, 0.04}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"wood - stage floor",                                          {{{0.10, 0.10, 0.07, 0.06, 0.06, 0.06, 0.06, 0.06}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"wood - solid door",                                           {{{0.14, 0.14, 0.10, 0.06, 0.08, 0.10, 0.10, 0.10}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        
        material{"floor - standard reflective",                                 {{{0.02, 0.02, 0.03, 0.03, 0.03, 0.03, 0.02, 0.02}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - cotton carpet",                                       {{{0.07, 0.07, 0.31, 0.49, 0.81, 0.66, 0.54, 0.48}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - thick carpet",                                        {{{0.10, 0.10, 0.40, 0.62, 0.70, 0.63, 0.88, 0.88}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - thin carpet",                                         {{{0.02, 0.02, 0.04, 0.08, 0.20, 0.35, 0.40, 0.40}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - open foam-backed carpet",                             {{{0.03, 0.03, 0.09, 0.25, 0.31, 0.33, 0.44, 0.44}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - closed foam-backed carpet",                           {{{0.03, 0.03, 0.09, 0.20, 0.54, 0.70, 0.72, 0.72}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - tufted felt-backed carpet",                           {{{0.08, 0.08, 0.08, 0.30, 0.60, 0.75, 0.80, 0.80}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - needle felt",                                         {{{0.02, 0.02, 0.02, 0.05, 0.15, 0.30, 0.40, 0.40}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - soft carpet",                                         {{{0.09, 0.09, 0.08, 0.21, 0.26, 0.27, 0.37, 0.37}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - hairy carpet",                                        {{{0.11, 0.11, 0.14, 0.37, 0.43, 0.27, 0.25, 0.25}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"floor - rubber carpet",                                       {{{0.04, 0.04, 0.04, 0.08, 0.12, 0.10, 0.10, 0.10}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        
        material{"curtains - heavy cotton",                                     {{{0.30, 0.30, 0.45, 0.65, 0.56, 0.59, 0.71, 0.71}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"curtains - light",                                            {{{0.05, 0.05, 0.06, 0.39, 0.63, 0.70, 0.73, 0.73}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
        material{"curtains - studio",                                           {{{0.36, 0.36, 0.26, 0.51, 0.45, 0.62, 0.76, 0.76}}, {{0, 0, 0, 0, 0, 0, 0, 0}}}},
};
// clang-format on

}  // namespace model
}  // namespace combined
}  // namespace wayverb
