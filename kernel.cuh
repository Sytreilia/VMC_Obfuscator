#pragma once
#ifdef __CUDACC__
__global__ void applyNoise(char* data, int len);
#endif

void processAndSend(char* buffer, int len, SOCKET sendSock, sockaddr_in& dest);
