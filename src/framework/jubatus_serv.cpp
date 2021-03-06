// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2011,2012 Preferred Infrastracture and Nippon Telegraph and Telephone Corporation.
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

#include "jubatus_serv.hpp"
#include "../common/util.hpp"
#include "../common/cht.hpp"
#include "../common/exception.hpp"
#include "server_util.hpp"

#include <fstream>
#include <sstream>
#include <pficommon/system/time_util.h>

using std::vector;
using std::string;
using pfi::network::mprpc::rpc_client;
using pfi::lang::function;

using pfi::system::time::clock_time;
using pfi::system::time::get_clock_time;

namespace jubatus { namespace framework {

    jubatus_serv::jubatus_serv(const server_argv& a, const std::string& base_path):
      a_(a),
      update_count_(0),
#ifdef HAVE_ZOOKEEPER_H
      mixer_(new mixer(a_.name, a_.interval_count, a_.interval_sec,
                       pfi::lang::bind(&jubatus_serv::do_mix, this, pfi::lang::_1))),
      use_cht_(false),
#endif
      base_path_(a_.tmpdir)
    {
    };
    
    int jubatus_serv::start(pfi::network::mprpc::rpc_server& serv){

#ifdef HAVE_ZOOKEEPER_H
      if(! a_.is_standalone()){
	zk_ = common::cshared_ptr<jubatus::common::lock_service>
	  (common::create_lock_service("zk", a_.z, a_.timeout, "logfile_jubatus_serv"));
	ls = zk_;
	jubatus::common::prepare_jubatus(*zk_);
	
	if( a_.join ){ // join to the existing cluster with -j option
	  join_to_cluster(zk_);
	}
	
	if( use_cht_ ){
	  jubatus::common::cht::setup_cht_dir(*zk_, a_.name);
	  jubatus::common::cht ht(zk_, a_.name);
	  ht.register_node(a_.eth, a_.port);
	}
	
	mixer_->set_zk(zk_);
	register_actor(*zk_, a_.name, a_.eth, a_.port);
	mixer_->start();
      }
#endif

      if( serv.serv(a_.port, a_.threadnum) ){
	LOG(INFO) << "running in port=" << a_.port;
	return 0;
      }else{
	LOG(ERROR) << "failed starting server: any process using port " << a_.port << "?";
	return -1;
      }
    }
    
    void jubatus_serv::register_mixable(mixable0* m){
#ifdef HAVE_ZOOKEEPER_H
      try{
        m->get_diff(); // #22 ensure m is good pointer at process startup
      }catch(const jubatus::config_not_set& e){
      }

      mixables_.push_back(m);
#endif
    };
    
    void jubatus_serv::use_cht(){
#ifdef HAVE_ZOOKEEPER_H
      use_cht_ = true;
#endif
    };

  std::map<std::string, std::map<std::string,std::string> > jubatus_serv::get_status() const {
    std::map<std::string, std::string> data;
    util::get_machine_status(data);

    data["timeout"] = pfi::lang::lexical_cast<std::string>(a_.timeout);
    data["threadnum"] = pfi::lang::lexical_cast<std::string>(a_.threadnum);
    data["tmpdir"] = a_.tmpdir;
    data["interval_sec"] = pfi::lang::lexical_cast<std::string>(a_.interval_sec);
    data["interval_count"] = pfi::lang::lexical_cast<std::string>(a_.interval_count);
    data["is_standalone"] = pfi::lang::lexical_cast<std::string>(a_.is_standalone());
    data["VERSION"] = JUBATUS_VERSION;
    data["PROGNAME"] = JUBATUS_APPNAME;

    data["update_count"] = pfi::lang::lexical_cast<std::string>(update_count_);

#ifdef HAVE_ZOOKEEPER_H
    mixer_->get_status(data);
    data["zk"] = a_.z;
    data["use_cht"] = pfi::lang::lexical_cast<std::string>(use_cht_);
#endif

    std::map<std::string, std::map<std::string,std::string> > ret;
    ret[get_server_identifier()] = data;
    return ret;
  };

    std::string jubatus_serv::get_server_identifier()const{
      std::stringstream ss;
      ss << a_.eth;
      ss << "_";
      ss << a_.port;
      return ss.str();
    };

    //here
#ifdef HAVE_ZOOKEEPER_H
    void jubatus_serv::join_to_cluster(common::cshared_ptr<jubatus::common::lock_service> z){
    std::vector<std::string> list;
    std::string path = common::ACTOR_BASE_PATH + "/" + a_.name + "/nodes";
    z->list(path, list);
    if(not list.empty()){
      common::lock_service_mutex zlk(*z, common::ACTOR_BASE_PATH + "/" + a_.name + "/master_lock");
      while(not zlk.try_lock()){ ; }
      size_t i = rand() % list.size();
      std::string ip;
      int port;
      common::revert(list[i], ip, port);
      pfi::network::mprpc::rpc_client c(ip, port, a_.timeout);

      pfi::lang::function<std::string()> f = c.call<std::string()>("get_storage");
      std::stringstream ss( f() );
      for(size_t i = 0;i<mixables_.size(); ++i){
        mixables_[i]->clear();
        mixables_[i]->load(ss);
      }
    }
  };

    std::string jubatus_serv::get_storage(int i){
    std::stringstream ss;
    for(size_t i=0; i<mixables_.size(); ++i){
      mixables_[i]->save(ss);
    }
    return ss.str();
  }

    std::vector<std::string> jubatus_serv::get_diff_impl(int){
    // if(mixables_.empty()){
    //   //throw config_not_set(); nothing to mix
    // }
    std::vector<std::string> o;
    {
      scoped_lock lk(rlock(m_));
      for(size_t i=0; i<mixables_.size(); ++i){
        o.push_back(mixables_[i]->get_diff());
      }
    }
    return o;
  };

    int jubatus_serv::put_diff_impl(std::vector<std::string> unpacked){
      scoped_lock lk(wlock(m_));
      if(unpacked.size() != mixables_.size()){
	//deserialization error
	return -1;
      }
      for(size_t i=0; i<mixables_.size(); ++i){
	mixables_[i]->put_diff(unpacked[i]);
      }
      mixer_->clear();
      return 0;
    };

    void jubatus_serv::do_mix(const std::vector<std::pair<std::string,int> >& v){
      vector<string> accs;
      vector<string> serialized_diffs;
      clock_time start = get_clock_time();
      for(size_t s = 0; s < v.size(); ++s ){
        try{
          rpc_client c(v[s].first, v[s].second, a_.timeout);
          function<vector<string>(int)> get_diff_fun = c.call<vector<string>(int)>("get_diff");
          serialized_diffs = get_diff_fun(0);
        }catch(std::exception& e){
          LOG(ERROR) << e.what();
          continue;
        }
        scoped_lock lk(rlock(m_)); // model_ should not be in mix (reduce)?
        if(accs.empty()){
          accs = serialized_diffs;
        }else{
          for(size_t i=0; i<mixables_.size(); ++i){
            // FIXME: very inefficient but what sucks is type system of C++
            mixables_[i]->reduce(serialized_diffs[i], accs[i]);
          }
        }
      }

      for(size_t s = 0; s < v.size(); ++s ){
        try{
          rpc_client c(v[s].first, v[s].second, a_.timeout);
          function<int(vector<string>)> put_diff_fun = c.call<int(vector<string>)>("put_diff");
          put_diff_fun(accs);
        }catch(std::exception& e){
          LOG(ERROR) << e.what();
          continue;
        }
      }
      clock_time end = get_clock_time();
      DLOG(INFO) << "mixed with " << v.size() << " servers in " << (double)(end - start) << " secs.";
      size_t s = 0;
      for(size_t i=0; i<accs.size(); ++i){
        s+=accs[i].size();
      }
      DLOG(INFO) << s << " bytes (serialized data) has been put.";
    }
#endif

    void jubatus_serv::updated(){
#ifdef HAVE_ZOOKEEPER_H
      update_count_ = mixer_->updated();
#else
      update_count_++;
#endif
    }

    bool jubatus_serv::save(std::string id)  {
      std::string ofile;
      build_local_path_(ofile, "jubatus", id);
    
      std::ofstream ofs(ofile.c_str(), std::ios::trunc|std::ios::binary);
      if(!ofs){
        throw std::runtime_error(ofile + ": cannot open (" + pfi::lang::lexical_cast<std::string>(errno) + ")" );
      }
      try{
        for(size_t i=0; i<mixables_.size(); ++i){
          mixables_[i]->save(ofs);
        }
        ofs.close();
        LOG(INFO) << "saved to " << ofile;
        return true;
      }catch(const std::runtime_error& e){
        LOG(ERROR) << e.what();
        throw e;
      }
    }

    bool jubatus_serv::load(std::string id) {
      std::string ifile;
      build_local_path_(ifile, "jubatus", id);
    
      std::ifstream ifs(ifile.c_str(), std::ios::binary);
      if(!ifs)throw std::runtime_error(ifile + ": cannot open (" + pfi::lang::lexical_cast<std::string>(errno) + ")" );
      try{
        for(size_t i = 0;i<mixables_.size(); ++i){
          mixables_[i]->clear();
          mixables_[i]->load(ifs);
        }
        ifs.close();
        this->after_load();
        return true;
      }catch(const std::runtime_error& e){
        ifs.close();
        LOG(ERROR) << e.what();
        throw e;
      }
    }

}}
