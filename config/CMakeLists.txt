# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

add_library(
  config_file
  STATIC
    config_file.cc
)
target_link_libraries(
  config_file
    yamlcpp
    spdlog
)

add_library(
  intake_config
  STATIC
    intake_config.cc
)
target_link_libraries(
  intake_config
    render_ebpf_net_ingest_writer
    tcp_channel
    libuv-interface
    args_parser
    file_ops
    environment_variables
)
