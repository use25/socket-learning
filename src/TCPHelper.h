#pragma once

#include <cstddef>

namespace TCPHelper
{

enum ReturnCode : int
{
    SUCCESS = 0,
    ERROR_GENERIC = -1,
    ERROR_MAX_LENGTH = -2
};

ReturnCode WriteN(const int i_sockFd, const void* i_buffer, const size_t i_len);
ReturnCode ReadN(const int i_sockFd, const size_t i_maxLen, void* o_buffer, size_t& o_len);

} // namespace TCPHelper