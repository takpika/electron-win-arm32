// win-arm32 real-header supplement: AI_DNS_ONLY (getaddrinfo hint) the SDK <ws2def.h> defines
// (0x00000010) and llvm-mingw omits; net/dns/host_resolver_system_task.cc uses it.
#ifndef _WRT_WS2DEF_AI_DNS_ONLY_SHIM_
#define _WRT_WS2DEF_AI_DNS_ONLY_SHIM_
#include_next <ws2def.h>
#ifndef AI_DNS_ONLY
#define AI_DNS_ONLY 0x00000010
#endif
// SO_RANDOMIZE_PORT (SOL_SOCKET option) the SDK <ws2def.h>:212 defines and llvm-mingw omits;
// net/socket/udp_socket_win.cc:465 passes it to setsockopt to randomize wildcard-port assignment.
#ifndef SO_RANDOMIZE_PORT
#define SO_RANDOMIZE_PORT 0x3005
#endif
#endif
