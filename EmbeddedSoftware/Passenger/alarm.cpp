#include "mbed.h"
#include "commond.h"
PwmOut buzzer (PC_8);
DigitalIn enable(D9);
uint8_t alarm=0x00;
void beep_onoff()
{
    while(1) {
        // loop
        //osEvent evt = Thread::signal_wait(0); // wait fot command
        //if (evt.value.signals & 0x01) {
        if(!enable) {
            buzzer.period(0.005f);      // 1 ms period (1 kHz)
            buzzer.write(0.50f);
            wait(0.5);
            alarm = 0x01;
        } else {
            buzzer.write(0);
            alarm = 0x00;
            //Thread::wait(0.5);
        }

        //}
        //if (evt.value.signals & 0x02) {
        //  Beep = 1;
        //}
    }
}
void alarmFcn()
{

    Thread beepThread(osPriorityNormal, (DEFAULT_STACK_SIZE * 2.25), NULL);
    beepThread.start(beep_onoff);
    while(1) {
        //loop
        osEvent evt = alarmQueue.get();
        if(evt.status == osEventMessage) {
            message_t *msg = (message_t *)evt.value.p;
            switch (msg->command) {
                case 0x00:
                    message_t res;
                    res.destAddr = 0x00;
                    res.srcAddr = devices[3].id;
                    res.ctrl = 0x02;
                    res.command = 0x00;
                    res.arg = 0x00;
                    res.size = 0x02;
                    res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                    txQueue.put(&res);
                    break;
                case 0x01:
                    if(devices[3].status==0) {
                        devices[3].id=msg->arg;
                        devices[3].status=2;
                        res.destAddr = 0x00;
                        res.srcAddr = devices[3].id;
                        res.ctrl = 0x02;
                        res.command = msg->command;
                        res.arg = devices[3].id;
                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        txQueue.put(&res);
                    } else {
                        res.destAddr = 0x00;
                        res.srcAddr = devices[3].id;
                        res.ctrl = 0x02;
                        res.command = msg->command;
                        res.arg = devices[3].id;
                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        txQueue.put(&res);
                    }

                    break;

                case 0x02:
                    if(devices[3].status==2) {

                        res.destAddr = 0x00;
                        res.srcAddr = devices[3].id;
                        res.ctrl = 0x02;
                        res.command = devices[3].function >> 8;
                        res.arg = devices[3].function;
                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        txQueue.put(&res);
                    } else {

                    }

                    break;

                case 0x03:
                    if(devices[3].status==2) {
                        //if(msg->arg == 0x01) {
//                            stop = false;

                        res.arg = alarm;
                        // } else {
//                            Beep=1;
//                            stop = true;
//
//                            res.arg = 0x00;
//                        }
                        res.destAddr = 0x00;
                        res.srcAddr = devices[3].id;
                        res.ctrl = 0x02;
                        res.command = msg->command;



                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        txQueue.put(&res);
                    } else {

                    }

                    break;
                case 0x04:
                    if(devices[3].status==2 && msg->srcAddr==0x00) {
                        devices[3].status=0;
                        res.destAddr = 0x00;
                        res.srcAddr = devices[3].id;
                        res.ctrl = 0x02;
                        res.command = msg->command;
                        res.arg = 0x00;


                        res.size = 0x02;
                        res.crc = res.ctrl + res.destAddr + res.srcAddr + res.command + res.arg + res.size;
                        txQueue.put(&res);

                        
                    }
                    break;


            }
        }
    }
}