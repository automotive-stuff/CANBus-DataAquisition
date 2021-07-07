#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "SocketCAN.h"

struct SocketCAN_T{
    SocketCAN_Config_T interface_config;
    int fd;
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct can_frame frame;

    /* Object Id */
    size_t id;
};

static int num_sockets_created = 0;
static struct SocketCAN_T objectPool[MAX_SOCKETCAN_T] = {0};

static void SocketCAN_SetInterfaceDown(SocketCAN_InterfaceNumber_T iface_num){

    char iface_num_str[16] = {0};
	
    snprintf(iface_num_str, sizeof(iface_num_str) - 1, "can%d", iface_num);

    pid_t pid = 0;
    pid = fork();
    
    if(pid < 0){
        perror("Fork failed");
        exit(1);
    }
    else if(pid == 0){
        // Child now exec's
        if(-1 == execlp("ifconfig",
                    "ifconfig", iface_num_str, "down", (char*)NULL)){

                perror("execlp failed");
                exit(EXIT_FAILURE);
        }
    }
    wait(NULL);
}

static void SocketCAN_SetInterfaceUp(SocketCAN_InterfaceNumber_T iface_num){

    char iface_num_str[16] = {0};
	
    snprintf(iface_num_str, sizeof(iface_num_str) - 1, "can%d", iface_num);

    pid_t pid = 0;
    pid = fork();
    
    if(pid < 0){
        perror("Fork failed");
        exit(1);
    }
    else if(pid == 0){
        // Child now exec's
        if(-1 == execlp("ifconfig",
                    "ifconfig", iface_num_str, "up", (char*)NULL)){

                perror("execlp failed");

                exit(EXIT_FAILURE);
        }
    }
    wait(NULL);
}

static unsigned char SocketCAN_FindCANInterface(SocketCAN_InterfaceNumber_T can_interface_num){

	char buf[64] = {0};

	snprintf(buf, sizeof(buf) - 1, "/sys/class/net/can%d", can_interface_num);

	return ((access(buf, F_OK) == 0));
}

/**
 * @brief Create a CAN socket with the passed SocketCAN_Config_T configuration.
 * 
 * @return On success, the file descriptor for the newly created socket is returned. On error, -1 is returned.
 */ 
static int SocketCAN_CreateSocket(void){
    /* Create socket */
    int s;
    if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
        perror("Socket");
        return -1;
    }

    return s;
}

/**
 * @brief Retrieve the interface index from the interface name i.e. can0, can1, vcan etc.
 * 
 * @param fd The File descriptor of the CAN socket.
 * @param can_iface 
 * @return On succsess, the value of the ioctl request is returned. On error, -1 is returned.
 * 
 */ 
static int SocketCAN_RetrieveIfaceIndex(SocketCAN_T_ptr const can_iface){


    /* Specify can device */
    switch(can_iface->interface_config.iface_number){

        case IFACE_CAN0:
            strcpy(can_iface->ifr.ifr_name, "can0");
            break;
        case IFACE_CAN1:
            strcpy(can_iface->ifr.ifr_name, "can1");
            break;
        default:
            strcpy(can_iface->ifr.ifr_name, "vcan0");
            break;
    }

    int ret = ioctl(can_iface->fd, SIOCGIFINDEX, &can_iface->ifr);

    if (ret < 0) {
        perror("ioctl interface index failed!");
        return -1;
    }

    return ret;
}


/**
 * @brief Bind a CAN socket to the selected CAN interface.
 * 
 * @param fd The File descriptor of the CAN socket.

 * @return On success, zero is returned. On error, -1 is returned, and errno is set.
 * 
 */ 
static int SocketCAN_BindSocket(SocketCAN_T_ptr const can_iface){

    assert(can_iface);

    /* Bind the socket to can device */
    can_iface->addr.can_family = PF_CAN;
    can_iface->addr.can_ifindex = can_iface->ifr.ifr_ifindex;

    int ret = bind(can_iface->fd, (struct sockaddr *)&can_iface->addr, sizeof(can_iface->addr));

    if (ret < 0) {
        perror("bind failed");
        return -1;
    }

    return ret;
}

/**
 * @brief Creates a useable CAN socket with the given configuration. A
 * default configuration of 500KB/s on the can0 interface is used if parameter config is
 * NULL.
 * 
 * @param config The configuration structure for the CAN socket.

 * @return On success, a pointer to a SocketCAN_T structure. On error, NULL is returned.

 */ 
SocketCAN_T_ptr SocketCAN_Create(const SocketCAN_Config_T* const config){

    SocketCAN_T_ptr can_iface = NULL;

    if(num_sockets_created > MAX_SOCKETCAN_T){
        return NULL;
    }

    for(int i = 0; i < MAX_SOCKETCAN_T; i++){

        if(*(&objectPool[i].id) == 0){
            can_iface = &objectPool[i];
        }
    }
    
    if(!can_iface){
        return NULL;
    }

    if(!config){

        /* Create With Default Settings */
        can_iface->interface_config.bitrate = BITRATE_500000KBS;
        can_iface->interface_config.iface_number = IFACE_CAN0;

    }
    else{

        /* Create with Config Settings */
        can_iface->interface_config.bitrate = config->bitrate;
        can_iface->interface_config.iface_number = config->iface_number;

    }

    /* Create Socket */
    if((can_iface->fd = SocketCAN_CreateSocket()) < 0){

        fprintf(stderr, "Unable to create CAN socket\r\n");
        exit(EXIT_FAILURE);
    }

    /* Retrieve and index */
    if((SocketCAN_RetrieveIfaceIndex(can_iface)) < 0){
        fprintf(stderr, "Unable to retrieve valid interface index for CAN socket\r\n");
        exit(EXIT_FAILURE);
    }

    /* Bind Socket */
    if((SocketCAN_BindSocket(can_iface)) < 0){
 
        fprintf(stderr, "Unable to bind CAN socket\r\n");
        exit(EXIT_FAILURE);
    }

    num_sockets_created++;
    
    return can_iface;
    
}

int SocketCAN_SetBitrate(SocketCAN_T_ptr const can_iface){
    
    assert(can_iface);

    SocketCAN_InterfaceNumber_T iface_num = can_iface->interface_config.iface_number;
    SocketCAN_Bitrate_T bitrate = can_iface->interface_config.bitrate;

    // CAN device must be closed before setting the bitrate
    SocketCAN_SetInterfaceDown(iface_num);

    char iface_num_str[64] = {0};
    char bitrate_str[64] = {0};
    
	
    snprintf(iface_num_str, sizeof(iface_num_str) - 1, "can%d", iface_num);
    snprintf(bitrate_str, sizeof(bitrate_str) - 1, "%d", bitrate);


    pid_t pid = 0;
    pid = fork();

    if(pid < 0){
        perror("Fork failed");
        exit(1);
    }
    else if(pid == 0){
        // Child now exec's

        if(-1 == execlp("ip",
                    "ip", "link", "set", iface_num_str, "type", "can",
                        "bitrate", bitrate_str, (char*)NULL)){

                perror("execlp failed");
                exit(EXIT_FAILURE);
        }
    }
   
    wait(0);

    // Bring CAN device back up
    SocketCAN_SetInterfaceUp(iface_num);

    return 0;
}

int SocketCAN_GetBitrate(SocketCAN_T_ptr const can_iface){

    assert(can_iface);

    int ret = -1;
    char iface_num_str[16] = {0};
    
    char buffer[4096];
    int stdout_save;
	
    snprintf(iface_num_str, sizeof(iface_num_str) - 1, "can%d", can_iface->interface_config.iface_number);
    pid_t pid = 0;
    
    int fd[2];
    if(-1 == pipe(fd)){
        perror("Pipe Failed");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) == 0){ // first fork
        // Child now exec's
    
        close(fd[0]);

        dup2(fd[1], 1);
        dup2(fd[1], 2);
        close(fd[1]);

        if(-1 == execlp("ip",
                    "ip", "-det", "link", "show", iface_num_str, (char*)NULL)){

                perror("execlp failed");
                exit(EXIT_FAILURE);
        }
    }
    else{
        close(fd[1]);

        while (read(fd[0], buffer, sizeof(buffer)) != 0){
            write(1, buffer, strlen(buffer));
        }
    }

    //printf("\r\nBuffer:\r\n%s\r\n\r\n", buffer);

    char* substr = strstr(buffer, "bitrate") + strlen("bitrate") + 1;
    //printf("Substr:\r\n%s\r\n\r\n", substr);
    
    char bitrate_str[32] = {0};

    for(int i = 0; i < strlen(substr) - (strlen("bitrate") + 1); ++i){
        bitrate_str[i] = substr[i];
        if(isspace(substr[i])){
            bitrate_str[i] = '\0';
            break;
        }
    }

    //printf("Bitrate:\r\n%s\r\n", bitrate_str);

    return atoi(bitrate_str);
}

/**
 * @brief Send a CAN frame/message.
 * 
 * @param can_iface The CAN interface to send the frame/message.
 * @param frame_config The populated pointer to a CAN frame configuration structure.

 * @return On success, zero is returned. On error, -1 is returned.
 * 
 */ 
int SocketCAN_SendMessage(SocketCAN_T_ptr const can_iface, const SocketCAN_FrameConfig_T* const frame_config){
    
    assert(can_iface);
    assert(frame_config);

    can_iface->frame.can_id = frame_config->can_id;
    can_iface->frame.can_dlc = frame_config->can_dlc;

    memcpy(can_iface->frame.data, frame_config->data, sizeof(frame_config->data));


    if(write(can_iface->fd, &can_iface->frame, sizeof(struct can_frame)) != sizeof(struct can_frame)){
        perror("write");
        return -1;
    }

    return 0;
}

/**
 * @brief Filter out CAN frames that are not relevant. This happens at the driver level.
 * 
 * @param can_iface The CAN interface to send the frame/message.
 * @param frame_filter The populated pointer to a CAN frame filter configuration structure.

 * @return On success, zero is returned. On error, -1 is returned.
 * 
 */
int SocketCAN_SetFrameFilter(SocketCAN_T_ptr const can_iface, SocketCAN_FrameFilter_T* frame_filter){
    int ret = 0;

    assert(can_iface);
    
    if(frame_filter){

        assert(frame_filter->length >= 0);

        struct can_filter rfilter[frame_filter->length];
    
        for(int i = 0; i < frame_filter->length; i++){

            rfilter[i].can_id = frame_filter->can_id;
            rfilter[i].can_mask = frame_filter->can_mask;
        }

        ret = setsockopt(can_iface->fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    }

    return ret;
}

/**
 * @brief Read a CAN frame/message.
 * 
 * @param can_iface The CAN interface to send the frame/message.
 * @param frame_config The populated pointer to a CAN frame configuration structure.

 * @return On success, zero is returned. On error, -1 is returned.
 * 
 */ 
int SocketCAN_ReadMessage(SocketCAN_T_ptr const can_iface){
    
    assert(can_iface);

    int nbytes;
    nbytes = read(can_iface->fd, &can_iface->frame, sizeof(struct can_frame));

    if(nbytes < 0){
        perror("Read");
        return -1;
    }

    printf("0x%03X [%d] ",can_iface->frame.can_id, can_iface->frame.can_dlc);


    for (int i = 0; i < can_iface->frame.can_dlc; i++){

        printf("%02X ", can_iface->frame.data[i]);
    }

    printf("\r\n");

    return 0;
}


/**
 * @brief Closes the open CAN interface socket.
 * 
 * @param can_iface The SocketCAN interface structure for the CAN socket.

 * @return On success, a zero is returned. On error, -1 is returned.

 */ 
int SocketCAN_Destroy(SocketCAN_T_ptr const can_iface){

    assert(can_iface);

    if(close(can_iface->fd) < 0){
        perror("close");
        return -1;
    }

    can_iface->id = 0;


    return 0;
}

