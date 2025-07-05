#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <vector>

template < class T, class Vec_t = std::vector< T > >
size_t find_position_in_vec ( const Vec_t &vec, const T &val ) {
  auto iter =
  std::find_if ( vec.begin (), vec.end (), [ &val ] ( const T &element ) { return element == val; } );

  size_t index = std::distance ( vec.begin (), iter );

  if ( iter == vec.end () ) {
    return vec.size (); // not found
  }

  return index;
}

std::string extension_of ( const std::string &path ) {
  return path.substr ( path.find_last_of ( "." ) + 1 );
}

#endif