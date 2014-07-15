#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h> 
#include <Adafruit_NeoPixel.h>

#define PIN 6

#define COLOR_WIPE 1
#define RAINBOW 2
#define ROTATE_SINGLE_PIXEL 3
#define ROTATE_SINGLE_PIXEL_BOUNCE 4
#define THEATER_CHASE 5
#define THEATER_CHASE_RAINBOW 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{  
  ble_set_name("PixelBracelet");
  ble_begin();
  strip.begin();
  strip.show();

  // Enable serial debug
  Serial.begin(57600);
}

unsigned char buf[16] = {0};
unsigned char len = 0;

unsigned char lightMode;

int rgb[] = {};
int mode;

void loop()  
{
  digitalWrite(13, LOW);
  
  if ( ble_available() )
  {
    while ( ble_available() ) {
    
      mode = (char)ble_read() - '0';
      
      if ( mode == COLOR_WIPE ) {
        lightMode = 'C';
      } else if ( mode == ROTATE_SINGLE_PIXEL ) {
        lightMode = 'S';
      } else if ( mode == ROTATE_SINGLE_PIXEL_BOUNCE ) {
        lightMode = 'B';
      } else if ( mode == THEATER_CHASE ) {
        lightMode = 'T';
      } else if ( mode == RAINBOW ) {
        lightMode = 'R';
      } else if ( mode == THEATER_CHASE_RAINBOW ) {
        lightMode = 'E';
      }
      
      switch ( mode ) {
      case COLOR_WIPE:
      case ROTATE_SINGLE_PIXEL:
      case ROTATE_SINGLE_PIXEL_BOUNCE:
      case THEATER_CHASE:
        Serial.println("SETTING COLOR");
        
        for ( int i = 0; i < 3; i++ ) {
          String code = "";
          boolean isContinue = true;

          while (isContinue) {
            char ch = (char)ble_read();

            if ( ch != ',' && code.length() < 3 ) {
              code += ch;
            } 
            else {
              isContinue = false;
            }
          }

          int intCode = code.toInt();
          rgb[i] = intCode;      
        }
        
        break;

      default: 
        break;

      }      

    }

  }
 
   switch ( lightMode ) {

    case 'C':
      Serial.println("COLORWIPE");
      colorWipe(strip.Color(rgb[0], rgb[1], rgb[2]), 50); 
      break;
    case 'R':
      Serial.println("RAINBOW");
      rainbow(20);
      break;
    case 'S':
      Serial.println("ROTATE");
      rotateSinglePixel(strip.Color(rgb[0], rgb[1], rgb[2]), 50, true);
      break;
    case 'B':
      Serial.println("ROTATE_SINGLE_PIXEL_BOUNCE");
      rotateSinglePixel(strip.Color(rgb[0], rgb[1], rgb[2]), 50, false);
      break;
    case 'T':
      Serial.println("THEATER_CHASE");
      theaterChase(strip.Color(rgb[0], rgb[1], rgb[2]), 50);
      break;
    case 'E':
      Serial.println("THEATER_CHASE_RAINBOW");
      theaterChaseRainbow(50);
      break;
    default:
      break;
    }   

  if ( Serial.available() )
  {
    delay(5);

    while ( Serial.available() )
      ble_write( Serial.read() );
  }

  ble_do_events();
}

void rotateSinglePixel(uint32_t c, uint8_t wait, bool isSingle) {
  int totalPixels = strip.numPixels();
  
  for (int i = 0; i < totalPixels; i++) {
    strip.setPixelColor(i, c);    //turn every third pixel on
    
    for ( int j = 0; j < totalPixels; j++ ) {
      if ( j != i ) {
        strip.setPixelColor(j, 0);
      }
    }
    
    strip.show();
    delay(wait);
  }
  
  if ( isSingle ) {
    return;
  } 
  
  for (int i = totalPixels-1; i >= 0; i--) {
    strip.setPixelColor(i, c);    //turn every third pixel on
    
    for ( int j = totalPixels-1; j >= 0; j-- ) {
      if ( j != i ) {
        strip.setPixelColor(j, 0);
      }
    }
    
    strip.show();
    delay(wait);
  }
}

void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
      
      if ( ble_available() ) {
        break; 
      }
    }
    strip.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

