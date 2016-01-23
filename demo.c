/* demo.c */


// demo
void demo_light(){
  int l=analogRead(lightPin);
  // Serial.print("L=");
  // Serial.println(l);
  analogWrite(ledR, l/4);
  // disp_light(l);
  print_sens(LightSensor_id, normLight(l));
  // print_sens(LightSensor_id, l);
}
void demo_temp(){
  signed int t=analogRead(tempPin);
  Serial.print("T=");
  Serial.print(analogRead(tempPin));
  Serial.print(" = ");
  Serial.println(normTemp(t));
  print_sens(TempSensor_id, normTemp(t));
}
void demo_time(){
  byte disp[4]={FNT_minus,FNT_minus&FNT_dot,FNT_minus,FNT_minus};
  char string[5];
  unsigned long t = millis()-start;
  // uint16_t t = start- (uint16_t)millis();
  // uint16_t t = now();
  Serial.println(t);
  sprintf(string, "%4d", t/1000);
  // if(t%100==0)timerINT();
  for(byte i=0;i<4;i++){
    if((string[i] >= '0')&&(string[i] <='9')) disp[i]=FNT_Digit[(byte)string[i]-'0'];
    else
      disp[i]=FNT_Digit[0];
  }
  display(disp);
}
void demo_alphabet(){
  for(byte i=0;i<sizeof(FNT_Alphabet);i++){
    D[0]=FNT_Alphabet[i];
    D[1]=((i>sizeof(FNT_Alphabet)+1)-1)&FNT_Alphabet[i+1];
    D[2]=((i>sizeof(FNT_Alphabet)+2)-1)&FNT_Alphabet[i+2];
    D[3]=((i>sizeof(FNT_Alphabet)+3)-1)&FNT_Alphabet[i+3];
    display(D);
    if(button_state==HIGH)return; // !!!: is it correct?
    delay(500);
  }
}
void demo_animation(){
  for(byte i=0;i<10;i++){
    // D[1]=FNT_Digit[i+1];
    D[2]=FNT_blank;
    animation_rotUp(~FNT_Digit[i],~FNT_Digit[(i+1)%10]);
    // animation_rotUp(~FNT_minus,~FNT_Digit[0]);
    // animation_rotUp(~FNT_Digit[i],~FNT_minus);
    D[0]=~animate.AB[0];
    // D[1]=~animation__shift_U(~D[0]);
    D[1]=~animate.AB[3];
    // D[1]=~animate.AB[1];
    // D[3]=~animate.AB[3];
    // D[2]=~animation__shift_D(~D[3]);
    // D[2]=~animate.AB[2];
    for (int i = 0; i < 4; ++i) {D[3]=~animate.AB[i]; display(D); delay(250);}
    display(D);
    delay(1000);
  }
}
