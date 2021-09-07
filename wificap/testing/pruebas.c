/* Compile line:

make pruebas

*/

#include "../main/packetheaders.h"
#include <stdint.h>
#include <stdio.h>

void hexaprint(void *structure, size_t size) {
    for (int i=0; i < size; i++) {
        printf("%02X ", *((uint8_t*)structure + i));
    }

    printf("\n");

}

int main(int argv, char **argc) {
    /* pcap */
    pcap_header_t pcaphead = {
        .magic = PCAP_MAGIC_NUMBER_US,
        .mayor = PCAP_VERSION_MAYOR,
        .minor = PCAP_VERSION_MINOR,
        .snaplen = 0x40000,
        .linktype = LINKTYPE_IEEE802_11_RADIOTAP,
    };

    hexaprint(&pcaphead, sizeof(pcaphead));

    /****** frame ******/

    frame_header_t framehead = {
        .timestamp = 0,
        .microseconds = 0,
        .size_file = 121,
        .size_wire = 121,
    };

    hexaprint(&framehead, sizeof(framehead));


    /****** radiotap ******/

    radiotap_header_t radiohead = {
        .version = RADIOTAP_HEADER_VERSION,
        .header_size = sizeof(radiohead),
        .present_fields = RADIOTAP_PRESENT_FIELDS_NONE,
    };

    hexaprint(&radiohead, sizeof(radiohead));


}
