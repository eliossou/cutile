#include <stdio.h>

#define CUT_NETWORK_IMPL
#define CUT_SHORT_NAMES
#include "../network.c"

#define CUT_PRINT_IMPL
#include "../print.c"

int main()
{
    Ipv4_Endpoint server = ipv4_from_str("127.0.0.1");
    server.endpoint.port = 3490;

    Socket socket = sock_open(SOCK_KIND_IPV4, SOCK_PROTOCOL_TCP);
    if (socket == INVALID_SOCKET) {
        print(fstr0("Could not open the client socket.\n"));
        return 1;
    }
    if (!sock_connect(socket, server.endpoint)) {
        print(fstr0("Could not connect to 127.0.0.1:3490. Start the server first.\n"));
        sock_close(socket);
        return 1;
    }
    print(fstr0("Connected to 127.0.0.1:3490.\n"));

    print(fstr0("Type a message and press Return. Type disconnect to close the connection.\n"));
    cut_u8 input[1024];
    while (fgets((char *)input, sizeof(input), stdin)) {
        cut_u64 input_size = 0;
        while (input[input_size])
            input_size++;

        if (!sock_send(socket, input, input_size)) {
            print(fstr0("Could not send the message.\n"));
            sock_close(socket);
            return 1;
        }

        cut_u64 message_size = input_size;
        while (message_size && (input[message_size - 1] == '\n' || input[message_size - 1] == '\r'))
            message_size--;

        static cut_u8 disconnect[] = "disconnect";
        int disconnect_requested = message_size == sizeof(disconnect) - 1;
        for (cut_u64 i = 0; disconnect_requested && i < message_size; i++)
            disconnect_requested = input[i] == disconnect[i];

        if (disconnect_requested) {
            print(fstr0("Disconnect requested.\n"));
            break;
        }

        print(fstr0("Message sent.\n"));
    }

    sock_close(socket);
    print(fstr0("Client stopped.\n"));
    return 0;
}
