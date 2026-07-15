#define CUT_NETWORK_IMPL
#define CUT_SHORT_NAMES
#include "../network.c"

#define CUT_PRINT_IMPL
#include "../print.c"

int main()
{
    Ipv4_Endpoint endpoint = ipv4_from_str("127.0.0.1");
    endpoint.endpoint.port = 3490;

    Socket listener = sock_open(SOCK_KIND_IPV4, SOCK_PROTOCOL_TCP);
    if (listener == INVALID_SOCKET) {
        print(fstr0("Could not open the listening socket.\n"));
        return 1;
    }
    if (!sock_bind(listener, endpoint.endpoint) || !sock_listen(listener, 8)) {
        print(fstr0("Could not bind or listen on 127.0.0.1:3490.\n"));
        sock_close(listener);
        return 1;
    }
    print(fstr0("Listening on 127.0.0.1:3490.\n"));

    Socket client;
    Endpoint client_endpoint;
    if (!sock_accept(listener, &client, &client_endpoint)) {
        print(fstr0("Could not accept a client connection.\n"));
        sock_close(listener);
        return 1;
    }
    print(fstr0("Client connected.\n"));

    cut_u8 received[256];
    cut_u8 line[1024];
    cut_u64 line_size = 0;
    int disconnect_requested = 0;
    while (!disconnect_requested) {
        cut_u64 received_size;
        if (!sock_receive(client, received, sizeof(received), &received_size)) {
            print(fstr0("Could not receive a client message.\n"));
            sock_close(client);
            sock_close(listener);
            return 1;
        }
        if (received_size == 0) {
            print(fstr0("Client disconnected.\n"));
            break;
        }

        for (cut_u64 i = 0; i < received_size; i++) {
            if (received[i] == '\n') {
                if (line_size && line[line_size - 1] == '\r')
                    line_size--;

                print(fstr0("Client: %\n"), format_str(u8arrview_ptr(line, line_size)));

                static cut_u8 disconnect[] = "disconnect";
                disconnect_requested = line_size == sizeof(disconnect) - 1;
                for (cut_u64 j = 0; disconnect_requested && j < line_size; j++)
                    disconnect_requested = line[j] == disconnect[j];

                line_size = 0;
                if (disconnect_requested) {
                    print(fstr0("Disconnect requested by client.\n"));
                    break;
                }
            } else if (line_size < sizeof(line)) {
                line[line_size++] = received[i];
            } else {
                print(fstr0("Client message was too long; discarding it.\n"));
                line_size = 0;
            }
        }
    }

    sock_close(client);
    sock_close(listener);
    print(fstr0("Server stopped.\n"));
    return 0;
}
