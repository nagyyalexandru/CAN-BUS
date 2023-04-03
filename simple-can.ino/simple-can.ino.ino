#include <Arduino.h>
#include <SPI.h>

const int CS_PIN = 8;
const int SO_PIN = 9;
const int SI_PIN = 12;
const int SCK_PIN = 11;

SPISettings spiSettings(10000000, MSBFIRST, SPI_MODE0);

const byte MCP2515_CANCTRL = 0x0F;
const byte MCP2515_CANCTRL_REQOP = 0xE0;
const byte MCP2515_CANCTRL_REQOP_NORMAL = 0x00;
const byte MCP2515_CANSTAT = 0x0E;
const byte MCP2515_CANINTF = 0x2C;
const byte MCP2515_CANINTF_RX0IF = 0x01;
const byte MCP2515_CANINTF_RX1IF = 0x02;
const byte MCP2515_CANINTF_TX0IF = 0x04;
const byte MCP2515_CANINTF_TX1IF = 0x08;
const byte MCP2515_CANINTF_TX2IF = 0x10;
const byte MCP2515_CANINTF_ERRIF = 0x20;
const byte MCP2515_CANINTF_WAKIF = 0x40;
const byte MCP2515_CANINTF_MERRF = 0x80;
const byte MCP2515_RXB0CTRL = 0x60;
const byte MCP2515_RXB0CTRL_RXM = 0x60;
const byte MCP2515_RXB0CTRL_RXM_ANY = 0x60;
const byte MCP2515_RXB0CTRL_BUKT = 0x04;
const byte MCP2515_RXB1CTRL = 0x70;
const byte MCP2515_RXB1CTRL_RXM = 0x60;
const byte MCP2515_RXB1CTRL_RXM_ANY = 0x60;
const byte MCP2515_RXB1CTRL_BUKT = 0x04;
const byte MCP2515_TXB0CTRL = 0x30;
const byte MCP2515_TXB1CTRL = 0x40;
const byte MCP2515_TXB2CTRL = 0x50;
const byte MCP2515_TXBCTRL_TXREQ = 0x08;
const byte MCP2515_TXRTSCTRL = 0x0D;
const byte MCP2515_TXRTSCTRL_B0RTS = 0x01;
const byte MCP2515_TXRTSCTRL_B1RTS = 0x02;
const byte MCP2515_TXRTSCTRL_B2RTS = 0x04;

void initMCP2515() {
  // set MCP2515 to configuration mode
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(MCP2515_CANCTRL | MCP2515_CANCTRL_REQOP);
  SPI.transfer(MCP2515_CANCTRL_REQOP_NORMAL);
  digitalWrite(CS_PIN, HIGH);
 
  // set receive buffer 0 to receive any message
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(MCP2515_RXB0CTRL);
  SPI.transfer(MCP2515_RXB0CTRL_RXM | MCP2515_RXB0CTRL_BUKT);
  digitalWrite(CS_PIN, HIGH);
 
  // set receive buffer 1 to receive any message
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(MCP2515_RXB1CTRL);
  SPI.transfer(MCP2515_RXB1CTRL_RXM | MCP2515_RXB1CTRL_BUKT);
  digitalWrite(CS_PIN, HIGH);
}

void sendMessage(uint32_t id, uint8_t* data, uint8_t length) {
  // select transmit buffer 0
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(MCP2515_TXB0CTRL);
  SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);

  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x31);
  SPI.transfer(id >> 21);
  SPI.transfer((id >> 13) & 0xFF);
  SPI.transfer((id >> 5) & 0xFF);
  SPI.transfer((id << 3) & 0xFF);
  
  for (uint8_t i = 0; i < length; i++)
  {
    SPI.transfer(data[i]);
  }
  
  digitalWrite(CS_PIN, HIGH);
  // set transmit request bit
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(MCP2515_TXB0CTRL | MCP2515_TXBCTRL_TXREQ);
  digitalWrite(CS_PIN, HIGH);
}


bool receiveMessage(uint32_t& id, uint8_t* data, uint8_t& length) {
// check if there is a message in receive buffer 0
digitalWrite(CS_PIN, LOW);
SPI.transfer(MCP2515_CANINTF);
uint8_t canintf = SPI.transfer(0x00);
digitalWrite(CS_PIN, HIGH);

if (canintf & MCP2515_CANINTF_RX0IF) {
   // read message identifier and data from receive buffer 0
   digitalWrite(CS_PIN, LOW);
   SPI.transfer(MCP2515_RXB0CTRL);
   uint8_t rxctrl = SPI.transfer(0x00);
   digitalWrite(CS_PIN, HIGH);
   digitalWrite(CS_PIN, LOW);
   SPI.transfer(0x90);
   id = ((uint32_t) SPI.transfer(0x00) << 21) |
        ((uint32_t) SPI.transfer(0x00) << 13) |
        ((uint32_t) SPI.transfer(0x00) << 5) |
        ((uint32_t) SPI.transfer(0x00) >> 3);
   length = SPI.transfer(0x00) & 0x0F;
   for (uint8_t i = 0; i < length; i++) {
     data[i] = SPI.transfer(0x00);
   }
   digitalWrite(CS_PIN, HIGH);
   
   // release receive

  digitalWrite(CS_PIN, LOW);
   SPI.transfer(MCP2515_CANINTF);
   SPI.transfer(~MCP2515_CANINTF_RX0IF);
   digitalWrite(CS_PIN, HIGH);
   
   return true;
 }
 
 // check if there is a message in receive buffer 1
 digitalWrite(CS_PIN, LOW);
 SPI.transfer(MCP2515_CANINTF);
 canintf = SPI.transfer(0x00);
 digitalWrite(CS_PIN, HIGH);
 
 if (canintf & MCP2515_CANINTF_RX1IF) {
   // read message identifier and data from receive buffer 1
   digitalWrite(CS_PIN, LOW);
   SPI.transfer(MCP2515_RXB1CTRL);
   uint8_t rxctrl = SPI.transfer(0x00);
   digitalWrite(CS_PIN, HIGH);
   digitalWrite(CS_PIN, LOW);
   SPI.transfer(0x94);
   id = ((uint32_t) SPI.transfer(0x00) << 21) |
        ((uint32_t) SPI.transfer(0x00) << 13) |
        ((uint32_t) SPI.transfer(0x00) << 5) |
        ((uint32_t) SPI.transfer(0x00) >> 3);
   length = SPI.transfer(0x00) & 0x0F;
   for (uint8_t i = 0; i < length; i++) {
     data[i] = SPI.transfer(0x00);
   }
   digitalWrite(CS_PIN, HIGH);
   
   // release receive buffer 1
   digitalWrite(CS_PIN, LOW);
   SPI.transfer(MCP2515_CANINTF);
   SPI.transfer(~MCP2515_CANINTF_RX1IF);
   digitalWrite(CS_PIN, HIGH);
   
   return true;
 }
 
 return false;
}

void setup()
{
  Serial.begin(9600);
}
 
 
void loop() {
  // send a message with identifier 0x123 and data [0x01, 0x02, 0x03]
  uint8_t data[] = {0x01, 0x02, 0x03};
  sendMessage(0x123, data, sizeof(data));
  // check for incoming messages
  uint32_t id;
  uint8_t receivedData[8];
  uint8_t length;
  if (receiveMessage(id, receivedData, length)) {
    Serial.print("Message received");
  }
}

