#pragma once

template <typename t>
struct cl_representation;

template <typename t>
const auto cl_representation_v = cl_representation<t>::value;
