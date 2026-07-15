#ifndef CUT_NETWORK
    #define CUT_NETWORK

    #include "base.c"

    typedef cut_uptrsize Cut_Socket;

    #define CUT_INVALID_SOCKET ((Cut_Socket)-1)

    typedef enum Cut_Sock_Kind {
        CUT_SOCK_KIND_NONE,
        CUT_SOCK_KIND_IPV4,
        CUT_SOCK_KIND_IPV6
    } Cut_Sock_Kind;

    typedef enum Cut_Sock_Protocol {
        CUT_SOCK_PROTOCOL_TCP,
        CUT_SOCK_PROTOCOL_UDP
    } Cut_Sock_Protocol;

    // The port is in host byte order. "address" is in network byte order.
    typedef struct Cut_Endpoint {
        Cut_Sock_Kind kind;
        cut_u16 port;
        cut_u8 address[16];
    } Cut_Endpoint;

    typedef struct Cut_Ipv4_Endpoint {
        Cut_Endpoint endpoint;
    } Cut_Ipv4_Endpoint;

    typedef struct Cut_Ipv6_Endpoint {
        Cut_Endpoint endpoint;
    } Cut_Ipv6_Endpoint;

    /*
        Failure handling:

        Except for cut_sock_open, a network operation returning 0 has failed.
        Read the platform error immediately, before another networking or system call:
        - Windows: WSAGetLastError() (declared by <winsock2.h>).
        - Linux/macOS: errno (declared by <errno.h>).

        This applies to cut_sock_blocking, cut_sock_non_blocking, cut_sock_bind,
        cut_sock_connect, cut_sock_listen, cut_sock_accept, cut_sock_receive, and
        cut_sock_send. They use, respectively, ioctlsocket/fcntl, bind, connect,
        listen, accept, recv, and send. Consult the native error documentation for
        the operation that failed. EWOULDBLOCK/EAGAIN (or WSAEWOULDBLOCK) is expected
        when a non-blocking operation cannot proceed yet.

        cut_sock_open returns CUT_INVALID_SOCKET on failure. Read WSAGetLastError()
        on Windows or errno on Linux/macOS. A Windows WSAStartup failure is also made
        available through WSAGetLastError().

        An invalid string passed to cut_ipv4_from_str or cut_ipv6_from_str produces an
        endpoint whose kind is CUT_SOCK_KIND_NONE. This is not a socket error. Likewise,
        invalid arguments rejected by cutile (unsupported kind/protocol, invalid endpoint,
        null required output pointer, or a buffer larger than INT_MAX) have no meaningful
        native error code.

        On a successful cut_sock_receive, a received size of 0 means a TCP peer closed
        the connection cleanly; it is not a failure. cut_sock_close intentionally discards
        close errors. cut_sock_send returns 0 after a partial send, for which the native
        error code is not reliable; use a blocking socket when the whole buffer is required.
    */

    // Returns CUT_INVALID_SOCKET when the socket could not be opened.
    Cut_Socket cut_sock_open(Cut_Sock_Kind kind, Cut_Sock_Protocol protocol);
    void cut_sock_close(Cut_Socket socket);

    int cut_sock_blocking(Cut_Socket socket);
    int cut_sock_non_blocking(Cut_Socket socket);

    // An invalid string produces an endpoint whose kind is CUT_SOCK_KIND_NONE.
    Cut_Ipv4_Endpoint cut_ipv4_from_str(void *str0);
    Cut_Ipv6_Endpoint cut_ipv6_from_str(void *str0);

    int cut_sock_bind(Cut_Socket socket, Cut_Endpoint endpoint);
    int cut_sock_connect(Cut_Socket socket, Cut_Endpoint endpoint);
    int cut_sock_listen(Cut_Socket socket, int backlog);
    int cut_sock_accept(Cut_Socket socket, Cut_Socket *incoming_socket, Cut_Endpoint *incoming_endpoint);

    // A successful receive may report zero bytes when a stream peer has closed its connection.
    int cut_sock_receive(Cut_Socket socket, void *buf, cut_u64 buf_size, cut_u64 *received_size);
    // Sends the whole buffer. A partial send is reported as failure.
    int cut_sock_send(Cut_Socket socket, void *buf, cut_u64 buf_size);

    #if defined(CUT_NETWORK_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        typedef Cut_Socket Socket;
        typedef Cut_Sock_Kind Sock_Kind;
        typedef Cut_Sock_Protocol Sock_Protocol;
        typedef Cut_Endpoint Endpoint;
        typedef Cut_Ipv4_Endpoint Ipv4_Endpoint;
        typedef Cut_Ipv6_Endpoint Ipv6_Endpoint;

        #define INVALID_SOCKET CUT_INVALID_SOCKET

        #define SOCK_KIND_NONE CUT_SOCK_KIND_NONE
        #define SOCK_KIND_IPV4 CUT_SOCK_KIND_IPV4
        #define SOCK_KIND_IPV6 CUT_SOCK_KIND_IPV6

        #define SOCK_PROTOCOL_TCP CUT_SOCK_PROTOCOL_TCP
        #define SOCK_PROTOCOL_UDP CUT_SOCK_PROTOCOL_UDP

        #define sock_open         cut_sock_open
        #define sock_close        cut_sock_close
        #define sock_blocking     cut_sock_blocking
        #define sock_non_blocking cut_sock_non_blocking
        #define ipv4_from_str     cut_ipv4_from_str
        #define ipv6_from_str     cut_ipv6_from_str
        #define sock_bind         cut_sock_bind
        #define sock_connect      cut_sock_connect
        #define sock_listen       cut_sock_listen
        #define sock_accept       cut_sock_accept
        #define sock_receive      cut_sock_receive
        #define sock_send         cut_sock_send
    #endif
#endif

#if !defined(CUT_NETWORK_IMPL_INCLUDED) && (defined(CUT_NETWORK_IMPL) || defined(CUT_IMPL))
    #define CUT_NETWORK_IMPL_INCLUDED

    #if CUT_TARGET_OS == CUT_WINDOWS
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif
        #include <winsock2.h>
        #include <ws2tcpip.h>
        #include <limits.h>
        #pragma comment(lib, "Ws2_32.lib")
        typedef int cut_sockaddr_size;
    #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        #include <arpa/inet.h>
        #include <fcntl.h>
        #include <limits.h>
        #include <sys/socket.h>
        #include <unistd.h>
        typedef socklen_t cut_sockaddr_size;
    #endif

    cut_internal int cut_sock_api_init()
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            cut_persist int initialized;
            if (!initialized) {
                WSADATA data;
                int error = WSAStartup(MAKEWORD(2, 2), &data);
                if (error != 0) {
                    WSASetLastError(error);
                    return 0;
                }
                initialized = 1;
            }
        }
        #endif

        return 1;
    }

    cut_internal int cut_endpoint_to_sockaddr(Cut_Endpoint endpoint, struct sockaddr_storage *out, cut_sockaddr_size *out_size)
    {
        if (endpoint.kind == CUT_SOCK_KIND_IPV4) {
            struct sockaddr_in *address = (struct sockaddr_in *)out;
            *address = (struct sockaddr_in){
                .sin_family = AF_INET,
                .sin_port = htons(endpoint.port)
            };
            for (int i = 0; i < 4; i++)
                address->sin_addr.s_addr |= ((cut_u32)endpoint.address[i]) << (i * 8);
            *out_size = sizeof(*address);
            return 1;
        }

        if (endpoint.kind == CUT_SOCK_KIND_IPV6) {
            struct sockaddr_in6 *address = (struct sockaddr_in6 *)out;
            *address = (struct sockaddr_in6){
                .sin6_family = AF_INET6,
                .sin6_port = htons(endpoint.port)
            };
            for (int i = 0; i < 16; i++)
                address->sin6_addr.s6_addr[i] = endpoint.address[i];
            *out_size = sizeof(*address);
            return 1;
        }

        return 0;
    }

    cut_internal int cut_endpoint_from_sockaddr(struct sockaddr_storage *address, Cut_Endpoint *out)
    {
        if (address->ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)address;
            *out = (Cut_Endpoint){ .kind = CUT_SOCK_KIND_IPV4, .port = ntohs(ipv4->sin_port) };
            for (int i = 0; i < 4; i++)
                out->address[i] = ((cut_u8 *)&ipv4->sin_addr.s_addr)[i];
            return 1;
        }

        if (address->ss_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)address;
            *out = (Cut_Endpoint){ .kind = CUT_SOCK_KIND_IPV6, .port = ntohs(ipv6->sin6_port) };
            for (int i = 0; i < 16; i++)
                out->address[i] = ipv6->sin6_addr.s6_addr[i];
            return 1;
        }

        return 0;
    }

    Cut_Socket cut_sock_open(Cut_Sock_Kind kind, Cut_Sock_Protocol protocol)
    {
        if (!cut_sock_api_init())
            return CUT_INVALID_SOCKET;

        int family;
        if (kind == CUT_SOCK_KIND_IPV4)
            family = AF_INET;
        else if (kind == CUT_SOCK_KIND_IPV6)
            family = AF_INET6;
        else
            return CUT_INVALID_SOCKET;

        int type;
        int native_protocol;
        if (protocol == CUT_SOCK_PROTOCOL_TCP) {
            type = SOCK_STREAM;
            native_protocol = IPPROTO_TCP;
        } else if (protocol == CUT_SOCK_PROTOCOL_UDP) {
            type = SOCK_DGRAM;
            native_protocol = IPPROTO_UDP;
        } else {
            return CUT_INVALID_SOCKET;
        }

        #if CUT_TARGET_OS == CUT_WINDOWS
            SOCKET native_socket = socket(family, type, native_protocol);
            if (native_socket == INVALID_SOCKET)
                return CUT_INVALID_SOCKET;
        #else
            int native_socket = socket(family, type, native_protocol);
            if (native_socket == -1)
                return CUT_INVALID_SOCKET;

            #if CUT_TARGET_OS == CUT_MACOS
            {
                int no_sigpipe = 1;
                setsockopt(native_socket, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe));
            }
            #endif
        #endif

        return (Cut_Socket)native_socket;
    }

    void cut_sock_close(Cut_Socket socket)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
            closesocket((SOCKET)socket);
        #else
            close((int)socket);
        #endif
    }

    int cut_sock_blocking(Cut_Socket socket)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            u_long mode = 0;
            return ioctlsocket((SOCKET)socket, FIONBIO, &mode) == 0;
        }
        #else
        {
            int flags = fcntl((int)socket, F_GETFL, 0);
            return flags != -1 && fcntl((int)socket, F_SETFL, flags & ~O_NONBLOCK) != -1;
        }
        #endif
    }

    int cut_sock_non_blocking(Cut_Socket socket)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            u_long mode = 1;
            return ioctlsocket((SOCKET)socket, FIONBIO, &mode) == 0;
        }
        #else
        {
            int flags = fcntl((int)socket, F_GETFL, 0);
            return flags != -1 && fcntl((int)socket, F_SETFL, flags | O_NONBLOCK) != -1;
        }
        #endif
    }

    Cut_Ipv4_Endpoint cut_ipv4_from_str(void *str0)
    {
        Cut_Ipv4_Endpoint result = { .endpoint.kind = CUT_SOCK_KIND_IPV4 };
        if (inet_pton(AF_INET, str0, result.endpoint.address) != 1)
            result.endpoint.kind = CUT_SOCK_KIND_NONE;
        return result;
    }

    Cut_Ipv6_Endpoint cut_ipv6_from_str(void *str0)
    {
        Cut_Ipv6_Endpoint result = { .endpoint.kind = CUT_SOCK_KIND_IPV6 };
        if (inet_pton(AF_INET6, str0, result.endpoint.address) != 1)
            result.endpoint.kind = CUT_SOCK_KIND_NONE;
        return result;
    }

    int cut_sock_bind(Cut_Socket socket, Cut_Endpoint endpoint)
    {
        struct sockaddr_storage address = {0};
        cut_sockaddr_size address_size;
        if (!cut_endpoint_to_sockaddr(endpoint, &address, &address_size))
            return 0;
        #if CUT_TARGET_OS == CUT_WINDOWS
            return bind((SOCKET)socket, (struct sockaddr *)&address, address_size) == 0;
        #else
            return bind((int)socket, (struct sockaddr *)&address, address_size) == 0;
        #endif
    }

    int cut_sock_connect(Cut_Socket socket, Cut_Endpoint endpoint)
    {
        struct sockaddr_storage address = {0};
        cut_sockaddr_size address_size;
        if (!cut_endpoint_to_sockaddr(endpoint, &address, &address_size))
            return 0;
        #if CUT_TARGET_OS == CUT_WINDOWS
            return connect((SOCKET)socket, (struct sockaddr *)&address, address_size) == 0;
        #else
            return connect((int)socket, (struct sockaddr *)&address, address_size) == 0;
        #endif
    }

    int cut_sock_listen(Cut_Socket socket, int backlog)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
            return listen((SOCKET)socket, backlog) == 0;
        #else
            return listen((int)socket, backlog) == 0;
        #endif
    }

    int cut_sock_accept(Cut_Socket socket, Cut_Socket *incoming_socket, Cut_Endpoint *incoming_endpoint)
    {
        if (!incoming_socket)
            return 0;

        struct sockaddr_storage address = {0};
        cut_sockaddr_size address_size = sizeof(address);
        #if CUT_TARGET_OS == CUT_WINDOWS
            SOCKET accepted = accept((SOCKET)socket, (struct sockaddr *)&address, &address_size);
            if (accepted == INVALID_SOCKET)
                return 0;
        #else
            int accepted = accept((int)socket, (struct sockaddr *)&address, &address_size);
            if (accepted == -1)
                return 0;
        #endif

        *incoming_socket = (Cut_Socket)accepted;
        if (incoming_endpoint && !cut_endpoint_from_sockaddr(&address, incoming_endpoint)) {
            #if CUT_TARGET_OS == CUT_WINDOWS
                closesocket(accepted);
            #else
                close(accepted);
            #endif
            return 0;
        }
        return 1;
    }

    int cut_sock_receive(Cut_Socket socket, void *buf, cut_u64 buf_size, cut_u64 *received_size)
    {
        if (!received_size || buf_size > INT_MAX)
            return 0;
        #if CUT_TARGET_OS == CUT_WINDOWS
            int received = recv((SOCKET)socket, buf, (int)buf_size, 0);
            if (received == SOCKET_ERROR)
                return 0;
        #else
            int received = (int)recv((int)socket, buf, (size_t)buf_size, 0);
            if (received < 0)
                return 0;
        #endif
        *received_size = (cut_u64)received;
        return 1;
    }

    int cut_sock_send(Cut_Socket socket, void *buf, cut_u64 buf_size)
    {
        if (buf_size > INT_MAX)
            return 0;
        #if CUT_TARGET_OS == CUT_WINDOWS
            int sent = send((SOCKET)socket, buf, (int)buf_size, 0);
            return sent == (int)buf_size;
        #else
            #if CUT_TARGET_OS == CUT_LINUX
                int sent = (int)send((int)socket, buf, (size_t)buf_size, MSG_NOSIGNAL);
            #else
                int sent = (int)send((int)socket, buf, (size_t)buf_size, 0);
            #endif
            return sent == (int)buf_size;
        #endif
    }
#endif
