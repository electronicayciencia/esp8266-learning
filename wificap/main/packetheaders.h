#include <stdint.h>
#include <sys/types.h>

#define LINKTYPE_IEEE802_11 105          /* IEEE 802.11 wireless LAN. */
#define LINKTYPE_IEEE802_11_RADIOTAP 127 /* Radiotap link-layer information followed by an 802.11 header. */
#define PCAP_MAGIC_NUMBER_US 0xA1B2C3D4
#define PCAP_VERSION_MAYOR 2
#define PCAP_VERSION_MINOR 4

/* PCAP header (https://wiki.wireshark.org/Development/LibpcapFileFormat)
D4 C3 B2 A1 <- magic, time in microseconds
02 00       <- mayor version
04 00       <- minor version
00 00 00 00 <- reserved
00 00 00 00 <- reserved
00 00 04 00 <- snaplen
7F 00 00 00 <- linktype 
*/
typedef struct {
    uint32_t magic;
    uint16_t mayor;
    uint16_t minor;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t snaplen;
    uint32_t linktype;
} pcap_header_t;

/* frame header
DF FF B1 57 <- timestamp
4C EB 08 00 <- microseconds
4C 00 00 00 <- size (captured)
4C 00 00 00 <- size (real)
*/
typedef struct {
    uint32_t timestamp;
    uint32_t microseconds;
    uint32_t size_file;
    uint32_t size_wire;
} frame_header_t;

/* minimal radiotap header: http://www.radiotap.org/
00 00 <- version v0 + pad
08 00 <- length (8)
00 00 00 00 <- present fields (none)
*/
#define RADIOTAP_HEADER_VERSION 0
#define RADIOTAP_PRESENT_FIELDS_NONE 0
#define RADIOTAP_PRESENT_FIELDS_FLAGS 0b10
typedef struct {
    uint8_t version;
    uint8_t pad;
    uint16_t header_size;
    uint32_t present_fields;
} radiotap_header_t;

#define RADIOTAP_FLAGS_INCLUDE_FCS 0x10
typedef struct {
    uint8_t flags;
    uint8_t pad;
} radiotap_header_flags_t;

