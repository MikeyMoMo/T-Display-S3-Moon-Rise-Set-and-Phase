//**************************************************************************************//
//   Created by Mike Morrow on May 10th, 2025. GitHub: MikeyMoMo                        //
//                                                                                      //
//   The latest update will be available on GitHub.                                     //
//                                                                                      //
//   You may use this for any non-commercial purpose as long as you leave in the        //
//    creator attribution.                                                              //
//**************************************************************************************//

// This define controls setting of WAP SSID & Password, location and timezone.
//#define CONFIG_FOR_JOE  // My friend.  We co-develop code so this makes it easy.

#include "TFT_eSPI.h"
TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite spriteBG = TFT_eSprite(&tft);    // Background sprite.
TFT_eSprite spriteSF = TFT_eSprite(&tft);    // Starfield  sprite.
// The Base version of the sprite is needed for scrolling the starfield without extra
//  clutter leaving a trail across the sprite from the scrolling.  After the .scroll,
//  Base is coped into the SF then the moon is added.  Rinse, repeat...
TFT_eSprite spriteSF_Base = TFT_eSprite(&tft);    // Starfield  sprite work area.
TFT_eSprite spriteMoonInvis = TFT_eSprite(&tft);  // Moon intermediary for invisiblility.
TFT_eSprite spriteMenu   = TFT_eSprite(&tft);    // Menu sprite.

int         dispLine1 = 8, dispLine2, dispLine3, dispLine4, dispLine5, dispLine6;

#include "Moon75.h"

#include <MoonRise.h>
MoonRise mr;
struct tm * moonTimes;

#include <moonPhase.h>
moonPhase moonPhase;  // include a MoonPhase instance
moonData_t moon;      // variable (struct?) to receive the data
struct tm *tmlocalTime;

#include <WiFi.h>
String   stringIP;  // IP address.
int      tftBL_Lvl = 75, prevBL_Lvl = -1;      // tft brightness, prev brightness.

#include "esp_sntp.h"      // Get UTC epoch here.
time_t      TS_Epoch = 0;  // Set by Time Sync, printed by loop if not 0.
time_t      UTC;
time_t      now;
struct tm   timeinfo;
int         prevSec = -1;  // One second gate.
int         iDOM, iMonth, iYear, iHour, iPrevHour = -1, iMin, iSec;
int         brightness;
int         count;
int         incrPin, decrPin;              // Will be set depending on board orientation.
int         nTemp;
bool        WakeUp;

time_t      startMillis;

#include <JPEGDecoder.h>  // JPEG decoder library

char        chBuffer[100];                   // Work area for sprintf, etc.
char        chHour[3];                       // Hour.

#if defined CONFIG_FOR_JOE    // My friend's WAP credentials and location
const char* chSSID      = "N_Port";           // Your router SSID.
const char* chPassword  = "helita1943";       // Your router password.
const double lat        = 38.052147;          // Your location.
const double lon        = -122.153893;
String Hemisphere       = "north";            // or "south"
int WakeupHour          = 10;  // Default turn on display time
int SleepHour           = 23;  // Default turn off display time
const int defaultBright = 28;

#else                         // My WAP credentials and location.

const char* chSSID      = "MikeysWAP";        // Your router SSID.
const char* chPassword  = "Noogly99";         // Your router password.
const double lat        = 18.5376;            // Your location.
const double lon        = 120.7671;
String Hemisphere       = "north";  // or "south"
int WakeupHour          = 10;  // Default turn on display time
int SleepHour           = 23;  // Default turn off display time
const int defaultBright = 32;
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

#define ORIENT_POWER_RIGHT 1
#define ORIENT_POWER_LEFT  3
int     myOrientation = ORIENT_POWER_LEFT;
int     BLchange;                        // Backlight change amount
int     prevHour;                        // We have the BL for this hour
int     dRead;                           // Reading from digitalRead.
int     moonPhaseNo;
int     font4Height;

time_t  BLChangeMillis = 0;
time_t  menuHide;

#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))

/*******************************************************************************************/
void setup()
/*******************************************************************************************/
{
  Serial.begin(115200); delay(5000);
  Serial.println("This is Moon Phase and Time on T-Display S3.");
  Serial.println("Running from:");
  Serial.println(__FILE__);

  font4Height = spriteBG.fontHeight(4);
  dispLine2 = dispLine1 + font4Height;
  dispLine3 = dispLine2 + font4Height;
  dispLine4 = dispLine3 + font4Height;
  dispLine5 = dispLine4 + font4Height;
  dispLine6 = dispLine5 + font4Height;

  tft.init();  // Init the tft.  What else?
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  spriteBG.createSprite(tft.width(), tft.height());
  spriteBG.setSwapBytes(false);
  tft.setTextPadding(tft.width());

  if (myOrientation = ORIENT_POWER_LEFT) {  // Asjust pins based on display orientation.
    incrPin = 0; decrPin = 14;  // Increase brightness is always on top, and...
  } else {
    incrPin = 14; decrPin = 0;  // Decrease brightness is always on the bottom button.
  }

  spriteSF.createSprite(spriteBG.width() / 2 - 15, dispLine6);
  spriteSF_Base.createSprite(spriteSF.width(), spriteSF.height());

  spriteSF_Base.fillSprite(TFT_BLACK);
  count = 30; Serial.printf("Creating %i stars.\r\n", count);
  for (int i = 0; i < count; i++) { // Few new stars each frame
    brightness = random(155) + 100;
    // I took out the final color so it will use whatever is around the dot to be put in.
    spriteSF_Base.fillSmoothCircle(random(spriteSF.width() - 4), random(spriteSF.height()) - 4,
                                   random(3), RGB565(brightness, brightness, brightness));
  }

  spriteMoonInvis.createSprite(75, 75);

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

  setHourBrightness();
  Serial.printf("Setup setting brightness level for hour %i of %i\r\n",
                iHour, tftBL_Lvl);

  tft.setTextPadding(0);
  tft.setTextDatum(TL_DATUM);
  //  ledcWrite(TFT_BL, tftBL_Lvl);  // Set the display at default level for operation.

  Serial.println("Setup is finished.");
}
/*******************************************************************************************/
void loop()
/*******************************************************************************************/
{
  static int timeForStars = 0;

  CheckButtons();
  SaveOptions();

  spriteBG.fillSprite(RGB565(0, 0, 166));
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
    prevHour = iHour;
    setHourBrightness();
  }

  if (iSec % 10 == 0) {
    spriteSF_Base.scroll(-1, 0);  // Scroll left by one pixel
    if (timeForStars++ == 1) {  // Was 3
      timeForStars = 0;
      // Redraw new stars appearing on the right
      count = random(3);
      Serial.printf("Creating %i stars.\r\n", count);
      for (int i = 0; i < count; i++) { // Few new stars each frame
        brightness = random(155) + 100;
        // Serial.printf("Brightness %i ", brightness);
        // I took out the final color so it will use whatever is around the dot to be put in.
        spriteSF_Base.fillSmoothCircle(spriteSF.width() - 4, random(spriteSF.height()) - 4,
                                       random(3), RGB565(brightness, brightness, brightness));
      }
    }
  }
  spriteSF_Base.pushToSprite(&spriteSF, 0, 0);
                               
  spriteMoonInvis.fillSprite(TFT_BLACK);
  // Draw a jpeg image stored in memory at x,y
  drawArrayJpeg(Moon75, sizeof(Moon75), 0, 0);
  spriteMoonInvis.pushToSprite(&spriteSF, 37, 37, TFT_BLACK);
  // Leave off corners.
  spriteSF.pushToSprite(&spriteBG, 0, 0);  // , RGB565(0, 0, 166));
  // x, y, DOM, month, year, north/south
  AddMoonShadow(0, 0, iDOM, iMonth, iYear, Hemisphere);

  //Date
  strftime(chBuffer, sizeof(chBuffer), "%D", &timeinfo);
  //  Serial.print(chBuffer);  Serial.print(", ");
  spriteBG.drawString("Date:",  spriteBG.width() / 2 - 15, dispLine1, 4);
  spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine1, 4);

  // Time
  strftime(chBuffer, sizeof(chBuffer), "%T", &timeinfo);
  //  Serial.println(chBuffer);
  spriteBG.drawString("Time:",  spriteBG.width() / 2 - 15, dispLine2, 4);
  spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine2, 4);

  // Moon times
  time(&now);
  mr.calculate(lat, lon, now);  // Get all of the answsers
  //  Serial.print("Local time ");            Serial.print(now);
  //  Serial.print(", Calculate said rise "); Serial.print(mr.riseTime);
  //  Serial.print(", set ");                 Serial.println(mr.setTime);
  //  Serial.printf("Local time %lu, Calculate said rise %lu, set %lu\r\n",
  //                now, mr.riseTime, mr.setTime);  // Error here!

  //Rise
  spriteBG.drawString("Rise:",  spriteBG.width() / 2 - 15, dispLine3, 4);
  if (mr.hasRise) {
    moonTimes = localtime(&mr.riseTime);
    //    Serial.printf("Moonrise Time: %02d:%02d:%02d\n",
    //                  moonTimes->tm_hour, moonTimes->tm_min, moonTimes->tm_sec);
    strftime(chBuffer, sizeof(chBuffer), "%T", moonTimes);
    spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine3, 4);
  } else {
    spriteBG.drawString("None", spriteBG.width() / 2 + 54, dispLine3, 4);
  }

  // Set
  spriteBG.drawString("Set:",   spriteBG.width() / 2 - 15, dispLine4, 4);
  if (mr.hasSet) {
    moonTimes = localtime(&mr.setTime);
    //    Serial.printf("Moonset  Time: %02d:%02d:%02d\n",
    //                  moonTimes->tm_hour, moonTimes->tm_min, moonTimes->tm_sec);
    strftime(chBuffer, sizeof(chBuffer), "%T", moonTimes);
    spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine4, 4);
  } else {
    spriteBG.drawString("None", spriteBG.width() / 2 + 50, dispLine4, 4);
  }

  spriteBG.drawString("Visible:", spriteBG.width() / 2 - 15, dispLine5, 4);
  if (mr.isVisible)
    spriteBG.drawString("Yes",   spriteBG.width() / 2 + 74, dispLine5, 4);
  else
    spriteBG.drawString("No",   spriteBG.width() / 2 + 74, dispLine5, 4);
  //  Serial.printf("Visible: %s\r\n", mr.isVisible ? "Yes" : "No");

  // Phase
  spriteBG.setTextColor(TFT_YELLOW, RGB565(0, 0, 166));
  spriteBG.drawString(MoonPhase(timeinfo.tm_mday, timeinfo.tm_mon + 1,
                                timeinfo.tm_year + 1900, Hemisphere),
                      4, dispLine6 + 5, 4);
  spriteBG.pushSprite(0, 0);

  // Serial.printf("Loop took %lu ms.\r\n", millis() - startMillis);
}
/*******************************************************************************************/
void AddMoonShadow(int x, int y, int dd, int mm, int yy, String hemisphere)
/*******************************************************************************************/
{
  const int diameter = 75;
  double Phase = NormalizedMoonPhase(dd, mm, yy);
  //  Serial.print("Phase: "); Serial.println(Phase);

  hemisphere.toLowerCase();
  if (hemisphere == "south") Phase = 1 - Phase;
  // Draw dark part of moon
  spriteBG.drawCircle(x + diameter - 1, y + diameter, diameter / 2 + 1, TFT_BLACK);
  const int number_of_lines = diameter * 1.5; // 90;
  for (double Ypos = 0; Ypos <= number_of_lines / 2; Ypos++) {
    double Xpos = sqrt(number_of_lines / 2 * number_of_lines / 2 - Ypos * Ypos);
    // Determine the edges of the lighted part of the moon
    double Rpos = 2 * Xpos;
    double Xpos1, Xpos2;
    if (Phase < 0.5) {
      Xpos1 = -Xpos;
      Xpos2 = Rpos - 2 * Phase * Rpos - Xpos;
    }
    else {
      Xpos1 = Xpos;
      Xpos2 = Xpos - 2 * Phase * Rpos + Rpos;
    }
    //    Serial.printf("XPos1 %f, XPos2 %f, Ypos %f, #lines %i\r\n",
    //                  Xpos1, Xpos2, Ypos, number_of_lines);
    // Draw light part of moon
    double pW1x = (-Xpos1 + number_of_lines) / number_of_lines * diameter + x;
    double pW1y = (number_of_lines - Ypos)   / number_of_lines * diameter + y;
    double pW2x = (Xpos2 + number_of_lines)  / number_of_lines * diameter + x;
    double pW2y = (number_of_lines - Ypos)   / number_of_lines * diameter + y;
    double pW3x = (-Xpos1 + number_of_lines) / number_of_lines * diameter + x;
    double pW3y = (Ypos + number_of_lines)   / number_of_lines * diameter + y;
    double pW4x = (Xpos2 + number_of_lines)  / number_of_lines * diameter + x;
    double pW4y = (Ypos + number_of_lines)   / number_of_lines * diameter + y;
    spriteBG.drawLine(pW1x, pW1y, pW2x, pW2y, TFT_BLACK);
    spriteBG.drawLine(pW3x, pW3y, pW4x, pW4y, TFT_BLACK);
  }
}
/*******************************************************************************************/
void initTime()
/*******************************************************************************************/
{
  sntp_set_sync_interval(21601358);  // Get updated time every 6 hours.
  //  sntp_set_sync_interval(86405432);  // 1 day in ms plus a little.
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

#if defined CONFIG_FOR_JOE
  configTzTime("PST8PDT,M3.2.0,M11.1.0", "time.nist.gov");
#else
  configTzTime("PHT-8", "time.nist.gov");
#endif

  Serial.print("Waiting for correct time..."); delay(5000);
  while (!TS_Epoch); // Wait for time hack.

  strftime(chBuffer, sizeof(chBuffer), "%Y", localtime(&UTC));
  iYear = atoi(chBuffer);
  int iLooper = 0;
  while (iYear < 2025) {
    Serial.print(".");
    time(&UTC);
    strftime (chBuffer, 100, "%Y", localtime(&UTC));
    iYear = atoi(chBuffer);
    if (iLooper++ > 30) {
      Serial.println("\r\nCannot get time set. Rebooting.");
      ESP.restart();
    }
    delay(2000);
  }
  Serial.println();
  //  tft.setTextPadding(50);  // Don't need.  Using sprite now.
  spriteBG.setTextDatum(TL_DATUM);
}
/*******************************************************************************************/
double NormalizedMoonPhase(int d, int m, int y)
/*******************************************************************************************/
{
  int j = JulianDate(d, m, y);
  //Calculate the approximate phase of the moon
  double Phase = (j + 4.867) / 29.53059;
  return (Phase - (int) Phase);
}
/*******************************************************************************************/
int JulianDate(int d, int m, int y)
/*******************************************************************************************/
{
  int mm, yy, k1, k2, k3, j;
  yy = y - (int)((12 - m) / 10);
  mm = m + 9;
  if (mm >= 12) mm = mm - 12;
  k1 = (int)(365.25 * (yy + 4712));
  k2 = (int)(30.6001 * mm + 0.5);
  k3 = (int)((int)((yy / 100) + 49) * 0.75) - 38;
  // 'j' for dates in Julian calendar:
  j = k1 + k2 + d + 59 + 1;
  if (j > 2299160) j = j - k3;
  // 'j' is the Julian date at 12h UT (Universal Time) For Gregorian calendar:
  return j;
}
/*******************************************************************************************/
void timeSyncCallback(struct timeval * tv)
/*******************************************************************************************/
{
  //  struct timeval {  // Instantiated as "*tv"
  //    Number of whole seconds of elapsed time.
  //   time_t      tv_sec;
  //    Number of microseconds of rest of elapsed time minus tv_sec.
  //     Always less than one million.
  //   long int    tv_usec;
  //  Serial.print("\n** > Time Sync Received at ");
  //  Serial.println(ctime(&tv->tv_sec));
  TS_Epoch = tv->tv_sec;
}
/*******************************************************************************************/
String MoonPhase(int d, int m, int y, String hemisphere)
/*******************************************************************************************/
{
  int c, e;
  double jd;
  int b;
  if (m < 3) {
    y--;
    m += 12;
  }
  ++m;
  c   = 365.25 * y;
  e   = 30.6  * m;
  jd  = c + e + d - 694039.09;  /* jd is total days elapsed */
  jd /= 29.53059;               /* divide by the moon cycle (29.53 days) */
  b   = jd;                     /* int(jd) -> b, take integer part of jd */
  jd -= b;                   // subtract integer part to leave fractional part of original jd
  b   = jd * 8 + 0.5;           /* scale fraction from 0-8 and round by adding 0.5 */
  b   = b & 7;                  /* 0 and 8 are the same phase so modulo 8 for 0 */
  if (hemisphere == "south") b = 7 - b;
  moonPhaseNo = b;
  if (b == 0) return TXT_MOON_NEW;              // New;              0%  illuminated
  if (b == 1) return TXT_MOON_WAXING_CRESCENT;  // Waxing crescent; 25%  illuminated
  if (b == 2) return TXT_MOON_FIRST_QUARTER;    // First quarter;   50%  illuminated
  if (b == 3) return TXT_MOON_WAXING_GIBBOUS;   // Waxing gibbous;  75%  illuminated
  if (b == 4) return TXT_MOON_FULL;             // Full;            100% illuminated
  if (b == 5) return TXT_MOON_WANING_GIBBOUS;   // Waning gibbous;  75%  illuminated
  if (b == 6) return TXT_MOON_THIRD_QUARTER;    // Third quarter;   50%  illuminated
  if (b == 7) return TXT_MOON_WANING_CRESCENT;  // Waning crescent; 25%  illuminated
  return "";
}
/*******************************************************************************************/
// Draw a JPEG on the TFT pulled from a program memory array
/*******************************************************************************************/
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);
  // jpegInfo();  // Print information from the JPEG file (could comment this line out)
  renderJPEG(x, y);
  //  Serial.println("#########################");
}
/*******************************************************************************************/
// Print image information to the serial port (optional)
/*******************************************************************************************/
void jpegInfo() {
  Serial.println(F("==============="));
  Serial.println(F("JPEG image info"));
  Serial.println(F("==============="));
  Serial.print(F(  "Width      :")); Serial.println(JpegDec.width);
  Serial.print(F(  "Height     :")); Serial.println(JpegDec.height);
  Serial.print(F(  "Components :")); Serial.println(JpegDec.comps);
  Serial.print(F(  "MCU / row  :")); Serial.println(JpegDec.MCUSPerRow);
  Serial.print(F(  "MCU / col  :")); Serial.println(JpegDec.MCUSPerCol);
  Serial.print(F(  "Scan type  :")); Serial.println(JpegDec.scanType);
  Serial.print(F(  "MCU width  :")); Serial.println(JpegDec.MCUWidth);
  Serial.print(F(  "MCU height :")); Serial.println(JpegDec.MCUHeight);
  Serial.println(F("==============="));
}
/*******************************************************************************************/
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
/*******************************************************************************************/
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {

  // retrieve information about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.read()) {

    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    spriteMoonInvis.startWrite();

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= spriteMoonInvis.width() && ( mcu_y + win_h ) <= spriteMoonInvis.height())
    {

      // Now set a MCU bounding window on the TFT to push pixels
      //  into (x, y, x + width - 1, y + height - 1)
      spriteMoonInvis.setAddrWindow(mcu_x, mcu_y, win_w, win_h);

      // Write all MCU pixels to the TFT window
      while (mcu_pixels--) {
        // Push each pixel to the TFT MCU area
        spriteMoonInvis.pushColor(*pImg++);
      }
    }
    else if ( (mcu_y + win_h) >= spriteMoonInvis.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding

    spriteMoonInvis.endWrite();
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

  // print the results to the serial port
  //  Serial.print("Total render time was "); Serial.print(drawTime); Serial.println(" ms");
}
/*******************************************************************************************/
time_t get_epoch_time()
/*******************************************************************************************/
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}
