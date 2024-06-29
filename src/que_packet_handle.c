#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "que_packet_handle.h"
#include "neighbor_table.h"

#include "mesh.h"
NeighborInfo_t my_lora_devInfo ={0};

uint16_t generate_random_uint16(void) {
    // Combine two calls to rand() to get a more uniformly distributed 16-bit random number
    return (uint16_t)((rand() << 15) | (rand() & 0x7FFF));
}
/**
 * @brief Prepare the packet for sending based on the input configuration.
 * 
 * @param data_in Pointer to the input packet configuration structure.
 * @param data_out Pointer to the output byte array where the packet will be stored.
 */
void que_send_config(que_packet_config *data_in, uint8_t *data_out) {
    // Check for NULL pointers
    if (data_in == NULL || data_out == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to que_send_config.\n");
        return;
    }

    // Check message length to prevent buffer overflow
    if (data_in->lens_msg > MAXD - 8) {
        fprintf(stderr, "Error: Message length exceeds buffer size.\n");
        return;
    }

    get_devide(&my_lora_devInfo);
    printf("** 0x%04x ** %s\n", my_lora_devInfo.ID, my_lora_devInfo.Name);

    uint16_t message_id = generate_random_uint16(); // Generate a random message ID

    // Copy mtypes directly
    data_out[0] = data_in->mtypes; // Message types

    // Copy MYID (assuming MYID is defined elsewhere as a uint16_t)
    data_out[1] = (uint8_t)(my_lora_devInfo.ID >> 8);
    data_out[2] = (uint8_t)my_lora_devInfo.ID;

    // Copy destin_addr (destination address)
    data_out[3] = (uint8_t)((data_in->destin_addr & 0xff00) >> 8);
    data_out[4] = (uint8_t)(data_in->destin_addr & 0x00ff);

    // Copy lens_msg (length of the message)
    data_out[5] = data_in->lens_msg;

    // Copy message_id
    data_out[6] = (uint8_t)(message_id >> 8);
    data_out[7] = (uint8_t)(message_id & 0x00ff);

    // Copy the message string
    for (int i = 0; i < data_in->lens_msg; i++) {
        data_out[8 + i] = data_in->message[i];
    }

    // Ensure null termination in the output (optional)
    // This is only necessary if data_out needs to be a null-terminated string.
    // Adjust the index if you want to include the null terminator after the message.
    data_out[8 + data_in->lens_msg] = '\0';
}
/**
 * @brief Receive and parse the packet configuration.
 * 
 * @param data_in Pointer to the input byte array containing the received packet.
 * @param data_out Pointer to the output packet configuration structure.
 * @param msg_id Pointer to store the extracted message ID.
 */
void que_recv_config(uint8_t *data_in, que_packet_config *data_out, uint16_t *msg_id) {
    // Check for NULL pointers
    if (data_in == NULL || data_out == NULL || msg_id == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to que_recv_config.\n");
        return;
    }

    // Copy mtypes directly
    data_out->mtypes = data_in[0];

    // Copy source_addr
    data_out->source_addr = ((uint16_t)data_in[1] << 8) | (uint16_t)data_in[2];

    // Copy destin_addr
    data_out->destin_addr = ((uint16_t)data_in[3] << 8) | (uint16_t)data_in[4];

    // Copy lens_msg (length of the message)
    data_out->lens_msg = data_in[5];

    // Copy message_id
    *msg_id = ((uint16_t)data_in[6] << 8) | (uint16_t)data_in[7];

    // Ensure that the message length does not exceed the buffer size
    if (data_out->lens_msg > (MAXD - 8)) {
        fprintf(stderr, "Warning: Message length exceeds buffer size, truncating message.\n");
        data_out->lens_msg = MAXD - 8; // Adjust to prevent overflow
    }

    // Copy the message string
    for (int i = 0; i < data_out->lens_msg; i++) {
        data_out->message[i] = data_in[8 + i];
    }

    // Null-terminate the message string
    data_out->message[data_out->lens_msg] = '\0';
}


/**
 * @brief Convert NeighborInfo_t structure to a user byte array.
 * 
 * @param nei Pointer to the NeighborInfo_t structure.
 * @param usr Pointer to the output byte array where the neighbor info will be stored.
 * @return int 0 on success, -1 on error.
 */
int neib_to_user(NeighborInfo_t *nei, uint8_t *usr) {
    // Check for NULL pointers
    if (nei == NULL || usr == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to neib_to_user.\n");
        return -1; // Return an error code if input pointers are NULL
    }

    // Assign the ID to the first two bytes of usr
    usr[0] = (uint8_t)(nei->ID >> 8);
    usr[1] = (uint8_t)(nei->ID & 0xFF);

    // Copy the Name field to the usr array
    for (int i = 0; i < 10; i++) {
        usr[i + 2] = (uint8_t)nei->Name[i];
    }

    // Ensure the Name field is null-terminated if it's meant to be a string
    if (strlen(nei->Name) < 10) {
        usr[strlen(nei->Name) + 2] = '\0';
    }

    return 0;
}


/**
 * @brief Prepare the user configuration packet for sending.
 * 
 * @param data Pointer to the input user configuration structure.
 * @param data_out Pointer to the output byte array where the packet will be stored.
 * @return int 0 on success, -1 on error.
 */
int usr_send_config(user_config_t *data, uint8_t *data_out) {
    // Check for NULL pointers
    if (data == NULL || data_out == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to usr_send_config.\n");
        return -1; // Return an error code if input pointers are NULL
    }

    // Copy message type
    data_out[0] = data->mtypes; // Message types

    // Copy destination address
    data_out[1] = (uint8_t)((data->destin & 0xff00) >> 8);
    data_out[2] = (uint8_t)(data->destin & 0x00ff);

    // Calculate the message length
    uint8_t lens = (uint8_t)strlen(data->msg);
    if (lens > 247) {
        lens = 247; // Ensure the message length does not exceed 247
    }
    data_out[3] = lens;

    // Copy the message string
    for (int i = 0; i < lens; i++) {
        data_out[4 + i] = data->msg[i];
    }

    // Ensure null termination in the output (optional)
    data_out[4 + lens] = '\0';

    return 0; // Return 0 on success
}

int user_rec_config(uint8_t *usr,user_config_t *rec)
{
  rec->mtypes = usr[0];
  rec->destin = (((uint16_t)usr[1])<<8)+((uint16_t)usr[2]);
  char msgs[247];
  for (int i = 0; i <usr[3] ; i++)
    {
        msgs[i]=usr[i+4];
    }
    strncpy(rec->msg,msgs,usr[3]);
  return usr[3];
}

int u16to8(uint16_t *din, uint8_t *dout,int lens)
{
  for (int i = 0; i < lens; i++)
  {
    dout[i] = (uint8_t)((din[i] & 0xff00)>>8);
    dout[2*i+1]= (uint8_t)(din[i] & 0x00ff);
    if (lens>=127)
    {
      printf("<buffer full>\n");
      break;
    }
    
  }
  return 1;
}
void CreateGroupChatInfo_Uint8t(create_group_chat_info_t data,uint8_t *dataout){
  for (int i = 0; i < 20; i++)
  {
    dataout[i] = data.group_chat_name[i];
  }
  for (int i = 0; i < 10; i++)
  {
    dataout[i+20] = data.member.Name[i];
  }

    dataout[30]=(uint8_t)((data.member.ID & 0xff00) >> 8); 

    dataout[31]=(uint8_t)(data.member.ID & 0x00ff);
}

void Uint8tTo_CreateGroupChatInfo(uint8_t *datain,create_group_chat_info_t *data){
  for (int i = 0; i < 20; i++)
  {
    data->group_chat_name[i]= datain[i];
  }
  for (int i = 0; i < 10; i++)
  {
    data->member.Name[i] = datain[i+20];
  }
  data->member.ID = (((uint16_t)datain[30]) << 8) + (uint16_t)datain[31];
  return;
}