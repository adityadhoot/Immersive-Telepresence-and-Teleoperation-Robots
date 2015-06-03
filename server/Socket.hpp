/**
 * \file Socket.hpp
 * \author Billy Jun
 * \brief Socekt class declaration
 */
#ifndef SOCKET_HPP
#define SOCKET_HPP

typedef unsigned char byte;

class Socket
{
  public:
    enum Side { Server, Client };
    enum Protocol { TCP, UDP };
    enum BlockingMode { NonBlocking, Blocking };
    enum DelayMode { NoDelay, Delay };

  public:
    /// default constructor
    Socket();
    /// opens a socket while instantiating
    Socket(const char* ip, unsigned port, Side side, Protocol prot = TCP,
           BlockingMode block = Blocking, DelayMode delay = NoDelay);
    ~Socket();

    bool isOpen();
    bool open(const char* ip, unsigned port, Side side, Protocol prot = TCP,
              BlockingMode block = Blocking, DelayMode delay = NoDelay);
    bool close();

    bool write(const byte* data, unsigned size);
    bool read(byte* data, unsigned size);

  private:
    int m_sockfd;
    int m_newsockfd;
    Side m_side;
};

#endif
