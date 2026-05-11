#pragma once

#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "allocator.h"
#include "client_service.h"
#include "types.h"
#include "utils.h"

namespace mooncake{

inline std::shared_ptr<BufferAllocatorBase> CreateTestAllocator(
    const std::string& segment_name, size_t base_offset,
    BufferAllocatorType allocator_type, size_t size = 64 * 1024 * 1024) {
    const size_t base = 0x100000000ULL + base_offset;
    switch (allocator_type) {
        case BufferAllocatorType::CACHELIB:
            return std::make_shared<CachelibBufferAllocator>(
                segment_name, base, size, segment_name);
        case BufferAllocatorType::OFFSET:
            return std::make_shared<OffsetBufferAllocator>(
                segment_name, base, size, segment_name);
        default:
            throw std::invalid_argument("Invalid allocator type");
    }
}

inline std::string FormatClientId(const UUID& client_id) {
    return std::to_string(client_id.first) + "-" +
           std::to_string(client_id.second);
}

inline UUID ParseClientId(const std::string& client_id_str) {
    UUID client_id{0, 0};
    size_t dash_pos = client_id_str.find('-');
    if (dash_pos != std::string::npos) {
        try {
            client_id.first = std::stoull(client_id_str.substr(0, dash_pos));
            client_id.second = std::stoull(client_id_str.substr(dash_pos + 1));
        } catch (const std::exception& e) {
            LOG(ERROR) << "Failed to parse client_id: " << e.what();
        }
    } else {
        LOG(ERROR) << "Invalid client_id format. Expected format: first-second";
    }
    return client_id;
}

namespace testing {

class ClientIdCaptureSink : public google::LogSink {
   public:
    std::string captured_client_id;

    void send(google::LogSeverity severity, const char* full_filename,
              const char* base_filename, int line, const struct ::tm* tm_time,
              const char* message, size_t message_len) override {
        (void)severity;
        (void)full_filename;
        (void)base_filename;
        (void)line;
        (void)tm_time;

        std::string msg(message, message_len);

        size_t pos = msg.find("client_id=");
        if (pos != std::string::npos) {
            std::string client_id_str = msg.substr(pos + 10);
            client_id_str.erase(0, client_id_str.find_first_not_of(" \t\n\r"));
            client_id_str.erase(client_id_str.find_last_not_of(" \t\n\r") +
                                1);

            std::regex uuid_pattern(R"((\d+)-(\d+))");
            std::smatch match;
            if (std::regex_search(client_id_str, match, uuid_pattern)) {
                captured_client_id = match[0].str();
            }
        }
    }
};

static std::shared_ptr<Client> CreateTestClient (
    const std::string& hostname, const std::string& metadata_connstring,
    const std::string& protocol, const std::optional<std::string>& device_names,
    const std::string& master_server_entry,
    const std::shared_ptr<TransferEngine>& transfer_engine,
    std::map<std::string, std::string> labels) {
    auto client_opt =
        Client::Create(hostname,       // Local hostname
                        metadata_connstring,  // Metadata connection string
                        protocol,  // Transfer protocol
                        std::nullopt,  // RDMA device names (auto-discovery)
                        master_server_entry  // Master server address (non-HA)
        );

    EXPECT_TRUE(client_opt.has_value())
        << "Failed to create client with host_name: " << hostname;
    if (!client_opt.has_value()) {
        return nullptr;
    }
    return client_opt.value();
}

}  // namespace testing

// inline Replica::Descriptor CreateMemoryReplicaDescriptor(
//     ReplicaID id, const std::string& segment_name, size_t size = 1024) {
//     Replica::Descriptor replica;
//     replica.id = id;
//     replica.status = ReplicaStatus::COMPLETE;
//     MemoryDescriptor mem_desc;
//     mem_desc.buffer_descriptor.size_ = size;
//     mem_desc.buffer_descriptor.buffer_address_ = 0x1000;
//     mem_desc.buffer_descriptor.transport_endpoint_ = segment_name;
//     replica.descriptor_variant = mem_desc;
//     return replica;
// }

// inline QueryResult CreateQueryResultWithReplicas(
//     const std::vector<Replica::Descriptor>& replicas) {
//     std::vector<Replica::Descriptor> replicas_copy = replicas;
//     std::chrono::steady_clock::time_point lease_timeout =
//         std::chrono::steady_clock::now() + std::chrono::seconds(60);
//     return QueryResult(std::move(replicas_copy), lease_timeout);
// }

}  // namespace mooncake
