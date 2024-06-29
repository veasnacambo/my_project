#include "mesh.h"
#include "que_packet_handle.h"
#include "neighbor_table.h"
usr_data_info_t UserDataC  = {0};
add_node_to_group_info_t datay = {0};
uint8_t us[14];
NeighborInfo_t gg;
int main(){

    datay.group_chat_id = 0x00AB;
    datay.adding_node.ID= 0x00BA;
    strcpy(datay.adding_node.Name,"ABDC");
    gg.ID = 0xEA62;
    // add_nodeINF_Uint8t_convert(datay,UserDataC.data);

    // initialize nvs-storage
    nvs_memo_init();
    // initialize mesh-network.
    lora_mesh_init();
    neib_to_user(&gg,UserDataC.data);
    UserDataC.usr_req_id = user_check_deviceInfo;

    while (1)
    {
        mesh_handle(&UserDataC);  
    }   

    return 0;
}