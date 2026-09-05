#include "TCPHelper.h"

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace TCPHelper
{

ReturnCode WriteN(const int i_sockFd, const void* i_buffer, const size_t i_len)
{
    ssize_t bytesSent;

    uint32_t lenNetwork = htonl(i_len);
    bytesSent = write(i_sockFd, &lenNetwork, sizeof(lenNetwork));
    if (bytesSent != sizeof(lenNetwork))
    {
        perror("[TCPHelper] write length failed");
        printf("[TCPHelper] write length failed. bytesSent: %zd\n", bytesSent);
        return ReturnCode::ERROR_GENERIC;
    }

    size_t totalBytesSent = 0;
    do
    {
        bytesSent = write(i_sockFd, (void*)(i_buffer + totalBytesSent), i_len - totalBytesSent);
        if (bytesSent < 0)
        {
            perror("[TCPHelper] write message failed");
            return ReturnCode::ERROR_GENERIC;
        }
        if (bytesSent == 0)
        {
            printf("[TCPHelper] socket was closed unexpectedly");
            return ReturnCode::ERROR_GENERIC;
        }
        totalBytesSent += bytesSent;
    } while (totalBytesSent < i_len);
    return ReturnCode::SUCCESS;
}

ReturnCode ReadN(const int i_sockFd, const size_t i_maxLen, void* o_buffer, size_t& o_len)
{
    uint32_t lenNetwork;
    ssize_t bytesReceived = read(i_sockFd, &lenNetwork, sizeof(lenNetwork));

    if (bytesReceived != sizeof(lenNetwork))
    {
        perror("[TCPHelper] read length failed");
        printf("[TCPHelper] read length failed. bytesReceived: %zd\n", bytesReceived);
        return ReturnCode::ERROR_GENERIC;
    }

    o_len = ntohl(lenNetwork);
    if (o_len > i_maxLen)
    {
        printf("[TCPHelper] read length failed: buffer length (%zd) is higher than max length (%zd)\n", o_len, i_maxLen);
        return ReturnCode::ERROR_MAX_LENGTH;
    }

    size_t totalBytesReceived = 0;
    do
    {
        bytesReceived = read(i_sockFd, (o_buffer + totalBytesReceived), o_len - totalBytesReceived);
        if (bytesReceived < 0)
        {
            perror("[TCPHelper] read failed");
            return ReturnCode::ERROR_GENERIC;
        }
        if (bytesReceived == 0)
        {
            printf("[TCPHelper] socket was closed unexpectedly");
            return ReturnCode::ERROR_GENERIC;
        }
        totalBytesReceived += bytesReceived;
    } while (totalBytesReceived < o_len);
    return ReturnCode::SUCCESS;
}

} // namespace TCPHelper