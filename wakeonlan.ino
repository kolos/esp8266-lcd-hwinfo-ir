bool wolMacMatches(byte* packet, byte* macAddr) {
  byte macInit[6] = WOL_INIT;
  if(memcmp(packet, macInit, 6) != 0)
    return false;

  for(byte i=1;i<=16;i++) {
    if(memcmp(packet + i * 6, macAddr, 6) != 0) {
      return false;
    }
  }

  return true;
}

void handleWolPackets() {
  int pktSize = udp.parsePacket();
  if (pktSize) {
    byte pktBuf[pktSize];
    udp.read(pktBuf, pktSize);
    udp.stop();
    udp.begin(LISTEN_PORT);

    if(pktSize == 102) { // WoL packet size
      byte wol_pc_on[6] = WOL_PC_ON;
      if(wolMacMatches(pktBuf, wol_pc_on)) {
          irsend.sendSAMSUNG(TV_ON_CODE, 32); // TV - ON
      }
    }
  }
}
