#define NUMDEVICES 4
typedef struct message_t
{
    uint8_t destAddr; 
    uint8_t srcAddr;
    uint8_t ctrl;
    uint8_t size;
    uint8_t command;
    uint8_t arg;
    uint8_t crc;
}message_t;
typedef struct device
{
    uint8_t id;
    int status;
    uint16_t function;
    Queue<message_t, 10> *q;
 }device;
void lightingFcn(void);
void airconFcn(void);
void alarmFcn(void);
void doorFcn(void);
void rxFcn(void);
void txFcn(void);



// Global variable reference
extern RawSerial pc;
extern RawSerial RS485;
extern DigitalOut led1;
extern device devices[4];
extern Queue<message_t, 10> lightingQueue;
extern Queue<message_t, 10> airconQueue;
extern Queue<message_t, 10> doorQueue;
extern Queue<message_t, 10> alarmQueue;
extern Queue<message_t, 10> rxQueue;
extern Queue<message_t, 10> txQueue;