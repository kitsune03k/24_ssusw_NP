#include "hw2_header_20181755.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
            address_buffer, sizeof(address_buffer),
            service_buffer, sizeof(service_buffer),
            NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);


    printf("Creating socket...\n");
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
            peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Connecting...\n");
    if (connect(socket_peer,
                peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);

    printf("Connected.\n");
    printf("To send data, enter text followed by enter.\n\n");

    // local variable - 반복 사용시 초기화 필요
    char buf_in[SIZE_BUF];
    char buf_out[SIZE_BUF];

    while(1) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(0, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer+1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        if (FD_ISSET(socket_peer, &reads)) {
            memset(buf_in, 0, SIZE_BUF); // fx2에서 여러번 send/recv -> buf 초기화 해줄 필요 있음.

            int bytes_received = recv(socket_peer, buf_in, SIZE_BUF, 0); // buf에 recv 받음
            if (bytes_received < 1) { // recv 함수는 받은 데이터 크기를 return함
                printf("Connection closed by USER.\n");
                break;
            }
            printf("%s\n", buf_in);
#if DEBUG
    printf("bytes_received : %d\n", bytes_received);
#endif
        }

        if(FD_ISSET(0, &reads)) {
            memset(buf_out, 0, SIZE_BUF); // fx2에서 여러번 send/recv -> buf 초기화 해줄 필요 있음.

            if (!fgets(buf_out, SIZE_BUF, stdin)) break;
            int bytes_sent = send(socket_peer, buf_out, strlen(buf_out), 0);

#if DEBUG
    printf("bytes_sent: %d\n", bytes_sent);
#endif
            printf("\n");
        }
    } //end while(1)

    printf("Closing socket...\n");
    CLOSESOCKET(socket_peer);

    printf("Finished.\n");
    return 0;
}
