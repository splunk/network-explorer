//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <util/counter_to_rate.h>
#include <util/logger.h>
#include <util/stop_watch.h>

#include <generated/flowmill/kernel_collector/handles.h>
#include <generated/flowmill/kernel_collector/index.h>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Turn this on to enable tgid table debugging feature
// via: kill -USR1 <agent_pid>
#ifndef NDEBUG
//#define DEBUG_TGID 1
#endif // NDEBUG

class ProcessHandler {
public:
  ProcessHandler(
      ::flowmill::ingest::Writer &writer, ::flowmill::kernel_collector::Index &collector_index, logging::Logger &log);

  ~ProcessHandler();

  void on_new_process(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_info const &msg);

  void on_process_end(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_close const &msg);

  void on_cgroup_move(std::chrono::nanoseconds timestamp, struct jb_agent_internal__cgroup_attach_task const &msg);

  void set_process_command(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_set_comm const &msg);

  void pid_exit(std::chrono::nanoseconds timestamp, struct jb_agent_internal__pid_exit const &msg);

#ifdef DEBUG_TGID
  void debug_tgid_dump();
#endif // DEBUG_TGID

private:
  struct PerPidData {

    template <typename RHS> PerPidData &operator+=(RHS &&rhs) { return *this; }
  };

  struct PerTgidData {

    std::chrono::nanoseconds timestamp = {};
    bool pending() const { return static_cast<bool>(timestamp.count()); }

    void reset() { timestamp = std::chrono::nanoseconds::zero(); }

    void update(std::chrono::nanoseconds t, PerPidData &rhs, bool on_exit) { timestamp = t; }

    template <typename T> static void update_field(T &field, data::CounterToRate<T> &rhs, bool on_exit)
    {
      field += rhs.commit_rate(!on_exit);
    }

    template <typename T> static void update_field(T &field, data::Gauge<T> &rhs, bool)
    {
      if (rhs.max() > field) {
        field = rhs.max();
      }
      rhs.reset();
    }
  };

  struct ThreadGroupData {
    flowmill::kernel_collector::handles::tracked_process handle;
    absl::flat_hash_map<u32, PerPidData> by_pid;
    PerTgidData by_tgid;
    monotonic_clock::time_point last_check;
#ifdef DEBUG_TGID
    std::chrono::nanoseconds timestamp;
    u64 cgroup;
    std::string command;
#endif // DEBUG_TGID

    monotonic_clock::duration check_interval()
    {
      auto const now = monotonic_clock::now();
      auto const interval = now - last_check;
      last_check = now;
      return interval;
    }
  };

  // rpc components
  ::flowmill::ingest::Writer &writer_;
  ::flowmill::kernel_collector::Index &collector_index_;

  // process data
  absl::flat_hash_map<u32, ThreadGroupData> processes_;

  // logging
  logging::Logger &log_;

  std::size_t memory_page_bytes_;

  struct InternalStats {
    std::size_t duplicate_tgid = 0;
    std::size_t missing_tgid = 0;
  } stats_;
};
