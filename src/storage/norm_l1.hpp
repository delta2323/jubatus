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
#include <cmath>
#include <pficommon/data/unordered_map.h>
#include "norm_base.hpp"

namespace jubatus {
namespace storage {

class norm_l1 : public norm_base{
public:
  norm_l1();
  ~norm_l1();
  void clear();
  void notify(const std::string& row, float old_val, float new_val);
  float calc_norm(const std::string& row) const;

private:
  pfi::data::unordered_map<std::string, float> sq_norms_;
};

}
}
