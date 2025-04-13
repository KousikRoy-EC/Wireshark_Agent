#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 2048

// 802.11 Frame Control Bitfield Structure
struct __attribute__((__packed__)) FrameControl
{
    unsigned protocol_version : 2;
    unsigned type : 2;
    unsigned subtype : 4;
    unsigned to_ds : 1;
    unsigned from_ds : 1;
    unsigned more_frag : 1;
    unsigned retry : 1;
    unsigned pwr_mgmt : 1;
    unsigned more_data : 1;
    unsigned protected_frame : 1;
    unsigned order : 1;
};

// MAC Header Structure
struct __attribute__((__packed__)) MACHeader
{
    struct FrameControl fc;
    unsigned short duration;
    unsigned char addr1[6];
    unsigned char addr2[6];
    unsigned char addr3[6];
    unsigned short seq_ctrl;
};

// Helper to print MAC address
void print_mac(const unsigned char *mac)
{
    for (int i = 0; i < 6; i++)
    {
        printf("%02X", mac[i]);
        if (i < 5)
            printf(":");
    }
}

// Interpret frame type/subtype
const char *frame_type_str(unsigned type, unsigned subtype)
{
    if (type == 0)
        return "Management";
    if (type == 1)
        return "Control";
    if (type == 2)
        return "Data";
    return "Reserved";
}

// Main packet capture and decode loop
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <monitor-interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *iface = argv[1];
    int sockfd;
    unsigned char buffer[BUFFER_SIZE];

    // Create raw socket
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind to the given interface
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = if_nametoindex(iface);

    if (bind(sockfd, (struct sockaddr *)&sll, sizeof(sll)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on interface %s...\n\n", iface);

    while (1)
    {
        ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (len < sizeof(struct MACHeader))
            continue;

        struct MACHeader *hdr = (struct MACHeader *)buffer;

        printf("----- 802.11 Frame -----\n");
        printf("Type: %s (%u), Subtype: %u\n",
               frame_type_str(hdr->fc.type, hdr->fc.subtype),
               hdr->fc.type, hdr->fc.subtype);
        printf("To DS: %u, From DS: %u, Retry: %u, Protected: %u\n",
               hdr->fc.to_ds, hdr->fc.from_ds, hdr->fc.retry, hdr->fc.protected_frame);

        printf("Address 1 (Receiver): ");
        print_mac(hdr->addr1);
        printf("\n");

        printf("Address 2 (Transmitter): ");
        print_mac(hdr->addr2);
        printf("\n");

        printf("Address 3 (BSSID/Source): ");
        print_mac(hdr->addr3);
        printf("\n");

        printf("Sequence Control: 0x%04X\n", ntohs(hdr->seq_ctrl));
        printf("-------------------------\n\n");

        usleep(100000); // 100ms delay to reduce flooding
    }

    close(sockfd);
    return 0;
}
