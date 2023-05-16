bool simActive;
bool simEnable;


void setup() {

}

void loop() {

// SIM stuff
else if (cmd == "SIM"){
  if (cmdarg == "ENABLE\n"){
    cmdecho = "SIME";
    simEnable = true;
    flightMode = 'S';
  }
  else if (cmdarg == "DISABLE\n"){
    cmdecho = "SIMD";
    simActive = false;
    simEnable = false;
    flightMode = 'F';
  }
  else if (cmdarg == "ACTIVATE\n" and simEnable){
    cmdecho = "SIMA";
    simActive = true;
  }
}


// inside the void loop right at the beginning
if (!(simActive and simEnable) and Serial1.available()) {
  String packet = Serial1.readString();
  
  if (itemAt(packet, 0) == "CMD" and itemAt(packet, 1) == "1070") {
    readcommands(itemAt(packet, 2), itemAt(packet, 3)); 
    // maybe check if item 3 is CMD?? that would happen if there is no arg like in CAL
  }
} 
else if (simActive and simEnable){
  while (true){
    while (!Serial1.available()){
      ;
    }
      
    String packet = Serial1.readString();
    
    if (itemAt(packet,0) == "CMD" && itemAt(packet, 1) == "1070"){
      cmd = itemAt(packet, 2);
      cmdarg = itemAt(packet, 3);
      
      if (cmd == "SIMP\n"){
        pressure = float(cmdarg.toInt()); // convert string to float, fix this line smile
        altitude = round(10 * (bmp.readAltitude(pressure)))/10.0;
        break;
      }
      else {
        readcommands(cmd, cmdarg);
      }
    }
  }
}

// Reading bmpdata
  if (bmpWorking) {
    if (!(simActive && simEnable))
    {  
      pressure = round(bmp.pressure / 100.0)/10.0;
      last_alt = altitude;
      altitude = round(10 * (bmp.readAltitude(SEALEVELPRESSURE_HPA) - alt_offset))/10.0;
    }
    temperature = round(10 * bmp.temperature)/10.0;
  }

}
