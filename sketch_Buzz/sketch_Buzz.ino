#define BUZZPIN 8
#define LEDPIN 13

void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int d=1;
  for(int i=0;i<100;i++){
  digitalWrite(BUZZPIN, HIGH);
  digitalWrite(LEDPIN, HIGH);
  delay(d+i/50);
  digitalWrite(BUZZPIN, LOW);
  digitalWrite(LEDPIN, LOW);
  delay(d+i/50);
  }
  delay(100);
}
