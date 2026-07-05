#include "raftkv/kv/store.hpp"

#include <gtest/gtest.h>

using raftkv::kv::KvStore;

TEST(KvStore, GetMissingKeyReturnsNullopt) {
    KvStore store;
    EXPECT_FALSE(store.get("absent").has_value());
}

TEST(KvStore, SetThenGetReturnsValue) {
    KvStore store;
    store.set("foo", "bar");
    auto value = store.get("foo");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "bar");
}

TEST(KvStore, SetOverwritesExistingValue) {
    KvStore store;
    store.set("foo", "bar");
    store.set("foo", "baz");
    EXPECT_EQ(store.get("foo").value(), "baz");
}

TEST(KvStore, DeleteRemovesKey) {
    KvStore store;
    store.set("foo", "bar");
    EXPECT_TRUE(store.del("foo"));
    EXPECT_FALSE(store.get("foo").has_value());
}

TEST(KvStore, DeleteMissingKeyReturnsFalse) {
    KvStore store;
    EXPECT_FALSE(store.del("absent"));
}
