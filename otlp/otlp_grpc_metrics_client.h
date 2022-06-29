//
// Copyright 2022 Splunk Inc.
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

#include <config.h>

#include <platform/types.h>

#include <grpcpp/grpcpp.h>
#include <opentelemetry/proto/collector/metrics/v1/metrics_service.grpc.pb.h>

#include <unordered_map>

using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceResponse;
using opentelemetry::proto::collector::metrics::v1::MetricsService;

namespace otlp_client {

class OtlpGrpcMetricsClient {
public:
  OtlpGrpcMetricsClient(std::shared_ptr<grpc::Channel> channel);
  ~OtlpGrpcMetricsClient();

  grpc::Status Export(ExportMetricsServiceRequest const &request);

  void AsyncExport(ExportMetricsServiceRequest const &request);

  void process_async_responses();

  u64 requests_sent() const { return requests_sent_; }
  u64 bytes_sent() const { return bytes_sent_; }
  u64 metrics_sent() const { return metrics_sent_; }
  u64 successful_requests() const { return successful_requests_; }
  u64 failed_requests() const { return failed_requests_; }
  u64 unknown_response_tags() const { return unknown_response_tags_; }

private:
  std::unique_ptr<MetricsService::Stub> stub_;

  grpc::CompletionQueue cq_;

  struct AsyncResponse {
    grpc::ClientContext context_;
    grpc::Status status_;
    ExportMetricsServiceResponse response_;

    std::unique_ptr<grpc::ClientAsyncResponseReader<ExportMetricsServiceResponse>> response_reader_;
  };

  static u64 next_async_response_tag_;
  std::unordered_map<u64, std::unique_ptr<AsyncResponse>> async_responses_;

  u64 requests_sent_ = 0;
  u64 bytes_sent_ = 0;
  u64 metrics_sent_ = 0;
  u64 successful_requests_ = 0;
  u64 failed_requests_ = 0;
  u64 unknown_response_tags_ = 0;
};

} /* namespace otlp_client */
