#include <FastLED.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a software SPI connection (recommended):
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// LED pins
#define LED_PIN     10
#define LED_PIN2    11

#define NUM_STRIPS 2
#define NUM_LEDS    50
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
CRGB leds[NUM_STRIPS][NUM_LEDS];


CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

// Twinkle
enum twink { SteadyDim, GettingBrighter, GettingDimmerAgain };
twink TwinkleState[NUM_STRIPS][NUM_LEDS];


void setup() {
    delay( 3000 ); // power-up safety delay
    
    Serial.begin(115200);
    while (!Serial) delay(10); // for Leonardo/Micro/Zero
    Serial.println("Hello!");

    for( int i = 0; i < 256; i++) {
      Serial.print(i);
      Serial.print(", ");
      Serial.println(Step(i, 12));
    }
    
    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
      Serial.print("Didn't find PN53x board");
      while (1); // halt
    }
    // Got ok data, print it out!
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
    // configure board to read RFID tags
    nfc.setPassiveActivationRetries(0x00); // seems important so the NFC read doesn't block control
    nfc.SAMConfig();
  
    Serial.println("Waiting for an ISO14443A Card ...");

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds[0], NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.addLeds<LED_TYPE, LED_PIN2, COLOR_ORDER>(leds[1], NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
    

    memset( TwinkleState, sizeof(TwinkleState), SteadyDim); // initialize all the pixels to SteadyDim.
}


// Registered Dream Tags
byte glitter_tag[]={0x04, 0x1D, 0xDA, 0xD4, 0x70, 0x00, 0x00};
byte test_tag[]={0x1E, 0x1F, 0x3D, 0x2A};
byte null_tag[]={0x00, 0x00, 0x00, 0x00};

byte thor_tag[]={0x04,0x88,0xA0,0xD4,0x70,0x00,0x00};
byte desi_tag[]={0x04,0xF7,0xA0,0xD4,0x70,0x00,0x00};
byte hugs_tag[]={0x04,0x6A,0x8E,0xD4,0x70,0x00,0x00};
byte paul_tag[]={0x04,0x0B,0x8E,0xD4,0x70,0x00,0x00};
byte rose_tag[]={0x04,0x43,0x97,0xD4,0x70,0x00,0x00};
byte hari_tag[]={0x04,0x47,0x7C,0xD4,0x70,0x00,0x00};
byte krishna_tag[]={0x04,0xC1,0xAA,0xD4,0x70,0x00,0x00};
byte amanda_tag[]={0x04,0xD5,0x96,0xD4,0x70,0x00,0x00};
byte nat_tag[]={0x04,0x99,0x85,0xD4,0x70,0x00,0x00};
byte hila_tag[]={0x04,0xF5,0x7B,0xD4,0x70,0x00,0x00};
byte kevin_tag[]={0x04,0x3E,0x85,0xD4,0x70,0x00,0x00};
byte zach_tag[]={0x04,0x69,0x61,0xD4,0x70,0x00,0x00};
byte feli_tag[]={0x04,0x4A,0xAA,0xD4,0x70,0x00,0x00};

//not camping with us 
byte jeremy_tag[]={0x04,0xC8,0x8E,0xD4,0x70,0x00,0x00};
byte enizzy_tag[]={0x04,0x28,0x73,0xD4,0x70,0x00,0x00};
byte jess_tag[]={0x04,0xB0,0x61,0xD4,0x70,0x00,0x00};
byte robb_tag[]={0x04,0xB9,0x72,0xD4,0x70,0x00,0x00};

const unsigned long READ_TIMEOUT = 1000;
int reads_since_success = 0;

boolean equalTags(uint8_t tag1[], uint8_t tag2[]) {
  return memcmp(tag1, tag2, sizeof(tag2)) == 0;
}
    
boolean runMatchedPattern(uint8_t uid[]) {
 if (equalTags(uid, glitter_tag)) {
  glitterBug();
  return true;
 }
 if (equalTags(uid, test_tag)) {
  Space();
  return true;
 }
 if (equalTags(uid, thor_tag)) {
  Serial.println("Thor tag");
  Lust();
  return true;
 }
 if (equalTags(uid, desi_tag)) {
  Serial.println("Desi tag");
  Lust();
  return true;
 }
 if (equalTags(uid, hugs_tag)) {
  Serial.println("Hugs tag");
  Lust();
  return true;
 }
 if (equalTags(uid, paul_tag)) {
  Serial.println("Paul tag");
  Lust();
  return true;
 }
 if (equalTags(uid, rose_tag)) {
  Serial.println("Rose tag");
  Lust();
  return true;
 }
 if (equalTags(uid, hari_tag)) {
  Serial.println("Hari tag");
  Lust();
  return true;
 }
 if (equalTags(uid, krishna_tag)) {
  Serial.println("Krishna tag");
  Lust();
  return true;
 }
 if (equalTags(uid, amanda_tag)) {
  Serial.println("Amanda tag");
  Lust();
  return true;
 }
 if (equalTags(uid, nat_tag)) {
  Serial.println("Nat tag");
  Lust();
  return true;
 }
 if (equalTags(uid, hila_tag)) {
  Serial.println("Hila tag");
  Lust();
  return true;
 }
 if (equalTags(uid, kevin_tag)) {
  Serial.println("Kevin tag");
  Lust();
  return true;
 }
 if (equalTags(uid, zach_tag)) {
  Serial.println("Zach tag");
  Lust();
  return true;
 }
 if (equalTags(uid, feli_tag)) {
  Serial.println("Feli tag");
  Lust();
  return true;
 }
 return false;
}

uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

void loop()
{
//    uint8_t success;
//    uint8_t uid[] = { 1, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
//    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
/*
    Serial.print("millis(): ");
    Serial.println(millis());
    Serial.print("last_read_time: ");
    Serial.println(last_read_time);
    Serial.print("millis() - last_read_time: ");
    Serial.println(millis() - last_read_time);
    */   

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 0);
    reads_since_success++;
    if (success){
      reads_since_success=0;
    }
  
    if (reads_since_success>20) {
      Serial.println("No tag");
<<<<<<< HEAD:sheepcatcher.ino
    } else if (memcmp(uid, glitter_tag, sizeof(glitter_tag)) == 0){
      glitterBug();
      Serial.println("  Glitter Tag ");
      nfc.PrintHex(uid, uidLength);
      Serial.println("");
    } else if (memcmp(uid, test_tag, sizeof(test_tag)) == 0){
      Firework();
//      Space();
//      Flight();
//      Pulse();
//      Lust();
//      FillLEDsWaves();
//      Rainbow();
//      Serial.println("  Lust Tag ");
//      Serial.println(millis());
//      nfc.PrintHex(uid, uidLength);
//      Serial.println("");
      // Rotate();
    } else if (memcmp(uid, null_tag, sizeof(null_tag)) == 0){
      // Serial.println("  Null Tag ");
      // nfc.PrintHex(uid, uidLength);
      // Serial.println("");
=======
      Twinkle(1, 208, 255);
    } else if (runMatchedPattern(uid)) {
      Serial.println("Matched tag");
>>>>>>> dev-2022:electric-sheepcatcher.ino
    } else {
      nfc.PrintHex(uid, uidLength);
      Serial.println("");
      Serial.println("  Unknown Tag ");
      glitterBug();
    }
    
//    memcmp(uid, glitter_tag, sizeof(glitter_tag)) == 0){
//      glitterBug();
//      Serial.println("  Glitter Tag ");
//      nfc.PrintHex(uid, uidLength);
//      Serial.println("");
//    } else if (memcmp(uid, thor_tag, sizeof(thor_tag)) == 0){
//      Lust();
//      Serial.println("  Thor Tag ");
//    } else if (memcmp(uid, test_tag, sizeof(test_tag)) == 0){
////      Firework();
//      Space();
////      Flight();
////      Pulse();
////      Lust();
////      FillLEDsWaves();
////      Rainbow();
//      Serial.println("  Lust Tag ");
//      Serial.println(millis());
////      nfc.PrintHex(uid, uidLength);
////      Serial.println("");
//      // Rotate();
//    } else if (memcmp(uid, null_tag, sizeof(null_tag)) == 0){
//      // Serial.println("  Null Tag ");
//      // nfc.PrintHex(uid, uidLength);
//      // Serial.println("");
//    } else {
//      nfc.PrintHex(uid, uidLength);
//      Serial.println("");
//      Serial.println("  Unknown Tag ");
//      glitterBug();
//    }
    
    // ChangePalettePeriodically();
  /*
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; // motion speed 
    

    if (seconds < 30) {
      Pulse();
    } else if (seconds < 60) {
      Rotate(startIndex);
    } else if (seconds < 75) {
      glitterBug();
    } else if (seconds < 105) {
      FillLEDsWaves();
    } else if (seconds < 135) {
      Twinkle(1, 208, 255);
    } else if (seconds < 165) {
      FillLEDsFromPaletteColors(startIndex);
    } else if (seconds < 195) {
      Twinkle(1, currentPalette);
    } else {
      Twinkle(1, 64, 128);
    }
    */
    
    FastLED.show();
    //FastLED.delay(10);
}

void Firework()
{

  
  uint8_t phase = millis() / 8;
  uint8_t hue1 = Step(millis() / (8 * 12) , 11);
  uint8_t hue2 = hue1 + 255 /3;
  
  

  
  uint8_t index1 = phase * NUM_LEDS / 255;
  uint8_t index2 = phase * NUM_LEDS / 255 / 2;

  if ( phase < 32 ) {
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[0][i] = CHSV(0, 0, 0);
      leds[1][i] = CHSV(0, 0, 0);
    }
  } else {
    
  
  
    for( int i = 0; i < NUM_LEDS; i++) {
      if ( i == index1 ||  i == index1 - 1 || i == index1 - 2 ) {
        leds[0][NUM_LEDS - i] = CHSV(hue1, 255, 255);
        leds[1][NUM_LEDS - i] = CHSV(hue1, 255, 255);
      } else if ( i == index2) {
        leds[0][NUM_LEDS - i] = CHSV(hue2, 255, 255);
        leds[1][NUM_LEDS - i] = CHSV(hue2, 255, 255);
      } else{
      
        leds[0][NUM_LEDS - i] = CHSV(0, 0, 0);
        leds[1][NUM_LEDS - i] = CHSV(0, 0, 0);
      }
    }
  }

  if ( phase > 220 ) {
    glitterBug();
  }
    

}

void Space()
{
    uint8_t offset_1 = millis()/6;

    for( int i = 0; i < NUM_LEDS; i++) {
      leds[0][i] = CHSV(0, 0, Step(255 * 2 * i / NUM_LEDS + offset_1, 1));
      leds[1][i] = CHSV(0, 0, Step(255 * 2 * i / NUM_LEDS + offset_1, 1));
    }
    

}

uint8_t Step( uint8_t phase, uint8_t n)
{
  uint8_t amplitude = (floor((n + 1) * phase / 256))  * 255 / n;
  return amplitude;
}


void Flight()
{
    uint8_t offset = millis()/7;
    uint8_t intensity = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[0][i] = CHSV(180, Step(255 * i / NUM_LEDS + offset, 2), intensity);
      leds[1][i] = CHSV(180, Step(255 * i / NUM_LEDS + offset, 2), intensity);
    }
    

}




void Lust()
{
    uint8_t main_clock = millis() / 100;

    
    uint8_t offset_1;
    uint8_t offset_2;
    uint8_t intensity = 96;
    if (main_clock < 240) {
      
      if (main_clock < 32) {
        offset_1 = millis() / 7;
        offset_2 = millis() / 7 - 30;
      } else if (main_clock < 64) {
        intensity = 64;
        offset_1 = millis() / 6;
        offset_2 = millis() / 6 - 15;
      } else if (main_clock < 128) {
        intensity = 96;
        offset_1 = millis() / 5;
        offset_2 = millis() / 5 - 15;
      } else if (main_clock < 160) {
        intensity = 128;
        offset_1 = millis() / 4;
        offset_2 = millis() / 4 - 10;
      } else if (main_clock < 228) {
        intensity = 160;
        offset_1 = millis() / 3;
        offset_2 = millis() / 3 - 2 ;
      } else {
        intensity = 192;
        offset_1 = millis() / 2.5;
        offset_2 = millis() / 2.5;
      }
      for( int i = 0; i < NUM_LEDS; i++) {
        leds[0][i] = CHSV(245, 255, intensity / 2 * i / NUM_LEDS  + intensity / 2 * triwave8(offset_1) / 255);
        leds[1][i] = CHSV(245, 255, intensity / 2 * i / NUM_LEDS  + intensity / 2 * triwave8(offset_2) / 255);
      }
    } else {
      for( int i = 0; i < NUM_LEDS; i++) {
        leds[0][i] = CHSV(245, 255, 255);
        leds[1][i] = CHSV(240, 255, 255);
      }
      glitterBug();
    }
    
    

}

void Rainbow()
{
    uint8_t hue_offset = millis() / 20;
    
    
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[0][i] = CHSV(i * 255 / NUM_LEDS - hue_offset * 255, 255, 255);
      leds[1][i] = CHSV(i * 255 / NUM_LEDS - hue_offset * 255, 255, 255);
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[0][i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        leds[1][i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void Rotate()
{
    uint8_t offset = millis()/200;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[0][i] = ColorFromPalette( currentPalette, offset, BRIGHTNESS, currentBlending);
        leds[1][NUM_LEDS - i - 1] = ColorFromPalette( currentPalette, 255 - offset, BRIGHTNESS, currentBlending);
    }
}

void Pulse()
{
    uint8_t brightness = 255;
    uint8_t offset = millis()/200;
    uint16_t position = millis()/20 % (NUM_LEDS * 2);

    if (position < NUM_LEDS) {
      leds[0][position] = ColorFromPalette( currentPalette, offset, brightness, currentBlending);
    } else {
      leds[1][NUM_LEDS * 2 - position - 1 ] = ColorFromPalette( currentPalette, offset, brightness, currentBlending);
    }
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[0][i].fadeLightBy( 16 );
        leds[1][i].fadeLightBy( 16 );
    }
}

void FillLEDsWaves()
{
    static uint16_t position1 = 0;
    static uint16_t position2 = 0;
    
    position1 += sin8(millis() / 200);
    position2 += cos8(millis() / 200);
    
    for( uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[1][i] = ColorFromPalette( currentPalette, uint8_t(position1 /256) + 3 * i, BRIGHTNESS, currentBlending);
        leds[0][i] = ColorFromPalette( currentPalette, uint8_t(position2 /256) + 3 * i, BRIGHTNESS, currentBlending);
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 280;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }        
        if( secondHand == 80)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
    //    if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 120)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 160)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 200)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 240)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }

    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    uint8_t brightoffset = 150;
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, (random8()/2) + brightoffset);
    }
}



// This function sets up a palette of purple and pink stripes. I fixed
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB pink  = CHSV( HUE_PINK, 255, 255);
    CRGB white  = CRGB::White;
    
    currentPalette = CRGBPalette16(
                                   pink,  pink,  pink, pink, pink, 
                                   purple, purple, purple,
                                   purple, purple, purple,  
                                   white, white, white, white, white);
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};


void glitterBug() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[0][i].fadeLightBy( 16 );
    leds[1][i].fadeLightBy( 16 );
    addGlitter(10);
  }
}

//glitter effect
void addGlitter( fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[0][ random16(NUM_LEDS) ] = CHSV(64, 40, 255);}
  if( random8() < chanceOfGlitter) {
    leds[1][ random16(NUM_LEDS) ] = CHSV(64, 40, 255);}
}






void Twinkle( fract8 chanceOfTwinkle, uint8_t hue, uint8_t sat ) {
  CRGB BASE_COLOR = CHSV(hue, sat, 64);
  CRGB PEAK_COLOR = CHSV(hue, sat, 128);
  CRGB DELTA_COLOR_UP = CHSV(hue, sat, 4);
  CRGB DELTA_COLOR_DOWN = CRGB(1,1, 1);
  
  // Strip 1
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( TwinkleState[0][i] == SteadyDim) {
      leds[0][i] = BASE_COLOR;
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if( random8() < chanceOfTwinkle) {
        TwinkleState[0][i] = GettingBrighter;
      }
      
    } else if( TwinkleState[0][i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if( leds[0][i] >= PEAK_COLOR ) {
        TwinkleState[0][i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        //leds[1][i] = CHSV(hue, sat, leds[1][i].getLuma() + 6);
        leds[0][i] += DELTA_COLOR_UP;
      }
      
    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if( leds[0][i] <= BASE_COLOR ) {
        leds[0][i] = BASE_COLOR; // reset to exact base color, in case we overshot
        TwinkleState[0][i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[0][i] -= DELTA_COLOR_DOWN;
      }
    }
  }

    // Strip 2
   for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( TwinkleState[1][i] == SteadyDim) {
      leds[1][i] = BASE_COLOR;
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if( random8() < chanceOfTwinkle) {
        TwinkleState[1][i] = GettingBrighter;
      }
      
    } else if( TwinkleState[1][i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if( leds[1][i] >= PEAK_COLOR ) {
        TwinkleState[1][i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        // leds[1][i] = CHSV(hue, 255, leds[1][i].getLuma() + 4);
        leds[1][i] += DELTA_COLOR_UP;
      }
      
    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if( leds[1][i] <= BASE_COLOR ) {
        leds[1][i] = BASE_COLOR; // reset to exact base color, in case we overshot
        TwinkleState[1][i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[1][i] -= DELTA_COLOR_DOWN;
      }
    }
  }
}  


void Twinkle( fract8 chanceOfTwinkle, CRGBPalette16 palette ) {

  CRGB BASE_COLOR;
  CRGB PEAK_COLOR;
  CRGB DELTA_COLOR_UP;
  CRGB DELTA_COLOR_DOWN;
  uint8_t offset = millis()/200;

  BASE_COLOR = ColorFromPalette( currentPalette, offset, 64, currentBlending);
  PEAK_COLOR = ColorFromPalette( currentPalette, offset, 128, currentBlending);
  DELTA_COLOR_UP = ColorFromPalette( currentPalette, offset, 3, currentBlending);
  DELTA_COLOR_DOWN = CRGB(1,1,1);
  
  // Strip 1
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( TwinkleState[0][i] == SteadyDim) {
      leds[0][i] = BASE_COLOR;
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if( random8() < chanceOfTwinkle) {
        TwinkleState[0][i] = GettingBrighter;
      }
      
    } else if( TwinkleState[0][i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if( leds[0][i] >= PEAK_COLOR ) {
        TwinkleState[0][i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        //leds[1][i] = CHSV(hue, sat, leds[1][i].getLuma() + 6);
        leds[0][i] += DELTA_COLOR_UP;
      }
      
    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if( leds[0][i] <= BASE_COLOR ) {
        leds[0][i] = BASE_COLOR; // reset to exact base color, in case we overshot
        TwinkleState[0][i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[0][i] -= DELTA_COLOR_DOWN;
      }
    }
  }

    // Strip 2
   for( uint16_t i = 0; i < NUM_LEDS; i++) {
    if( TwinkleState[1][i] == SteadyDim) {
      leds[1][i] = BASE_COLOR;
      // this pixels is currently: SteadyDim
      // so we randomly consider making it start getting brighter
      if( random8() < chanceOfTwinkle) {
        TwinkleState[1][i] = GettingBrighter;
      }
      
    } else if( TwinkleState[1][i] == GettingBrighter ) {
      // this pixels is currently: GettingBrighter
      // so if it's at peak color, switch it to getting dimmer again
      if( leds[1][i] >= PEAK_COLOR ) {
        TwinkleState[1][i] = GettingDimmerAgain;
      } else {
        // otherwise, just keep brightening it:
        // leds[1][i] = CHSV(hue, 255, leds[1][i].getLuma() + 4);
        leds[1][i] += DELTA_COLOR_UP;
      }
      
    } else { // getting dimmer again
      // this pixels is currently: GettingDimmerAgain
      // so if it's back to base color, switch it to steady dim
      if( leds[1][i] <= BASE_COLOR ) {
        leds[1][i] = BASE_COLOR; // reset to exact base color, in case we overshot
        TwinkleState[1][i] = SteadyDim;
      } else {
        // otherwise, just keep dimming it down:
        leds[1][i] -= DELTA_COLOR_DOWN;
      }
    }
  }
}  
  


// Additionl notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.
