#include <winsock2.h>
#include <cuda_runtime.h>
#include "kernel.cuh"

__global__ void applyNoise(char* data, int len) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    if (i < len) {
        data[i] ^= (i % 2) ? 1 : 0;
    }
}

void processAndSend(char* buffer, int len, SOCKET sendSock, sockaddr_in& dest) {
    char* d_buffer;
    cudaMalloc(&d_buffer, len);
    cudaMemcpy(d_buffer, buffer, len, cudaMemcpyHostToDevice);
    applyNoise << <(len + 255) / 256, 256 >> > (d_buffer, len);
    cudaMemcpy(buffer, d_buffer, len, cudaMemcpyDeviceToHost);
    cudaFree(d_buffer);
    sendto(sendSock, buffer, len, 0, (sockaddr*)&dest, sizeof(dest));
}
