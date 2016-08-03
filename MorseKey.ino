/* 
 Morse Key receiver - interprets morse code received from a key
 
 Quentin McDonald
 August 2016
 
 */
#include <SoftwareSerial.h>

// Pin definitions:
#define LED_ORANGE_PIN 6
#define LED_GREEN_PIN 5
#define BUZZER_PIN 10
#define KEY_IN_PIN 14



#define LED_OFF    0
#define LED_ORANGE 1
#define LED_GREEN  2

// Software serial is used for the LCD display
#define RX_PIN 2
#define TX_PIN 3
SoftwareSerial ss =  SoftwareSerial( RX_PIN, TX_PIN );

// 20x4 Serial Controlled LCD:
class SerialLCD {
public:
  SerialLCD(); 
  void displayScreen( char* theText );
  void displayLine(int lineNum, char *theText);
  void displayChar(int lineNum, int charNum, char theChar);
  void clear();
  void backlight(int Percentage);

private:
  int d_pin;

};

SerialLCD lcd;

// Sets the BiColor LED state
void setLED( int led_state );

void setup( ) {

  pinMode(LED_ORANGE_PIN, OUTPUT );
  pinMode(LED_GREEN_PIN, OUTPUT );
  setLED(LED_GREEN);

  lcd.displayLine( 1, "Morse Code Disp");
  lcd.displayLine( 2, "v1.0     ");
  delay(5000);
  ss.flush();
  lcd.clear();
}


void loop ( ) {


}


// Sets the BiColor LED state
void setLED( int led_state )
{
  switch( led_state ) {

  case LED_OFF:
    digitalWrite( LED_ORANGE_PIN, LOW );
    digitalWrite( LED_GREEN_PIN, LOW );
    break;

  case LED_ORANGE:
    digitalWrite( LED_ORANGE_PIN, HIGH );
    digitalWrite( LED_GREEN_PIN, LOW );
    break;

  case LED_GREEN:
    digitalWrite( LED_ORANGE_PIN, LOW );
    digitalWrite( LED_GREEN_PIN, HIGH );
    break;   


  default:
    break;
  } 


}


