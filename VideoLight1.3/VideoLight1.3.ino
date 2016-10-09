//Video Light 1.3
//Code by iDexe

#include <U8glib.h>
#include <PinChangeInterrupt.h>
#include <avr/sleep.h>
#include <Wire.h>
#include <Encoder.h>

//Input and Output
#define BUTTON1 8
#define BUTTON_ON 7
#define DT 2
#define CLK 3

#define COLD 9
#define WARM 10

#define OLED_RESET 4


//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);// for u8g2lib


class Button
{
  public:
    Button(uint8_t pin);
    void begin(bool pull);
    bool pressed();
    bool hold(int timer);
    bool status();
  private:
    bool _oldvalue;
    uint8_t _pin;
    unsigned long _time;
};

class Light
{
  public:
    Light(int pinWarm, int pinCold);
    void begin();
    void brightness(int Int);
    void color(int color);
    void insane(int Int);
  private:
    int _warm;
    int _cold;
    int _int;
    int _color;
    int _superInt;
};

class Screen
{
  public:
    Screen();
    void reset();
    void standby();
    void setBigText(String BigText, bool def);
    void setBattery(int Bat, bool def);
    void setBottom(String Bottom, bool def);
    void setCaption(String Caption, bool def);
    void update();
    void renderChanges();
  private:
    bool _valueChange;
    String _BigText;
    bool _showBigText;
    int _Bat;
    bool _showBat;
    String _Bottom;
    bool _showBottom;
    String _Caption;
    bool _showCaption;
};
Encoder knob(DT, CLK);
class RotEnc
{
  public:
    RotEnc();
    bool check(int lbound, int rbound);
    int getPos();
    void setPos(int lbound, int rbound, int pos);
  private:
    int _pos;
};

class Thermistor
{
  public:
    Thermistor(int analogPin);
    double read(int interval);
  private:
    int _analogPin;
    unsigned long _Time;
    double _Temp;
};

class Voltage
{
  public:
    Voltage(int analogPin);
    double read(int interval);
  private:
    int _analogPin;
    unsigned long _Time;
    double _Volt;
};




RotEnc rotenc;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);
Screen screen;
Button button1(BUTTON1);
Button buttonOn(BUTTON_ON);
Light light(WARM, COLD);
Thermistor therm1(0);
Thermistor therm2(1);
//Thermistor therm3(2);
Voltage volt1(3);

void setup() {
  // put your setup code here, to run once:
  TCCR1B = (TCCR1B & 0b11111000) | 0x01;
  light.begin();
  Serial.begin(9600);
  button1.begin(true);
  buttonOn.begin(true);

}
enum prog_state {STANDBY, ON};
prog_state Prog = ON;
enum state {TMP, INT, SUPER_INT};
state State = TMP;
bool superint = false;
bool stateChange = true;
bool progChange = false;
long Tmp = 5000;
long Int = 50;
long Super_Int = 50;



void loop() {
  switch (Prog) {
    case ON: {
        switch (State) {
          case TMP: {//Farbtemperatur einstellen
              if (stateChange) {//Beim ersten ausführen
                screen.setBigText(String(Tmp) + "K", true);
                screen.setCaption("Temperatur", true);
                screen.update();
                rotenc.setPos(-10, 10, (Tmp - 5000) / 100);
                stateChange = false;
              }

              if (rotenc.check(-10, 10)) { //Wenn jemand dreht
                Tmp = rotenc.getPos() * 100 + 5000;
                screen.setBigText(String(Tmp) + "K", true);
                light.color((Tmp - 4000) / 100 * 255 / 20);
              }

              if (button1.pressed()) {//Wenn jemand drückt
                if (superint) {
                  State = SUPER_INT;
                } else {
                  State = INT;
                }

                stateChange = true;
              }
              break;
            }
          case INT: {//Helligkeit einstellen
              if (stateChange) {//Beim ersten ausführen
                screen.setBigText(String(Int) + "%", true);
                screen.setCaption("Helligkeit", true);
                screen.update();
                rotenc.setPos(-10, 10, (Int - 50) / 5);
                stateChange = false;
              }

              if (rotenc.check(-10, 10)) { //Wenn jemand dreht
                Int = rotenc.getPos() * 5 + 50;
                screen.setBigText(String(Int) + "%", true);
                light.brightness(Int / 5 * 255 / 20);
              }

              if (button1.pressed()) {//Wenn jemand drückt
                State = TMP;
                stateChange = true;
              }
              if (button1.hold(1000)) {
                State = SUPER_INT;
                superint = true;
                stateChange = true;
              }
              break;
            }
          case SUPER_INT: {
              if (stateChange) {//Beim ersten ausführen
                screen.setBigText(String(Super_Int) + "%", true);
                screen.setCaption("Insane", true);
                screen.update();
                rotenc.setPos(-10, 10, (Super_Int - 50) / 5);
                stateChange = false;
              }

              if (rotenc.check(-10, 10)) { //Wenn jemand dreht
                Super_Int = rotenc.getPos() * 5 + 50;
                screen.setBigText(String(Super_Int) + "%", true);
                light.insane(Super_Int / 5 * 255 / 20);
              }

              if (button1.pressed()) {//Wenn jemand drückt
                State = TMP;
                stateChange = true;
              }
              if (button1.hold(1000)) {
                State = INT;
                superint = false;
                stateChange = true;
              }
              break;
            }
        }

        //Allways running Code, like screen rendering, temperature and battery monitoring.

        //2 Temp sensor configuration
        char voltstr[5];
        dtostrf(volt1.read(1000) * 16.8 / 810,4,1,voltstr);
        screen.setBottom("LED:" + String(int(therm1.read(1000))) + (char)176 + "C BAT:" + String(int(therm2.read(1000))) + (char)176 + "C " + String(voltstr) + "V", true);

        //3 Temp sensor configuration
        //screen.setBottom("LED:" + String(int(therm1.read(1000))) + (char)176 + "C BAT:" + String(int(therm2.read(1000))) + (char)176 + "C " + String(int(therm3.read(1000))) + (char)176 + "C", true);

        screen.renderChanges();

        if (buttonOn.pressed() && millis() > 1000) {
          //Prog = STANDBY;
          //progChange = true;
          if (superint) {
            superint = false;
            State = INT;
          } else {
            superint = true;
            State = SUPER_INT;
          }
          stateChange = true;
        }
        /*
        if (buttonOn.hold(1000)) {
          Prog = STANDBY;
          progChange = true;
        }
        */
        break;
      }
    case STANDBY: {
        if (millis() <= 1500) {
          Prog = ON;
          stateChange = true;
          break;
        }
        if (progChange) {
          screen.standby();
          screen.renderChanges();
          progChange = false;
        } else {
          if (buttonOn.hold(2000)) {
            Prog = ON;
            stateChange = true;
          }
          //delay(1000);
          //sleepNow();
        }
      }
  }



}


//Button-methods

Button::Button(uint8_t pin) {
  _pin = pin;
  _oldvalue = true;
}

void Button::begin(bool pull) {
  if (pull) {
    pinMode(_pin, INPUT_PULLUP);
  } else {
    pinMode(_pin, INPUT);
  }

}

bool Button::pressed() {
  bool value = digitalRead(_pin);
  if (_oldvalue != value && value == false) {
    _oldvalue = value;
    return true;
  } else {
    _oldvalue = value;
    return false;
  }
}

bool Button::status() {
  return !digitalRead(_pin);
}

bool Button::hold(int timer) {
  if (pressed()) {
    _time = millis();
  }
  if (status()) {
    if (millis() - _time >= timer && millis() - _time <= timer + 100) {
      return true;
    }
  }
  return false;
}

//Light-methods

Light::Light(int pinCold, int pinWarm) {
  _cold = pinCold;
  _warm = pinWarm;
  _int = 128;
  _color = 128;
}

void Light::begin() {
  pinMode(_cold, OUTPUT);
  pinMode(_warm, OUTPUT);
  analogWrite(_warm, _int * _color / 255);
  analogWrite(_cold, _int * (255 - _color) / 255);
}

void Light::color(int color) {
  //Color mix between 0 (cold) and 255 (warm)
  _color = color;
  analogWrite(_warm, _int * _color / 255);
  analogWrite(_cold, _int * (255 - _color) / 255);
}

void Light::brightness(int Int) {
  //Brightness between 0 (dark) and 255 (bright)
  _int = Int;
  analogWrite(_warm, _int * _color / 255);
  analogWrite(_cold, _int * (255 - _color) / 255);
}

void Light::insane(int Int) {
  //Brightness between 0 (dark) and 510 (very bright)
  _superInt = Int;
  int warm = 2* _superInt * _color / 255;
  if (warm > 255) {
    warm = 255;
  }
  int cold = 2* _superInt * (255 - _color) / 255;
  if (cold > 255) {
    cold = 255;
  }
  analogWrite(_warm, warm);
  analogWrite(_cold, cold);
  
}

//Screen-methods

Screen::Screen() {
#ifndef u8glib_h
#define u8glib_h
#include <u8glib.h>
#endif
  //U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);// for u8g2lib
  //U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);
  _valueChange = true;
  _showBigText = false;
  _showBat = false;
  _showBottom = false;
  _showCaption = false;

}

void Screen::reset() {
  _valueChange = false;
}

void Screen::standby() {
  _valueChange = true;
  _showBigText = false;
  _showBat = false;
  _showBottom = false;
  _showCaption = false;

}

void Screen::setBigText(String BigText, bool def) {
  _showBigText = def;
  if (_BigText != BigText) {
    _BigText = BigText;
    _valueChange = true;
  }

}

void Screen::setBattery(int Bat, bool def) {
  _showBat = def;
  if (_Bat != Bat) {
    _Bat = Bat;
    _valueChange = true;
  }
}

void Screen::setBottom(String Bottom, bool def) {
  _showBottom = def;
  if (_Bottom != Bottom) {
    _Bottom = Bottom;
    _valueChange = true;
  }
}

void Screen::setCaption(String Caption, bool def) {
  _showCaption = def;
  if (_Caption != Caption) {
    _Caption = Caption;
    _valueChange = true;
  }
}

void Screen::update() {
  _valueChange = true;
}

void Screen::renderChanges() {
  if (_valueChange == true) {
    u8g.firstPage();
    do {
      //BigText
      if (_showBigText) {
        u8g.setFont(u8g_font_helvB24r);
        u8g.setFontPosCenter();
        char BigText[_BigText.length() + 1];
        _BigText.toCharArray(BigText, _BigText.length() + 1);
        u8g.setPrintPos((128 - u8g.getStrWidth(BigText)) / 2, 35);
        u8g.print(_BigText);
      }

      //Battery

      //Bottom
      if (_showBottom) {
        u8g.setFont(u8g_font_helvB08);
        u8g.setFontPosBaseline();
        u8g.setPrintPos(0, 64);
        u8g.print(_Bottom);
      }

      //Caption
      if (_showCaption) {
        u8g.setFont(u8g_font_helvB08);
        u8g.setFontPosTop();
        char Caption[_Caption.length() + 1];
        _Caption.toCharArray(Caption, _Caption.length() + 1);
        u8g.setPrintPos((128 - u8g.getStrWidth(Caption)) / 2, 0);
        u8g.print(_Caption);
      }

    } while ( u8g.nextPage() );
    reset();
  }
}

//RotEnc-Methods


RotEnc::RotEnc() {
  _pos = -999;
}

bool RotEnc::check(int lbound, int rbound) {
  int pos = knob.read() / 4;
  if (pos != _pos) {
    if (pos < lbound) {
      pos = lbound;
      knob.write(pos * 4);
    }
    if (pos > rbound) {
      pos = rbound;
      knob.write(pos * 4);
    }
    if (pos != _pos) {
      _pos = pos;
      return true;
    } else {
      return false;
    }
  }

}

int RotEnc::getPos() {
  return _pos;
}

void RotEnc::setPos(int lbound, int rbound, int pos) {
  if (pos < lbound) {
    pos = lbound;
  }
  if (pos > rbound) {
    pos = rbound;
  }
  _pos = pos;
  knob.write(_pos * 4);
}

//Thermistor-Methods

Thermistor::Thermistor(int analogPin) {
  _analogPin = analogPin;
  _Time = 0;
}

double Thermistor::read(int interval) {
  unsigned long Time = millis();
  if (_Time + interval < Time) {
    _Temp = analogRead(_analogPin);
    _Temp = log(10000.0 * ((1024.0 / _Temp - 1)));
    _Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * _Temp * _Temp )) * _Temp );
    _Temp = _Temp - 273.15;            // Convert Kelvin to Celcius
    _Time = Time;
    return _Temp;
  }
  return _Temp;
}

//Voltage-Methods

Voltage::Voltage(int analogPin) {
  _analogPin = analogPin;
  _Time = 0;
}

double Voltage::read(int interval) {
  unsigned long Time = millis();
  if (_Time + interval < Time) {
    _Volt = analogRead(_analogPin);
    _Time = Time;
    return _Volt;
  }
  return _Volt;
}



void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  //attachInterrupt(0,wakeUpNow,LOW);
  attachPinChangeInterrupt(digitalPinToPCINT(BUTTON_ON), wakeUpNow, FALLING);
  sleep_mode();
  sleep_disable();
  //detachInterrupt(0);
  detachPinChangeInterrupt(digitalPinToPCINT(BUTTON_ON));
}

void wakeUpNow() {

}


