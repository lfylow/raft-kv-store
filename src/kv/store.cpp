#include "raftkv/kv/store.hpp"

namespace raftkv::kv {

void KvStore::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mu_);
    data_[key] = value;
}

std::optional<std::string> KvStore::get(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = data_.find(key);
    if (it == data_.end()) return std::nullopt;
    return it->second;
}

bool KvStore::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mu_);
    return data_.erase(key) > 0;
}

}  // namespace raftkv::kv
