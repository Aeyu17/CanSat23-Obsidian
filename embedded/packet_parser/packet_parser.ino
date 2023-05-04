String itemAt(String packet, int pos){
  // pos is 0 indexed
  int lastpos = -1;
  for (int i = 0; i < pos; i++){
    lastpos = packet.indexOf(',', lastpos + 1);
  }

  String item = packet.substring(lastpos + 1, packet.indexOf(',', lastpos + 1));
  return item;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

  String packets = "This,is,a,packet";
  Serial.println(itemAt(packets, 0));
  Serial.println(itemAt(packets, 1));
  Serial.println(itemAt(packets, 2));
  Serial.println(itemAt(packets, 3));
  Serial.println(itemAt(packets, 4));
  Serial.println(itemAt(packets, 5));
}
