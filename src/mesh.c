#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zephyr/types.h>
#include <cmsis_os2.h>
#include <string.h>
#include "mesh.h"
#include "que_packet_handle.h"
#include "neighbor_table.h"
#include "read.h"
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MESH_LORA);
#include <zephyr/logging/log.h>



uint8_t ack_stae ;
uint16_t k;
NeighborInfo_t my_device_info = {0};
NeighborTableInfo_t NTable ={0};

GroupInfo_t GroupChatData;



// stror group to delect in temporary
NeighborInfo_t deldata;
uint16_t data_id[255];
int  num;
uint8_t retry;
uint8_t recheck ;
uint16_t previous_id;
uint8_t lora_data[300];
que_packet_config resp_data;
uint8_t dsend[255];

que_packet_config ack_data ={0};
que_packet_config data_recv ={0};
que_packet_config previosu_data_recv;
uint16_t msg_id;
const struct device *lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
struct lora_modem_config config;
int ret;
int16_t rssi;
uint8_t snr;
volatile bool state;
uint8_t user_req_response_buffer[248];
uint8_t send_resp_buf[256];
que_packet_config resp_buffer = {0};


int lora_mesh_init(void)
{
    if (!device_is_ready(lora_dev)) {
        LOG_ERR("LoRa device not ready\n");
        return 0;
    }
    config.frequency = 915000000;
    config.bandwidth = BW_125_KHZ;  // Adjust bandwidth as needed
    config.datarate = SF_9;           // Adjust datarec rate as needed
    config.preamble_len = 8;
    config.coding_rate = CR_4_5;
    config.iq_inverted = false;
    config.public_network = false;
    config.tx_power = 20;              // Adjust transmit power as needed
    config.tx = true;
    ret = lora_config(lora_dev, &config);
    if (ret < 0) {
        LOG_INF("LoRa config failed: %d\n", ret);
        return -1;
    }   
    return 0;
}

int lorasend(uint8_t *data_in)
{
    ret= lora_send(lora_dev,data_in,data_in[5]+6);
    if(ret <0){
        LOG_INF("datarec send fail\n");
        return -1;
    }
    times();
    return 1;
}
// use to receive datarec over lora 'datarec_out' struc
int lorarecv(uint8_t *datarec_in,uint32_t ms)
{
    ret = lora_recv(lora_dev,datarec_in,255,K_MSEC(ms),&rssi,&snr);
    if(ret<0){
        LOG_INF("datarec Received fail...\n");
        return 0;
    }
    printf("RSSI: %d dBm -> SNR: %d dBm \n",rssi,snr);
    times1(); 
    return 0;
}

// NeighborInfo_t neighbor_list[50]; 

int mesh_handle(usr_data_info_t *user_data)
{   
    //get device information for nvs for later use.
    get_devide(&my_device_info);

    //get Neighbor table for user.
    if (NTable.num_members ==0)
    {
        GetNeighborTable(&NTable);
    }
    ret = lora_recv(lora_dev,lora_data,255,K_MSEC(3000),&rssi,&snr);  
        if (ret < 0)
        {
            LOG_INF("NO Data Received...\n");
            
            // printf("0x%04x\n",user_data->usr_req_id);
            usr_req_handle(user_data);
            // printf("AAA\n");         
        }
        else{
            // times1();
            que_recv_config(lora_data,&data_recv,&msg_id);
            // Handle duplicate message :
            printf("%s\n",data_recv.message);
            if(previous_id==msg_id){
                // printf("%d\n",data_recv.mtypes);
                LOG_ERR(" > Duplicate message\n");
            }
            else
            {
                // Handle received data message 
                switch (data_recv.mtypes)
                {
                case private_message_type :
                    printf("-------------------------------------------------------------------\n\n");
                    LOG_INF("**Received Private Message**\n");
                    // printf("RSSI: %d dBm -> SNR: %d dBm \n",rssi,snr);
                    // check if the destination address is the received device
                    if (data_recv.destin_addr == my_device_info.ID)
                    {
                        for (int i = 0; i < NTable.num_members; i++)
                        {
                            // Find if the received data from known node
                            if (data_recv.source_addr == NTable.neighbors[i].ID)
                            {
                                
                                printf("\t >>: SNR (%d dBm) RSSI (%d dBm ) \n",snr,rssi);
                                LOG_INF("Data Received : %s :\n",data_recv.message);
                                printf("Source Adress : * 0x%04x * || Sender Name : %s \n",NTable.neighbors[i].ID,NTable.neighbors[i].Name);
                                state = true;// LOG_INF("");
                            }
                        }
                        if (!state){
                            LOG_INF("Data Received from Unkwon source : * %s * \t Source Address : 0x%04x || Name : * Unknown *\n",data_recv.message,data_recv.source_addr);
                        }
                            ack_data.source_addr= my_device_info.ID;
                            ack_data.destin_addr=data_recv.source_addr;
                            ack_data.lens_msg=0;
                            ack_data.mtypes=acknown_message_type;
                            que_send_config(&ack_data,send_resp_buf);
                            int ss;
                            for (int i = 0; i < 3; i++)
                            {
                                ss = lorasend(send_resp_buf);
                                times1();
                                k_sleep(K_MSEC(1000));  
                            }
                            if (ss>0)
                            {
                                printf("Response Send %d\n",send_resp_buf[0]);
                            }
                            else{
                                printf("Fail Response!");
                            }
                    }
                    else
                    {
                        //int s = lorasend(lora_data);
                        // Case destination address is not the receiving device
                        for (int i =0; i < NTable.num_members ; i++)
                        {
                            // the the known source addres message will relay
                            if (data_recv.source_addr == NTable.neighbors[i].ID)
                            {
                                // Broadcast previous received data (Relay)
                                int s = lorasend(lora_data);
                                if (s>0)
                                {
                                    times();
                                    LOG_INF("Data was Relay successful\n");
                                }
                                else
                                {
                                    LOG_ERR("Relay Fail\n");
                                }  
                            }
                        }
                    }
                    printf("-------------------------------------------------------------------\n\n");
                    break;
                // handle community message (Group chat message)
                case communi_message_type :
                    LOG_INF("----## COMMUNITY MESSAGE ## ----\n");
                    GetGroupInfo(data_recv.destin_addr,&GroupChatData,&k);
                    if (GroupChatData.id == 0)
                    {
                        // unknown group chat
                        for (int i = 0; i < NTable.num_members; i++)
                        {
                            if (data_recv.source_addr == NTable.neighbors[i].ID)
                            {
                                int s = lorasend (lora_data);
                                if(s>0)
                                {
                                    LOG_INF("\t > Data was Relay successful\n");
                                }
                                else
                                {
                                    LOG_INF("\t > Relay Fail\n");
                                }
                            } 
                        }                        
                        break;
                    }
                    for(int i=0; i<NTable.num_members ; i++)
                    {
                        if (data_recv.source_addr == GroupChatData.neighbors[i].ID)
                        {
                        
                            LOG_INF("Data Received : << %s >> \n",data_recv.message);
                            LOG_INF("\t Source Address << 0x%04x >> \n", data_recv.source_addr);
                            LOG_INF("\t SNR : << (%ddBm) RSSI : (%dBm)>> \n", snr,rssi);
                            // forward data received to other member
                            int s = lorasend (lora_data);
                            if (s>0)
                            {
                                LOG_INF(" Data was Relay successful\n");
                            }
                            else
                            {
                                LOG_INF(" Relay Fail\n");
                            }
                        break;
                        }
                    }
                    break;

                case joi_req_message_type :
                    LOG_INF("joint request message\n");
                    // check 
                    for (int i = 0; i < NTable.num_members; i++)
                    {
                        if (data_recv.source_addr != NTable.neighbors[i].ID)
                        {
                            LOG_ERR("Request from unknown Source please register to neighber node fist!\n");
                            break;
                        }  
                    }
                    GetGroupInfo(data_recv.destin_addr,&GroupChatData,&k);
                    if (GroupChatData.id == 0)
                    {
                        printf("Unknown Group\n");
                        break;
                    }
                    else if (GroupChatData.admin.ID != my_device_info.ID )
                    {
                        printf("Not admin >... No authority for make decision\n");
                        break;
                    }
                    // Request to user
                    LOG_INF(" Requesting to user ...\n");
                    while (recheck <60)
                    {
                        k_sleep(K_MSEC(500));
                        if (user_data->usr_req_id == user_reject)
                        {
                            LOG_ERR(" > User_Reject\n");
                            break;
                        }
                        else if (user_data->usr_req_id == user_approve)
                        {
                            LOG_INF("\t > User Aprove request");
                            if(GroupChatData.num_members >=20)
                            {
                                break;
                            }
                            SerializeGroupInfo(&GroupChatData,user_req_response_buffer,sizeof(user_req_response_buffer));
                            
                            resp_buffer.destin_addr=data_recv.source_addr;
                            resp_buffer.mtypes = joi_res_message_type;
                            resp_buffer.source_addr = my_device_info.ID;
                            memcpy(resp_buffer.message,user_req_response_buffer,sizeof(user_req_response_buffer));
                            que_send_config(&resp_buffer,send_resp_buf);
                            ret = lora_send(lora_dev,send_resp_buf,sizeof(send_resp_buf));
                            if (ret>0)
                            {
                                printf("\t > : Data Response Send!");
                            }
                            else
                            {
                                LOG_ERR("\t : Response Faill");
                            }
                            
                        }
                        
                        
                        recheck++;
                    }
                    LOG_INF(" Waiting For User Response ...\n");
                    break;
                case acknown_message_type :
                    printf("\n#############Received Ack message tyes\n");
                    int s = lorasend(lora_data);
                    if (s>0)
                    {
                        printf(" Info : data relay successfully.\n");
                    }
                    else
                    {
                        printf(" Info : data relay fail.\n");
                    }
                    printf("\n --------------------------------------\n");
                    break;
                default:
                    LOG_INF("%d Unknow Types\n",data_recv.mtypes);
                    break;
                }
            }
            previous_id = 0;
        }
    return 0;
}
rmv_group_mem_info_t dat_rmv ={0};
// uint8_t lo_send[255];
user_config_t td ={0};
que_packet_config send_data;

uint8_t nei_list[255];
//use for add node
NeighborInfo_t ndata;
GroupInfo_t GroupReqData_recv = {0};
crt_group_Info_t group_chatName;

crt_group_Info_t group_create_data = {0};

    uint8_t lo_send[256];  // Buffer for sending data over LoRa
    NeighborInfo_t ndata;//, deldata;  // Neighbor information structures
    GroupInfo_t GroupReqData_recv;  // Group request data structure
add_node_to_group_info_t add_node_data = {0};

/**
 * @brief Handle user requests.
 * 
 * @param user_data Pointer to the user data structure.
 * @return int 0 on success, non-zero on error.
 */
int usr_req_handle(usr_data_info_t *user_data) {

    switch (user_data->usr_req_id) {
        case user_send_req: 
        {

            // Process user request configuration
            int lens = user_rec_config(user_data->data, &td);
            if (lens < 0) {
                fprintf(stderr, "Failed to receive user configuration\n");
                return -1;
            }

            // Setup send_data structure with received configuration

            send_data.source_addr = my_device_info.ID;
            send_data.mtypes = td.mtypes;
            send_data.lens_msg = lens;
            send_data.destin_addr = td.destin;
            strncpy(send_data.message, td.msg, sizeof(send_data.message) - 1);
            send_data.message[sizeof(send_data.message) - 1] = '\0';  // Ensure null termination

            // Serialize send_data into lo_send buffer
            que_send_config(&send_data, lo_send);

        if (td.mtypes == private_message_type || td.mtypes == communi_message_type) { 
            for (uint8_t retry = 0; retry < 3; retry++) {
                ret = lora_send(lora_dev, lo_send, send_data.lens_msg);
                if (ret < 0) {
                    LOG_ERR(" >: Data Send to destination : 0x%04x : fail .\n", send_data.destin_addr);
                } else {   
                    LOG_INF(" Data Sent to destination : 0x%04x \n", send_data.destin_addr);
                    times1();
                    ret = lora_recv(lora_dev, lora_data, 255, K_MSEC(1000), &rssi, &snr);  
                    if (ret < 0) {
                        LOG_INF("NO acknowledgment message Received...\n");
                    } else {  
                        printf("*****************************************************************************************\n");
                        printf("RSSI: %d dBm -> SNR: %d dBm \n", rssi, snr);
                        que_recv_config(lora_data, &ack_data, &msg_id);

                        if (ack_data.mtypes == acknown_message_type) {
                            times();
                            printf("*****************************************************************************************\n\n");
                            LOG_INF("Destination :< 0x%04X >: Successful Received Data\n", ack_data.source_addr);
                            printf("\n*****************************************************************************************\n");
                            break;
                        }
                        if (msg_id == previous_id) {
                            LOG_ERR("Duplicate Message\n");
                            break;
                        }
                    }
                }
            }
        } else if (td.mtypes == joi_req_message_type) {
            for (int i = 0; i < 3; i++) {
                ret = lora_send(lora_dev, lo_send, send_data.lens_msg);
                if (ret < 0) {
                    LOG_ERR("Data Joint Request Fail\n");
                } else {
                    LOG_INF(" Data Sent >> Joint Request destination : 0x%04x \n", send_data.destin_addr);
                    LOG_INF("%s\n", send_data.message);
                    ret = lora_recv(lora_dev, lora_data, 255, K_MSEC(1000), &rssi, &snr);  
                    if (ret < 0) {
                        LOG_INF("NO Response Message Received...\n");
                    } else {
                        que_recv_config(lora_data, &ack_data, &msg_id);
                        if (ack_data.mtypes != joi_res_message_type) {
                            break;
                        } else {
                            LOG_INF("-------------------\n");
                            DeserializeGroupInfo(ack_data.message, &GroupReqData_recv);
                            printf("Group data: %04x\n\n", GroupReqData_recv.admin.ID);
                        }
                    }
                    k_sleep(K_MSEC(500));
                    LOG_INF("Retrying send data ... (%d/2)\n", retry);
                    if (i > 2) {
                        LOG_ERR("DATA SEND FAIL\n");
                        break;
                    }
                }
            }
        }
        break;
        }
        // successfully tested
        case user_req_sysReset:
            Sys_Reset();
            user_data->usr_req_id = 0;
            break;
        // successfully tested
        case user_check_deviceInfo:
            GetDeviceInfo();
            printf("\nDevice information retrieved successfully.\n");
            user_data->usr_req_id = 0;
            break;
        // successfully tested
        case user_check_neigbor_table_req:
            printf("Retrieving neighbor table...\n");
            Print_AllNeighbors();
            user_data->usr_req_id = 0;
            break;
        case user_delete_node_req:
            printf("--> : Deleting node from neighbor table...\n");
            // Call delete neighbor function
            user_data->usr_req_id = 0;
            break;
        // successfully tested
        case user_create_group_req:
            printf("--> : -----------------------------------------------------------\n");
            printf("--> : Creating group...\n");
            Uint8tTO_crtgroupInfo(user_data->data, &group_create_data);
            printf("--> : Group Name: %s\n", group_create_data.groupName);
            printf("--> : Neighbor Name: %s\n", group_create_data.Neighbors.Name);
            printf("--> : Neighbor ID: %02X\n", group_create_data.Neighbors.ID);
            CreateGroupChat(group_create_data.Neighbors, group_create_data.groupName);
            printf("--> : -----------------------------------------------------------\n");
            user_data->usr_req_id = 0;
            break;
        // successfully tested
        case user_check_group_req:
            printf("Retrieving all group information...\n");
            GetAllGroupInfo();
            user_data->usr_req_id = 0;
            break;
        // successfully tested
        case user_left_group_req:
            printf("Leaving group...\n");
            usr_to_neib(user_data->data, &deldata);
            DeleteGroupChat(deldata.ID);
            user_data->usr_req_id = 0;
            break;
        case user_response_req:
            printf("Processing user response request...\n");
            user_data->usr_req_id = 0;
            break;
        //sucessfully tested
        case user_addnNdeto_group_Chatreq:
            printf("Adding neighbor to group chat...\n");
            int a = Uint8t_addnodeINF_convert(user_data->data,&add_node_data);
            if (a>0)
            {
                Add_NeighborToGroupChat(add_node_data.group_chat_id,add_node_data.adding_node);
            }
            else{
                printf("--> : request process fail.\n");
            }
            user_data->usr_req_id = 0;
            break;
        //successfully tested
        case user_addNodeToTable_req:
            printf("Adding neighbor to table...\n");
            usr_to_neib(user_data->data, &ndata);
            Add_NeighborToTable(ndata);
            printf("Neighbor added to table successfully.\n");
            user_data->usr_req_id = 0;
            break;
        case user_remove_from_group_req :
            printf(" info : --------------------------------------------------\n");
            Uint8t_to_rmv_fgc(user_data->data,&dat_rmv);
            remove_node_fr_group(dat_rmv);
            break;
        default:
            printf("Unknown user request ID: %d\n", user_data->usr_req_id);
            user_data->usr_req_id = 0;
            break;
    }

    return 0;
}
uint8_t Scan_Data[300];
void Scan_Group_Chanel(void){
    ret = lora_recv(lora_dev,Scan_Data,255,K_MSEC(5000),NULL,NULL);
    if (ret >0)
    {
    }
}
void Broadcast_One_Channel(NeighborInfo_t GroupIdandName);
void Broadcasr_My_Info(void);


