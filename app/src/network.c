// Use: netcat -u 192.168.7.2 12345
// to test on host

#define MAX_LEN 1500
#define PORT 12345

#include "network.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "shutdown.h"
#include "../../hal/include/hal/beatPattern.h"
#include "../../hal/include/hal/joystick.h"

static pthread_t networkThreadId;

static void* rx(void* args);
static void readFile(char* filePath, char* buff, unsigned int maxLength);

static void sendResponse(struct sockaddr_in sinRemote, int socketDescriptor,
                        char* messageRx);

void Network_init(void)
{
    pthread_create(&networkThreadId, NULL, &rx, NULL);
}

void Network_cleanup(void)
{
    pthread_join(networkThreadId, NULL);
}

static void* rx(void* args)
{
    (void)args;

    // Address
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                   // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long
	sin.sin_port = htons(PORT);                 // Host to Network short

    // Create the socket for UDP
    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    // Bind the socket to the port (PORT) that we specify
    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

    while (!Shutdown_isShutdown()) {
        // Get the data (blocking)
        // Will change sin (the address) to be the address of the client.
        // Note: sin passes information in and out of call!
        struct sockaddr_in sinRemote;
        unsigned int sin_len = sizeof(sinRemote);
        char messageRx[MAX_LEN];

        // Pass buffer size - 1 for max # bytes so room for the null (string data)
        int bytesRx = recvfrom(socketDescriptor,
                messageRx, MAX_LEN - 1, 0, 
                (struct sockaddr *) &sinRemote, &sin_len);

        // Make it null terminated (so string functions work)
        // - recvfrom given max size - 1, so there is always room for the null
        messageRx[bytesRx] = 0;

        // Extract the value from the message:
        // (process the message any way your app requires).

        sendResponse(sinRemote, socketDescriptor, messageRx);

    }
    
    // Close
    close(socketDescriptor);

    return NULL;
}

static void sendResponse(struct sockaddr_in sinRemote, int socketDescriptor,
                        char* messageRx)
{
        // Compose the reply message:
        // (NOTE: watch for buffer overflows!).
        char messageTx[MAX_LEN];
        
        // change drum beat mode
        // change volume & tempo
        // play any one of the sounds your drum beats use
        // shutting down program

        if (strncmp(messageRx, "stop", strnlen("stop", MAX_LEN)) == 0) {
            Shutdown_signalShutdown();
            snprintf(messageTx, MAX_LEN, "Program terminating.\n");
        } else if ((strncmp(messageRx, "help", strnlen("help", MAX_LEN)) == 0) || 
                   (strncmp(messageRx, "?", strnlen("?", MAX_LEN)) == 0)) {
            char* helpMessage = 
                "Accepted command examples: \n"
                "volumeUp     -- increase the volume by 5 (MAX: 100).\n"
                "volumeDown   -- decrease the volume by 5 (MIN: 0).\n"
                "tempoUp      -- increase the bpm by 5    (MAX: 300).\n"
                "tempoDown    -- decrease the bpm by 5    (MIN: 40).\n"
                "mode #       -- change beat mode to mode # (between 0 and 2).\n"
                "play #       -- play drum sound # (between 1 and 3).\n"
                "stop         -- cause the server program to end.\n";
            snprintf(messageTx, MAX_LEN, helpMessage);
        } 
        else if ((strncmp(messageRx, "volumeUp", strnlen("volumeUp", MAX_LEN)) == 0)){
            increaseVolume();
            snprintf(messageTx, MAX_LEN, "volume %d\n", 
                    getVolume());
        } 
        else if (strncmp(messageRx, "volumeDown", strnlen("volumeDown", MAX_LEN)) == 0){
            decreaseVolume();
            snprintf(messageTx, MAX_LEN, "volume %d\n", 
                    getVolume());
        } else if (strncmp(messageRx, "volume", strnlen("volume", MAX_LEN)) == 0) {
            snprintf(messageTx, MAX_LEN, "volume %d\n", 
                    getVolume());
        }
        else if (strncmp(messageRx, "tempoUp", strnlen("tempoUp", MAX_LEN)) == 0){
            increaseTempo();
            snprintf(messageTx, MAX_LEN, "tempo %d\n", 
                    getBPM());
        } 
        else if (strncmp(messageRx, "tempoDown", strnlen("tempoDown", MAX_LEN)) == 0){
            decreaseTempo();
            snprintf(messageTx, MAX_LEN, "tempo %d\n", 
                    getBPM());
        } 
        else if (strncmp(messageRx, "tempo", strnlen("tempo", MAX_LEN)) == 0){
            snprintf(messageTx, MAX_LEN, "tempo %d\n", 
                    getBPM());
        } else if (strncmp(messageRx, "curmode", strnlen("curmode", MAX_LEN)) == 0) {
            snprintf(messageTx, MAX_LEN, "mode %d\n", getBeatNum());
        }
        else if (strncmp(messageRx, "mode ", strnlen("mode ", MAX_LEN)) == 0){
            int num = (messageRx[strnlen("mode ", MAX_LEN)]) - '0';
            if (num >= 0 && num <= 2) {
                snprintf(messageTx, MAX_LEN, "mode %d\n", 
                        num);
                changeBeatMode(num);
            } else {
                snprintf(messageTx, MAX_LEN, "Invalid beat mode\n");
            }
        } else if (strncmp(messageRx, "play ", strnlen("play ", MAX_LEN)) == 0) {
            int num = messageRx[strnlen("play ", MAX_LEN)] - '0';
            // printf("%d", num);
            // int num = atoi(c);
            if (num >= 1 && num <= 3) {
                snprintf(messageTx, MAX_LEN, "Playing drum sound #%d\n", 
                        num);
                playDrumSound(num);
            } else {
                snprintf(messageTx, MAX_LEN, "Invalid drum sound #\n");
            }
        } else if (strncmp(messageRx, "uptime", strnlen("uptime", MAX_LEN)) == 0) {
            char buff[MAX_LEN];
            readFile("/proc/uptime", buff, MAX_LEN);
            snprintf(messageTx, MAX_LEN*2, "uptime %s", buff);
        } else {
            snprintf(messageTx, MAX_LEN, "Unknown command.\n");
        }

        // Transmit a reply:
        unsigned int sin_len = sizeof(sinRemote);
        sendto( socketDescriptor, 
                messageTx, strnlen(messageTx, MAX_LEN), 
                0, 
                (struct sockaddr*) &sinRemote, sin_len);
}

static void readFile(char* filePath, char* buff, unsigned int maxLength)
{
    FILE *pFile = fopen(filePath, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", filePath);
        exit(-1);
    }
    fgets(buff, maxLength, pFile);
    fclose(pFile);
}

