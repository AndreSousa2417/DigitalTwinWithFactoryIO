#include "open62541.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <iostream> 
#include <unistd.h>

#define BlueProductLid   1
#define	BlueProductBase  2
#define BlueRawMaterial  3
#define NONE 11 
#define MODE_NO_PATH 7
#define PROCESSING 1
#define TO_BASE 0
#define TO_LID 1
#define MODE_GIVEN_PATH 6

using namespace std;

int index_  = 1;
int id_counter = 1;
int block_id = 1;
int read_order_status, read_order_id;
int flag_order_done[10];
int flag_order_index = 0;



UA_Int16 read_attribute(UA_Client *client, int nodeId, char *Identifier, int TYPE){

    UA_StatusCode retval;
    UA_Int16 value = 0;

    UA_Variant *val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(nodeId, Identifier), val);
    if(retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
    val->type == &UA_TYPES[TYPE])  {
            value = *(UA_Int16*)val->data;
            //printf("the value is: %i\n", value);
            return value;
    }
    else
   std::cout << "ERROR IN READ \n " << TYPE;
   return 0;

    UA_Variant_delete(val);

}

int write_attribute(UA_Client *client, int nodeId, char *Identifier, int TYPE, UA_Int16 value)
{
    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = UA_WriteValue_new();
    wReq.nodesToWriteSize = 1;
    wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC(nodeId, Identifier);
    wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wReq.nodesToWrite[0].value.hasValue = true;
    wReq.nodesToWrite[0].value.value.type = &UA_TYPES[TYPE];
    wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */
    wReq.nodesToWrite[0].value.value.data = &value;
    UA_WriteResponse wResp = UA_Client_Service_write(client, wReq);
    
    if(wResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
    return 1;
    
    else
    printf("ERROR IN WRITE \n");
    return 0;    
    
    UA_WriteRequest_clear(&wReq);
    UA_WriteResponse_clear(&wResp);

}

uint8_t sig(UA_Client *client)
{
    while ((read_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.new_data", UA_TYPES_BOOLEAN) == 1)) {}    
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.new_data", UA_TYPES_BOOLEAN,1);
    sleep(1); 
    while ((read_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.new_data", UA_TYPES_BOOLEAN) == 0)) {}
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.new_data", UA_TYPES_BOOLEAN,0); 
    sleep(1); 
    return 1;
}

uint8_t create_block(UA_Client *client, uint8_t target, uint8_t block_id, uint8_t dest, uint8_t order_id, uint8_t type) {

    write_attribute(client,4,"|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.id", UA_TYPES_INT16, block_id);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_lid", UA_TYPES_INT16,BlueProductLid);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_base", UA_TYPES_INT16,BlueProductBase);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.type_part", UA_TYPES_INT16,type);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.routing_mode", UA_TYPES_INT16,MODE_NO_PATH);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.order_id", UA_TYPES_INT16,order_id);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.id", UA_TYPES_INT16,order_id);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.order_type", UA_TYPES_INT16,1);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.target_group[1]", UA_TYPES_INT16,target);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_conveyor", UA_TYPES_INT16,dest);

    if(target == 11)
    printf("\nCreating part in the left emitter\n",target);
    else if (target == 1)
    printf("\nCreating part in the top emitter\n",target);
    else if (target == 14)
    printf("\nCreating part in the right emitter\n",target);
    return 1;
}

uint8_t change_op(UA_Client *client, uint8_t target, uint8_t op){

    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.order_type", UA_TYPES_INT16,10);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.target_group[1]", UA_TYPES_INT16,target);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.op", UA_TYPES_INT16,op);
    sig(client);

    printf("\nSending a CHANGE_OP order!\n");
}


uint8_t prepare_order(UA_Client *client, uint8_t n) {
    switch (n) {
        case 1: create_block(client,1,block_id,6,8, BlueRawMaterial); break;

        case 2: write_attribute(client,4,"|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.id", UA_TYPES_INT16, block_id);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_lid", UA_TYPES_INT16,BlueProductLid);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_base", UA_TYPES_INT16,BlueProductBase);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.type_part", UA_TYPES_INT16,BlueProductBase);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.routing_mode", UA_TYPES_INT16,MODE_GIVEN_PATH);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.order_id", UA_TYPES_INT16,id_counter);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.id", UA_TYPES_INT16,id_counter);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.order_type", UA_TYPES_INT16,1);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.target_group[1]", UA_TYPES_INT16,11);
                write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_conveyor", UA_TYPES_INT16,15); 
                printf("\nCreating part in the left emitter  \n");
                break;

        case 3: create_block(client,14,block_id,15,id_counter, BlueProductLid); break;
        case 4: create_block(client,14,block_id,16,id_counter,BlueRawMaterial); break;
        }
    return 1;
}

uint8_t ask_info(UA_Client *client)
{
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.target_group[-7]", UA_TYPES_INT16,6);
    write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.order_type", UA_TYPES_INT16,4);
    sig(client);
    return 1;  
}

uint8_t read_status(UA_Client *client)
{
    printf("\nReading orders' status...\n");
    flag_order_index = 0;
    int while_counter = 1;
    char straux[100];
    sprintf(straux,"|var|CODESYS Control Win V3 x64.Application.commands.status[%d].id",while_counter);

    while (read_attribute(client, 4, straux, UA_TYPES_INT16) > 0)
    {
        sprintf(straux,"|var|CODESYS Control Win V3 x64.Application.commands.status[%d].block_info.order_id",while_counter);
        if (read_attribute(client, 4, straux, UA_TYPES_INT16) == 8){
            sprintf(straux,"|var|CODESYS Control Win V3 x64.Application.commands.status[%d].id",while_counter);
            flag_order_done[flag_order_index] = read_attribute(client, 4, straux, UA_TYPES_INT16);
            flag_order_index++;  
        }
        while_counter++;
    }

    if (flag_order_done) {
        while ((read_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.read_sig", UA_TYPES_BOOLEAN) == 1)) {}
        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.read_sig", UA_TYPES_BOOLEAN,1);
        sleep(1);
        while ((read_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.read_sig", UA_TYPES_BOOLEAN) == 0)) {}
        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.read_sig", UA_TYPES_BOOLEAN,0);
        return 1;  
    }

}

int main(int argc, char *argv[]) {

    UA_Int16 value = 0;
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval;

    // Connect to a server 
    retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); 
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }
 // printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

    int k = 1;
    while(1)
    {
        flag_order_index = 0;
        read_status(client);
        ask_info(client);

        while (flag_order_done[flag_order_index-1] > 0) {
            switch (flag_order_done[flag_order_index-1])
            {
                case 6:change_op(client,6,TO_LID); break;
                case 7: write_attribute(client,4,"|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.id", UA_TYPES_INT16, 0);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_lid", UA_TYPES_INT16,BlueProductLid);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_base", UA_TYPES_INT16,BlueProductBase);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.type_part", UA_TYPES_INT16,BlueRawMaterial);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.routing_mode", UA_TYPES_INT16,MODE_GIVEN_PATH);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.order_id", UA_TYPES_INT16,0);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.id", UA_TYPES_INT16,id_counter);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.order_type", UA_TYPES_INT16,2);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.target_group[1]", UA_TYPES_INT16,7);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.PATH[0]", UA_TYPES_INT16,8);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.PATH[1]", UA_TYPES_INT16,17);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.PATH[2]", UA_TYPES_INT16,13);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.PATH[3]", UA_TYPES_INT16,15);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.PATH[4]", UA_TYPES_INT16,-1);
                        write_attribute(client, 4, "|var|CODESYS Control Win V3 x64.Application.commands.orders_master.block_info.final_conveyor", UA_TYPES_INT16,0);
                        printf("\nChanging the path for part in the curved conveyor\n");      
                        sig(client); break;
            }
            flag_order_index++;
        }

        prepare_order(client, k);
        sig(client);

        if (k == 4)
        k=1;
        else 
        k++;

        block_id++;
        id_counter++;
        sleep(1);
    }

    UA_Client_disconnect(client);
    UA_Client_delete(client);
    
    printf("... end of program ...\n");
    return EXIT_SUCCESS;
}


