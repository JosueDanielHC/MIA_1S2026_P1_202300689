#include "session_manager.h"

namespace extreamfs {
namespace manager {

namespace {
    Session g_session;
}

void setSession(const std::string& idPartition, const std::string& username, int uid, int gid) {
    g_session.active = true;
    g_session.id_particion = idPartition;
    g_session.username = username;
    g_session.uid = uid;
    g_session.gid = gid;
}

void clearSession() {
    g_session.active = false;
    g_session.id_particion.clear();
    g_session.username.clear();
    g_session.uid = 0;
    g_session.gid = 0;
}

Session getSession() {
    return g_session;
}

bool isSessionActive() {
    return g_session.active;
}

bool isRoot() {
    return g_session.active && g_session.username == "root";
}

std::string getSessionId() {
    return g_session.id_particion;
}

std::string getSessionUsername() {
    return g_session.username;
}

int getSessionUid() {
    return g_session.uid;
}

int getSessionGid() {
    return g_session.gid;
}

} // namespace manager
} // namespace extreamfs
