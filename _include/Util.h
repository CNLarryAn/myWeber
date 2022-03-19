#pragma once
#include <cstdlib>
#include <string>

ssize_t writen(int fd, std::string &sbuff);
ssize_t readn(int fd, std::string &inBuffer);