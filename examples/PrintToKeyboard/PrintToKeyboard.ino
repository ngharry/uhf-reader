#include "Database.h"
#include "UHFRecv.h"

#define RS485_CONTROL           4  //< Pin for RS485 Direction Control
#define LED                     17 //< LED Pin
#define BAUD_RATE               9600

// Comment out this to speed up the setup process (no flash())
#define DEBUG

void flash();

Database* database;
UHFRecv TictagUhf(Serial1, BAUD_RATE, RS485_CONTROL);

uint32_t now = 0;
uint32_t period = 600; //< Change this value to modify time between inventory
                       //  sessions (600 means 600 ms).

void setup()
{
    /* Setting Serial */
    Serial.begin(BAUD_RATE);
    flash();

    /* Setting  UHF Receiver */
    #ifdef DEBUG 
    Serial.println("Initialise UHF Receiver.");
    flash();
    #endif
    TictagUhf.begin();

    /* Setting database */
    #ifdef DEBUG
    Serial.println("Initialise Database");
    flash();
    #endif
    database = new Database;
    database->begin();

    /* Setting prefix */
    String prefix = "";
    #ifdef DEBUG
    Serial.print("Setting prefix to ");
    if (prefix == "")
        Serial.println("default prefix: empty string");
    else
        Serial.println(prefix);
    #endif
    _setPrefix(prefix);
    
    delay(300);
    Serial.println("Scanning cards...");
}

void loop()
{
    Status status = 0x00;

    byte reData[INV_MAX_SIZE] = {0};
    
    byte* cmd = TictagUhf.setCommand(READER_ADDRESS, TID_ARRESSS, LENGTH_TID);
    size_t sizeCmd = TictagUhf.getSizeCommand();

    if (millis() > (now + period)) { 
        now = millis();
        TictagUhf.getRawData(reData, cmd, sizeCmd); //< Get raw data from 
                                                    //  inventory command
        if (TictagUhf.isDataPreserved(reData, sizeof(reData))) {
            if ((status = database->updateDB(reData)) != STATUS_SUCCESS)
                return ;
            database->printToKeyboard();
        } else {
            Keyboard.println("CSERR");
        }
    }
}

void flash()
{
    digitalWrite(LED, HIGH);
    delay(800);
    digitalWrite(LED, LOW);
}
