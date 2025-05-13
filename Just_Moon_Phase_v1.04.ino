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
TFT_eSprite spriteMenu   = TFT_eSprite(&tft);     // Menu sprite.
#include "Moon75.h"

int         dispLine1 = 8, dispLine2, dispLine3, dispLine4, dispLine5, dispLine6;

#define ORIENT_POWER_RIGHT 1
#define ORIENT_POWER_LEFT  3
#define BUTTON_PRESSED 0
#define BUTTON_NOT_PRESSED 1

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

#include "esp_sntp.h"      // Get UTC epoch here.
time_t      TS_Epoch = 0;  // Set by Time Sync, printed by loop if not 0.
time_t      UTC;
time_t      now;
time_t      startMillis;

struct tm   timeinfo;
int         prevSec = -1;  // One second gate.
int         iDOM, iMonth, iYear, iHour, iPrevHour = -1, iMin, iSec;
int         brightness;
int         count;
int         topButton, bottomButton;  // Will be set depending on board orientation.
int         nTemp;
bool        SleepTime;

#include <JPEGDecoder.h>  // JPEG decoder library

char        chBuffer[100];                   // Work area for sprintf, etc.
char        chHour[3];                       // Hour.
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

const char* chSSID      = "MikeysWAP";        // Your router SSID.
const char* chPassword  = "Noogly99";         // Your router password.
String      Hemisphere  = "north";  // or "south"
const double lat        = 18.5376;            // Your location.
const double lon        = 120.7671;
const int WakeupHour    = 10;  // Default turn on display time
const int SleepHour     = 23;  // Default turn off display time
const int defaultBright = 60;
const int myOrientation = ORIENT_POWER_LEFT;
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
int     prevHour;                        // We have the BL for this hour
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

  spriteBG.createSprite(tft.width(), tft.height());  // The big kahuna. The big cheese!
  spriteBG.setSwapBytes(false);

  font4Height = spriteBG.fontHeight(4);
  dispLine2 = dispLine1 + font4Height;
  dispLine3 = dispLine2 + font4Height;
  dispLine4 = dispLine3 + font4Height;
  dispLine5 = dispLine4 + font4Height;
  dispLine6 = dispLine5 + font4Height;

  // I build the star field here then shove it into spriteSF.  spriteSF_Base is wider so it
  //  cuts off 20 columns on the right side. This lets me create stars (circles) on the right
  //  and have them scroll onto screen more naturally vs. having them pop up fully formed.
  spriteSF.createSprite(spriteBG.width() / 2 - 15, dispLine6);
  spriteSF_Base.createSprite(spriteSF.width() + 20, spriteSF.height());

  spriteSF_Base.fillSprite(TFT_BLACK);
  count = 32;  // Serial.printf("Creating %i stars.\r\n", count);
  for (int i = 0; i < count; i++) { // Few new stars each frame
    brightness = random(155) + 100;
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
    spriteSF_Base.fillSmoothCircle(random(spriteSF_Base.width()), random(spriteSF.height()),
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

  //  setHourBrightness();
  //  Serial.printf("Setup setting brightness level for hour %i of %i\r\n",
  //                iHour, tftBL_Lvl);

  tft.setTextPadding(0);
  tft.setTextDatum(TL_DATUM);
  //  ledcWrite(TFT_BL, tftBL_Lvl);  // Set the display at default level for operation.

  /* Testing */
  // To clear out all of the hourly brightness values, use the following four.
  //    preferences.begin("Hourlys", RW_MODE);
  //    preferences.clear();
  //    Serial.println("Hourlys entries cleared.");
  //    preferences.end();

  Serial.println("Setup is finished.");
}
/*******************************************************************************************/
void loop()
/*******************************************************************************************/
{
  while (Serial.available() > 0)  // Check if data is available to read
    HandleSerialInput();

  CheckButtons();
  if (prevBL_Lvl != tftBL_Lvl) {
    prevBL_Lvl = tftBL_Lvl;
    for (int i = 0; i < 2000; i++) {
      delay(1);
      // Wait for 2 seconds or exit on button press.
      if ((digitalRead(topButton) == BUTTON_PRESSED) ||
          (digitalRead(bottomButton) == BUTTON_PRESSED)) break;
    }
  }

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

  if (tftBL_Lvl == 0) {
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
  }

  if (iSec % 10 == 0) AddStars();
  /* Well, the following needs a little explanation.  It took a while to figure out the order
      of stuff to do and I want to remember and you to know what is happening here.  It is
      fairly simple to understand now that it is complete.

      Step 1: First, I have to put the starfield that is kept in another sprite onto the
      display starfield.  The "other" starfield sprite is a bit wider then the display
      starfield sprite.  That's so I can create the "stars" and have them appear to scroll
      onto the screen rather than suddenly appear, fully formed on the right edge of the moon
      window. So extra stars are waiting, hidden, to come onto the screen in a minute or two.
      That's done by the next code statement of pushToSprite.  The right end of the Base
      starfield gets truncated and that's exactly what I wanted to happen. So, now, we have
      a  black background and a starfield in the left side of the spriteBG sprite.

      Step 2: Next, I have to push the moon jpg from moon75.h onto a sprite.  This is
      done by drawArrayJpeg.  This ia a Bodmer creation.  THANK YOU FOR THIS CODE!!! (I could
      not have done this without this code and some David Bird code.)  The moon jpg has been
      run through a converter that emits 8 bit for each byte and both the jpg and the sprite
      are 75x75.  So they fit exactly.  But, since all jpgs are square, it has four sorta-
      triangles on it in four places -- the "corners".  I had to edit these and make them
      totally black for the next step. drawArrayJpeg calls renderJPEG to actually load up
      the bitmap jpg into the spriteMoonInvis sprite.

      Step 3: Now that I have the moon jpg with its totally black corners in a sprite, I need
      to get rid of the black corners.  To do this, I have to push the moon sprite onto
      another sprite with the invisible color (black) noted so those pixels will not be
      copied to the second sprite.  That is done with the pushToSprite, 4 code lines below.
      The target sprite already has a black background and you would not see the four corners
      but there is still a problem. Those four corners would obscure the stars that happened
      along where they are.  So, they have to be lopped off with the pushToSprite setting
      black as the invisible color to leave them behind.  So, now, we have a black sprite
      that has had a slightly wide starfield pushed onto it and then a moon image that is
      now round, without any corners pressed on top of the start field.  Yes, that obscures
      some stars but that's exactly what should be happening.

      Step 4: So we are almost there.  Now, I have to draw in a moon shadow on all images
      except for full moon.  That's the David Bird code that I modified very slightly so that
      I could draw in the shadow part and not the part that was supposed to be showing.
      That's the AddMoonShadow routine.

      And, suddenly, we are done.

      Step 5 and onward: That just involves putting text on the screen.  Nothing special here.
      All of that stuff goes right onto the spriteBG and when I am done with the text, I
      throw spriteBG onto the screen for the happy users to see and watch the stars scroll by.

      This all happens every second and, last time I checked,
      takes about 60 ms.  That's very little of the one second I have to wait to do the next
      update so there is lots of just waiting around.

      Is it soup yet?  No.
      Is it soup yet?  No.
      Is it soup yet?  No.
      Is it soup yet?  No.
      Is it soup yet?  No.

      Cards, anyone?
  */
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
  // Testing //
  moonTimes = localtime(&mr.riseTime);
  strftime(chBuffer, sizeof(chBuffer), "%T", moonTimes);
  Serial.printf("Moon rise: %s - ", chBuffer);
  // End Testing //

  //Rise
  spriteBG.drawString("Rise:",  spriteBG.width() / 2 - 15, dispLine3, 4);
  if (mr.hasRise) {
    moonTimes = localtime(&mr.riseTime);
    strftime(chBuffer, sizeof(chBuffer), "%T", moonTimes);
    spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine3, 4);
  } else {
    spriteBG.drawString("None", spriteBG.width() / 2 + 54, dispLine3, 4);
  }

  // Testing //
  moonTimes = localtime(&mr.setTime);
  strftime(chBuffer, sizeof(chBuffer), "%T", moonTimes);
  Serial.printf("Moon set : %s\r\n", chBuffer);
  // End Testing //

  // Set
  spriteBG.drawString("Set:",   spriteBG.width() / 2 - 15, dispLine4, 4);
  if (mr.hasSet) {
    moonTimes = localtime(&mr.setTime);
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

  // Phase
  spriteBG.setTextColor(TFT_YELLOW, RGB565(0, 0, 166));
  spriteBG.drawString(MoonPhase(timeinfo.tm_mday, timeinfo.tm_mon + 1,
                                timeinfo.tm_year + 1900, Hemisphere),
                      4, dispLine6 + 5, 4);
  spriteBG.pushSprite(0, 0);

  //  Serial.printf("Loop took %lu ms.\r\n", millis() - startMillis);
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
///*******************************************************************************************/
//String MoonPhase(int d, int m, int y, String hemisphere)
///*******************************************************************************************/
//{
//  int c, e;
//  double jd;
//  int b;
//
//  if (m < 3) {
//    y--;
//    m += 12;
//  }
//  ++m;
//  c   = 365.25 * y;
//  e   = 30.6  * m;
//  jd  = c + e + d - 694039.09;  /* jd is total days elapsed */
//  jd /= 29.53059;               /* divide by the moon cycle (29.53 days) */
//  b   = jd;                     /* int(jd) -> b, take integer part of jd */
//  jd -= b;                   // subtract integer part to leave fractional part of original jd
//  b   = jd * 8 + 0.5;           /* scale fraction from 0-8 and round by adding 0.5 */
//  b   = b & 7;                  /* 0 and 8 are the same phase so modulo 8 for 0 */
//  if (hemisphere == "south") b = 7 - b;
//  if (b == 0) return TXT_MOON_NEW;              // New;              0%  illuminated
//  if (b == 1) return TXT_MOON_WAXING_CRESCENT;  // Waxing crescent; 25%  illuminated
//  if (b == 2) return TXT_MOON_FIRST_QUARTER;    // First quarter;   50%  illuminated
//  if (b == 3) return TXT_MOON_WAXING_GIBBOUS;   // Waxing gibbous;  75%  illuminated
//  if (b == 4) return TXT_MOON_FULL;             // Full;            100% illuminated
//  if (b == 5) return TXT_MOON_WANING_GIBBOUS;   // Waning gibbous;  75%  illuminated
//  if (b == 6) return TXT_MOON_THIRD_QUARTER;    // Third quarter;   50%  illuminated
//  if (b == 7) return TXT_MOON_WANING_CRESCENT;  // Waning crescent; 25%  illuminated
//  return "";
//}
/*******************************************************************************************/
String MoonPhase(int d, int m, int y, String hemisphere)
/*******************************************************************************************/
{
  //  New Moon: 0°
  //  Waxing Crescent: ~45°
  //  First Quarter: ~90°
  //  Waxing Gibbous: ~135°
  //  Full Moon: 180°
  //  Waning Gibbous: ~225°
  //  Last Quarter: ~270°
  //  Waning Crescent: ~315°
  time(&now);
  moon = moonPhase.getPhase(now);
  if (iInRange(moon.angle, 355, 5)) return TXT_MOON_NEW;
  if (iInRange(moon.angle, 6, 84)) return TXT_MOON_WAXING_CRESCENT;
  if (iInRange(moon.angle, 85, 95)) return TXT_MOON_FIRST_QUARTER;
  if (iInRange(moon.angle, 96, 174)) return TXT_MOON_WAXING_GIBBOUS;
  if (iInRange(moon.angle, 175, 185)) return TXT_MOON_FULL;
  if (iInRange(moon.angle, 186, 264)) return TXT_MOON_WANING_GIBBOUS;
  if (iInRange(moon.angle, 265, 275)) return TXT_MOON_THIRD_QUARTER;
  if (iInRange(moon.angle, 286, 354)) return TXT_MOON_WANING_CRESCENT;
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
// Draw a JPEG on the TFT, images will be cropped on the right & bottom if they do not fit
/*******************************************************************************************/
// This function assumes xpos,ypos is a valid screen coordinate.
// For convenience images that do not fit totally on the screen are cropped to the
//  nearest MCU size and may leave right/bottom borders.
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
    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
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
    if (( mcu_x + win_w ) <= spriteMoonInvis.width() &&
        ( mcu_y + win_h ) <= spriteMoonInvis.height())
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
    // Image has run off bottom of screen so abort decoding
    else if ( (mcu_y + win_h) >= spriteMoonInvis.height()) JpegDec.abort();

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
