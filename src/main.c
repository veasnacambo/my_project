#include "mesh.h"
#include "que_packet_handle.h"
#include "neighbor_table.h"
usr_data_info_t UserDataC  = {0};
int main(){

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
