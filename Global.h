#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>
#include <string>

extern std::map<int, std::string> versionRepository;
extern int currentVersion;

void saveVersionRepository(const std::string& filename = "version_repository.dat");
bool loadVersionRepository(const std::string& filename = "version_repository.dat");

#endif // GLOBAL_H
