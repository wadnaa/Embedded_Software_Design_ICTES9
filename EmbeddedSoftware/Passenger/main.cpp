#include "mbed.h"
#include "commond.h"

DigitalOut led1(LED1);
RawSerial pc(USBTX, USBRX);
RawSerial RS485(PA_11, PA_12,115200);
DigitalOut WriteEnable(D2);


Thread rxDevice;
Thread txDevice;
Thread airconDevice;
Thread lightDevice;
Thread alarmDevice;
Thread doorDevice;

Queue<message_t, 10> lightingQueue;
Queue<message_t, 10> airconQueue;
Queue<message_t, 10> doorQueue;
Queue<message_t, 10> alarmQueue;
Queue<message_t, 10> txQueue;
Queue<message_t, 10> rxQueue;

device devices[4];


void txFcn(){
    while(1){
        osEvent evt = txQueue.get();
        if(evt.status == osEventMessage) {
            message_t *msg = (message_t *)evt.value.p;
            WriteEnable = 1;
            Thread::wait(20);
            RS485.putc(0x7E);
            RS485.putc(msg->destAddr);
            RS485.putc(msg->srcAddr);
            RS485.putc(msg->ctrl);
            RS485.putc(msg->size);
            RS485.putc(msg->command);
            RS485.putc(msg->arg);
            RS485.putc(msg->size);
            RS485.putc(msg->crc);
            Thread::wait(5);
            WriteEnable = 0;
        }  
    }
}
void rxFcn()
{
    //setup
    while(1) {
        //loop
        osEvent evt = rxQueue.get();
        if(evt.status == osEventMessage) {
            message_t *msg = (message_t *)evt.value.p;
            if(msg->destAddr == 0xFF) {
                Thread::wait(300);
                lightingQueue.put(msg);
                airconQueue.put(msg);
                doorQueue.put(msg);
                alarmQueue.put(msg);
            }
            else{
                for(int i = 0 ; i < NUMDEVICES ; i++){
                    if(devices[i].id == msg->destAddr){
                        devices[i].q->put(msg);
                    }      
                }
            }
        }
    }
//        msg.destAddr = 10;
//        lightingQueue.put(&msg);
//        Thread::wait(2000);
//
}
// main() runs in its own thread in the OS
void init()
{
    
//    devices[0].id = 16;
    devices[0].id = 0x40;
    //pc.printf("ID:%d\n\r",devices[0].id);
    devices[0].status = 0;
    devices[0].function = 0x0100;
    devices[0].q = &lightingQueue;

//    devices[1].id = 32;
    devices[1].id = 0x41;
    devices[1].status = 0;
    devices[1].function = 0x0200;
    devices[1].q = &airconQueue;
    //pc.printf("ID:%d\n\r",devices[1].id);
    
//    devices[2].id = 48;
    devices[2].id = 0x42;
    devices[2].status = 0;
    devices[2].function = 0x0300;
    devices[2].q = &doorQueue;
    //pc.printf("ID:%d\n\r",devices[2].id);
    
//    devices[3].id = 64;
    devices[3].id = 0x43;
    devices[3].status = 0;
    devices[3].function = 0x0400;
    devices[3].q = &alarmQueue;
    //pc.printf("ID:%d\n\r",devices[3].id);
}

void rs485_listen()
{
    static int index = 0;
    char ch = RS485.getc();
    static message_t msgk;

    switch (index) {
        case 0:
            if (ch == 0x7E)
                index++;
            break;
        case 1:
            //Destination
            msgk.destAddr = ch;
            index++;
            break;
        case 2:
            //Source
            msgk.srcAddr = ch;
            index++;
            break;
        case 3:
            //Link Control
            msgk.ctrl = ch;
            index++;
            break;
        case 4:
            //Size
            msgk.size = ch;

            index++;
            break;
        case 5:
            //Command
            msgk.command = ch; // << 8;
            //txQueue.put(&msg);
            index++;
            break;
        case 6:
            //Arguement
            msgk.arg = ch;
            //txQueue.put(&msg);

            index++;
            break;
        case 7:
            //CRC
            msgk.crc = ch;
            rxQueue.put(&msgk);
            index=0;
            WriteEnable = 0;
            break;
        default:
            index =0;
    }
}
int main()
{
    WriteEnable = 0;
    init();

    lightDevice.start(lightingFcn);
    airconDevice.start(airconFcn);
    alarmDevice.start(alarmFcn);
    doorDevice.start(doorFcn);
    txDevice.start(txFcn);
    rxDevice.start(rxFcn);


    // message_t msg;
//    msg.destAddr = 0xFF;
//    msg.srcAddr = 0x00;
//    msg.ctrl = 0x00;
//    msg.command = 0x00;
//    msg.arg = 0;
//    msg.size = 0x01;
//    txQueue.put(&msg);

    //pc.attach(pc_listen, Serial::RxIrq);
    RS485.attach(rs485_listen, Serial::RxIrq);
//    while (true) {
//        //led1 = !led1;
//        wait(0.5);
//    }
}

