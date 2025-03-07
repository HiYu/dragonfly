// Copyright 2023, DragonflyDB authors.  All rights reserved.
// See LICENSE for licensing terms.
//

#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"
#include "server/acl/acl_commands_def.h"

namespace dfly::acl {

class User final {
 public:
  enum class Sign : int8_t { PLUS, MINUS };

  struct UpdateRequest {
    std::optional<std::string> password{};

    std::vector<std::pair<Sign, uint32_t>> categories;

    // DATATYPE_BITSET commands;

    std::optional<bool> is_active{};

    bool is_hashed{false};
  };

  /* Used for default user
   * password = nopass
   * acl_categories = +@all
   * is_active = true;
   */
  User();

  User(const User&) = delete;
  User(User&&) = default;

  // For single step updates
  void Update(UpdateRequest&& req);

  bool HasPassword(std::string_view password) const;

  uint32_t AclCategory() const;

  // TODO
  // For ACL commands
  // void SetAclCommand()
  // void AclCommand() const;

  bool IsActive() const;

  std::string_view Password() const;

 private:
  // For ACL categories
  void SetAclCategories(uint64_t cat);
  void UnsetAclCategories(uint64_t cat);

  // For is_active flag
  void SetIsActive(bool is_active);

  // For passwords
  void SetPasswordHash(std::string_view password, bool is_hashed);

  // when optional is empty, the special `nopass` password is implied
  // password hashed with xx64
  std::optional<std::string> password_hash_;
  uint32_t acl_categories_{NONE};

  // we have at least 221 commands including a bunch of subcommands
  //  LARGE_BITFIELD_DATATYPE acl_commands_;

  // if the user is on/off
  bool is_active_{false};
};

}  // namespace dfly::acl
