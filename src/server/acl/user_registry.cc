// Copyright 2023, DragonflyDB authors.  All rights reserved.
// See LICENSE for licensing terms.
//

#include "server/acl/user_registry.h"

#include <mutex>

#include "core/fibers.h"
#include "facade/facade_types.h"
#include "server/acl/acl_commands_def.h"

namespace dfly::acl {

UserRegistry::UserRegistry() {
  std::pair<User::Sign, uint32_t> acl{User::Sign::PLUS, acl::ALL};
  User::UpdateRequest req{{}, {acl}, true};
  MaybeAddAndUpdate("default", std::move(req));
}

void UserRegistry::MaybeAddAndUpdate(std::string_view username, User::UpdateRequest req) {
  std::unique_lock<util::SharedMutex> lock(mu_);
  auto& user = registry_[username];
  user.Update(std::move(req));
}

bool UserRegistry::RemoveUser(std::string_view username) {
  std::unique_lock<util::SharedMutex> lock(mu_);
  return registry_.erase(username);
}

UserRegistry::UserCredentials UserRegistry::GetCredentials(std::string_view username) const {
  std::shared_lock<util::SharedMutex> lock(mu_);
  auto it = registry_.find(username);
  if (it == registry_.end()) {
    return {};
  }
  return {it->second.AclCategory()};
}

bool UserRegistry::IsUserActive(std::string_view username) const {
  std::shared_lock<util::SharedMutex> lock(mu_);
  auto it = registry_.find(username);
  if (it == registry_.end()) {
    return false;
  }
  return it->second.IsActive();
}

bool UserRegistry::AuthUser(std::string_view username, std::string_view password) const {
  std::shared_lock<util::SharedMutex> lock(mu_);
  const auto& user = registry_.find(username);
  if (user == registry_.end()) {
    return false;
  }

  return user->second.IsActive() && user->second.HasPassword(password);
}

UserRegistry::RegistryViewWithLock UserRegistry::GetRegistryWithLock() const {
  std::shared_lock<util::SharedMutex> lock(mu_);
  return {std::move(lock), registry_};
}

UserRegistry::RegistryWithWriteLock UserRegistry::GetRegistryWithWriteLock() {
  std::unique_lock<util::SharedMutex> lock(mu_);
  return {std::move(lock), registry_};
}

UserRegistry::UserWithWriteLock::UserWithWriteLock(std::unique_lock<util::SharedMutex> lk,
                                                   const User& user, bool exists)
    : user(user), exists(exists), registry_lk_(std::move(lk)) {
}

UserRegistry::UserWithWriteLock UserRegistry::MaybeAddAndUpdateWithLock(std::string_view username,
                                                                        User::UpdateRequest req) {
  std::unique_lock<util::SharedMutex> lock(mu_);
  const bool exists = registry_.contains(username);
  auto& user = registry_[username];
  user.Update(std::move(req));
  return {std::move(lock), user, exists};
}

}  // namespace dfly::acl
