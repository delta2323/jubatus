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

#include "num_feature.hpp"
#include <string>
#include <map>

using namespace std;

class my_num_feature : public jubatus::fv_converter::num_feature {
 public:
  void add_feature(const std::string& key, double value,
                   vector<pair<string, float> >& ret_fv) const {
    ret_fv.push_back(make_pair(key, value + 1));
  }
};

extern "C" {

jubatus::fv_converter::num_feature* create(const map<string, string>& params) {
  return new my_num_feature();
}

}
