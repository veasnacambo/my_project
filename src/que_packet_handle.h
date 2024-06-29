#ifndef QUE_PACKET_HANDLE_H
#define QUE_PACKET_HANDLE_H
#include<stdint.h>
#include "neighbor_table.h"
#define MAXD 0xff

typedef struct USER_DATA
{
  uint8_t usr_req_id ;
  uint8_t data[MAXD-1];
}usr_data_info_t;
typedef struct user_cofing{
  uint8_t mtypes;
  uint16_t destin; //destination address
  char msg[247];
}user_config_t;

typedef struct que_pack {
  uint8_t mtypes;       //  1 byte message types
  uint16_t source_addr; //  2 byte source address
  uint16_t destin_addr; //  2 byte destination address
  uint8_t lens_msg;     //  1 byte message lengths
  char message[MAXD]; // Assuming message is null-terminated
}que_packet_config;
void que_send_config(que_packet_config *data_in, uint8_t *data_out);
void que_recv_config( uint8_t *data_in, que_packet_config *data_out,uint16_t *msg_id);
uint16_t generate_random_uint16(void);

/*
*@param num_nei stor the number of neighbor in the community
*@param nei_list pointer to the neighbor list data array type uint8_t 
*@return 
* 1 for successful rsponse
* -1 for fail to respone
*/

// int respone_handle(uint8_t num_nei,NeighborInfo_t *nei_list);
// int neib_to_user(NeighborInfo_t *nei,uint8_t *usr);
// int usr_to_neib(uint8_t *usr,NeighborInfo_t *neib);
int usr_send_config(user_config_t *data,uint8_t *data_out);
int user_rec_config(uint8_t *usr,user_config_t *rec);
int u16to8(uint16_t *din, uint8_t *dout,int lens);


// void CreateGroupChatInfo_Uint8t(create_group_chat_info_t data,uint8_t *dataout);
// void Uint8tTo_CreateGroupChatInfo(uint8_t *datain,create_group_chat_info_t *data);
#endif /* QUE_PACKET_HANDLE_H */  