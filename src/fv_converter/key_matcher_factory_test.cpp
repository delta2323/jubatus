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

#include <gtest/gtest.h>

#include <pficommon/lang/scoped_ptr.h>
#include "key_matcher_factory.hpp"
#include "key_matcher.hpp"
#include "exception.hpp"

using namespace std;
using namespace jubatus;
using namespace jubatus::fv_converter;
using namespace pfi::lang;

TEST(fv_converter, key_matcher_factory) {
  typedef scoped_ptr<key_matcher> m_t;

  key_matcher_factory f;
  ASSERT_TRUE(m_t(f.create_matcher("*"))->match("hogehgeo"));
  ASSERT_TRUE(m_t(f.create_matcher(""))->match("hogehgeo"));
  ASSERT_TRUE(m_t(f.create_matcher("*hogehoge"))->match("adslfjaldsfjadshogehoge"));
  ASSERT_FALSE(m_t(f.create_matcher("*hogehoge"))->match("hogehogea;lsdufi"));
  ASSERT_FALSE(m_t(f.create_matcher("hogehoge*"))->match("adslfjaldsfjadshogehoge"));
  ASSERT_TRUE(m_t(f.create_matcher("hogehoge*"))->match("hogehogea;lsdufi"));
  ASSERT_FALSE(m_t(f.create_matcher("hogehoge"))->match("hogefuga"));
  ASSERT_TRUE(m_t(f.create_matcher("hogehoge"))->match("hogehoge"));

#ifdef HAVE_RE2
  ASSERT_TRUE(m_t(f.create_matcher("/.*/hoge/"))->match("fuga/hoge"));
#else
  ASSERT_THROW(m_t(f.create_matcher("/.*/hoge/"))->match("fuga/hoge"),
               converter_exception);
#endif
}
