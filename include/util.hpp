#pragma once

#include <algorithm>
#include <vector>
#include <string>

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

std::vector< std::string > char_arr_to_vector( char** arr, const size_t& start = 0 ) {
  std::vector< std::string > result;
    
  for (size_t i{ start }; arr[ i ] != nullptr; i++) {
    result.emplace_back( arr[ i ] );
  }
    
  return result;
}

std::string extension_of ( const std::string &path ) {
  const auto dot_pos = path.find_last_of ( "." );

  if (dot_pos == std::string::npos) {
    return ""; // not found
  }

  return path.substr ( dot_pos + 1 );
}