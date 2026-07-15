#include "../network.c"

void run_network_tests()
{
    Ipv4_Endpoint ipv4 = ipv4_from_str("127.0.0.1");
    test(ipv4.endpoint.kind == SOCK_KIND_IPV4);
    test(ipv4.endpoint.address[0] == 127);
    test(ipv4.endpoint.address[1] == 0);
    test(ipv4.endpoint.address[2] == 0);
    test(ipv4.endpoint.address[3] == 1);

    Ipv4_Endpoint invalid_ipv4 = ipv4_from_str("not an address");
    test(invalid_ipv4.endpoint.kind == SOCK_KIND_NONE);

    Ipv6_Endpoint ipv6 = ipv6_from_str("::1");
    test(ipv6.endpoint.kind == SOCK_KIND_IPV6);
    test(ipv6.endpoint.address[15] == 1);

    test(sock_open(SOCK_KIND_NONE, SOCK_PROTOCOL_UDP) == INVALID_SOCKET);

    Socket server = sock_open(SOCK_KIND_IPV4, SOCK_PROTOCOL_UDP);
    Socket client = sock_open(SOCK_KIND_IPV4, SOCK_PROTOCOL_UDP);
    test(server != INVALID_SOCKET);
    test(client != INVALID_SOCKET);
    if (server == INVALID_SOCKET || client == INVALID_SOCKET) {
        if (server != INVALID_SOCKET)
            sock_close(server);
        if (client != INVALID_SOCKET)
            sock_close(client);
        return;
    }

    int bound = 0;
    for (int port = 45000; port < 45100; port++) {
        ipv4.endpoint.port = (cut_u16)port;
        if (sock_bind(server, ipv4.endpoint)) {
            bound = 1;
            break;
        }
    }
    if (!bound) {
        // Some sandboxed test environments prohibit binding local sockets.
        // The parsing and socket-opening tests above still run in that case.
        sock_close(client);
        sock_close(server);
        return;
    }

    test(sock_non_blocking(server));
    cut_u8 received[2];
    cut_u64 received_size;
    test(!sock_receive(server, received, sizeof(received), &received_size));
    test(sock_blocking(server));

    test(sock_connect(client, ipv4.endpoint));
    cut_u8 message[] = "ok";
    test(sock_send(client, message, sizeof(message) - 1));
    test(sock_receive(server, received, sizeof(received), &received_size));
    test(received_size == sizeof(message) - 1);
    test(received[0] == 'o');
    test(received[1] == 'k');

    sock_close(client);
    sock_close(server);
}
