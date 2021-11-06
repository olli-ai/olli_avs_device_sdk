
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdbool.h>
#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_IP, IPPROTO_ICMP, INET_ADDRSTRLE
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>
#include "SampleApp/olli_alexa.h"

#ifdef __cplusplus
extern "C"
{
#endif

static struct olli_data_control {
    int thread_working;
    int fd;
} olli_data;
#define OLLI_ALEXA_SOCK "/tmp/alexa.sock"

// Return socket_id
int olli_create_socket(const char *addrr, bool server) {
    struct sockaddr_un saun, fsaun;
    int socket_id_return;
    int socket_id_local_server = -1;
    int len;

socket_renew:
    if (socket_id_local_server >= 0) {
        close(socket_id_local_server);
    }
    // socket create and verification
    socket_id_local_server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_id_local_server == -1) {
        printf("socket creation failed...\n");
        sleep(2);
        goto socket_renew;
    }
    bzero(&saun, sizeof(saun));
    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, addrr);
    if (server) {
        unlink(addrr);
        len = sizeof(saun.sun_family) + strlen(saun.sun_path);
        // Binding newly created socket to given IP and verification
        if ((bind(socket_id_local_server, (struct sockaddr *)&saun, len)) != 0) {
            printf("socket server bind failed...\n");
            sleep(2);
            goto socket_renew;
        }
        // Now server is ready to listen and verification
        if ((listen(socket_id_local_server, 5)) != 0) {
            printf("Listen server failed...\n");
            sleep(2);
            goto socket_renew;
        } else {
            printf("Socket server listening with address\n");
        }
        len = sizeof(fsaun);
        // Accept the data packet from client and verification
        socket_id_return = accept(socket_id_local_server, (struct sockaddr *)&fsaun, (socklen_t*)&len);
        return socket_id_return;
    } else {
        socket_id_return = socket_id_local_server;
        len = sizeof(saun.sun_family) + strlen(saun.sun_path);
        if (connect(socket_id_return, (struct sockaddr *)&saun, len) != 0) {
            printf("connection with the server failed...\n");
            sleep(2);
            goto socket_renew;
        } else {
            printf("connected to the server with address\n");
            return socket_id_return;
        }
    }
    return -1;
}

void olli_report_state(enum alexa_state state)
{
    if (olli_data.fd < 0) {
        printf("report not success by socket failed\n");
    }
    ssize_t n;
    struct olli_msg m_data;
    m_data.state = state;
    do {
        n = send(olli_data.fd, &m_data, sizeof(m_data), 0);
    } while (n < 0 && errno == EINTR);
}

void olli_work_loop_read_data(void (*cb_func)(void *msg, void *ptr), void *user_data) 
{
    int ret = -1;
    struct olli_msg msg = {};
    olli_data.thread_working = true;
    while (olli_data.thread_working) {
        olli_data.fd = olli_create_socket(OLLI_ALEXA_SOCK, true);
        printf("Alexa server open success\n");
        while (olli_data.fd >= 0)
        {
            ret = read(olli_data.fd, &msg, sizeof(msg));
            if (ret != sizeof(msg)) {
                close(olli_data.fd);
                olli_data.fd = -1;
                break;
            }
            printf("Receive state %d\n", msg.state);
            cb_func((void *)&msg, user_data);
        }
        sleep(5);
    }
}

#ifdef __cplusplus
}
#endif