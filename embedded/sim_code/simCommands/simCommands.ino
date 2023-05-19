void setup() {
  /*
    ACT,FL0 - flag is stowed
    ACT,FL1 - flag is raised
    ACT,RL0 - release mech is fully closed
    ACT,RL1 - first release from the container
    ACT,RL2 - second release of the parachute
    ACT,HS0 - heat shield is closed
    ACT,HS1 - hs in position 1, falling
    ACT,HS2 - hs in position 2, uprighting
    ACT,RES - set all mechanisms to position 0
  */
}

void loop() {
  else if (cmd == "ACT"){
    if (cmdarg == "MR\n") { // don't know if we still need
      Serial.println("ACTMR");
      cmdecho = "ACTMR";
      releaseContainer();
    } else if (cmdarg == "HS\n") { // don't know if we still need
      Serial.println("ACTHS");
      cmdecho = "ACTHS";
      upright();
    } else if (cmdarg == "PC\n") { // don't know if we still need
      Serial.println("ACTPC");
      cmdecho = "ACTPC";
      pc_deployed = 'C';
      releaseParachute();
    } else if (cmdarg == "AB\n") {
      Serial.println("ACTAB");
      cmdecho = "ACTAB";
      buzzer();
    } else if (cmdarg == "LED\n") {
      Serial.println("ACTLED");
      cmdecho = "ACTLED";
      ledBlink();
    } else if (cmdarg == "FL0\n") { // not done
      Serial.println("ACTFL0");
      cmdecho = "ACTFL0";
      flagControl(0);
    } else if (cmdarg == "FL1\n") { // not done
      Serial.println("ACTFL1");
      cmdecho = "ACTFL1";
      flagControl(1);
    } else if (cmdarg == "RL0\n") { // not done
      Serial.println("ACTRL0");
      cmdecho = "ACTRL0";
      releaseControl(0);
    } else if (cmdarg == "RL1\n") { // not done
      Serial.println("ACTRL1");
      cmdecho = "ACTRL1";
      releaseControl(1);
    } else if (cmdarg == "RL2\n") { // not done
      Serial.println("ACTRL2");
      cmdecho = "ACTRL2";
      releaseControl(2);
    } else if (cmdarg == "HS0\n") { // not done
      Serial.println("ACTHS0");
      cmdecho = "ACTHS0";
      panelControl(0);
    } else if (cmdarg == "HS1\n") { // not done
      Serial.println("ACTHS1");
      cmdecho = "ACTHS1";
      panelControl(1);
    } else if (cmdarg == "HS2\n") { // not done
      Serial.println("ACTHS2");
      cmdecho = "ACTHS2";
      panelControl(2);
    } else if (cmdarg == "RES\n") {
      Serial.println("ACTRES");
      cmdecho = "ACTRES";
      flagcontrol(0);
      releaseControl(0);
      panelControl(0);
    }
    else {
      Serial.println("Invalid command received.");
    }
  }
}

void flagControl(int num) {
  switch(num) {
    case 0: flagServo.write(0); // lower flag
    case 1: flagServo.write(180); // raise flag
  }
}

void releaseControl(int num) {
  switch(num) {
    case 0: releaseServo.write(0); break; // fully close release
    case 1: releaseServo.write(90); break; // first release
    case 2: releaseServo.write(180); break; // second release
    default: Serial.println("Error: Invalid Option");
  }
}

void panelControl(int num) {
  switch(num){
    case 0: // closing heat shield
            if (panelPosition == 0){
              Serial.println("Heat Shield is already closed");
              break;
            }
            panelServo.write(50); // make sure this is right
            delay(3000);
            panelServo.write(93);
            panelPosition = 0; break;
    case 1: // opening heat shield
            if (panelPosition == 1){
              Serial.println("Heat Shield is already falling");
              break;
            }
            panelServo.write(50); // make sure this is right
            delay(6000);
            panelServo.write(93);
            panelPosition = 1; break;
    case 2: // uprighting heat shield
            if (panelPosition == 2){
              Serial.println("Heat Shield is already uprighting");
              break;
            }
            panelServo.write(130); // make sure this is right
            delay(14000);
            panelServo.write(93);
            panelPosition = 2; break;
    default: Serial.println("Error: Invalid Option"); break;
  }
}
