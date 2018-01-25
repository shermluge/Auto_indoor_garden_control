// vi:ts=4


#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

hd44780_I2Cexp lcd; // declare lcd object: auto locate & config exapander chip

byte smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

byte full[8] = {
  B10101,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

byte medium[8] = {
  B00000,
  B00000,
  B00000,
  B10101,
  B11111,
  B11111,
  B11111,
};

byte low[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10101,
};

//#define DEBUG

// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

//sensors:
int pirsensor = 6;
int waterlevelsensor1 = 7;
int waterlevelsensor2 = 8;
int leftbutton = 9;
int rightbutton = 10;
int lcdbacklight = 11;
int pump1 = 3;
int pump2 = 5;
int soilsensor1 = A0;
int soilsensor2 = A1;
int soilsensor3 = A2;
int soilsensor4 = A3;

int soilsensor1state = 0;
int soilsensor2state = 0;
int soilsensor3state = 0;
int soilsensor4state = 0;
int readwaterlevelsensor1 = 1;
int prevreadwaterlevelstate1 = 1;
int readwaterlevelsensor2 = 1;
int prevreadwaterlevelstate2 = 1;
int leftbuttonstate = 0;
int rightbuttonstate = 0;
int pump1state = 0;
int pump2state = 0;


int pirsensorstate = 0;
int buttoncount = 0;
int menustate = 0;
int menupumpmanual = 0;
int mainwindow = HIGH;
int waterlevel = 0;
unsigned long windowholdtime = 1000;
unsigned long pumpmaxtime = 9000;
unsigned long ledontime = 60000;
int pump1countreset = 30;
int pump1count = 0;
int pump2countreset = 30;
int pump2count = 0;
unsigned long prevmillis = 0;
unsigned long prevmilliswindow = 0;
unsigned long prevmillisled = 0;


void setup()
{
int status;
  
	status = lcd.begin(LCD_COLS, LCD_ROWS);
	if(status) // non zero status means it was unsuccesful
	{
		status = -status; // convert negative status value to positive number

		// begin() failed so blink error code using the onboard LED if possible
		hd44780::fatalError(status); // does not return
	}
  lcd.createChar(2, full);
  lcd.createChar(1, medium);
  lcd.createChar(0, low);
  lcd.print(char(0));
 pinMode(lcdbacklight,OUTPUT);
 pinMode(pump1,OUTPUT);
 pinMode(pump2,OUTPUT);
 pinMode(soilsensor1,INPUT);
 pinMode(soilsensor2,INPUT);
 pinMode(soilsensor3,INPUT);
 pinMode(soilsensor4,INPUT);
 pinMode(leftbutton,INPUT);
 pinMode(rightbutton,INPUT);
 pinMode(pirsensor,INPUT);
 pinMode(waterlevelsensor1,INPUT);
 pinMode(waterlevelsensor2,INPUT);
 digitalWrite(pump1,LOW);
 digitalWrite(pump2,LOW);
}

void loop() {  
    //lcd.print("Hello, World!");
    readwaterlevelsensor1 = digitalRead(waterlevelsensor1);
    readwaterlevelsensor2 = digitalRead(waterlevelsensor2);

    leftbuttonstate = digitalRead(leftbutton);
    rightbuttonstate = digitalRead(rightbutton);
    pirsensorstate = digitalRead(pirsensor);
    
    unsigned long currentMillis = millis();

    if(pirsensorstate == HIGH){
      digitalWrite(lcdbacklight,LOW);// low is on    
    }else{
      if (currentMillis - prevmillisled >= ledontime){ // && menupumpmanual > 0 ) {
        prevmillisled = currentMillis;
        digitalWrite(lcdbacklight,HIGH);//High is off
      }
    }

    
    if (currentMillis - prevmillis >= pumpmaxtime){ // && menupumpmanual > 0 ) {
      // Save next check state time:
      prevmillis = currentMillis;
      digitalWrite(pump1,LOW);
      digitalWrite(pump2,LOW);
      #ifdef DEBUG
      Serial.print("\n current: ");
      Serial.print(currentMillis);
      Serial.print("\n");
      #endif
    }
    

    
    if (currentMillis - prevmilliswindow >= windowholdtime && buttoncount==0) {
      // Save next check state time:
      prevmilliswindow = currentMillis;
      for(int i = 1;i<8;i++){
        soilsensor1state = soilsensor1state + analogRead(soilsensor1);
        soilsensor2state = soilsensor2state + analogRead(soilsensor2);
        soilsensor3state = soilsensor3state + analogRead(soilsensor3);
        soilsensor4state = soilsensor4state + analogRead(soilsensor4);        
      }
      soilsensor1state = soilsensor1state/7;
      soilsensor2state = soilsensor2state/7;
      soilsensor3state = soilsensor3state/7;
      soilsensor4state = soilsensor4state/7;


      if(mainwindow){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(soilsensor1state);
        lcd.print(" ");
        lcd.print(soilsensor2state);
        lcd.print(" ");
        lcd.print(soilsensor3state);
        lcd.print(" ");
        lcd.print(soilsensor4state);
        lcd.setCursor(0, 1);
        lcd.print(digitalRead(pirsensor)); 
        lcd.print(" ");
        lcd.print(waterlevel);
        lcd.print(" ");
        lcd.print(pump1state);
        lcd.print(" ");
        lcd.print(pump2state);
        lcd.print(" ");
        lcd.print(pump1count);
        lcd.print(" ");
        lcd.print(pump2count);
        delay(10);
        //digitalWrite(pump1,LOW);
        //digitalWrite(pump2,LOW);
        
        if(soilsensor1state < 900 && soilsensor2state < 900){
          pump1state = HIGH;
          pump1count++;
        }
        if(pump1count >= pump1countreset && pump1state == HIGH){
          digitalWrite(pump1,HIGH);          
          pump1count = 0;
          pump1state = LOW;
        }
        
        if(soilsensor3state < 900 && soilsensor4state < 900){
          pump2state = HIGH;
          pump2count++;
        }
        if(pump2count >= pump2countreset && pump2state == HIGH){
          digitalWrite(pump2,HIGH);          
          pump2count = 0;
          pump2state = LOW;
        }
        if(pump1count>=5){
          digitalWrite(pump1,LOW);
        }
        if(pump2count>=5){
          digitalWrite(pump2,LOW);
        }
        
        soilsensor1state = 0;
        soilsensor2state = 0;
        soilsensor3state = 0;
        soilsensor4state = 0; 
      }

      //water level
      lcd.setCursor(15, 1); // bottom right
      if(readwaterlevelsensor1==0){
        waterlevel=1;
      }else{
        waterlevel=0;
      }
      if(readwaterlevelsensor2==0){
        waterlevel=2;
      }
      lcd.print(char(waterlevel));
      //lcd.setCursor(13,1);
      //lcd.print(readwaterlevelsensor1);
      //lcd.setCursor(14,1);
      //lcd.print(readwaterlevelsensor2);
            
      #ifdef DEBUG
      Serial.print("\n current: ");
      Serial.print(currentMillis);
      Serial.print("\n");
      #endif
    }
    

    if(leftbuttonstate==HIGH){
      digitalWrite(pump1,LOW);
      digitalWrite(pump2,LOW);
      buttoncount++;
      if(buttoncount==1){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Pump 1 Man Water");
        lcd.setCursor(0,1);
        lcd.print("Press Okay");
        menupumpmanual = 1;
      }
      if(buttoncount==2){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Pump 2 Man Water");
        lcd.setCursor(0,1);
        lcd.print("Press Okay");
        menupumpmanual = 2;
      }
      if(buttoncount==3){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("No manual pump");
        lcd.setCursor(0,1);
        lcd.print("Press OKAY");
        menupumpmanual=0;
        buttoncount=0;
      }
      delay(500);
      leftbuttonstate = LOW;
    }

    if(rightbuttonstate==HIGH){
      digitalWrite(pump1,LOW);
      digitalWrite(pump2,LOW);
      if(menupumpmanual==1){
        digitalWrite(pump1,HIGH);
        //prevmillis = millis();
      }
      if(menupumpmanual==2){
        digitalWrite(pump2,HIGH);
        //prevmillis = millis();
      }
      if(menupumpmanual==0){
        mainwindow=HIGH;      
      }else{
        mainwindow=LOW;
      }
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("OKAY was pressed");
      delay(500);
      prevmillis = millis();
    }
    

}
