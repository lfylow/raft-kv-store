#pragma once
 
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
 
namespace raftkv::kv {
 
// A thread-safe in-memory key-value store.
// In later phases this becomes the Raft "state machine": committed log
// entries are applied here in order, identically on every node.
class KvStore {
public:
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);
 
private:
    mutable std::mutex mu_;
    std::unordered_map<std::string, std::string> data_;
};
 
}  // namespace raftkv::kv