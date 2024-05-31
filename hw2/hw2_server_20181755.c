#include "hw2_header_20181755.h"

#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// time function
void fx1(char *buf_out) {
    time_t t = time(NULL);

    struct tm tm = *localtime(&t);

    sprintf(buf_out, "current time is:\n%04d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm
    .tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// calc function
void fx2(char *buf_in, char* buf_out) {
    int i1, i2, result;
    char op;

#if DEBUG
    printf("*** fx2 *** in: %s\n", buf_in);
#endif

    sscanf(buf_in, "%d %c %d", &i1, &op, &i2);

#if DEBUG
    printf("*** fx2 *** calc: %d %c %d\n", i1, op, i2);
#endif

    bool errswitch = false;
    switch(op) {
        case '+':
            result = i1 + i2;
            break;
        case '-':
            result = i1 - i2;
            break;
        case '*':
            result = i1 * i2;
            break;
        default:
            errswitch = true;
    }

    if(errswitch) {
        sprintf(buf_out, "calc error!\n");
    }
    else {
        sprintf(buf_out, "result: %d\n", result);
    }
#if DEBUG
    printf("*** fx2 *** out: %s\n", buf_out);
#endif
}

int whatfx(const char *s) {
#if DEBUG
    printf("*** whatfx *** s: %s\n", s);
#endif

    int ret = 0;

    if(strcmp(s, "quit") == 0) {
        ret = 3;
    }
    else if(strcmp(s, "1") == 0) {
        ret = 1;
    }
    else if(strcmp(s, "2") == 0) {
        ret = 2;
    }
    else {
        ret = -1;
    }
#if DEBUG
    printf("*** whatfx *** ret: %d\n", ret);
#endif
    return ret;
}

int main() {
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address);


    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                           bind_address->ai_socktype, bind_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Binding socket to local address...\n");
    if(bind(socket_listen,
            bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);


    printf("Listening...\n");
    if(listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Waiting for connections...\n");
    while(1) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        SOCKET socket_client = accept(socket_listen,
                                      (struct sockaddr *) &client_address, &client_len);
        if(!ISVALIDSOCKET(socket_client)) {
            fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        char address_buffer[100];
        getnameinfo((struct sockaddr *) &client_address,
                    client_len, address_buffer, sizeof(address_buffer), 0, 0,
                    NI_NUMERICHOST);
        printf("New connection from %s\n", address_buffer);


        const char * const msgsel = "====================\nchoices\n1 -> current time\n2 -> calculator\nquit -> "
                                    "exit this program\n";
        const char * const msgfx2 = "send any math equation, division not supported\n";
        const int msgsel_size = strlen(msgsel);
        const int msgfx2_size = strlen(msgfx2);


        int pid = fork();

        if(pid == 0) { //child process
            CLOSESOCKET(socket_listen); // socket_listen은 parent proc이 써야하기에, child proc에서는 닫아야 한다.

            char read[SIZE_BUF];
            char buf_fx2[SIZE_BUF];
            char buf_rslt[SIZE_BUF];

            while(1) {
                memset(read, 0, SIZE_BUF); // read 초기화
                memset(buf_rslt, 0, SIZE_BUF); // buf_rslt 초기화
                memset(buf_fx2, 0, SIZE_BUF); // buf_fx2 초기화

                send(socket_client, msgsel, msgsel_size, 0); // 선택 메세지 보냄
                recv(socket_client, read, SIZE_BUF, 0); // read에 선택 값 입력받음
                read[strlen(read)-1] = 0; // 엔터 제거

                switch(whatfx(read)) {
                    case 1:{ // fx1
                        fx1(buf_rslt); // fx1 -> buf_rslt에 값(현재 시간) 기록
                        break;
                    }
                    case 2: { // fx2
                        send(socket_client, msgfx2, msgfx2_size, 0); // 수학 식 입력하라는 메세지 보냄
                        recv(socket_client, buf_fx2, SIZE_BUF, 0); // buf_fx2에 입력 받음
                        fx2(buf_fx2, buf_rslt); // fx2 -> buf_fx2를 파싱하여, buf_rslt에 값(계산 결과) 기록
                        break;
                    }
                    case 3: { // quit
                        send(socket_client, "", 0, 0); // client는 received byte가 1보다 작을경우 종료함
                        printf("closing %d...\n", socket_client);
                        CLOSESOCKET(socket_client); // child proc에서 쓰던 socket_client 닫고 연결 종료
                        exit(0); // child proc 종료
                    }
                    default:
                        sprintf(buf_rslt, "input correctly!!\n"); // 오류 : buf_rslt에 값(에러메세지) 기록
                        break;
                }

                send(socket_client, buf_rslt, strlen(buf_rslt), 0); // buf_rslt 보내기
            }
        }

        CLOSESOCKET(socket_client); // socket_client는 child proc이 써야하기에, parent proc에서는 닫아야 한다.
    }

    CLOSESOCKET(socket_listen); // 서버가 최종적으로 종료될 떄, socket_listen을 닫는다.

    printf("Finished.\n");

    return 0;
}
