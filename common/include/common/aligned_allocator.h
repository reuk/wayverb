#include <boost/align/aligned_allocator.hpp>

#include <map>
#include <set>
#include <type_traits>
#include <vector>

namespace mem {

//  TODO write own aligned allocator at some point
//  for now I'll have to use boost

template <typename T>
using aligned_vector =
        std::vector<T, boost::alignment::aligned_allocator<T>>;

template <typename T, typename U>
using aligned_map = std::map<
        T,
        U,
        std::less<T>,
        boost::alignment::aligned_allocator<std::pair<const T, U>>>;

}  //  namespace mem
