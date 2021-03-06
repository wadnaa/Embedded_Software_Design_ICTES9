#include "mbed.h"
#include "commond.h"

void powerFcn(){
    // setup
    while(1){
        //loop
        osEvent evt = powerQueue.get();
        if(evt.status == osEventMessage){
            message_t *msg = (message_t *)evt.value.p;
            switch (msg->command) {
                case 0x00:
                    message_t res;
                    res.destAddr = 0x00;
                    res.srcAddr = devices[0].id;
                    res.ctrl = 0x02;
                    res.command = 0x00;
                    res.arg = 0x00;
                    res.size = 0x02;
                    res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                    rxQueue.put(&res);
                    break;
                case 0x01:
                    if(devices[0].status==0){
                        devices[0].id=msg->arg;
                        devices[0].status=2;
                        res.destAddr = 0x00;
                        res.srcAddr = devices[0].id;
                        res.ctrl = 0x02;
                        res.command = msg->command;
                        res.arg = devices[0].id;
                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        rxQueue.put(&res);
                    }else{
                        
                    }
                    
                    break;
                    
                case 0x02:
                    if(devices[0].status==2){
                        res.destAddr = 0x00;
                        res.srcAddr = devices[0].id;
                        res.ctrl = 0x02;
                        res.command = devices[0].function >> 8;
                        res.arg = devices[0].function;
                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        rxQueue.put(&res);
                    }else{
                        
                    }
                    
                    break;
                    
                case 0x03:
                    if(devices[0].status==2){
                        res.destAddr = 0x00;
                        res.srcAddr = devices[0].id;
                        res.ctrl = 0x02;
                        res.command = msg->command;
                        if(position >= 127){
                            res.arg = (position - 127) / 12;
                        }else{
                            res.arg = 0;
                        }
                        
                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        rxQueue.put(&res);
                    }else{
                        
                    }
                    break;
            }
        }
    }
}