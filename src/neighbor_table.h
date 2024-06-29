#ifndef NEIGHBOR_TABLE_H
#define NEIGHBOR_TABLE_H
#include <stdint.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include "que_packet_handle.h"
#include "neighbor_table.h"
#include <string.h>
#define device_Info_address 0xAAFF
#define MAX_GROUP_MEMBERS 20
#define MAX_GROUPS 10

// Define your neighbor information structure

//uint16_t neighborm[]={};
//uint16_t group_chat[]={};

/*
* @param id typically range 0xEA61 - 0xFFFF
* @param name is name of group chat
* @param num_members if number of members in group include adminn
*/

typedef struct neighbor {
    uint16_t ID;
    char Name[10]; 
}NeighborInfo_t;
typedef struct GroupChat 
{
    uint16_t id;
    char name[20];
    uint8_t num_members;
    NeighborInfo_t admin;
    NeighborInfo_t neighbors[10];
}GroupInfo_t;


typedef struct NeighborTable
{
    uint8_t num_members;
    NeighborInfo_t neighbors[50];
}NeighborTableInfo_t;


typedef struct Device_Information
{
    NeighborInfo_t Device;
    char Build_Date[50];
    char Developer_Info[100];

}DeviceInfo_t;

typedef struct create_group_chat
{
    NeighborInfo_t member;
    char group_chat_name[20];
}create_group_chat_info_t;


typedef struct crt_group_Info
{
    NeighborInfo_t Neighbors;
    char groupName[20];
}crt_group_Info_t;

typedef struct broadcast_info
{
    NeighborInfo_t Source;
    NeighborInfo_t Group_Channel;
}broadcast_info_t;

// @brief store add node to group info.
typedef struct add_node_to_group_inf
{
  uint16_t group_chat_id;
  NeighborInfo_t adding_node;
}add_node_to_group_info_t;

typedef struct rmv_group_mem
{
    uint16_t group_id;
    NeighborInfo_t remove_node;
}rmv_group_mem_info_t;


// NeighborTableInfo_t Table = {0};


int usr_to_neib(uint8_t *usr,NeighborInfo_t *neib);
// int respone_handle(uint8_t num_nei,NeighborInfo_t *nei_list);
int neib_to_user(NeighborInfo_t *nei,uint8_t *usr);

/*
@param neighbor_save is structure that store information of neighbor need to save.
*/
int save_neighbor(NeighborInfo_t *neighbor_save);
/*
@param group_del is structure that store information of group need to save.
*/
//int create_group(group_info *group_save);
/*
@param group_del is structure that store information of group need to delect.
*/
//int delet_group(group_info *group_del);
/*
@param delet_neighbor() is structure that store information of neighbor need to delect.
@param start_key : the start index to delete, it must be bigger or equal to 1;
@param end_key : is the end index in case the user want to delete only one index end_key="NULL"
@param id : is the id of the specific neighbor that user want to delete. In this case start_key and end_key must equal "NULL"

If use key please set id to '0';
*/
int delet_neighbor(NeighborInfo_t *neighbor);
/*
@param nvs_memo_init() use to intialize nvs memory parameter
*/
int nvs_memo_init(void);
/*
@param NEI is a array pointer for stor neibor data
@return number_neighbor The number of neighbor that saved. and 
return -1; for no neighbor in the community.
*/
int check_neighbor(NeighborInfo_t *NEI);

int nei_cnt();
/*
* it's return number of group count @cite
*@param ID is a group id that want to check memeber and admin
*/
int check_group(uint16_t group_id,NeighborInfo_t *nei_read);
int create_group(NeighborInfo_t *membor_data);
int group_check();
int read_group_id(uint16_t *id);
int delete_group(uint16_t gorup_id);

int add_node_to_group(uint16_t group_key, NeighborInfo_t new_member) ;

void store_group_members(uint8_t *data, int length, NeighborInfo_t *members, int max_members);




void Add_NeighborToTable(NeighborInfo_t neighbors);

void Remove_NeighborFromTable(uint16_t neighborID) ;


void Print_AllNeighbors() ;

void Remove_NeighborFromGroupChat(uint16_t group_id, uint16_t neighborID);
void Add_NeighborToGroupChat(uint16_t group_id, NeighborInfo_t neighbor) ;
void Add_NeighborToGroupChat(uint16_t group_id, NeighborInfo_t neighbor) ;
void GetGroupInfo(uint16_t group_id,GroupInfo_t * GroupData,uint16_t *keyID);
void CreateGroupChat(NeighborInfo_t neighbor,char *groupName);

/*
* @note this function use for getting Neighbor tabel from NVS-API
* @param *NeiTable  is a pointer for store data neighbor table
*/
void GetNeighborTable(NeighborTableInfo_t *Table);

void GetAllGroupInfo(void);
/*
* @note This function use for System Reset ( all data will clear!)
*/
void Sys_Reset(void);
/*
* @note : This function GetDeviceInfo 
*/
void GetDeviceInfo(void);

void SetDeviceInfo(DeviceInfo_t Device);
/*
*@param device is a struct for store device info (ID and Name)
*/
void get_devide(NeighborInfo_t *device);

/*
*@param *group 
*@param *buffer
*@param buffer_size
*@note for
*
*/
void SerializeGroupInfo(const GroupInfo_t *group, uint8_t *buffer, size_t buffer_size);
/*
* @param *buffer 
* @param *group 
* @note : This function
*/
void DeserializeGroupInfo(const uint8_t *buffer, GroupInfo_t *group);
// void UserRespHandle(usr_data_info_t *Usr_Data);

/*
* @note This function use to converter data type structure for crt_groupInfo_t to uint8t that use for create group message
* @param group_Info 
* @param *data
* @return
*/
void crt_groupInfoTOtoUint8t(crt_group_Info_t group_Info,uint8_t *data);
void Uint8tTO_crtgroupInfo(uint8_t *data,crt_group_Info_t *group_info);

void DeleteGroupChat(uint16_t group_id);

/* @brief user for convert add_node_to_group_info_t to uint8_t array pointer
* @param data input add_node_to_group_info_t struncture 
* @param the converted array pointer uint8_t
* @return 1 on success and -1 on fail
*/
int add_nodeINF_Uint8t_convert(add_node_to_group_info_t data,uint8_t *data_out);

/*  @brief reverse data from add add_nodeINF_Uint8t_convert function
*   @param data_in
*   @param data_out
*   @return 1 on success and -1 fail
*/
int Uint8t_addnodeINF_convert(uint8_t *data_in,add_node_to_group_info_t *data_out);

int remove_node_fr_group(rmv_group_mem_info_t data_rm);

int rmv_fgcTO_Uint8t(rmv_group_mem_info_t data,uint8_t * data_out);
int Uint8t_to_rmv_fgc(uint8_t *dataIn, rmv_group_mem_info_t *dataOut);
int Crt_non_AdminGroup(GroupInfo_t group_data);
#endif //NEIGHBOR_TABLE_H
