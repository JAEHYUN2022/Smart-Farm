#include "CO2Sensor.h"

CO2Sensor co2Sensor(A0, 0.99, 100);
int relay_fan = 2; //relay
int relay_peltier = 9; //relay
int relay_ptc = 10; //relay
int relay_soil = 11; //relay
int relay_co2 = 12; //relay
int relay_led = 13; //relay
int n = 2; // 작물 초기값 방울토마토로 지정
int soil, cdsValue, i, f, val;
int cdsValue_m, soil_m;
int cds, so; // cds, soil 매핑 변수
int Index, cX, cY;
int mode = 0;
int crops = 1;
int rx_cnt;  // 문자 개수를 세어서 rx_buffer의 배열 개수를 정해줌
int rx_complete; // 통신이 끝남을 알리기 위함
int rx_flag;  // 통신이 시작됬음을 알림

// 당근(0), 딸기(1), 방울토마토(2), 오이(3)
int TE_MAX[4] = {21, 20, 27, 28}; // 작물에 따른 온도 최대기준값
int TE_MIN[4] = {18, 10, 15, 25}; // 작물에 따른 온도 최소기준값
int HU_MAX[4] = {800, 900, 850, 890}; // 작물에 따른 토양 습도 최대기준값
int HU_MIN[4] = {400, 400, 400, 470}; // 작물에 따른 토양 습도 최소기준값
int CD[4] = {520, 500, 580, 510}; // 작물에 따른 조도 기준값
int CO_MAX[4] = {3000, 800, 800, 1500}; // 작물에 따른 CO2농도 최대기준값
int CO_MIN[4] = {1000, 300, 500, 800}; // 작물에 따른 CO2농도 최소기준값

char rx_dummy;  // 문자를 하나씩 받을 곳
char rx_buffer[13] ;  // rx_dummy 로부터 문자를 받아 총 최대13개 배열로 문자를 저장하고 있음
char rx_temp0[13]; //rx_buffer로부터 문자들을 그대로 받음
char rx_temp1[13]; // 밑에서 Index, cX, cY 등을 읽어내기위한 배열
char type = 'A';
char TEMP, water, led, san, check, check1, check2, choose;

unsigned long current_time = 0;
unsigned long previous_time = 0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);

  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  delay(300);
  digitalWrite(7, LOW);
  delay(300);
  digitalWrite(7, HIGH);
  delay(300);
  pinMode(A0, INPUT); // CO2 SENSOR DATA LINE
  pinMode(A1, INPUT); // SOIL HUMIDITY DATA LINE
  pinMode(A8, INPUT); // cdsValue DATA LINE
  pinMode(relay_soil, OUTPUT);
  pinMode(relay_co2, OUTPUT);
  pinMode(relay_led, OUTPUT);
  pinMode(relay_ptc, OUTPUT);
  pinMode(relay_peltier, OUTPUT);
  pinMode(relay_fan, OUTPUT);

  Serial2.write("$i,1,0,status.bmp#");
  Serial2.write("$i,1,1,setting.bmp#");
  Serial2.write("$i,1,2,backward.bmp#");
  Serial2.write("$i,1,3,temperature.bmp#");
  Serial2.write("$i,1,4,soilhumidity.bmp#");
  Serial2.write("$i,1,5,illumination.bmp#");
  Serial2.write("$i,1,6,co2.bmp#");
  Serial2.write("$i,1,7,selectcrops.bmp#");
  Serial2.write("$i,1,8,manual.bmp#");
  Serial2.write("$i,1,9,developer.bmp#");
  Serial2.write("$i,1,10,tomato.bmp#");
  Serial2.write("$i,1,11,carrot.bmp#");
  Serial2.write("$i,1,12,strawberry.bmp#");
  Serial2.write("$i,1,13,cucumber.bmp#");
  Serial2.write("$i,1,14,information.bmp#");
  Serial2.write("$i,1,15,connect.bmp#");
  Serial2.write("$i,1,16,peltier.bmp#");
  Serial2.write("$i,1,17,ptcheater.bmp#");
  Serial2.write("$i,1,18,temperature1.bmp#");
  Serial2.write("$i,1,19,soilhumidity1.bmp#");
  Serial2.write("$i,1,20,illumination1.bmp#");
  Serial2.write("$i,1,21,co21.bmp#");
  CLEAR_WHITESCREEN( );
  Serial2.write("$i,4,0,150,170,200,150#");
  Serial2.write("$i,4,1,450,170,200,150#");
  Serial2.write("$s, 6#");
  co2Sensor.calibrate();
}

void loop() {
  if (Serial1.available() > 0) {
    check = Serial1.read();
    if (check == 'A' || check == 'M') {
      type = check;  // 블루투스로 AUTO/MANUAL 선택.
    }
    else type = type;

    if (check == 'D') {
      n = 0; // 당근
    }
    else if (check == 'E') {
      n = 1; // 딸기
    }
    else if (check == 'F') {
      n = 2; // 방울토마토
    }
    else if (check == 'G') {
      n = 3; // 오이
    }
    else n = n;
  }

  if (Serial2.available() > 0) {
    if (check2 == 'D') {
      n = 0; // 당근
    }
    else if (check2 == 'E') {
      n = 1; // 딸기
    }
    else if (check2 == 'F') {
      n = 2; // 방울토마토
    }
    else if (check2 == 'G') {
      n = 3; // 오이
    }
    else n = n;
  }

  switch (type) {
    case 'A' :
      f = TEMP_AUTO(n);
      soil = SOLENOIDVALVE_AUTO(n);
      cdsValue = LED_AUTO(n);
      val = CO2_AUTO(n);
      break;
    case 'M' :
      f = TEMP_MANUAL();
      soil = SOLENOIDVALVE_MANUAL();
      cdsValue = LED_MANUAL();
      val = CO2_MANUAL();
      break;
  }

  cdsValue_m = 1023 - cdsValue;
  cds = map(cdsValue_m, 0, 1023, 0, 100);
  so = map(soil, 0 , 1015, 100, 0);

  check2 = TFT_LCD (f, cds, so, val, n, type);
}

// 온도 자동 제어
int TEMP_AUTO (int a) {

  if (Serial3.available() > 0) {
    f = Serial3.read();
  }

  Serial1.print("온도:");
  Serial1.println(f);
  Serial.println("온도1:");
  Serial.println(f);

  digitalWrite(relay_peltier, HIGH);
  digitalWrite(relay_fan, HIGH);

  if (f > TE_MAX[a]) {
    digitalWrite(relay_ptc, HIGH);
    digitalWrite(relay_peltier, LOW);
    digitalWrite(relay_fan, LOW);
  }
  else if (TE_MIN[a] < f && f  <= TE_MAX[a]) {
    digitalWrite(relay_ptc, HIGH);
    digitalWrite(relay_peltier, HIGH);
    digitalWrite(relay_fan, HIGH);
  }
  else {
    digitalWrite(relay_ptc, LOW);
    digitalWrite(relay_peltier, HIGH);
    digitalWrite(relay_fan, HIGH);
  }
  return f;
}

// WATER SOLENOID VALVE 자동 제어
int SOLENOIDVALVE_AUTO (int a) {

  soil = analogRead(A1);
  Serial.print("soil:");
  Serial.print(soil);
  Serial1.print("soil:");
  Serial1.println(soil);

  if (soil > HU_MAX[a]) {
    digitalWrite(relay_soil, LOW);
  }
  else if (soil < HU_MIN[a]) {
    digitalWrite(relay_soil, HIGH);
  }
  else {
    relay_soil = relay_soil;
  }
  return soil;
}

// LED BAR 자동 제어
int LED_AUTO (int a) {
  cdsValue = analogRead(A8);
  Serial.println("cdsValue:");
  Serial.println(cdsValue);
  Serial1.print("조도:");
  Serial1.println(cdsValue);

  digitalWrite(relay_led, LOW);

  if (cdsValue > CD[a]) {
    digitalWrite(relay_led, LOW);
  }
  else if (cdsValue < (CD[a] - 100)) {
    digitalWrite(relay_led, HIGH);
  }
  else {
    relay_led = relay_led;
  }
  return cdsValue;
}

// CO2 자동 제어
int CO2_AUTO (int a) {
  val = co2Sensor.read();
  Serial1.print("CO2:");
  Serial1.println(val);


  if (val < CO_MIN[a]) {
    digitalWrite(relay_co2, LOW);
  }
  else if (val > CO_MAX[a]) {
    digitalWrite(relay_co2, HIGH);
  }
  else {
    digitalWrite(relay_co2, HIGH);
  }

  return val;
}

// 온도 수동 제어
int TEMP_MANUAL () {

  if (Serial3.available() > 0) {
    f = Serial3.read();
  }

  Serial1.print("온도:");
  Serial1.println(f);
  TEMP = check;

  switch (TEMP) {
    case '0' : // PTC ON
      digitalWrite(relay_ptc, LOW);
      break;
    case '1' : // PTC OFF
      digitalWrite(relay_ptc, HIGH);
      break;
    case '2' : // PELTIER ON
      digitalWrite(relay_peltier, LOW);
      digitalWrite(relay_fan, LOW);
      break;
    case '3' : // PELTIER OFF
      digitalWrite(relay_peltier, HIGH);
      digitalWrite(relay_fan, HIGH);
      break;
  }

  return f;
}

// WATER SOLENOID VALVE 수동 제어
int SOLENOIDVALVE_MANUAL () {

  soil = analogRead(A1);
  Serial1.print("soil:");
  Serial1.println(soil);
  water = check;

  switch (water) {
    case '4' : // VALVE ON
      digitalWrite(relay_soil, LOW);
      break;
    case '5' : // VALVE OFF
      digitalWrite(relay_soil, HIGH);
      break;
  }

  return soil;
}

// LED BAR 수동 제어
int LED_MANUAL () {

  cdsValue = analogRead(A8);
  Serial1.print("조도:");
  Serial1.println(cdsValue);
  led = check;

  switch (led) {
    case '6' : // LED ON
      digitalWrite(relay_led, LOW);
      break;
    case '7' : // LED OFF
      digitalWrite(relay_led, HIGH);
      break;
  }

  return cdsValue;
}

// CO2 SOLENOID VALVE 수동 제어
int CO2_MANUAL() {
  val = co2Sensor.read();
  Serial1.print("CO2:");
  Serial1.println(val);
  san = check;

  switch (san) {
    case '8' : // VALVE ON
      digitalWrite(relay_co2, LOW);
      break;
    case '9' : // VALVE OFF
      digitalWrite(relay_co2, HIGH);
      break;
  }

  return val;
}

int TFT_LCD(int f, int cdsValue, int soil, int val, int k, char typ) {

  ////////////*UART 통신*///////////////
  if (Serial2.available() > 0)
  {
    rx_dummy = Serial2.read();

    if (rx_dummy == '$')
    {
      rx_flag = 1;
      rx_buffer[0] = rx_dummy;
      rx_cnt = 1;
    }
    else if (rx_dummy == '#')
    {
      rx_flag = 0;
      rx_buffer[rx_cnt] = rx_dummy;
      strcpy(rx_temp0, rx_buffer);
      rx_cnt = 0;
      rx_complete = 1;
    }
    else if (rx_flag == 1)
    {
      rx_buffer[rx_cnt] = rx_dummy;
      rx_cnt++;
    }
    if (rx_complete == 1)
    {
      rx_complete = 0;
      if (rx_temp0[3] == '0')
      {
        sscanf(rx_temp1, "$t,%d,%d,%d#", & Index, &cX, &cY);
        Serial.println(Index);
        Serial.println(cX);
        Serial.println(cY);
      }
      else
      {
        strcpy(rx_temp1, rx_temp0);
      }
    }
  }
  current_time = millis(); /////////////////////////////////////////////온도 토양습도 조도 co2 를 lcd에 1초마다 띄우기 위해 millis사용


  ////////CASE 문 시작////////////////////////////////////
  switch (mode) {
    ////////////////첫 화면에서 setting or status 누르기//////////////////////////////////
    case 0 :

      if (typ == 'A') {
        if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (crops == 2)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,10,20,150#");
          Serial2.write("$F,0,Cherry Tomato,30,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          mode = 1;
        }

        else if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (crops == 0)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,11,20,150#");
          Serial2.write("$F,0,Carrot,70,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          mode = 1;
        }

        else  if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (crops == 1)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,12,20,150#");
          Serial2.write("$F,0,Strawberry,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          mode = 1;
        }

        else  if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (crops == 3)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,13,20,150#");
          Serial2.write("$F,0,Cucumber,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          mode = 1;
        }
      }

      if (typ == 'M') {
        if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (k == 2)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,10,20,150#");
          Serial2.write("$F,0,Cherry Tomato,30,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          mode = 1;
        }

        else if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (k == 0)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,11,20,150#");
          Serial2.write("$F,0,Carrot,70,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          mode = 1;
        }

        else  if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (k == 1)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,12,20,150#");
          Serial2.write("$F,0,Strawberry,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          mode = 1;
        }

        else  if ((cX >= 150 && cX <= 350) && (cY >= 170 && cY <= 320) && (k == 3)) {
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$F,1,0,0,0#");
          Serial2.write("$i,2,13,20,150#");
          Serial2.write("$F,0,Cucumber,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          mode = 1;
        }
      }

      else  if ((cX >= 450 && cX <= 650) && (cY >= 170 && cY <= 320)) {
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,7,225,80#");
        Serial2.write("$i,2,8,225,180#");
        Serial2.write("$i,2,9,225,280#");
        Serial2.write("$s, 6#");
        mode = 2;
      }

      break;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    case 1 :

      if (typ == 'A') {
        if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {    ///////////////////////////////첫 화면으로 돌아가기
          mode = 0;
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,4,0,150,170,200,150#");
          Serial2.write("$i,4,1,450,170,200,150#");
          Serial2.write("$s, 6#");
        }

        else if ((current_time - previous_time > 1000) && (crops == 2)) { //////////////////////////////////////////////// 방울토마토 선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,10,20,150#");
          Serial2.write("$F,0,Cherry Tomato,30,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          previous_time = current_time;
        }

        else if ((current_time - previous_time > 1000) && (crops == 0)) {    //////////////////////////////////////////////// 당근선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,11,20,150#");
          Serial2.write("$F,0,Carrot,70,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          previous_time = current_time;
        }

        else if ((current_time - previous_time > 1000) && (crops == 1)) {   //////////////////////////////////////////////// 딸기 선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,12,20,150#");
          Serial2.write("$F,0,Strawberry,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          previous_time = current_time;
        }

        else if ((current_time - previous_time > 1000) && (crops == 3)) { //////////////////////////////////////////////// 오이 선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,13,20,150#");
          Serial2.write("$F,0,Cucumber,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          previous_time = current_time;
        }

        break;

      }

      if (typ == 'M') {
        if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {    ///////////////////////////////첫 화면으로 돌아가기
          mode = 0;
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,4,0,150,170,200,150#");
          Serial2.write("$i,4,1,450,170,200,150#");
          Serial2.write("$s, 6#");
        }

        else if ((current_time - previous_time > 1000) && (k == 2)) { //////////////////////////////////////////////// 방울토마토 선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,10,20,150#");
          Serial2.write("$F,0,Cherry Tomato,30,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          previous_time = current_time;
        }

        else if ((current_time - previous_time > 1000) && (k == 0)) {    //////////////////////////////////////////////// 당근선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,11,20,150#");
          Serial2.write("$F,0,Carrot,70,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          previous_time = current_time;
        }

        else if ((current_time - previous_time > 1000) && (k == 1)) {   //////////////////////////////////////////////// 딸기 선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,12,20,150#");
          Serial2.write("$F,0,Strawberry,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          previous_time = current_time;
        }

        else if ((current_time - previous_time > 1000) && (k == 3)) { //////////////////////////////////////////////// 오이 선택되었을때 status 창에서 센서값 보기
          CLEAR_WHITESCREEN( );
          Serial2.write("$i,2,2,0,0#");
          Serial2.write("$i,2,18,225,62#");                              ////////////////////temperature
          Serial2.write("$i,2,19,225,164#");                            /////////////////////soil humidity
          Serial2.write("$i,2,20,225,256#");                           /////////////////////////illumination
          Serial2.write("$i,2,21,225,354#");                          //////////////////////////co2
          Serial2.write("$F,11,0,0,0#");
          Serial2.write("$i,2,13,20,150#");
          Serial2.write("$F,0,Cucumber,50,334#");
          Serial2.print("$F,10," + String(f) + " 'C,600,60#");
          Serial2.print("$F,10," + String(soil) + " %,600,162#");
          Serial2.print("$F,10," + String(cdsValue) + " %,600,252#");
          Serial2.print("$F,10," + String(val) + "PPM,600,346#");
          Serial2.write("$s, 6#");
          crops = k;
          previous_time = current_time;
        }

        break;
      }

    case 2 :
      if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {   ///////////////////////////////첫 화면으로 돌아가기
        mode = 0;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,4,0,150,170,200,150#");
        Serial2.write("$i,4,1,450,170,200,150#");
        Serial2.write("$s, 6#");

      }

      else  if ((cX >= 225 && cX <= 549) && (cY >= 80 && cY <= 152)) {
        mode = 3;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,10,225,40#");
        Serial2.write("$i,2,11,500,40#");
        Serial2.write("$i,2,12,225,260#");
        Serial2.write("$i,2,13,500,260#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,Cherry Tomato,235,222#");
        Serial2.write("$F,0,Carrot,558,222#");
        Serial2.write("$F,0,Strawberry,260,440#");
        Serial2.write("$F,0,Cucumber,535,440#");
        Serial2.write("$s,6#");        //////////////////////////작물 선택 누르면 화면 넘어가기

      }

      else if ((cX >= 225 && cX <= 549) && (cY >= 180 && cY <= 252)) {
        mode = 4;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,14,100,120#");
        Serial2.write("$i,2,15,450,120#");
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,Information,100,330#");
        Serial2.write("$F,10,Connect,485,330#");
        Serial2.write("$s,6#");     ///////////////////////////manual 누르기
      }

      ////////////////////////Developer////////////////////////////////////////////
      else if ((cX >= 225 && cX <= 549) && (cY >= 280 && cY <= 352)) {
        mode = 5;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$F,1.0,0,0#");
        Serial2.write("$F,0,Rule,130,80#");
        Serial2.write("$F,0,Name,360,80#");
        Serial2.write("$F,0,Phone Number,560,80#");
        Serial2.write("$F,0,Team Leader,80,120#");
        Serial2.write("$F,0,Team Member,80,200#");
        Serial2.write("$F,0,Team Member,80,280#");
        Serial2.write("$F,0,Team Member,80,360#");
        Serial2.write("$F,0,Seong Yeon Chae,300,120#");
        Serial2.write("$F,0,Ye Joon Shin,300,200#");
        Serial2.write("$F,0,Cheol Hoon Jeong,300,280#");
        Serial2.write("$F,0,Jae Hyun Park,300,360#");
        Serial2.write("$F,0,010-2950-8242,560,120#");
        Serial2.write("$F,0,010-9320-0524,560,200#");
        Serial2.write("$F,0,010-4617-6166,560,280#");
        Serial2.write("$F,0,010-4919-6777,560,360#");
        Serial2.write("$s,6#");
      }
      break;

    /////////////////// 작물 선택화면/////////////////////////////////////////////////////////
    case 3 :
      if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {
        mode = 2;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,7,225,80#");
        Serial2.write("$i,2,8,225,180#");
        Serial2.write("$i,2,9,225,280#");
        Serial2.write("$s, 6#");               //////////뒤로가기 버튼
      }

      else if ((cX > 225 && cX < 409) && (cY >= 40 && cY <= 224)) { ////////////////////////////방울토마토 선택했을때
        mode = 8;
        cX = 0;
        cY = 0;
        CLEAR_WHITESCREEN();
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,Cherry Tomato is selected!!,120,210#");
        check1 = 'F';
        Serial2.write("$s,6#");
        crops = 2;
        return check1;
      }

      else if ((cX > 500 && cX < 684) && (cY >= 40 && cY <= 224)
              ) { //////////////////////////////////당근 선택했을 때
        mode = 9;
        cX = 0;
        cY = 0;
        CLEAR_WHITESCREEN();
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,Carrot is selected!!,200,210#");
        check1 = 'D' ;
        crops = 0;
        Serial2.write("$s,6#");

        return check1;
      }

      else if ((cX > 225 && cX < 409) && (cY >= 260 && cY <= 444)) {
        mode = 10;
        cX = 0;
        cY = 0;
        CLEAR_WHITESCREEN();
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,Strawberry is selected!!,180,210#");        ////////////////////////////////딸기 선택했을 때
        check1 = 'E';
        crops = 1;
        Serial2.write("$s,6#");
        return check1;
      }

      else if ((cX > 500 && cX < 684) && (cY >= 260 && cY <= 444)) {
        mode = 11;
        cX = 0;
        cY = 0;
        CLEAR_WHITESCREEN();
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,Cucumber is selected!!,180,210#");
        check1 = 'G';
        Serial2.write("$s,6#");
        crops = 3;       ///////////////////////////////////////////////////////오이 선택했을 때
        return check1;
      }
      break;

    /////////////////////////////manual 화면//////////////////////////////////////////
    case 4 :
      if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {
        mode = 2;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,7,225,80#");
        Serial2.write("$i,2,8,225,180#");
        Serial2.write("$i,2,9,225,280#");
        Serial2.write("$s, 6#");    //////////뒤로가기 버튼
      }

      else if ((cX >= 100 && cX <= 347) && (cY >= 120 && cY <= 338)) { //////////////////////////////////information 눌렀을때
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,3,20,100#");
        Serial2.write("$i,2,4,20,162#");
        Serial2.write("$i,2,5,20,224#");
        Serial2.write("$i,2,6,20,286#");
        Serial2.write("$i,2,16,20,348#");
        Serial2.write("$i,2,17,20,410#");
        Serial2.write("$s,6#");
        mode = 6;
      }

      else if ((cX >= 450 && cX <= 697) && (cY >= 120 && cY <= 338)) { //////////////////////////////connect 눌렀을때
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,HOW TO CONNECT?,200,20#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,1. Smart farms basically use two power sources.,20,100#");
        Serial2.write("$F,0,    one for SMPS and the other for MCU.,20,130#");
        Serial2.write("$F,0,2. Connect RX TX and GND to MCU for communication,20,160#");
        Serial2.write("$F,0,    between LCD and MCU.,20,190#");
        Serial2.write("$F,0,3. The RESET line of the LCD is connected to force reset of the LCD.,20,220#");
        Serial2.write("$s,6#");
        mode = 7;
      }
      break;

    ///////////////////////////////////Developer////////////////////////////////////////
    case 5:
      if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {
        mode = 2;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,7,225,80#");
        Serial2.write("$i,2,8,225,180#");
        Serial2.write("$i,2,9,225,280#");
        Serial2.write("$s, 6#");               //////////뒤로가기 버튼
      }
      break;
    case 6:
      if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {
        mode = 4;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,14,100,120#");
        Serial2.write("$i,2,15,450,120#");
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,Information,100,330#");
        Serial2.write("$F,10,Connect,485,330#");
        Serial2.write("$s, 6#");               //////////뒤로가기 버튼
      }

      else if ((cX > 20 && cX <= 231) && (cY >= 100 && cY <= 152)) {
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,3,20,100#");
        Serial2.write("$i,2,4,20,162#");
        Serial2.write("$i,2,5,20,224#");
        Serial2.write("$i,2,6,20,286#");
        Serial2.write("$i,2,16,20,348#");
        Serial2.write("$i,2,17,20,410#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,- Operating Voltage : 3V ~ 5V,250,160#");
        Serial2.write("$F,0,- Sensor Model : Ozon AM2302 temperature, 250,190#");
        Serial2.write("$F,0,- Sensor Signal output format : Digital Signal, 250,220#");
        Serial2.write("$F,0,- Temperature range : - 40℃ ~ 80℃, 250,250#");
        Serial2.write("$F,0,- Temperature accuracy : 0.5'C, 250,280#");
        Serial2.write("$s,6#");
      }

      else if ((cX > 20 && cX <= 231) && (cY >= 162 && cY <= 214)) {
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,3,20,100#");
        Serial2.write("$i,2,4,20,162#");
        Serial2.write("$i,2,5,20,224#");
        Serial2.write("$i,2,6,20,286#");
        Serial2.write("$i,2,16,20,348#");
        Serial2.write("$i,2,17,20,410#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,- This is a simple moisture sensor can be used to ,250,160#");
        Serial2.write("$F,0,  detect the moisture of the soil,250,190#");
        Serial2.write("$F,0,- Operating Voltage : 3.3V-5V ,250,220#");
        Serial2.write("$s,6#");
      }

      else if ((cX > 20 && cX <= 231) && (cY >= 224 && cY <= 276)) {
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,3,20,100#");
        Serial2.write("$i,2,4,20,162#");
        Serial2.write("$i,2,5,20,224#");
        Serial2.write("$i,2,6,20,286#");
        Serial2.write("$i,2,16,20,348#");
        Serial2.write("$i,2,17,20,410#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,- Detecting light intensity and light intensity ,250,130#");
        Serial2.write("$F,0,  sensors smart car hunt optical module, 250,160#");
        Serial2.write("$F,0,- Using sensitive photoresistor type sensor, 250,190#");
        Serial2.write("$F,0,- The comparator output signal clean and good, 250,220#");
        Serial2.write("$F,0,  waveform and driving ability over 15mA, 250,250#");
        Serial2.write("$F,0,- with adjustable potentiometer can be adjusted, 250,280#");
        Serial2.write("$F,0,  to detect light intensity, 250,310#");
        Serial2.write("$F,0,- The operating voltage : 5V 3.3V, 250,340#");
        Serial2.write("$F,0,  to detect light intensity, 250,370#");
        Serial2.write("$s,6#");
      }

      else if ((cX > 20 && cX <= 231) && (cY >= 286 && cY <= 338)) {
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,3,20,100#");
        Serial2.write("$i,2,4,20,162#");
        Serial2.write("$i,2,5,20,224#");
        Serial2.write("$i,2,6,20,286#");
        Serial2.write("$i,2,16,20,348#");
        Serial2.write("$i,2,17,20,410#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,- Operating voltage : 5V ,250,130#");
        Serial2.write("$F,0,- Interface : Analog, 250,160#");
        Serial2.write("$F,0,- One digital output, 250,190#");
        Serial2.write("$F,0,- High quality connector, 250,220#");
        Serial2.write("$F,0,- Immersion gold surface,250,250#");
        Serial2.write("$F,0,- Onboard heating circuit, 250,280#");
        Serial2.write("$s,6#");
      }

      else if ((cX > 20 && cX <= 231) && (cY >= 348 && cY <= 400)) {
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,3,20,100#");
        Serial2.write("$i,2,4,20,162#");
        Serial2.write("$i,2,5,20,224#");
        Serial2.write("$i,2,6,20,286#");
        Serial2.write("$i,2,16,20,348#");
        Serial2.write("$i,2,17,20,410#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,- Operating Voltage : 12V ,250,160#");
        Serial2.write("$F,0,- Maximum Temperature : 230'C, 250,190#");
        Serial2.write("$F,0,- Error : 10'C, 250,220#");
        Serial2.write("$F,0,- Nonpolar Nature, 250,250#");
        Serial2.write("$s,6#");
      }

      else if ((cX > 20 && cX <= 231) && (cY >= 410 && cY <= 462)) {
        CLEAR_WHITESCREEN();
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,3,20,100#");
        Serial2.write("$i,2,4,20,162#");
        Serial2.write("$i,2,5,20,224#");
        Serial2.write("$i,2,6,20,286#");
        Serial2.write("$i,2,16,20,348#");
        Serial2.write("$i,2,17,20,410#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,- Operating Voltage : 12V ,250,160#");
        Serial2.write("$F,0,- Maximum Temperature Difference : 60'C, 250,190#");
        Serial2.write("$s,6#");

      }
      break;
    case 7 :
      if ((cX > 0 && cX <= 155) && (cY >= 20 && cY <= 80)) {
        mode = 4;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,14,100,120#");
        Serial2.write("$i,2,15,450,120#");
        Serial2.write("$F,11,0,0,0#");
        Serial2.write("$F,10,Information,100,330#");
        Serial2.write("$F,10,Connect,485,330#");
        Serial2.write("$s, 6#");               //////////뒤로가기 버튼

      }
      break;
    case 8:
      if (cX > 0 && cY > 0) {
        mode = 3;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,10,225,40#");
        Serial2.write("$i,2,11,500,40#");
        Serial2.write("$i,2,12,225,260#");
        Serial2.write("$i,2,13,500,260#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,Cherry Tomato,235,222#");
        Serial2.write("$F,0,Carrot,558,222#");
        Serial2.write("$F,0,Strawberry,260,440#");
        Serial2.write("$F,0,Cucumber,535,440#");
        Serial2.write("$s,6#");
      }
      break;
    case 9:
      if (cX > 0 && cY > 0) {
        mode = 3;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,10,225,40#");
        Serial2.write("$i,2,11,500,40#");
        Serial2.write("$i,2,12,225,260#");
        Serial2.write("$i,2,13,500,260#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,Cherry Tomato,235,222#");
        Serial2.write("$F,0,Carrot,558,222#");
        Serial2.write("$F,0,Strawberry,260,440#");
        Serial2.write("$F,0,Cucumber,535,440#");
        Serial2.write("$s,6#");

      }
      break;
    case 10:
      if (cX > 0 && cY > 0) {
        mode = 3;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,10,225,40#");
        Serial2.write("$i,2,11,500,40#");
        Serial2.write("$i,2,12,225,260#");
        Serial2.write("$i,2,13,500,260#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,Cherry Tomato,235,222#");
        Serial2.write("$F,0,Carrot,558,222#");
        Serial2.write("$F,0,Strawberry,260,440#");
        Serial2.write("$F,0,Cucumber,535,440#");
        Serial2.write("$s,6#");        //////////////////////////작물 선택 누르면 화면 넘어가기

      }
      break;
    case 11:
      if (cX > 0 && cY > 0) {
        mode = 3;
        CLEAR_WHITESCREEN( );
        Serial2.write("$i,2,2,0,0#");
        Serial2.write("$i,2,10,225,40#");
        Serial2.write("$i,2,11,500,40#");
        Serial2.write("$i,2,12,225,260#");
        Serial2.write("$i,2,13,500,260#");
        Serial2.write("$F,1,0,0,0#");
        Serial2.write("$F,0,Cherry Tomato,235,222#");
        Serial2.write("$F,0,Carrot,558,222#");
        Serial2.write("$F,0,Strawberry,260,440#");
        Serial2.write("$F,0,Cucumber,535,440#");
        Serial2.write("$s,6#");        //////////////////////////작물 선택 누르면 화면 넘어가기

      }
      break;
  }
  cX = 0;
  cY = 0;
}

void CLEAR_WHITESCREEN( ) {
  Serial2.write("$s,3#");
  Serial2.write("$l,0,255,255,255#");
  Serial2.write("$l,2,0,0,800,480,1#");
}
