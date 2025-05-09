
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#include <arduino.h>
#include <telnet.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ETH.h>
#include <CANguru.h>

static bool server_connected = false;

// liefert die Info, ob ETHERNET aufgebaut ist
bool getServerStatus()
{
  return server_connected;
}

// setzt den Status, dass ETHERNET aufgebaut ist
void setServerStatus(bool status)
{
  server_connected = status;
}

// Klasse, mit dem der Telnet-Client bedient wird
// mit dem Telnet-Client werden die Ausgaben für den CANguru-Server
// dorthin transportiert

// Initialisierung des Telnet-Clients
void canguruETHClient::telnetInit()
{
  telnethasconnected = false;
  ipBroadcastSet = false;
  telnetServer.setNoDelay(true);
}

// Start des Telnet-Client
bool canguruETHClient::startTelnetSrv()
{
  telnetServer.begin(23);
  if (telnetServer.hasClient())
  {
    if (!tlntClnt || !tlntClnt.connected())
    {
      if (tlntClnt)
        tlntClnt.stop();
      setTelnetClient(telnetServer.available());
      printTelnet(true, "CANguru-Bridge & WebServer Version " + CgVersionnmbr);
      printTelnet(true, "");
      printTelnet(true, "From (own)IP " + ip2strng(ETH.localIP()));
      printTelnet(true, "Connected to " + ip2strng(tlntClnt.remoteIP()));
      printTelnet(true, "");
      delay(100);
    }
  }
  return telnethasconnected;
}

bool canguruETHClient::lostTelnetSrv()
{
  bool lost = telnethasconnected && !tlntClnt.connected();
  telnethasconnected = false;
  return lost;
}
// Überprüfung, ob noch eine Verbindung besteht
void canguruETHClient::setTelnetClient(WiFiClient tC)
{
  tlntClnt = tC;
  ipBroadcast = tlntClnt.remoteIP();
  ipBroadcast[3] = 255;
  telnethasconnected = true;
}

// stellt die Ziel-IP-Adresse zur Verfügung
IPAddress canguruETHClient::getipBroadcast()
{
  return ipBroadcast;
}

// überträgt einen String, mit nl wahr zusätzlich mit neuer Zeile
void canguruETHClient::printTelnet(bool nl, String str, uint8_t indent)
{
  char charArray[str.length() + 1]; // +1 for the null terminator
  strcpy(charArray, str.c_str());
  log_i("%s", charArray);

  if (tlntClnt && tlntClnt.connected())
  {
    uint8_t strlen = str.length() + 1;
    if (nl && strlen == 1)
    {
      tlntClnt.write("\r\n", 2);
    }
    else
    {
      strlen += indent;
      unsigned char buffer[strlen];
      memset(buffer, 0x20, strlen);
      tlntClnt.write("!", 1);
      str.getBytes(&buffer[indent], strlen, 0x00);
      tlntClnt.write(buffer, strlen);
      if (nl)
        tlntClnt.write("\r\n", 2);
    }
//    delay(5);
  }
}

// überträgt ein array mit char
void canguruETHClient::printTelnetArr(uint8_t *buffer)
{
  tlntClnt.write(buffer, 30);
}

// überträgt eine Zahl
void canguruETHClient::printTelnetInt(bool nl, int num)
{
  char array[64];
  uint8_t strlen = sprintf(array, "%d", num);
  if (tlntClnt && tlntClnt.connected())
  {
    tlntClnt.write(array, strlen);
    if (nl)
      tlntClnt.write("\r\n", 2);
    delay(10);
  }
}

// überträgt zwei Zahlen
void canguruETHClient::printTelnetInts(int num1, int num2)
{
  char array[64];
  uint8_t strlen = sprintf(array, "%d - %d", num1, num2);
  if (tlntClnt && tlntClnt.connected())
  {
    tlntClnt.write(array, strlen);
    tlntClnt.write("\r\n", 2);
  }
}

// wandelt eine IP-Adresse in einen String um
String canguruETHClient::ip2strng(IPAddress adr)
{
  String address = "";
  address += adr[0];
  for (uint8_t j = 1; j < 4; j++)
  {
    address += ".";
    address += adr[j];
  }
  return address;
}
