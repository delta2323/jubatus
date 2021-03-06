// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2011 Preferred Infrastracture and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#pragma once

#include <string>
#include <pficommon/data/string/utility.h>

#include "key_matcher.hpp"

namespace jubatus {
namespace fv_converter {

class prefix_match : public key_matcher {
 public:
  prefix_match(const std::string& prefix) : prefix_(prefix) {}
  ~prefix_match() {}

  bool match(const std::string& key) {
    return pfi::data::string::starts_with(key, prefix_);
  }

 private:
  const std::string prefix_;
};

}
}
