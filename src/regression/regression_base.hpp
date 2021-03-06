// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2012 Preferred Infrastracture and Nippon Telegraph and Telephone Corporation.
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

#include "../common/type.hpp"

namespace jubatus {

namespace storage {
class storage_base;
}


class regression_base {
 public:
  regression_base(storage::storage_base* storage);

  virtual ~regression_base() {}

  virtual void train(const sfv_t& fv, const float value) = 0;
  float estimate(const sfv_t& fv) const;

 protected:
  storage::storage_base* get_storage() const {
    return storage_;
  }

  void update(const sfv_t& fv, float coeff);

 private:
  storage::storage_base* storage_;
};

}
