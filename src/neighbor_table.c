#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include <string.h>
#include <stdbool.h>
#include "neighbor_table.h"

/*
*   @note key that store neighbors data of the network
*/
#define nvs_neighbors_table_offset 5
#define max_group_chat 30
NeighborInfo_t my_info ={0};
rmv_group_mem_info_t dataIn;
uint16_t keyid;
//const NeighborInfo_t MYID = {0x00FA,"VEASNA"};// 0x08----
//#define NVS_PARTITION storage_partition2
#define NVS_PARTITION storage_partition
static struct nvs_fs fs;
int rc;
static bool nvs;

int nvs_memo_init(){
    // Configure the NVS file system (if using Zephyr)
    fs.flash_device = FIXED_PARTITION_DEVICE(NVS_PARTITION);
                    
    if (!device_is_ready(fs.flash_device)) {
        printk("Flash device %s is not ready\n", fs.flash_device->name);
        return 1;
    }
    fs.offset =FIXED_PARTITION_OFFSET(NVS_PARTITION);
    struct flash_pages_info info;
    rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (rc) {
        printk("Unable to get page info\n");
        return 1;
    }
    fs.sector_size =4096;//info.size;
    fs.sector_count = 4u;

    rc = nvs_mount(&fs);
    if (rc) {
        printk("NVS mount failed (code %d)\n", rc);
        return 1;
    }
    nvs = 1;
    return 0;
}

int save_neighbor(NeighborInfo_t *neighbor_save){
    uint32_t key=neighbor_save->ID;
    uint8_t cnt;
    cnt = nei_cnt();
    NeighborInfo_t temn[cnt];
    check_neighbor(temn);
    for (int i = 0; i < cnt; i++)
    {
        if (neighbor_save->ID==temn[i].ID)
        {
            printf("<<warining>>: Neighbor Have save befor!!\n");
            break;
        }
        else
        {
            rc = nvs_write(&fs, key, neighbor_save, sizeof(NeighborInfo_t)); // Replace 1 with a meaningful key
            if (rc == sizeof(NeighborInfo_t)) {
                printk("<<add neighbor>>: Neighbor info stored successfully: \n");
                return 1;
            }
            else if (rc==-2)
            {
                printf("<<err>>: neighbor save fail\n");
            }
            
            printf("<<warining>>: Neighbor Have save befor!!\n");
            break; 
        }
        
    }
    return 0;
}
    

int delet_neighbor(NeighborInfo_t *neighbor){
    uint32_t key =(uint32_t)neighbor->ID;
    uint32_t cnt=2;
    cnt=nei_cnt();
    NeighborInfo_t tnei[cnt];
    check_neighbor(tnei);
    volatile bool del_state;
    
    if(cnt==0){
        printf("No neighbor to delete\n");
    }
    else
    {
        for (int i = 0; i < cnt; i++)
        {
            if (tnei[i].ID==neighbor->ID)
            {
                del_state = true;
                rc = nvs_delete(&fs,key);
                if (rc < 0) 
                {
                    printk("Error erasing data with key %d: %d\n",key, rc);
                    break;
                } 
                else 
                {
                    printk("Data with key %d erased successfully\n",key); 
                }
            }          
        }
        while (!del_state)
        {
            printf("<<warning>> : No neighbor : <<Name>> : %s \t <ID> : 0x%04x in the neighbor table\n",neighbor->Name,neighbor->ID);
            break;
        }
    }
    return 0;
}

int check_neighbor(NeighborInfo_t *NEI){
    
    NeighborInfo_t read_neighbor;
    int read_len = sizeof(read_neighbor);
    int neighbor_count = 0;

    uint32_t key = 1;
    while (key<=0x0200) {
        rc = nvs_read(&fs,key, &read_neighbor,read_len);
        if (rc > 0) {
            NEI[neighbor_count] = read_neighbor;
            neighbor_count++;
            //printk("Read neighbor info: ID: 0x%04x, Name: %s with key: 0x%04x, @-%d-@\n", read_neighbor.ID, read_neighbor.Name,key ,neighbor_count);
        }
        key++;
    }

    if (rc == -ENOENT) {  // Key not found (reached limit?)
        //printk("There are ## %d ## neighbors in NVS\n", neighbor_count);
    } else {
        printf("Error checking NVS keys: %d\n", rc);
    }
    
    return neighbor_count;
}

int nei_cnt(){
    NeighborInfo_t read_neighbor;
    int read_len = sizeof(read_neighbor);
    int neighbor_count = 0;
    uint32_t key = 1;
    while (key<=0x0200) {
        rc = nvs_read(&fs,key, &read_neighbor,read_len);
        if (rc > 0) {
            neighbor_count++;
        }
        key++;
    }

    if (rc == -ENOENT) {  // Key not found (reached limit?)
        //printk("There are ## %d ## neighbors in NVS\n", neighbor_count);
    } else {
        printf("Error checking NVS keys: %d\n", rc);
    }
    
    return neighbor_count;
}


int num_mem;    // user for group chat
int gcount;
NeighborInfo_t all_mem[25];
int create_group(NeighborInfo_t *membor_data) {
    get_devide(&my_info);
    NeighborInfo_t nei_read[30];
    int rc;

    // Calculate the number of members correctly
    num_mem = sizeof(membor_data->ID) / sizeof(membor_data[0].ID);

    // Initialize the first member (admin)
    all_mem[0].ID = my_info.ID;
    strcpy(all_mem[0].Name, my_info.Name);

    // Copy member data to all_mem array
    for (int i = 0; i < num_mem; i++) {
        all_mem[i + 1].ID = membor_data[i].ID;
        strcpy(all_mem[i + 1].Name, membor_data[i].Name);
    }

    // Iterate through keys to find if an existing group already exists
    for (int key = 0x0201; key < 0x0300; key++) {
        rc = nvs_read(&fs, key, &nei_read, sizeof(NeighborInfo_t) * (num_mem + 1));
        if (rc > 0) {
            // Check if the existing group matches the new group
            if (memcmp(nei_read, all_mem, sizeof(NeighborInfo_t) * (num_mem + 1)) == 0) {
                printf("Group already exists with BASE_ID: 0x%04x\n", key);
                return 0;
            }
        } else if (rc == -2) {
            // Write new group data to NVS
            rc = nvs_write(&fs, key, all_mem, sizeof(NeighborInfo_t) * (num_mem + 1));
            if (rc > 0) {
                printf("Group Saved : BASE_ID : 0x%04x, there are : %d members\n", key, num_mem + 1);
                printf("Group admin : 0x%04x : Name : %s >>\n", all_mem[0].ID, all_mem[0].Name);
                return 0;
            }
            printf("Failed to save group\n");
            return -1;
        } else {
            printf("Error reading from NVS at key 0x%04x\n", key);
        }
    }

    printf("No available slot for a new group\n");
    return -1;
}

int check_group(uint16_t group_id,NeighborInfo_t *nei_read){
    uint32_t key =group_id;
    // nvs_write(&fs,0x0201,all_mem,sizeof(NeighborInfo_t)*(num_mem+1));
    if (key < 0x0200 || key > 0x0300)
    {
            // nvs_delete(&fs,key);
            printf("group id must in range of 0x0201 --> 0x0300\n");
            
            
    }
    else
    {
        
        rc=nvs_read(&fs,key,nei_read,sizeof(nei_read));
        
        if (rc>0)
        {
            printf("Group ID: 0x%04x\n",key);
        }
        else if (rc==-2)
        {

            //printf("There are ::< %d >:: groups\n",gcount);
            printf("***************\n");
            return 1;
        }
        else
        {
            //printf("0x%04x+++++++++++\n",key);
            printf("Data read fail!\n");
            return -1;   

        }
        
         
    }
    return 0;
}

int cnt;
int mem;
NeighborInfo_t data[50];
int group_check(){
    //
    // uint32_t key=0x0201;
    
    for(int key=0x001;key<0x0300;key++ ){
        rc = nvs_read(&fs,key,&data,sizeof(data));
        
        // rc=nvs_delete(&fs,key);
        
        if (rc>0)
        {

            mem = sizeof(data)/sizeof(data[0]);
            printf("******************************************\n");
            printf("*0x%04x :: %s-->> Admin*\n",data[0].ID,data[0].Name);
            // key++;
            cnt++;
            for (int  j = 1; j < mem; j++)
            {
                if (data[j].ID==0)
                {
                    goto mm;
                }
                else{
                    printf("*0x%04x :: %s*\n",data[j].ID,data[j].Name);
                    printf("Group :%d\n",cnt);
                    printf("******************************************\n");
                }
            }
            mm:
            printf("<< %d >> Group ID:: 0x%04x\n",cnt,key);

        }
        else if (rc==-2)
        {
            printf("ther are :%d group in the comunity\n",cnt);
            break;
        }
        else{
            printf("ERR\n");
            //break;
        }
    }
    return 0;
    
    
}


int i;
int read_group_id(uint16_t *id){
    NeighborInfo_t nei;
    
    for (int key = 0x0201; key < 0x0300; key++)
    {
        rc = nvs_read(&fs,key,&nei,sizeof(NeighborInfo_t));
        if (rc>0)
        {
            id[i] = key;
            i++;
            printf("0x%04x------\n",key);
        }
        else if(rc==-2)
        {
            printf("---there are %d groups\n",i);
            break;
        }
        else{
            printf("Read fail\n");
            break;
        }
        
    }
    return  0;
    
}
int delete_group(uint16_t gorup_id){

    if (gorup_id <0xea61 || gorup_id > 0xFFFF)
    {
        printf("group Id undefined :0x%04x\n",gorup_id);
        printf("group Id in range 0x0201 to 0x0300\n");
    }
    rc = nvs_delete(&fs,gorup_id);
    if (rc>0)
    {
        printf("Sucessful Left group chat.\n");
    }
    else if(rc == -2)
    {
        printf("Sucessful Left group chat\n");
    }
    else{
        printf("Process fail\n");
    }
    return 0;
    
}


// Function to add a node to the existing group
int add_node_to_group(uint16_t group_key, NeighborInfo_t new_member) {
    NeighborInfo_t nei_read[20];
    int rc;

    // Read existing group data
    rc = nvs_read(&fs, group_key, &nei_read, sizeof(nei_read));
    if (rc < 0) {
        printf("Failed to read group data from NVS\n");
        return -1;
    }

    // Find the number of current members
    int num_mem = rc / sizeof(NeighborInfo_t);

    // Check if there's space to add a new member
    if (num_mem >= sizeof(all_mem) / sizeof(all_mem[0])) {
        printf("Group is full, cannot add more members\n");
        return -1;
    }

    // Add the new member
    nei_read[num_mem].ID = new_member.ID;
    strcpy(nei_read[num_mem].Name, new_member.Name);

    // Write updated group data back to NVS
    rc = nvs_write(&fs, group_key, nei_read, sizeof(NeighborInfo_t) * (num_mem + 1));
    if (rc > 0) {
        printf("Node added to group with BASE_ID: 0x%04x, total members: %d\n", group_key, num_mem + 1);
        return 0;
    }

    printf("Failed to update group data\n");
    return -1;
}


void store_group_members(uint8_t *data, int length, NeighborInfo_t *members, int max_members) {


    int num_members = length / sizeof(NeighborInfo_t);
    if (num_members > max_members) {
        num_members = max_members;
    }

    for (int i = 0; i < num_members; i++) {
        //usr_to_neib(&data[i * sizeof(NeighborInfo_t)], &members[i]);
        printf("Member %d: ID: 0x%04x, Name: %s\n", i, members[i].ID, members[i].Name);
    }
}


/**
 * @brief Convert a user byte array to a NeighborInfo_t structure.
 * 
 * @param usr Pointer to the input byte array containing the user data.
 * @param neib Pointer to the output NeighborInfo_t structure.
 * @return int 0 on success, -1 on error.
 */
int usr_to_neib(uint8_t *usr, NeighborInfo_t *neib) {
    // Check for NULL pointers
    if (usr == NULL || neib == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to usr_to_neib.\n");
        return -1; // Return an error code if input pointers are NULL
    }

    // Combine the first two bytes of usr to form the ID
    neib->ID = ((uint16_t)usr[0] << 8) | (uint16_t)usr[1];

    // Copy the next 10 bytes to the Name field
    memcpy(neib->Name, usr + 2, 10);

    // Null-terminate the name in case usr doesn't provide a null-terminated string
    neib->Name[9] = '\0';

    return 0; 
}




NeighborTableInfo_t Table = {0};
NeighborInfo_t s = {0};
/**
 * @brief Add a neighbor to the table and save the table to non-volatile storage.
 * 
 * @param neighbor The neighbor information to add.
 */
void Add_NeighborToTable(NeighborInfo_t neighbor) {
    int status = 0;
    int rc;

    // Read the current state of the table from non-volatile storage.
    rc = nvs_read(&fs, 5, &Table, sizeof(NeighborTableInfo_t));
    if (rc > 0) {
        // Check if the table is full
        if (Table.num_members >= 200) {
            printf("< State > : Table Full\n");
            return;
        }

        // Check if the neighbor already exists in the table
        for (int i = 0; i < Table.num_members; i++) {
            if (neighbor.ID == Table.neighbors[i].ID) {
                status = 1;
                printf("Neighbor Already Saved in Previous Times!\n");
                break;
            }
        }

        // Add the neighbor if it does not already exist
        if (status == 0) {
            Table.neighbors[Table.num_members].ID = neighbor.ID;
            strncpy(Table.neighbors[Table.num_members].Name, neighbor.Name, sizeof(Table.neighbors[Table.num_members].Name) - 1);
            Table.neighbors[Table.num_members].Name[sizeof(Table.neighbors[Table.num_members].Name) - 1] = '\0';  // Ensure null termination
            Table.num_members++;
            rc = nvs_write(&fs, 5, &Table, sizeof(NeighborTableInfo_t));
            if (rc > 0) {
                printf("Neighbor Successfully Added to Table\n");
            } else {
                printf("Failed to Write Neighbor to Non-Volatile Storage\n");
            }
        }
    } else {
        // Initialize the table if it does not exist
        Table.num_members = 1;
        Table.neighbors[0] = neighbor;
        rc = nvs_write(&fs, 5, &Table, sizeof(NeighborTableInfo_t));
        if (rc > 0) {
            printf("Neighbor Successfully Added to Table\n");
        } else {
            printf("Failed to Write Neighbor to Non-Volatile Storage\n");
        }
    }

    // Print the number of members and the details of the last added neighbor for debugging
    if (Table.num_members > 0) {
        printf(" < %d > : %s 0x%04x\n", Table.num_members, Table.neighbors[Table.num_members - 1].Name, Table.neighbors[Table.num_members - 1].ID);
    }
}

/**
 * @brief Remove a neighbor from the table and save the updated table to non-volatile storage.
 * 
 * @param neighborID The ID of the neighbor to remove.
 */
void Remove_NeighborFromTable(uint16_t neighborID) {
    int rc;

    // Read the current state of the table from non-volatile storage.
    rc = nvs_read(&fs, 5, &Table, sizeof(NeighborTableInfo_t));
    if (rc <= 0) {
        printf("NVS Read Failed\n");
        return;
    }

    int index = -1;

    // Find the neighbor in the table
    for (int i = 0; i < Table.num_members; i++) {
        if (Table.neighbors[i].ID == neighborID) {
            index = i;
            break;
        }
    }

    // If neighbor not found, print message and return
    if (index == -1) {
        printf("Neighbor Not Found in Table\n");
        return;
    }

    // Remove the neighbor by shifting subsequent entries
    for (int i = index; i < Table.num_members - 1; i++) {
        Table.neighbors[i] = Table.neighbors[i + 1];
    }

    // Decrease the number of members
    Table.num_members--;

    // Write the updated table back to non-volatile storage
    rc = nvs_write(&fs, 5, &Table, sizeof(NeighborTableInfo_t));
    if (rc > 0) {
        printf("Neighbor Successfully Removed from Table\n");
    } else {
        printf("Failed to Write Updated Table to Non-Volatile Storage\n");
    }
}


/**
 * @brief Print all neighbors in the table.
 */
void Print_AllNeighbors() {
    int rc;

    // Read the current state of the table from non-volatile storage.
    rc = nvs_read(&fs, 0x5, &Table, sizeof(NeighborTableInfo_t));
    if (rc <= 0) {
        printf(" \n\n# No Available Neighbor in the Table\n");
        return;
    }

    // Check if the table is empty
    if (Table.num_members == 0) {
        printf("No neighbors in the table.\n");
        return;
    }

    // Print the total number of neighbors
    printf("Total neighbors: %d\n", Table.num_members);

    // Print details of each neighbor
    for (int i = 0; i < Table.num_members; i++) {
        printf("Neighbor < %d >: ID : 0x%04x, Name : %s\n", i + 1, Table.neighbors[i].ID, Table.neighbors[i].Name);
    }
}



GroupInfo_t GroupTable[MAX_GROUPS] = {0};
int groupkey;
GroupInfo_t GkeyCheck = {0};
GroupInfo_t Group ={0};
#define key 20
int count;
#define BaseGroupID 0xEA61

/**
 * @brief Create a group chat with the given neighbor and group name.
 * 
 * @param neighbor The neighbor information to add to the group.
 * @param groupName The name of the group.
 */
void CreateGroupChat(NeighborInfo_t neighbor, char *groupName) {
    // Check if the neighbor ID is valid
    if (neighbor.ID == 0) {
        printf("\n Group Create Fail: > Please Check Neighbor ID\n");
        return;
    }
    // Check if the neighbor name is defined
    if (strlen(neighbor.Name) == 0) {
        printf("\n Group Create Fail: > Please Define Neighbor's Name\n");
        return;
    }
    GetNeighborTable(&Table);
    if (Table.num_members == 0)
    {
        printf("You don't have neighbors node in the table!\n");
        return;
    }
    
    for (int i = 0; i < Table.num_members; i++)
    {
        printf("ne : %s\n",Table.neighbors[i].Name);
        if (Table.neighbors->ID!= neighbor.ID)
        {
            printf("The member node is not the neighbor! Please add to neighbors table first.\n");
            return;
        }  
    }
    
    // Get device info
    get_devide(&my_info);
    int found = 0;
    int count = 1;
    Group.id = BaseGroupID;

    // Search for an available slot in non-volatile storage
    for (int j = key; j < key + 20; j++) {
        int rc = nvs_read(&fs, j, &GkeyCheck, sizeof(GroupInfo_t));
        if (rc < 0) {
            // No existing group found, create a new group
            Group.id = Group.id + count;
            strncpy(Group.name, groupName, sizeof(Group.name) - 1);
            Group.name[sizeof(Group.name) - 1] = '\0';  // Ensure null termination
            Group.admin.ID = my_info.ID;
            strncpy(Group.admin.Name, my_info.Name, sizeof(Group.admin.Name) - 1);
            Group.admin.Name[sizeof(Group.admin.Name) - 1] = '\0';  // Ensure null termination
            Group.neighbors[0].ID = neighbor.ID;
            strncpy(Group.neighbors[0].Name, neighbor.Name, sizeof(Group.neighbors[0].Name) - 1);
            Group.neighbors[0].Name[sizeof(Group.neighbors[0].Name) - 1] = '\0';  // Ensure null termination
            Group.num_members = 1;
            rc = nvs_write(&fs, j, &Group, sizeof(GroupInfo_t));
            if (rc > 0) {
                printf("Group Successfully Created\n");
                count++;
                found = 1;
                break;
            } else {
                printf("Failed to Write Group to Non-Volatile Storage\n");
                return;
            }
        } else {
            Group.id = GkeyCheck.id;
        }
    }

    // If no available slots were found, print an error message
    if (!found) {
        printf("Failed to Create Group: No available slots\n");
    }
}


/**
 * @brief Get the neighbor table from non-volatile storage and print its contents.
 * 
 * @param NeiTable Pointer to the NeighborTableInfo_t structure to store the neighbor table.
 */
void GetNeighborTable(NeighborTableInfo_t *NeiTable) {
    // Check for NULL pointer
    if (NeiTable == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to GetNeighborTable.\n");
        return;
    }

    // Read the current state of the table from non-volatile storage.
    int rc = nvs_read(&fs, 0x5, NeiTable, sizeof(NeighborTableInfo_t));
    if (rc <= 0) {
        printf("\n\n# No Available Neighbor in the Table\n");
        NeiTable->num_members = 0;  // Set the number of members to 0 if read fails
        return;
    }

    // Check if the table is empty
    if (NeiTable->num_members == 0) {
        printf("No neighbors in the table.\n");
        return;
    }

    // Print the total number of neighbors
    printf("Total neighbors: %d\n", NeiTable->num_members);

    // Print details of each neighbor
    for (int i = 0; i < NeiTable->num_members; i++) {
        printf("Neighbor < %d >: ID : 0x%04x, Name : %s\n", i + 1, NeiTable->neighbors[i].ID, NeiTable->neighbors[i].Name);
    }
}


int u;
/**
 * @brief Get the group information from non-volatile storage.
 * 
 * @param group_id The ID of the group to retrieve.
 * @param GroupData Pointer to the GroupInfo_t structure to store the group data.
 */
void GetGroupInfo(uint16_t group_id, GroupInfo_t *GroupData,uint16_t *keyID) {
    // Check for NULL pointer
    if (GroupData == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to GetGroupInfo.\n");
        return;
    }

    u = 0;  // Reset the indicator for finding the group

    // Search for the group in the non-volatile storage
    for (int i = key; i < key + 20; i++) {
        int rc = nvs_read(&fs, i, GroupData, sizeof(GroupInfo_t));
        if (rc > 0 && GroupData->id == group_id) {
            *keyID = i;
            u = 1;
            break;
        }
    }

    // If the group was not found, set the group ID to 0 and print a message
    if (u == 0) {
        GroupData->id = 0;
        printf("\t > Group Not Found!\n");
    }
}

GroupInfo_t GK ={0};
/**
 * @brief Get and print all group information from non-volatile storage.
 */
void GetAllGroupInfo(void) {
    u = 0;  // Reset the indicator for finding groups

    // Iterate through possible keys in non-volatile storage to find groups
    for (int i = key; i < key + 20; i++) {
        int rc = nvs_read(&fs, i, &GK, sizeof(GroupInfo_t));
        if (rc > 0) {
            u = 1;
            printf("\n\n");
            printf("Group Admin : 0x(%04X)\t %s \n", GK.admin.ID, GK.admin.Name);
            printf("Group has < %d > members. and ID : :: 0x(%04X) :: Name :: %s ::\n", GK.num_members, GK.id, GK.name);
            for (int j = 0; j < GK.num_members; j++) {
                printf("(%d) : 0x(%04X) : %s \n", j + 1, GK.neighbors[j].ID, GK.neighbors[j].Name);
            }
        }
    }

    // Check if no groups were found
    if (u == 0) {
        printf("No available group chat\n");
    }
}




GroupInfo_t GroupTable[MAX_GROUPS];  // Assume GroupTable is declared globally or passed as a parameter

/**
 * @brief Add a neighbor to a group chat.
 * 
 * @param group_id The ID of the group to add the neighbor to.
 * @param neighbor The neighbor information to add to the group.
 */
void Add_NeighborToGroupChat(uint16_t group_id, NeighborInfo_t neighbor)
{
    // Get the group information
    GetGroupInfo(group_id, GroupTable,&keyid);
    // Find the group with the specified ID
    for (int g = 0; g < MAX_GROUPS; g++) {
        if (GroupTable[g].id == group_id) {
            // Check if the group is full
            if (GroupTable[g].num_members >= MAX_GROUP_MEMBERS) {
                printf("-->: < State > : Group Full\n");
                return;
            }
            // Check if the neighbor is already in the group
            for (int i = 0; i < GroupTable[g].num_members; i++) {
                if (neighbor.ID == GroupTable[g].neighbors[i].ID) {
                    printf("-->: Neighbor Already in Group\n");
                    return;
                }
            }
            // Add the neighbor to the group
            GroupTable[g].neighbors[GroupTable[g].num_members] = neighbor;
            GroupTable[g].num_members++;
            printf("-->: Neighbor Successfully Added to Group\n");

            // Save the updated group information back to non-volatile storage
            int rc = nvs_write(&fs, keyid, GroupTable, sizeof(GroupInfo_t));
            if (rc <= 0) {
                printf("Failed to Write Updated Group to Non-Volatile Storage\n");
            }
            return;
        }
    }

    // If the group was not found, print an error message
    printf("Group Not Found\n");
}

#define MAX_GROUPS 10

GroupInfo_t GroupTable[MAX_GROUPS];  // Assume GroupTable is declared globally or passed as a parameter

/**
 * @brief Remove a neighbor from a group chat.
 * 
 * @param group_id The ID of the group to remove the neighbor from.
 * @param neighborID The ID of the neighbor to remove from the group.
 */
void Remove_NeighborFromGroupChat(uint16_t group_id, uint16_t neighborID){
    // Find the group with the specified ID
    for (int g = 0; g < MAX_GROUPS; g++) {
        if (GroupTable[g].id == group_id) {
            int index = -1;

            // Find the neighbor in the group
            for (int i = 0; i < GroupTable[g].num_members; i++) {
                if (GroupTable[g].neighbors[i].ID == neighborID) {
                    index = i;
                    break;
                }
            }

            // If the neighbor was found, remove them from the group
            if (index != -1) {
                for (int i = index; i < GroupTable[g].num_members - 1; i++) {
                    GroupTable[g].neighbors[i] = GroupTable[g].neighbors[i + 1];
                }
                GroupTable[g].num_members--;
                printf("Neighbor Successfully Removed from Group\n");

                // Save the updated group information back to non-volatile storage
                int rc = nvs_write(&fs, g, &GroupTable[g], sizeof(GroupInfo_t));
                if (rc <= 0) {
                    printf("Failed to Write Updated Group to Non-Volatile Storage\n");
                }
            } else {
                printf("Neighbor Not Found in Group\n");
            }
            return;
        }
    }

    // If the group was not found, print an error message
    printf("Group Not Found\n");
}



/**
 * @brief Reset the system by deleting all entries in non-volatile storage.
 */
void Sys_Reset(void) {
    int rc = 0;
    int fail_count = 0;

    // Iterate through all possible keys and delete them from non-volatile storage
    for (size_t i = 0; i < 0x0A00; i++) {
        rc = nvs_delete(&fs, i);
        if (rc != 0) {
            fail_count++;
        }
    }
    // Check if any deletion operations failed
    if (fail_count == 0) {
        printf("System Successfully Reset!\n");
    } else {
        printf("System Reset Failed for %d entries\n", fail_count);
    }
}

DeviceInfo_t DeviceINF = {0};

/**
 * @brief Get and print device information from non-volatile storage.
 */
void GetDeviceInfo(void) {
    int rc = nvs_read(&fs, device_Info_address, &DeviceINF, sizeof(DeviceInfo_t));
    if (rc > 0) {
        printf("\n\t--------------------------------------------------\n");
        printf("\n\t Device Information : Lora Device version 1.0 \n");
        printf("\t Device Name : \t %s \n", DeviceINF.Device.Name);
        printf("\t Device Address : 0x%04X \n", DeviceINF.Device.ID);
        printf("\t Created : %s :\n", DeviceINF.Build_Date);
        printf("\t Developer Information :\n");
        printf("\t %s\n", DeviceINF.Developer_Info);
        printf("\n\t--------------------------------------------------\n");
    } else {
        printf(" < INF :> No Info to Show!\n");
    }
}



/**
 * @brief Set the device information in non-volatile storage.
 * 
 * @param Device The device information to set.
 */
void SetDeviceInfo(DeviceInfo_t Device) {
    int rc = nvs_write(&fs, device_Info_address, &Device, sizeof(DeviceInfo_t));
    if (rc > 0) {
        printf("Device Info Successfully Set.\n");
    } else {
        printf("\tFailed to Set Device Info\n");
    }
}

/**
 * @brief Get device information and store it in the provided NeighborInfo_t structure.
 * 
 * @param device Pointer to the NeighborInfo_t structure to store the device information.
 */
void get_devide(NeighborInfo_t *device) {
    // Check for NULL pointer
    if (device == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to get_devide.\n");
        return;
    }

    // Read device information from non-volatile storage
    int rc = nvs_read(&fs, device_Info_address, &DeviceINF, sizeof(DeviceInfo_t));
    if (rc > 0) {
        // Copy device information to the provided structure
        device->ID = DeviceINF.Device.ID;
        strncpy(device->Name, DeviceINF.Device.Name, sizeof(device->Name) - 1);
        device->Name[sizeof(device->Name) - 1] = '\0';  // Ensure null termination
    } else {
        fprintf(stderr, "Error: Failed to read device information from non-volatile storage.\n");
    }
}

/**
 * @brief Serialize the GroupInfo_t structure into a buffer.
 * 
 * @param group Pointer to the GroupInfo_t structure to serialize.
 * @param buffer Pointer to the buffer to store the serialized data.
 * @param buffer_size Size of the buffer.
 */
void SerializeGroupInfo(const GroupInfo_t *group, uint8_t *buffer, size_t buffer_size) {
    // Ensure the buffer and group pointers are not NULL
    if (group == NULL || buffer == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to SerializeGroupInfo.\n");
        return;
    }

    // Calculate the required buffer size for serialization
    size_t required_size = sizeof(group->id) + sizeof(group->name) + sizeof(group->num_members) +
                           sizeof(group->admin) + sizeof(group->neighbors);

    // Ensure the buffer is large enough
    if (buffer_size < required_size) {
        fprintf(stderr, "Buffer too small to serialize GroupInfo_t\n");
        return;
    }

    size_t offset = 0;

    // Copy group ID
    memcpy(buffer + offset, &group->id, sizeof(group->id));
    offset += sizeof(group->id);

    // Copy group name
    memcpy(buffer + offset, group->name, sizeof(group->name));
    offset += sizeof(group->name);

    // Copy number of members
    memcpy(buffer + offset, &group->num_members, sizeof(group->num_members));
    offset += sizeof(group->num_members);

    // Copy admin information
    memcpy(buffer + offset, &group->admin, sizeof(group->admin));
    offset += sizeof(group->admin);

    // Copy neighbors information
    memcpy(buffer + offset, group->neighbors, sizeof(group->neighbors));
    offset += sizeof(group->neighbors);
}


void DeserializeGroupInfo(const uint8_t *buffer, GroupInfo_t *group) {
    size_t offset = 0;

    // Copy group ID
    memcpy(&group->id, buffer + offset, sizeof(group->id));
    offset += sizeof(group->id);

    // Copy group name
    memcpy(group->name, buffer + offset, sizeof(group->name));
    offset += sizeof(group->name);

    // Copy number of members
    memcpy(&group->num_members, buffer + offset, sizeof(group->num_members));
    offset += sizeof(group->num_members);

    // Copy admin information
    memcpy(&group->admin, buffer + offset, sizeof(group->admin));
    offset += sizeof(group->admin);

    // Copy neighbors information
    memcpy(group->neighbors, buffer + offset, sizeof(group->neighbors));
    offset += sizeof(group->neighbors);
}




void crt_groupInfoTOtoUint8t(crt_group_Info_t group_Info,uint8_t *data){
    for (int i = 0; i < 20; i++)
    {
        data[i] = group_Info.groupName[i];
    }
    data[20] = (uint8_t)(group_Info.Neighbors.ID>>8 & 0x00ff) ;
    data[21] = (uint8_t)(group_Info.Neighbors.ID & 0x00ff);

    for (int i = 0; i < 10; i++)
    {
        data[i+22] = group_Info.Neighbors.Name[i];
    }
    return;
}

void Uint8tTO_crtgroupInfo(uint8_t *data,crt_group_Info_t *group_info)
{
    for (int i = 0; i < 20; i++)
    {
        group_info->groupName[i]=data[i];
    }

    group_info->Neighbors.ID = (((uint16_t)data[20])<<8 & 0xff00) | (((uint16_t)data[21]) & 0x00ff) ;
    for (int i = 0; i < 10; i++)
    {
        group_info->Neighbors.Name[i]=data[i+22];
    }
    
}
/**
 * @brief Delete a group chat by its ID.
 * 
 * @param group_id The ID of the group to delete.
 */
void DeleteGroupChat(uint16_t group_id) {
    int found = 0;
    // Search for the group in non-volatile storage
    for (int j = key; j < key + 30; j++) {
        GroupInfo_t Group;
        int rc = nvs_read(&fs, j, &Group, sizeof(GroupInfo_t));
        if (rc > 0 && Group.id == group_id) {
            // Group found, delete it
            rc = nvs_delete(&fs, j);
            if (rc == 0) {
                printf("Group with ID 0x%04X successfully deleted\n", group_id);
                found = 1;
                break;
            } else {
                printf("Failed to delete group with ID 0x%04X from non-volatile storage\n", group_id);
                return;
            }
        }
    }

    // If no group was found, print an error message
    if (!found) {
        printf("Group with ID 0x%04X not found\n", group_id);
    }
}




int add_nodeINF_Uint8t_convert(add_node_to_group_info_t data,uint8_t *data_out)
{
    data_out[0] = (uint8_t)((data.group_chat_id & 0xff00)>>8);
    data_out[1] = (uint8_t)(data.group_chat_id & 0x00ff);
    data_out[2] = (uint8_t)((data.adding_node.ID & 0xff00)>>8);
    data_out[3] = (uint8_t)(data.adding_node.ID & 0x00ff);
    for (int i = 0; i < 10; i++)
    {
        data_out[i+4] = data.adding_node.Name[i];
    }
    if (sizeof(data_out)==0)
    {
        printf("--> : Data convert Fail\n");
        return -1;
    }
    else 
    {
        return 1;
    }
}

int Uint8t_addnodeINF_convert(uint8_t *data_in,add_node_to_group_info_t *data_out)
{
    if (data_in == NULL)
    {
        fprintf(stderr, "Error: NULL pointer passed to GetNeighborTable.\n");
        printf(" -- > :please check your input data convertion.\n");
        return -1;
    }
    
    data_out->group_chat_id = ((((uint16_t)data_in[0])<<8) & 0xff00) | (((uint16_t)data_in[1]) & 0x00ff);
    data_out->adding_node.ID= ((((uint16_t)data_in[2])<<8) & 0xff00) | (((uint16_t)data_in[3]) & 0x00ff);
    for (int i = 0; i < 10; i++)
    {
        data_out->adding_node.Name[i] = data_in[i+4];
    }
    if (data_out == NULL)
    {
        fprintf(stderr, "Error: NULL pointer passed to GetNeighborTable.\n");
        printf(" --> : data convert fail \n");
        return -1;
    }
    else{
        return 1;
    }
    
}


int remove_node_fr_group(rmv_group_mem_info_t data_rm)
{
    // Initialize keyid
    // int keyid;

    // Retrieve group information and store it in GroupTable
    GetGroupInfo(data_rm.group_id, GroupTable, &keyid);
    
    // Iterate through the group members to find the node to remove
    for (int i = 0; i < GroupTable->num_members; i++)
    {
        // Check if the current neighbor's ID matches the ID of the node to be removed
        if (GroupTable->neighbors[i].ID == data_rm.remove_node.ID)
        {
            // Remove the neighbor from the group chat
            Remove_NeighborFromGroupChat(data_rm.group_id, data_rm.remove_node.ID);
            // Return 1 indicating success
            return 1;
        }
    }
    
    // Return -1 if the node was not found in the group
    return -1;
}



int rmv_fgcTO_Uint8t(rmv_group_mem_info_t dataIn, uint8_t *dataOut)
{
    if (dataOut == NULL)
    {
        printf("inf : config fail.\n");
        return -1;
    }

    dataOut[0] = (uint8_t)((dataIn.group_id >> 8) & 0x00ff);
    dataOut[1] = (uint8_t)(dataIn.group_id & 0x00ff);
    dataOut[2] = (uint8_t)((dataIn.remove_node.ID >> 8) & 0x00ff);
    dataOut[3] = (uint8_t)(dataIn.remove_node.ID & 0x00ff);

    for (int i = 0; i < 10; i++)
    {
        dataOut[i + 4] = dataIn.remove_node.Name[i];
    }

    return 1;
}

int Uint8t_to_rmv_fgc(uint8_t *dataIn, rmv_group_mem_info_t *dataOut)
{
    if (dataIn == NULL || dataOut == NULL)
    {
        printf("inf : config fail.\n");
        return -1;
    }

    dataOut->group_id = (uint16_t)(dataIn[0] << 8) | dataIn[1];
    dataOut->remove_node.ID = (uint16_t)(dataIn[2] << 8) | dataIn[3];

    for (int i = 0; i < 10; i++)
    {
        dataOut->remove_node.Name[i] = dataIn[i + 4];
    }

    return 1;
}

int Crt_non_AdminGroup(GroupInfo_t group_data)
{
    // Check if the group ID is valid
    if (group_data.id == 0)
    {
        printf("\n Group Create Fail: > Please Check Group ID\n");
        return -1;
    }
    // Check if the group name is defined
    if (strlen(group_data.name) == 0)
    {
        printf("\n Group Create Fail: > Please Define Group Name\n");
        return -1;
    }
    // Check if the group has at least one neighbor
    if (group_data.num_members == 0)
    {
        printf("\n Group Create Fail: > Please Add at least one Neighbor\n");
        return -1;
    }

    GetNeighborTable(&Table);
    if (Table.num_members == 0)
    {
        printf("You don't have neighbors node in the table!\n");
        return -1;
    }

    for (int i = 0; i < group_data.num_members; i++)
    {
        int neighbor_found = 0;
        for (int j = 0; j < Table.num_members; j++)
        {
            if (Table.neighbors[j].ID == group_data.neighbors[i].ID)
            {
                neighbor_found = 1;
                break;
            }
        }

        if (!neighbor_found)
        {
            printf("The member node %s is not the neighbor! Please add to neighbors table first.\n", group_data.neighbors[i].Name);
            return -1;
        }
    }

    // Get device info
    get_devide(&my_info);
    int found = 0;
    int count = 1;
    Group.id = BaseGroupID;

    // Search for an available slot in non-volatile storage
    for (int j = key; j < key + 20; j++)
    {
        int rc = nvs_read(&fs, j, &GkeyCheck, sizeof(GroupInfo_t));
        if (rc < 0)
        {
            // No existing group found, create a new group
            Group.id = Group.id + count;
            strncpy(Group.name, group_data.name, sizeof(Group.name) - 1);
            Group.name[sizeof(Group.name) - 1] = '\0';  // Ensure null termination
            Group.admin.ID = group_data.admin.ID;
            strncpy(Group.admin.Name, group_data.admin.Name, sizeof(Group.admin.Name) - 1);
            Group.admin.Name[sizeof(Group.admin.Name) - 1] = '\0';  // Ensure null termination
            Group.num_members = group_data.num_members;
            for (int k = 0; k < group_data.num_members; k++)
            {
                Group.neighbors[k].ID = group_data.neighbors[k].ID;
                strncpy(Group.neighbors[k].Name, group_data.neighbors[k].Name, sizeof(Group.neighbors[k].Name) - 1);
                Group.neighbors[k].Name[sizeof(Group.neighbors[k].Name) - 1] = '\0';  // Ensure null termination
            }
            rc = nvs_write(&fs, j, &Group, sizeof(GroupInfo_t));
            if (rc > 0)
            {
                printf("Group Successfully Created\n");
                count++;
                found = 1;
                break;
            }
            else
            {
                printf("Failed to Write Group to Non-Volatile Storage\n");
                return -1;
            }
        }
        else
        {
            Group.id = GkeyCheck.id;
        }
    }

    // If no available slots were found, print an error message
    if (!found)
    {
        printf("Failed to Create Group: No available slots\n");
    }

    return 1;
}
