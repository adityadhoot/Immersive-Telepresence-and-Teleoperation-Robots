/**
 * \file Socket.cpp
 * \author Billy Jun
 * \brief Socket implementation 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>

#include "Socket.hpp"

Socket::Socket()
{
    m_sockfd = m_newsockfd = -1;
    m_side = Socket::Server;
}

Socket::Socket(const char* ip, unsigned port, Side side, Protocol prot,
               BlockingMode block, DelayMode delay)
{
    open(ip, port, side, prot, block, delay);
}

Socket::~Socket()
{
    close();
}

bool Socket::isOpen()
{
    if (m_side == Socket::Server)
        return m_sockfd > 0 && m_newsockfd > 0;
    else if (m_side == Socket::Client)
        return m_sockfd > 0;
    return false;
}
    
bool Socket::open(const char* ip, unsigned port, Side side, Protocol prot,
                  BlockingMode block, DelayMode delay)
{
    int delayFlag = (delay == Socket::NoDelay);
    int fcntlFlags;

    // Server side code ///////////////////////////////////////////////////////
    if (side == Socket::Server)
    {
        m_sockfd = socket(
            AF_INET, (prot == Socket::TCP) ? SOCK_STREAM : SOCK_DGRAM, 0);
        if (m_sockfd < 0)
            return false;
        if (prot == Socket::TCP)
        {
            if (0 > setsockopt(
                    m_sockfd,            /* socket affected */
                    IPPROTO_TCP,     /* set option at TCP level */
                    TCP_NODELAY,     /* name of option */
                    (char *) &delayFlag,  /* the cast is historical cruft */
                    sizeof(int)))    /* length of option value */
            {
                close();
                return false;
            }
        }
        struct sockaddr_in serv_addr, cli_addr;
        socklen_t clilen;
        bzero((char*) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        if (bind(m_sockfd, (struct sockaddr*) &serv_addr,
                 sizeof(serv_addr)) < 0)
        {
            close();
            return false;
        }
        listen(m_sockfd,5);
        clilen = sizeof(cli_addr);
        m_newsockfd = accept(m_sockfd, (struct sockaddr*) &cli_addr, &clilen);
        if (m_newsockfd < 0)
        {
            close();
            return false;
        }
        if (prot == Socket::TCP)
        {
            if (0 > setsockopt(
                    m_newsockfd,            /* socket affected */
                    IPPROTO_TCP,     /* set option at TCP level */
                    TCP_NODELAY,     /* name of option */
                    (char *) &delayFlag,  /* the cast is historical cruft */
                    sizeof(int)))    /* length of option value */
            {
                close();
                return false;
            }
        }
        m_side = side;
        return true;
    }
    // Client side code ///////////////////////////////////////////////////////
    else if (side == Socket::Client) 
    {
        m_sockfd = socket(AF_INET, prot == Socket::TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
        if (m_sockfd < 0)
            return false;
        if (prot == Socket::TCP)
        {
            if (0 > setsockopt(
                    m_sockfd,            /* socket affected */
                    IPPROTO_TCP,     /* set option at TCP level */
                    TCP_NODELAY,     /* name of option */
                    (char *) &delayFlag,  /* the cast is historical cruft */
                    sizeof(int)))    /* length of option value */
            {
                close();
                return false;
            }
        }
        struct sockaddr_in serv_addr;
        struct hostent* server = gethostbyname(ip);
        if (server == NULL)
        {
            close();
            return false;
        }
        bzero((char*) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char*) server->h_addr,
              (char*) &serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(port);
        if (connect(m_sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        {
            close();
            return false;
        }
        m_side = side;
        return true;
    }
    return false;
}

bool Socket::close()
{
    bool retval = true;
    if (m_side == Socket::Server)
    {
        if (m_sockfd >= 0)
            ::close(m_sockfd);
        if (m_newsockfd >= 0)
            ::close(m_newsockfd);
        if (m_sockfd < 0 || m_newsockfd < 0)
            retval = false;
    }
    else if (m_side == Socket::Client)
    {
        if (m_sockfd >= 0)
            ::close(m_sockfd);
        else
            retval = false;
    }
    m_newsockfd = m_sockfd = -1;
    return retval;
}

bool Socket::write(const byte* data, unsigned size)
{
    int fd = m_side == Socket::Server ? m_newsockfd : m_sockfd;
    ssize_t bytesWritten, bytesToWrite;

    if (fd < 0)
        return false;

    bytesToWrite = size;
    while (bytesToWrite > 0)
    {
        bytesWritten = ::write(fd, data, bytesToWrite);
        if (bytesWritten < 0)
        {
            close();
            return false;
        }
        data += bytesWritten;
        bytesToWrite -= bytesWritten;
    }
    return true;
}

bool Socket::read(byte* data, unsigned size)
{
    int fd = m_side == Socket::Server ? m_newsockfd : m_sockfd;
    ssize_t bytesRead, bytesToRead;

    if (fd < 0)
        return false;

    bytesToRead = size;
    while (bytesToRead > 0)
    {
        bytesRead = ::read(fd, data, bytesToRead);
        if (bytesRead < 0)
        {
            close();
            return false;
        }
        data += bytesRead;
        bytesToRead -= bytesRead;
    }
    return true;
}
