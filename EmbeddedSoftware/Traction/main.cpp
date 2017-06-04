#include "mbed.h"
#include "commond.h"
#include "Nunchuk.h"
//DigitalOut led1(LED1);
RawSerial pc(USBTX, USBRX);
RawSerial RS485(PA_11, PA_12,115200);
DigitalOut WriteEnable(D2);
I2C i2c(D14, D15);
Nunchuk * nc;
int position;

Thread powerDevice;
Thread brakeDevice;
Thread statusDevice;
Thread alarmDevice;
Thread rxDevice;
Thread controlDevice;

Thread txDevice;
Queue<message_t, 10> txQueue;

Queue<message_t, 10> powerQueue;
Queue<message_t, 10> brakeQueue;
Queue<message_t, 10> alarmQueue;
Queue<message_t, 10> statusQueue;
Queue<message_t, 10> rxQueue;

device devices[4];
uint8_t passTABLE[PASSNUM];
void boardcastMSG(message_t *msg)
{
//    for(int i = 0 ; i < PASSNUM ; i++){
        WriteEnable = 1;
        Thread::wait(20);
        RS485.putc(0x7E);
//        RS485.putc(passTABLE[i]);
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
//        Thread::wait(50);
//    }
}
void forward(message_t *msg){
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
void txFcn(){
    while(1) {
    //loop
        osEvent evt = txQueue.get();
        if(evt.status == osEventMessage) {
            message_t *msg = (message_t *)evt.value.p; 
            if(msg->destAddr == 0xFF) {
                powerQueue.put(msg);
                brakeQueue.put(msg);
                statusQueue.put(msg);
                alarmQueue.put(msg);
                boardcastMSG(msg);
            }
            else{
                for(int i=0; i<NUMDEVICES; i++) {
                    if(devices[i].id == msg->destAddr) {
                        devices[i].q->put(msg);
                        //if(msg->command == 0x01){
//                            passTABLE[i] = msg -> arg;
//                        }
                    }
                }
                boardcastMSG(msg);
//                forward(msg);
//                if(msg->command == 0x01){
//                    for(int i = 0 ; i < PASSNUM; i++){
//                        if(msg->destAddr == passTABLE[i]){
//                            passTABLE[i] = msg -> arg;
//                        }
//                        break;
//                    }    
//                }
            }
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
            if(msg->destAddr == 0x00) {

                pc.putc(0x7E);
                pc.putc(msg->destAddr);
                pc.putc(msg->srcAddr);
                pc.putc(msg->ctrl);
                pc.putc(msg->size);
                pc.putc(msg->command);
                pc.putc(msg->arg);
                pc.putc(msg->crc);
            } 
        }
    }
}
void init()
{
    devices[0].id = 0x21;
    devices[0].status = 0;
    devices[0].function = 0x0001;
    devices[0].q = &powerQueue;

    devices[1].id = 0x22;
    devices[1].status = 0;
    devices[1].function = 0x0002;
    devices[1].q = &brakeQueue;

    devices[2].id = 0x23;
    devices[2].status = 0;
    devices[2].function = 0x0003;
    devices[2].q = &statusQueue;

    devices[3].id = 0x24;
    devices[3].status = 0;
    devices[3].function = 0x0004;
    devices[3].q = &alarmQueue;
    

    passTABLE[0] = 0x30; 
    passTABLE[1] = 0x31;
    passTABLE[2] = 0x32;
    passTABLE[3] = 0x33;
    
    passTABLE[4] = 0x40;
    passTABLE[5] = 0x41;
    passTABLE[6] = 0x42;
    passTABLE[7] = 0x43;
    
}
void pc_listen()
{
    static int index = 0;
    char ch = pc.getc();
    static message_t msg;

    switch (index) {
        case 0:
            if (ch == 0x7E)
                index++;
            break;
        case 1:
            //Destination
            msg.destAddr = ch;
            index++;
            break;
        case 2:
            //Source
            msg.srcAddr = ch;
            index++;
            break;
        case 3:
            //Link Control
            msg.ctrl = ch;
            index++;
            break;
        case 4:
            //Size
            msg.size = ch;

            index++;
            break;
        case 5:
            //Command
            msg.command = ch; // << 8;
            //nodeQueue.put(&msg);
            index++;
            break;
        case 6:
            //Arguement
            msg.arg = ch;
            //nodeQueue.put(&msg);

            index++;
            break;
        case 7:
            //CRC
            msg.crc = ch;
            txQueue.put(&msg);
            index=0;
            break;
        default:
            index =0;
    }
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
            if(ch == 0x00){
                //pc.putc('R');
                msgk.destAddr = ch;
                index++;
            }else{
                index=0;
            }
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
            //nodeQueue.put(&msg);
            index++;
            break;
        case 6:
            //Arguement
            msgk.arg = ch;
            //nodeQueue.put(&msg);

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
void controlFcn()
{
    Nunchuk::NunchukData data;
    while(1) {
        nc->GetData(data);
        position = data.yStick;
        wait(0.1);
    }
}
// main() runs in its own thread in the OS
int main()
{
    nc = new  Nunchuk(i2c);
    WriteEnable = 0;
    init();
    nc->Init();
    powerDevice.start(powerFcn);
    brakeDevice.start(brakeFcn);
    statusDevice.start(statusFcn);
    alarmDevice.start(alarmFcn);
    
    txDevice.start(txFcn);
    
    rxDevice.start(rxFcn);
    controlDevice.start(controlFcn);

    pc.attach(pc_listen, Serial::RxIrq);
    RS485.attach(rs485_listen, Serial::RxIrq);
}

