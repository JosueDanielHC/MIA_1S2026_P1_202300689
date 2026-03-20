#ifndef USERS_COMMANDS_H
#define USERS_COMMANDS_H

#include <string>

namespace extreamfs {
namespace commands {

std::string runLogin(const std::string& user, const std::string& pass, const std::string& id);
std::string runLogout();
std::string runMkgrp(const std::string& name);
std::string runRmgrp(const std::string& name);
std::string runMkusr(const std::string& user, const std::string& pass, const std::string& grp);
std::string runRmusr(const std::string& user);
std::string runChgrp(const std::string& user, const std::string& grp);

} // namespace commands
} // namespace extreamfs

#endif // USERS_COMMANDS_H
