#ifndef MESH_H
#define MESH_H
#include <stdint.h>
#include <string.h>
#include "que_packet_handle.h"
#define user_resp_timeout 5000

#define add_node_to_goup_size_t 14  //size of buffer add node
/*
    ... Message Types Define ...
    * There are five different types of message use to communication in this network.
*/
enum types_message{
    private_message_type = 0x01,
    communi_message_type,
    acknown_message_type,
    joi_req_message_type,
    joi_res_message_type,
    broadcast_Info_type
};
int mesh_handle(usr_data_info_t *user_data);

/*Use For send data In lora 
    'Befor use this function'
    need to call '"lora_init()"';
*/

int lora_mesh_init(void);

int lorasend(uint8_t *data_in);

// use to receive data over lora 'data_out' struc
int lorarecv(uint8_t *data_in, uint32_t ms);


enum usr_req{
/*01*/  user_approve    =   0xA0,         //  user for joint response
/*02*/  user_reject,
/*03*/  user_delete_node_req,             //  Only authorize by admin
/*04*/  user_addNodeToTable_req,
/*05*/  user_addnNdeto_group_Chatreq,
/*06*/  user_check_NTable_req,
/*07*/  user_send_req, // there are private and comunity message for send request/*04*/
/*08*/  user_create_group_req,
/*09*/  user_check_group_req,
/*10*/  user_left_group_req,
/*11*/  user_response_req,
/*12*/  user_req_sysReset,
/*13*/  user_check_deviceInfo,
/*14*/  user_check_neigbor_table_req,
/*10*/  user_broadcast_req,
        user_update_dev_name_req,
        user_remove_from_group_req
};

int usr_req_handle(usr_data_info_t *user_data);
int joint_resp_handle(usr_data_info_t *user_data);


void Scan_Group_Chanel(void);
void Broadcast_All_GroupChannel(void);
void Broadcast_One_Channel(NeighborInfo_t GroupIdandName);
void Broadcasr_My_Info(void);

void ack_msg_handle(void);

#endif