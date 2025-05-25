//**************************************************************************************//
//   Created by Mike Morrow on May 10th, 2025. GitHub: MikeyMoMo                        //
//                                                                                      //
//   The latest update will be available on GitHub.                                     //
//                                                                                      //
//   You may use this for any non-commercial purpose as long as you leave in the        //
//    creator attribution.                                                              //
//                                                                                      //
//   Parts of this code was either created or fixed by CoPilot and DeepSeek.            //
//   I promised that I would double CoPilot's current salary of $0.  It was so happy.   //
//                                                                                      //
//**************************************************************************************//

// This define controls setting of WAP SSID & Password, location and timezone.
//#define CONFIG_FOR_JOE  // My friend.  We co-develop code so this makes it easy.

#define DO_DEBUG_PRINTS

#include "TFT_eSPI.h"
TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spriteBG = TFT_eSprite(&tft);    // Background sprite.
TFT_eSprite spriteSF = TFT_eSprite(&tft);    // Starfield  sprite.
// The Base version of the sprite is needed for scrolling the starfield without extra
//  clutter leaving a trail across the sprite from the scrolling.  After the .scroll,
//  Base is coped into the SF then the moon is added.  Rinse, repeat...
TFT_eSprite spriteSF_Base = TFT_eSprite(&tft);    // Starfield  sprite work area.
TFT_eSprite spriteMoonInvis = TFT_eSprite(&tft);  // Moon intermediary for invisiblility.
TFT_eSprite spriteMenu   = TFT_eSprite(&tft);     // Menu sprite.
//#include "Moon75.h"  // Grayscale picture of the moon. 75x75 pixels jpg in binary format.

int         dispLine1 = 8, dispLine2, dispLine3, dispLine4, dispLine5, dispLine6;

#define ORIENT_POWER_RIGHT 1
#define ORIENT_POWER_LEFT  3
#define BUTTON_PRESSED 0
#define BUTTON_NOT_PRESSED 1

// Include the jpeg decoder library
#include <TJpg_Decoder.h>
JRESULT     JPB_RC;               // Return code from drawing BG pic.

#include <MoonRise.h>
MoonRise mr;
struct tm * moonTimes;
struct tm * sunTimes;

#include <moonPhase.h>
moonPhase moonPhase;  // include a MoonPhase instance
moonData_t moon;      // variable (struct?) to receive the data
struct tm *tmlocalTime;

#include <SunRise.h>
SunRise sr;

#include <WiFi.h>
String   stringIP;  // IP address.
int      tftBL_Lvl = 75, prevBL_Lvl = -1;      // tft brightness, prev brightness.
//int      usePic;

#include "esp_sntp.h"      // Get UTC epoch here.
time_t      TS_Epoch = 0;  // Set by Time Sync, printed by loop if not 0.
time_t      UTC;
time_t      now;
time_t      startMillis;
time_t      showPhase = 0;     // If millis() is larger than this, show phase
time_t      showPhaseDelay = 2000;  // How long to delay Phase show

struct tm   timeinfo;
int         prevSec = -1;  // One second gate.
int         iDOM, iMonth, iYear, iHour, iPrevHour = -1, iMin, iSec;
int         brightness;
int         count;
int         topButton, bottomButton;  // Will be set depending on board orientation.
int         colorChoice;
int         nTemp;
bool        SleepTime;

#include <JPEGDecoder.h>  // JPEG decoder library

#include <LITTLEFS.h>
#define FORMAT_LittleFS_IF_FAILED true

char        chBuffer[100];     // Work area for sprintf, etc.
char        moonPhasePic[100]; // Loaded moon phase picture
char        chBLChange[30];    // Save last brightness setting string
char        chHour[3];         // Hour.
String      sVer;

#if defined CONFIG_FOR_JOE    // My friend's WAP credentials and location
const char* chSSID      = "N_Port";           // Your router SSID.
const char* chPassword  = "helita1943";       // Your router password.
String      Hemisphere  = "north";            // or "south"
const double lat        = 38.052147;          // Your location.
const double lon        = -122.153893;
const int WakeupHour    = 10;  // Default turn on display time
const int SleepHour     = 23;  // Default turn off display time
const int defaultBright = 60;
const int myOrientation = ORIENT_POWER_LEFT;

#else                         // My WAP credentials and location.

const char*  chSSID        = "MikeysWAP";        // Your router SSID.
const char*  chPassword    = "Noogly99";         // Your router password.
String       Hemisphere    = "north";  // or "south"
const double lat           = 18.5376;            // Your location.
const double lon           = 120.7671;
int32_t      Phase         = 0.;
const int    WakeupHour    = 10;  // Default turn on display time
const int    SleepHour     = 23;  // Default turn off display time
const int    defaultBright = 60;
const int    myOrientation = ORIENT_POWER_LEFT;
#endif

#include "Preferences.h"
Preferences preferences;
#define RO_MODE true   // Preferences calls Read-only mode
#define RW_MODE false  // Preferences calls Read-write mode

// The maximum brightness that the button will allow.
#define MAX_BRIGHTNESS  254  // T-Display-S3 display brightness maximum.
// The minimum brightness that the button will allow.
#define MIN_BRIGHTNESS    0  // T-Display-S3 display brightness minimum.

#define MENU_HIDE_TIME 15000  // With no button press, menu will disappear in this many ms.

//Moon
const String TXT_MOON_NEW             = "New Moon";
const String TXT_MOON_WAXING_CRESCENT = "Waxing Crescent";
const String TXT_MOON_FIRST_QUARTER   = "First Quarter";
const String TXT_MOON_WAXING_GIBBOUS  = "Waxing Gibbous";
const String TXT_MOON_FULL            = "Full Moon";
const String TXT_MOON_WANING_GIBBOUS  = "Waning Gibbous";
const String TXT_MOON_THIRD_QUARTER   = "Third Quarter";
const String TXT_MOON_WANING_CRESCENT = "Waning Crescent";

#define      SPR_MENU_FONT_SIZE       4  // Menu sprite uses builtin font 4
#define      SPR_MENU_HEIGHT         40  // Menu sprite height is 42 pixels
#define      SPR_MENU_WIDTH         320  // Menu sprite width is the display width

int     BLchange;                        // Backlight change amount
int     prevHour = -1;                   // We have the BL for this hour
int     dRead;                           // Reading from digitalRead.
int     font4Height;                     // Height of the built-in font 4 for line spacing.

time_t  BLChangeMillis = 0;
time_t  menuHide;

#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))

/*******************************************************************************************/
void setup()
/*******************************************************************************************/
{
  Serial.begin(115200); delay(5000);
  Serial.println("This is Moon Phase and Time on T-Display S3.");
  //  Serial.println("Running from:");
  //  Serial.println(__FILE__);
  printVers();
  Serial.println("For the T-Display S3 target board, use tft_espi config# 206");

  tft.init();  // Init the tft.  What else?
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(tft.width());

  if (myOrientation == ORIENT_POWER_LEFT) {  // Asjust pins based on display orientation.
    topButton = 0; bottomButton = 14;  // Increase brightness is always on top, and...
  } else {
    topButton = 14; bottomButton = 0;  // Decrease brightness is always on the bottom button.
  }

  // Initialise LittleFS
  Serial.println("Initializing LittleFS.");
  if (!LittleFS.begin(FORMAT_LittleFS_IF_FAILED)) {
    Serial.println("LittleFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }

  // Initialize TJpgDec
  TJpgDec.setSwapBytes(true); // We need to swap the colour bytes (endianess)
  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);
  // The decoder must be given the exact name of the rendering function.
  TJpgDec.setCallback(jpg_output_Sprite);

  spriteBG.createSprite(tft.width(), tft.height());  // The big kahuna. The big cheese!
  spriteBG.setSwapBytes(false);

  font4Height = spriteBG.fontHeight(4);
  dispLine2 = dispLine1 + font4Height;
  dispLine3 = dispLine2 + font4Height;
  dispLine4 = dispLine3 + font4Height;
  dispLine5 = dispLine4 + font4Height;
  dispLine6 = dispLine5 + font4Height;

  // I build the star field here then shove it into spriteSF.  spriteSF_Base is wider so
  //  it cuts off 20 columns on the right side. This lets me create stars (circles) on
  //  the right and have them scroll onto screen more naturally vs. having them pop up
  //  fully formed.
  spriteSF.createSprite(spriteBG.width() / 2 - 15, dispLine6);
  spriteSF_Base.createSprite(spriteSF.width() + 20, spriteSF.height());

  spriteSF_Base.fillSprite(TFT_BLACK);
  count = 64; Serial.printf("Creating %i stars.\r\n", count);
  for (int i = 0; i < count; i++) {  // Initial star field.  Up to "count" stars.
    brightness = random(155) + 100;  // Brightness range from 100 to 255.
    // Note:
    // I took out the final color so it will use whatever is around the dot to be put in.
    // That means that the alpha blend will use whatever is there instead of forcing black.
    // If a new start goes in right next to another one, it will see some white as well as
    //  some black.  As it should be!
    //
    // Also note:
    // I made the spriteSF_Base wider than the spriteSF (SF stands for star field). Then
    //  I make stars on the right end of spriteSF_Base but they can't be seen right away.
    // Later, they scroll into view as if appearing in a window, as it should be.  The
    //  same effect happens when a star scrolls out from behind the disk of the moon.  It
    //  is occluded behind the moon then scrolls back into view on the left side of the
    //  disk as the star field scrolls left.  As it should be!
    //    spriteSF_Base.fillSmoothCircle(random(spriteSF_Base.width()),
    //                                   random(spriteSF.height()), random(3),
    //                                   RGB565(brightness, brightness, brightness));
    colorChoice = random(4);
    if (colorChoice < 2) {  // Usually going to be a 1, surely a 0
      spriteSF_Base.fillSmoothCircle(random(spriteSF_Base.width()),
                                     random(spriteSF_Base.height()), random(3),
                                     RGB565(brightness, brightness, brightness));
    }
    else if (colorChoice == 2) {  // Less often a 2 making a blue dot
      spriteSF_Base.fillSmoothCircle(random(spriteSF_Base.width()),
                                     random(spriteSF_Base.height()), random(3),
                                     RGB565(204, 211, 255));  // Little bit Blue.
      Serial.println("Made a blue circle.");
    }
    else if (colorChoice == 3) {  // Even more rare, a 3 making a redish/pinkish dot
      spriteSF_Base.fillSmoothCircle(random(spriteSF_Base.width()),
                                     random(spriteSF_Base.height()), random(3),
                                     RGB565(255, 211, 204));  // Little bit Red.
      Serial.println("Made a red circle.");
    }
  }

  spriteMoonInvis.createSprite(100, 100);

  spriteMenu.createSprite(tft.width(), SPR_MENU_HEIGHT);
  spriteMenu.setSwapBytes(true);
  //  spriteMenu.setSwapBytes(false);
  spriteMenu.setTextDatum(TL_DATUM);

  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  ledcWrite(TFT_BL, 200);       // Turn the display on bigly for init messages.

  tft.setTextDatum(TC_DATUM);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Awaiting WiFi connection", tft.width() / 2, dispLine2, 4);
  tft.drawString("to " + String(chSSID), tft.width() / 2, dispLine5, 4);
  Serial.printf("Connecting to WiFi %s ...", chSSID);
  WiFi.begin(chSSID, chPassword); delay(1500);

  while (WiFi.status() != WL_CONNECTED)  // Wait for a connection.
  {
    delay(500);
    Serial.print(".");
  }
  stringIP = WiFi.localIP().toString();
  tft.drawString("WiFi connected to:", tft.width() / 2, dispLine2, 4);
  tft.drawString(stringIP, tft.width() / 2, dispLine3, 4);
  Serial.printf("\r\nWiFi connected to %s at %s\r\n", chSSID, stringIP);
  tft.drawString("Awaiting correct time...", tft.width() / 2, dispLine5, 4);
  //  tft.drawString(" ", tft.width() / 2, dispLine5, 4);
  initTime();

  //  setHourBrightness();
  //  Serial.printf("Setup setting brightness level for hour %i of %i\r\n",
  //                iHour, tftBL_Lvl);

  tft.setTextPadding(0);
  tft.setTextDatum(TL_DATUM);
  //  ledcWrite(TFT_BL, tftBL_Lvl);  // Set the display at default level for operation.

  /* Testing */
  // To clear out all of the hourly brightness values, use the following four.
  //      preferences.begin("Hourlys", RW_MODE);
  //      preferences.clear();
  //      Serial.println("Hourlys entries cleared.");
  //      preferences.end();
  /* End Testing */

  Hemisphere.toLowerCase();
  moonPhasePic[0] = 0;  // Effectively set it to nothing for now.

  Serial.println("Setup is finished.");
}
/*******************************************************************************************/
void loop()
/*******************************************************************************************/
{
  static bool firstPass = true;

  while (Serial.available() > 0)  // Check if data is available to read
    HandleSerialInput();

  CheckButtons();  // See if brightness change or menu is requested.

  SaveOptions();  // Save any changed options every 10 minutes.

  spriteBG.fillRect(0, 0, tft.width(), dispLine6, RGB565(0, 0, 166));
  spriteBG.setTextColor(TFT_WHITE, RGB565(0, 0, 166));

  startMillis = millis();
  //  struct tm * now_local = gmtime(&now);  // Get UTC/GMT. (Thought it was needed. Nope!)
  getLocalTime(&timeinfo);  // This is what I needed.  Local time!

  iSec = timeinfo.tm_sec;
  if (prevSec == iSec) return;  // Wait for the next second to roll around...

  prevSec = iSec;

  iMonth  = timeinfo.tm_mon + 1;
  iDOM    = timeinfo.tm_mday;
  iYear   = timeinfo.tm_year + 1900;
  iMin    = timeinfo.tm_min;
  iHour   = timeinfo.tm_hour;

  if (prevHour != iHour) {
#if defined DO_DEBUG_PRINTS
    Serial.printf("Current date: %i/%i/%i\r\n", iMonth, iDOM, iYear);
    //    Serial.printf("AgeOfTheMoon: %f\r\n", AgeOfTheMoon(iDOM, iMonth, iYear));
    //    time_t epoch = get_epoch_time();
    //    moon = moonPhase.getPhase(epoch);
    //    Serial.printf("Moon angle: %i\r\n", moon.angle);
#endif
  }

  if (tftBL_Lvl == 0) {
    prevHour = iHour;
    delay(100);
    if (prevBL_Lvl != 0) {
      prevBL_Lvl = 0;
      tft.fillScreen(TFT_BLACK);
      //      ledcWrite(TFT_BL, defaultBright);  // Activate whatever was decided on.
      //      delay(10);
      //      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      ledcWrite(TFT_BL, 200);  // Turn off the backlight
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.setTextDatum(TC_DATUM);
      tft.drawString("Dark time now...", tft.width() / 2, dispLine2, 4);
      tft.drawString("Turning off", tft.width() / 2, dispLine3, 4);
      tft.drawString("screen.", tft.width() / 2, dispLine4, 4);
      Serial.println("Turning screen off.");
      delay(5000);
      tft.fillScreen(TFT_BLACK);
      ledcWrite(TFT_BL, 0);  // Turn off the backlight
      tft.writecommand(ST7789_DISPOFF);  // Turn off display hardware.
    }
    return;
  } else {  // if tftBL_Lvl > 0
    if (prevBL_Lvl == 0) {  // If we were previously in hardware sleep
      prevBL_Lvl = tftBL_Lvl;           // Remember the change and
      tft.writecommand(ST7789_DISPON);  // turn on display hardware.
    }
  }

  Build_and_Show();


  if (prevHour != iHour) {
    setHourBrightness();
    if (!firstPass) HourDance();
    firstPass = false;
    prevHour = iHour;
  }
  yield();  // I am getting a phantom Guru Meditation error from buttons. Hoping this helps.
  //  Serial.printf("Loop took %lu ms.\r\n", millis() - startMillis);
}
