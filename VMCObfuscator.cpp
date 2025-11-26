#define OSC_HOST_LITTLE_ENDIAN 1
#define OUTPUT_BUFFER_SIZE 1024

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <chrono>
#include <random>
#include "kernel.cuh"
#include "ip/UdpSocket.h" // ✅ ip before osc
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "osc/OscOutboundPacketStream.h"
#include <cstring>
#include <atomic> 
#include <algorithm>


std::atomic<float> obfuscationIntensity = 1.0f;

#pragma comment(lib, "ws2_32.lib")

SOCKET setupSocket(int port) {
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        exit(1);
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    InetPton(AF_INET, L"127.0.0.1", &addr.sin_addr);
    bind(sock, (sockaddr*)&addr, sizeof(addr));
    return sock;
}
float randomFloat(float min, float max) {
    static std::default_random_engine e(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(e);
}
bool shouldGlitch(float probability = 0.05f) {
    static std::default_random_engine e(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(e) < probability;
}
std::string timestamp() {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[26];
    ctime_s(buf, sizeof(buf), &now);
    buf[24] = '\0'; // Remove newline
    return std::string(buf);
}
void sendPosition(const char* name, float x, float y, float z, SOCKET sock, sockaddr_in& dest) {
    char buffer[OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

    p << osc::BeginMessage("/VMC/Ext/Tra/Pos")
        << name << x << y << z
        << osc::EndMessage;

    sendto(sock, p.Data(), (int)p.Size(), 0, (sockaddr*)&dest, sizeof(dest));
}
void sendBlendshape(const std::string& name, float value, SOCKET sock, sockaddr_in& dest) {
    char buffer[OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

    p << osc::BeginMessage("/VMC/Ext/Blend/Val")
        << name.c_str() << value
        << osc::EndMessage;

    sendto(sock, p.Data(), (int)p.Size(), 0, (sockaddr*)&dest, sizeof(dest));
}
void blinkLoop(SOCKET sock, sockaddr_in& dest) {
    while (true) {
        int interval = rand() % 2000 + 2000; // 2000–4000 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        char buffer[OUTPUT_BUFFER_SIZE];
        osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);
        p << osc::BeginMessage("/VMC/Ext/Blend/Val")
            << "blink" << 1.0f
            << osc::EndMessage;

        sendto(sock, p.Data(), (int)p.Size(), 0, (sockaddr*)&dest, sizeof(dest));
    }
}
void blendshapeLoop(SOCKET sock, sockaddr_in& dest) {
    std::vector<std::string> shapes = { "smile", "frown", "squint", "jawOpen", "eyeWide" };
    std::default_random_engine e(std::random_device{}());
    std::uniform_real_distribution<float> valueDist(0.3f, 1.0f);
    std::uniform_int_distribution<int> indexDist(0, shapes.size() - 1);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 3000 + 2000));
        std::string shape = shapes[indexDist(e)];
        float value = valueDist(e) * obfuscationIntensity.load(); // 👈 scaled here
        sendBlendshape(shape, value, sock, dest);
    }
}
class VMCListener : public osc::OscPacketListener {
public:
    VMCListener(SOCKET sock, sockaddr_in dest) : sendSock(sock), destAddr(dest) {}

protected:
    SOCKET sendSock;
    sockaddr_in destAddr;

    virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) override {
        std::string ts = timestamp();
        std::cout << "[" << ts << "] Received OSC: " << m.AddressPattern() << std::endl;

        try {
            if (std::strcmp(m.AddressPattern(), "/VMC/Ext/Tra/Pos") == 0) {
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                const char* name;
                float x, y, z;
                args >> name >> x >> y >> z >> osc::EndMessage;

                // Regular positional offset
                x += randomFloat(-0.3f, 0.3f) * obfuscationIntensity;
                y += randomFloat(-0.2f, 0.2f) * obfuscationIntensity;
                z += randomFloat(-0.3f, 0.3f) * obfuscationIntensity;


                // Thresholded glitch
                if (shouldGlitch()) {
                    x += randomFloat(-0.2f, 0.2f);
                    y += randomFloat(-0.1f, 0.1f);
                    z += randomFloat(-0.2f, 0.2f);
                }

                sendPosition(name, x, y, z, sendSock, destAddr);

                std::cout << "Sent position: " << name
                    << " x=" << x << " y=" << y << " z=" << z << std::endl;
            }

        }
        catch (osc::Exception& e) {
            std::cerr << "Error parsing message: " << e.what() << "\n";
        }
        if (std::strcmp(m.AddressPattern(), "/VMC/Ext/Tra/Rot") == 0) {
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            const char* name;
            float x, y, z;
            args >> name >> x >> y >> z >> osc::EndMessage;

            // Regular rotational jitter
            x += randomFloat(-2.0f, 2.0f) * obfuscationIntensity;
            y += randomFloat(-2.0f, 2.0f) * obfuscationIntensity;
            z += randomFloat(-2.0f, 2.0f) * obfuscationIntensity;


            // Thresholded glitch
            if (shouldGlitch(obfuscationIntensity * 0.1f)) {
                x += randomFloat(-0.2f, 0.2f) * obfuscationIntensity;
                // etc.
            }


            char buffer[OUTPUT_BUFFER_SIZE];
            osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);
            p << osc::BeginMessage("/VMC/Ext/Tra/Rot")
                << name << x << y << z
                << osc::EndMessage;
            sendto(sendSock, p.Data(), (int)p.Size(), 0, (sockaddr*)&destAddr, sizeof(destAddr));
        }
        if (std::strcmp(m.AddressPattern(), "/obfuscator/intensity") == 0) {
            try {
                float newIntensity;
                m.ArgumentStream() >> newIntensity >> osc::EndMessage;
                newIntensity = std::clamp(newIntensity, 0.0f, 1.0f);
                obfuscationIntensity.store(newIntensity);
                std::cout << "Updated intensity: " << newIntensity << std::endl;
            }
            catch (osc::Exception& e) {
                std::cerr << "Error parsing intensity: " << e.what() << "\n";
            }
            return;
        }





    }
};

int main() {
    // Setup destination socket
    SOCKET sendSock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(39540);
    InetPton(AF_INET, L"127.0.0.1", &dest.sin_addr);

    // 👁️ Start blink loop
    std::thread blinkThread([&]() { blinkLoop(sendSock, dest); });
    blinkThread.detach();

    // 😐 Start blendshape loop
    std::thread blendThread([&]() { blendshapeLoop(sendSock, dest); });
    blendThread.detach();

    // 🧠 Start listener for VMC tracking data
    VMCListener listener(sendSock, dest);
    UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, 39539), &listener);
    socket.Run(); // Blocking call
}