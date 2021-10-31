// https://stackoverflow.com/questions/4432989/c-header-files-for-udp-in-windows
#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
