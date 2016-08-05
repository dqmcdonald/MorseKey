/*
  Morse Key receiver - interprets morse code received from a key

  Quentin McDonald
  August 2016

*/


#include <SoftwareSerial.h>
#include <Bounce2.h>

// Pin definitions:
#define RX_PIN           2
#define TX_PIN           3 // LCD Screen
#define RESET_PIN        4 // Reset Switch (N.O.)
#define LED_ORANGE_PIN   6 // BiColor LED
#define LED_GREEN_PIN    5 // BiColor LED
#define BUZZER_PIN      10 // Piezo Buzzer
#define KEY_IN_PIN      14 // Morse Key (N.C.)

#define TONE_FREQUENCY 120 // The fequency of the buzzer tone
#define DOT_CUTOFF     175 // Pulses below this length are dots
#define LED_OFF    0
#define LED_ORANGE 1
#define LED_GREEN  2

#define DOT 0
#define DASH 1

#define MAX_PULSES  4  // No code has more than 4 pulses

#define  A_CODE  ((DOT<<0 ) | (DASH << 1 ) )
#define  A_LEN 2

#define  B_CODE  ((DASH<<0 ) | (DOT << 1 ) | (DOT << 2) | (DOT << 3 ) )
#define  B_LEN 4

#define  C_CODE  ((DASH<<0 ) | (DOT << 1 ) | (DASH << 2) | (DOT << 3 ) )
#define  C_LEN 4

#define D_CODE ((DASH<<0 ) | (DOT << 1 ) | (DOT << 2) )
#define D_LEN 3

#define E_CODE ((DOT<<0 ))
#define E_LEN 1

#define  O_CODE  ((DASH<<0) | (DASH << 1 ) | (DASH<<2))
#define O_LEN 3




// 20x2 Serial Controlled LCD:
class SerialLCD {
  public:
    SerialLCD();
    void displayScreen( const char* theText );
    void displayLine(int lineNum, const char *theText);
    void displayChar(int lineNum, int charNum, const char theChar);
    void clear();
    void backlight(int Percentage);

  private:
    int d_pin;

};

//*******************************************************
// Variables:
//*******************************************************
// Software serial is used for the LCD display
SoftwareSerial ss =  SoftwareSerial( RX_PIN, TX_PIN );

SerialLCD lcd;

int num_pulses = 0;
int num_chars = 0;
const int MAX_CHARS = 16;

char words[MAX_CHARS];
char dots_dashes[MAX_CHARS];

int pulse_bits = 0;

Bounce key_bouncer = Bounce();
Bounce reset_bouncer = Bounce();

int pulse_codes[] = { A_CODE, B_CODE, C_CODE, D_CODE, E_CODE };
int pulse_lens[] = { A_LEN, B_LEN, C_LEN, D_LEN, E_LEN };

//*******************************************************
// Functions:
//*******************************************************

// Clear the words and dots arrays
void clearWordsArray();
void clearDotsDashesArray();

// Display the current dots and dashes and words:
void updateLCD();

// Sets the BiColor LED state
void setLED( const int led_state );

// Adds a dot or dash to the dotdash array
void addDotDash( const char& dotdash );

// Handles the Morse Key Down situation
void handleKeyDown(long int& button_down_time, long int& pause_time);
void handleKeyUp( long int& button_down_time, long int& pause_time);

// Handles the reset of state
void handleReset();

// Tries to convert the current pulse bit pattern into a character
// Returns ' ' if there's no match
char getCharacterFromPulses();

// Adds a character to the array and updates the LCD:
void addCharacter( const char& c );

//****************************************************

void setup( ) {

  Serial.begin(9600);
  clearWordsArray();
  clearDotsDashesArray();
  pinMode(RESET_PIN, INPUT );
  digitalWrite( RESET_PIN, HIGH );
  reset_bouncer.attach( RESET_PIN );
  reset_bouncer.interval(10);


  pinMode(KEY_IN_PIN, INPUT );
  digitalWrite( KEY_IN_PIN, HIGH );
  key_bouncer.attach( KEY_IN_PIN );
  key_bouncer.interval(15);

  pinMode(BUZZER_PIN, OUTPUT );

  lcd.displayLine( 1, "Morse DeCoder");
  lcd.displayLine( 2, "v1.0     ");
  delay(2000);
  ss.flush();
  lcd.clear();
  pinMode(LED_ORANGE_PIN, OUTPUT );
  pinMode(LED_GREEN_PIN, OUTPUT );
  setLED(LED_GREEN);

}


void loop ( ) {

  static long int button_down_time = 0;
  static long int pause_time = 0;

  key_bouncer.update();
  reset_bouncer.update();

  // Morse key is normally closed so it's high (connected to 5v) when the key is down
  if ( key_bouncer.rose() ) {
    handleKeyDown(button_down_time, pause_time);
  }

  // Morse key is normally closed so it's low (connected to ground) when the key is up
  if ( key_bouncer.fell() ) {
    handleKeyUp(button_down_time, pause_time);
  }

  if ( reset_bouncer.fell() ) {
    handleReset();
  }

  if ( pause_time > 0 && ( ( millis() - pause_time) > DOT_CUTOFF * 5) ) {
    pause_time = 0;
    char c = getCharacterFromPulses();
    if ( c != ' '  ) {
      addCharacter(c);
      clearDotsDashesArray();
      num_pulses = 0;
      pulse_bits = 0;
      updateLCD();
    }
  }
}


/*******************************************************/

void handleKeyUp(long int& button_down_time ,
                 long int& pause_time )
{
  char pulse;
  long int button_elapsed_time = 0;

  setLED( LED_GREEN);
  noTone(BUZZER_PIN );
  if ( button_down_time > 0 ) {
    button_elapsed_time = millis() - button_down_time;
    Serial.print("Button down time = ");
    Serial.println(button_elapsed_time);
    if ( button_elapsed_time < DOT_CUTOFF ) {
      pulse = '.';
    }
    else {
      pulse = '-';
    }

    addDotDash(pulse);
    updateLCD();

  }
  button_down_time = 0;
  pause_time = millis();
}

/*******************************************************/

void handleKeyDown(long int& button_down_time,
                   long int& pause_time)
{
  setLED( LED_ORANGE);
  tone(BUZZER_PIN, TONE_FREQUENCY );
  button_down_time = millis();
  pause_time = 0;
}


/*******************************************************/

void handleReset()
{
  lcd.clear();
  clearWordsArray();
  clearDotsDashesArray();
  num_chars = 0;
  num_pulses = 0;
  pulse_bits = 0;
}


/*******************************************************/

// Sets the BiColor LED state
void setLED( const int led_state )
{
  switch ( led_state ) {

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

/*********************************************************/

// Clear the words and dots arrays
void clearDotsDashesArray()
{
  for ( char& d : dots_dashes )
  {
    d = ' ';
  }
}

/*********************************************************/

void clearWordsArray()
{
  for ( char& w : words )
  {
    w = ' ';
  }
}

/********************************************************/

void updateLCD() {
  lcd.displayLine(1, dots_dashes);
  lcd.displayLine(2, words );
}

/* ******************************************************/

// Adds a dot or dash to the dotdash array
void addDotDash( const char& dotdash )
{

  if ( num_pulses >= MAX_PULSES) {
    num_pulses = 0;
    pulse_bits = 0;
    clearDotsDashesArray();
    return;
  }
  dots_dashes[num_pulses] = dotdash;

  if ( dotdash == '-')
    pulse_bits |= (DASH << num_pulses);

  Serial.print("Pulse bits = ");
  Serial.println(pulse_bits, BIN );

  num_pulses++;

  
}

/* ******************************************************/

// Adds a a character to the words array
void addCharacter( const char& c )
{

  if ( num_chars >= MAX_CHARS) {
    num_chars = 0;
    clearWordsArray();
  }
  words[num_chars] = c;

  num_chars++;
}

/* ******************************************************/
char getCharacterFromPulses()
{
  char ret_char = ' ';

  int num_codes = sizeof(pulse_codes) / sizeof(int);
  for ( auto i = 0; i < num_codes; i++ ) {

    if ( pulse_bits == pulse_codes[i] && num_pulses == pulse_lens[i] ) {
      ret_char = 'A' + i;
      break;
    }

  }

  return ret_char;
}


