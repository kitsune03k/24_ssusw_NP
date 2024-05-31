#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>

typedef struct addrinfo adin;

int errHandler(int status4, int status6) {
    int errCase = 0;

    int ret = -1;
    if(status4 == 0 && status6 == 0) {      // 문제 없으면
        ret = 0;
    }
    else if(status4 != 0 && status6 != 0) { // 둘 다 문제있으면
        fprintf(stderr, "getaddrinfo IPv4: %s\n", gai_strerror(status4));
        fprintf(stderr, "getaddrinfo IPv6: %s\n", gai_strerror(status6));
        ret = 3;
    }
    else {
        if(status4 == 0) {    // v6만 오류시
            fprintf(stderr, "getaddrinfo IPv6: %s\n", gai_strerror(status6));
            ret = 1;
        }
        else {                // v4만 오류시
            fprintf(stderr, "getaddrinfo IPv4: %s\n", gai_strerror(status4));
            ret = 2;
        }
    }

    return ret;
}

void prnInet(adin *res4) { // IPv4
    char ipstr[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN = 16 (in.h)
    adin *p4;

    // linkedlist 정보 탐색
    for(p4 = res4; p4 != NULL; p4 = p4->ai_next) {
        void *addr;

        struct sockaddr_in *ipv4 = (struct sockaddr_in *) p4->ai_addr;
        addr = &(ipv4->sin_addr);

        inet_ntop(p4->ai_family, addr, ipstr, sizeof ipstr);
        printf("- IPv4: %s\n", ipstr);
    }
}

void prnInet6(adin *res6) { // IPv6
    char ipstr[INET6_ADDRSTRLEN]; // INET6_ADDRSTRLEN = 46 (in6.h)
    adin *p6;

    // linkedlist 정보 탐색
    for(p6 = res6; p6 != NULL; p6 = p6->ai_next) {
        void *addr;

        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p6->ai_addr;
        addr = &(ipv6->sin6_addr);

        inet_ntop(p6->ai_family, addr, ipstr, sizeof ipstr);
        printf("- IPv6: %s\n", ipstr);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "usage: getaddrinfo hostname\n");
        return 1;
    }

    char hostname[100];
    strcpy(hostname, argv[1]); // argv[1](터미널에서 입력값)에서 검색 원하는 주소 복사

    adin hints4;
    adin hints6;

    memset(&hints4, 0, sizeof hints4);
    memset(&hints6, 0, sizeof hints6);

    hints4.ai_flags = AI_PASSIVE; // 모든 인터페이스에서 들어오는것을 다 듣겠다.
    hints4.ai_family = AF_INET; // IPv4만 받겠다.
    hints4.ai_socktype = SOCK_STREAM; // TCP

    hints6.ai_flags = AI_PASSIVE; // 모든 인터페이스에서 들어오는것을 다 듣겠다.
    hints6.ai_family = AF_INET6; // IPv6만 받겠다.
    hints6.ai_socktype = SOCK_STREAM; // TCP

    adin *res4;
    adin *res6;

    // 주소 정보 불러옴, 에러처리: 0이 아닐경우 문제 있음
    int status4 = getaddrinfo(hostname, NULL, &hints4, &res4); // IPv4
    int status6 = getaddrinfo(hostname, NULL, &hints6, &res6); // IPv6

    int errsel = errHandler(status4, status6);

    switch(errsel) {
        case 0: { // 둘 다 오류없음!
            prnInet(res4);
            prnInet6(res6);
            freeaddrinfo(res4);
            freeaddrinfo(res6);
            break;
        }
        case 1: { // v6만 오류, v4 정보 출력
            prnInet(res4);
            freeaddrinfo(res4);
            break;
        }
        case 2: { // v4만 오류, v6 정보 출력
            prnInet6(res6);
            freeaddrinfo(res6);
            break;
        }
        default:;
    }

    return 0;
}