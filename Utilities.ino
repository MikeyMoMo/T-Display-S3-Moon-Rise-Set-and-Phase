/***************************************************************************/
bool iInRange (int v, int low, int high)  // Inclusive
/***************************************************************************/
{
  return (low <= high) ? (v >= low && v <= high) : (v >= low || v <= high);
}
/***************************************************************************/
bool xInRange (int v, int low, int high)  // Exclusive
/***************************************************************************/
{
  return (low <= high) ? (v > low && v < high) : (v > low || v < high);
}
/***************************************************************************/
void ShowFuturePhases()
/***************************************************************************/
{
  // The following is a lambda statement.  It must be inside a local scope
  //  like itis defined here, inside of the ShowFuturePhases routine.
  //  Seems like lots of power here.  Don't know much about them.
  //  CoPilot sent me this little snippet of code to use.
  //  Then, CoPilot offered up a different lambda that check for both
  //   wrapping around, i.e., 0 from 355 to 5 and for normal checking
  //   around 180 from 175 to 185.
  //  Sweet. I am falling in love with CoPilot! It is my new friend!  ;-))
  //
  // Update:
  // However, for general purpose usage outside of this one routine, I
  //  recoded it as a general subroutine so any other code can call it.

  //  auto inRange = [](int v, int low, int high) {
  //    return (low <= high) ? (v >= low && v <= high) : (v >= low || v <= high);
  //  };

  //  New Moon: 0°
  //  Waxing Crescent: ~45°
  //  First Quarter: ~90°
  //  Waxing Gibbous: ~135°
  //  Full Moon: 180°
  //  Waning Gibbous: ~225°
  //  Last Quarter: ~270°
  //  Waning Crescent: ~315°

  tft.setTextDatum(TL_DATUM);
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, tft.width(), dispLine5, RGB565(150, 53, 73));
  time_t epoch = get_epoch_time();
  int useLine = dispLine1;

  bool NM_Done = false;
  bool FQ_Done = false;
  bool FM_Done = false;
  bool LQ_Done = false;
  tft.setTextColor(TFT_WHITE, RGB565(150, 53, 73));

  for (int i = 0; i < 720; i++)  // Test the next 29.5 days
  {
    moon = moonPhase.getPhase(epoch);

    tmlocalTime = localtime(&epoch);
    // Testing
    //    strftime(chBuffer, sizeof(chBuffer), "%a %b %d %R", tmlocalTime);
    //    Serial.printf("Phase angle %3i, Date %s\r\n", moon.angle, chBuffer);
    // End Testing

    // Make it right again!
    strftime(chBuffer, sizeof(chBuffer), "%a %b %d", tmlocalTime);

    if (iInRange(moon.angle, 355, 5)) {
      if (!NM_Done) {
        NM_Done = true;
        tft.drawString("New Moon", 4, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (iInRange(moon.angle, 85, 95)) {
      if (!FQ_Done) {
        FQ_Done = true;
        tft.drawString("First Quarter", 4, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (iInRange(moon.angle, 175, 185)) {
      if (!FM_Done) {
        FM_Done = true;
        tft.drawString("Full Moon", 4, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (iInRange(moon.angle, 265, 285)) {
      if (!LQ_Done) {
        LQ_Done = true;
        tft.drawString("Last Quarter", 4, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    epoch += 3600;  // Check next hour.
  }
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Press either button to return.", 1, dispLine6, 4);
}
/*******************************************************************************************/
void CheckButtons()
/*******************************************************************************************/
{
  int pressLength = 0;
  int prevBL = tftBL_Lvl;

  if ((digitalRead(topButton) == BUTTON_PRESSED) &&
      (digitalRead(bottomButton) == BUTTON_PRESSED)) {
    Serial.printf("%lu - 1 Both pressed, do menuing.\r\n", millis());
    doMenu();
    return;
  }

  tft.setTextDatum(TL_DATUM);
  // Allow the user to adjust backlight brightness.
  while ((digitalRead(topButton) == BUTTON_PRESSED) &&
         (tftBL_Lvl <= MAX_BRIGHTNESS))
  {
    delay(100);
    if (digitalRead(bottomButton) == BUTTON_PRESSED) {
      Serial.printf("%lu - 2 Both pressed, do menuing.\r\n", millis());
      doMenu();
      return;
    }

    // Increase.
    pressLength++;
    if (pressLength > 10) BLchange = 8;
    else if (pressLength > 5) BLchange = 4;
    else BLchange = 2;
    tftBL_Lvl += BLchange;
    if (prevBL == 0) {
      tft.writecommand(ST7789_DISPON);  // Turn on display hardware.
      prevBL = tftBL_Lvl;
    }
    if (tftBL_Lvl > MAX_BRIGHTNESS)
      tftBL_Lvl = MAX_BRIGHTNESS;

    ledcWrite(TFT_BL, tftBL_Lvl);
    BLChangeMillis = millis();
    sprintf(chBLChange, "Brightness %i", tftBL_Lvl);
    // Serial.printf("Brightness %i\r\n", tftBL_Lvl);
    tft.setTextColor(TFT_YELLOW, RGB565(0, 0, 166));
    tft.setTextPadding(tft.width());
    tft.drawString(chBLChange, 4, dispLine6 + 5, 4);
    tft.setTextPadding(0);
    delay(100);
  }
  while ((digitalRead(bottomButton)) == BUTTON_PRESSED &&
         (tftBL_Lvl >= MIN_BRIGHTNESS))
  {
    delay(100);
    if (digitalRead(topButton) == BUTTON_PRESSED) {
      Serial.printf("%lu - 3 Both pressed, do menuing.\r\n", millis());
      doMenu();
      return;
    }

    // Decrease.
    pressLength++;
    if (pressLength > 10) BLchange = 8;
    else if (pressLength > 5) BLchange = 4;
    else BLchange = 2;
    tftBL_Lvl -= BLchange;

    if (tftBL_Lvl < MIN_BRIGHTNESS)
      tftBL_Lvl = MIN_BRIGHTNESS;

    ledcWrite(TFT_BL, tftBL_Lvl);
    BLChangeMillis = millis();
    sprintf(chBLChange, "Brightness %i", tftBL_Lvl);
    // Serial.printf("Brightness %i\r\n", tftBL_Lvl);
    tft.setTextColor(TFT_YELLOW, RGB565(0, 0, 166));
    tft.setTextPadding(tft.width());
    tft.drawString(chBLChange, 4, dispLine6 + 5, 4);
    tft.setTextPadding(0);
    delay(100);
  }
}
/*******************************************************************************************/
void doMenu()
/*******************************************************************************************/
{
  int highlight = 1;
  //  Serial.printf("%lu - In menuing.\r\n", millis());

  tft.fillScreen(TFT_BLACK);

  // Now, wait for unpress of both buttons.
  while ((digitalRead(topButton) == BUTTON_PRESSED) ||
         (digitalRead(bottomButton) == BUTTON_PRESSED));

  menuHide = millis() + MENU_HIDE_TIME;

  while (millis() < menuHide) {
    spriteMenu.fillSprite(TFT_BLACK);
    if (highlight == 1)
      spriteMenu.setTextColor(TFT_BLACK, TFT_YELLOW);
    else
      spriteMenu.setTextColor(TFT_YELLOW, TFT_BLACK);
    spriteMenu.drawString("Show Moon Phase Dates", 0, 10, 4);
    spriteMenu.drawFastHLine(0, SPR_MENU_HEIGHT - 1, tft.width(), TFT_YELLOW);
    spriteMenu.pushSprite(0, 10);

    spriteMenu.fillSprite(TFT_BLACK);
    if (highlight == 2)
      spriteMenu.setTextColor(TFT_BLACK, TFT_YELLOW);
    else
      spriteMenu.setTextColor(TFT_YELLOW, TFT_BLACK);
    spriteMenu.drawString("Show Sun Rise/Set Times", 0, 10, 4);
    spriteMenu.drawFastHLine(0, SPR_MENU_HEIGHT - 1, tft.width(), TFT_YELLOW);
    spriteMenu.pushSprite(0, 50);

    spriteMenu.fillSprite(TFT_BLACK);
    if (highlight == 3)
      spriteMenu.setTextColor(TFT_BLACK, TFT_YELLOW);
    else
      spriteMenu.setTextColor(TFT_YELLOW, TFT_BLACK);
    spriteMenu.drawString(" ", 0, 10, 4);
    spriteMenu.drawFastHLine(0, SPR_MENU_HEIGHT - 1, tft.width(), TFT_YELLOW);
    spriteMenu.pushSprite(0, 90);

    spriteMenu.fillSprite(TFT_BLACK);
    if (highlight == 4)
      spriteMenu.setTextColor(TFT_BLACK, TFT_YELLOW);
    else
      spriteMenu.setTextColor(TFT_YELLOW, TFT_BLACK);
    spriteMenu.drawString("Return", 0, 10, 4);
    spriteMenu.drawFastHLine(0, SPR_MENU_HEIGHT - 1, tft.width(), TFT_YELLOW);
    spriteMenu.pushSprite(0, 130);

    // Select item
    if (digitalRead(bottomButton) == BUTTON_PRESSED) {  // Bottom button pressed.
      delay(50);
      if (digitalRead(bottomButton) == BUTTON_PRESSED) {  // Still pressed?
        highlight++; if (highlight == 3) highlight = 4;  // Skip empty choice(s).
        if (highlight > 4) highlight = 1;
        menuHide = millis() + MENU_HIDE_TIME;  // Extend auto close time.
      }
      // Wait for unpress to avoid multiple actions.
      while (digitalRead(bottomButton) == BUTTON_PRESSED);
    }

    // Execute selected on top button press

    //#define BUTTON_PRESSED 0
    //#define BUTTON_NOT_PRESSED 1

    if (digitalRead(topButton) == BUTTON_PRESSED) {  // Top button pressed.
      delay(50);
      if (digitalRead(topButton) == BUTTON_PRESSED) {  // Still pressed?  Do menu.
        while (digitalRead(topButton) == BUTTON_PRESSED);  // Wait for unpress.
        switch (highlight)
        {
          case 1:
            ShowFuturePhases();
            while ((digitalRead(topButton) == BUTTON_NOT_PRESSED) &&  // Wait for button press
                   (digitalRead(bottomButton) == BUTTON_NOT_PRESSED));  // to exit.
            while ((digitalRead(topButton) == BUTTON_PRESSED) ||  // Wait for unpress of
                   (digitalRead(bottomButton) == BUTTON_PRESSED));  // all buttons.
            return; break;

          case 2:
            ShowSunTimes();
            while ((digitalRead(topButton) == BUTTON_NOT_PRESSED) &&  // Wait for button press
                   (digitalRead(bottomButton) == BUTTON_NOT_PRESSED));  // to exit.
            while ((digitalRead(topButton) == BUTTON_PRESSED) ||  // Wait for unpress of
                   (digitalRead(bottomButton) == BUTTON_PRESSED));  // all buttons.
            return; break;

          // case 3:
          //while ((digitalRead(topButton) == BUTTON_NOT_PRESSED) &&  // Wait for button press
          //       (digitalRead(bottomButton) == BUTTON_NOT_PRESSED));  // to exit.
          //   return; break;

          case 4:  // Nothing to see here.  Go about your business! Yeah, that means you!!!
            return; break;

          default:
            return; break;
        }
      }
      menuHide = millis() + MENU_HIDE_TIME;  // Extend auto close time.
    }
  }
}
/*******************************************************************************************/
void setHourBrightness()
/*******************************************************************************************/
{
  preferences.begin("Hourlys", RO_MODE);

  // If this hour has not been set, 1000 will be returned.  If it is
  //  and it is late, turn off the display.  It can be turned back on
  //  if anyone is awake and want to set it to be on for the hour.
  sprintf(chHour, "%d", iHour);
  tftBL_Lvl = preferences.getInt(chHour, 1000);  // Never been set?
  Serial.printf("%02i:%02i:%02i - Read brightness level for hour %i of %i\r\n",
                iHour, iMin, iSec, iHour, tftBL_Lvl);
  preferences.end();

  SleepTime = xInRange(iHour, SleepHour, WakeupHour);
  //  if (WakeupHour > SleepHour)
  //    SleepTime = (iHour >= WakeupHour || iHour <= SleepHour);
  //  else
  //    SleepTime = (iHour >= WakeupHour && iHour <= SleepHour);
  Serial.print("In sleep time? ");
  Serial.println(SleepTime ? "Yes" : "No");

  // If unchanged value still in there, pick default value.
  // If less than 500 then user set a value, let it be.
  if (tftBL_Lvl > 500) {  // Is it unchanged?
    if (SleepTime)
      tftBL_Lvl = 0;
    else
      tftBL_Lvl = defaultBright;
    // Serial.printf("tftBL_Lvl now set to %i\r\n", tftBL_Lvl);
  }

  ledcWrite(TFT_BL, tftBL_Lvl);  // Activate whatever was decided on.
  Serial.printf("%02i:%02i:%02i - Brightness level for hour %i set to %i\r\n",
                iHour, iMin, iSec, iHour, tftBL_Lvl);
}
/*******************************************************************************************/
void SaveOptions()
/*******************************************************************************************/
{
  // On the modulo 10 minute, see if saving the brightness value is needed.
  // Also on the second to last second of the hour.
  // Since this was an evolution rather than a design, it turns out that the
  //  brightness during dark time gets written to the Hourlys preferences values
  //  but the Hourlys during display on time do not get written to the Hourlys
  //  preferences entries.  It only happens the first time so not a big deal but if
  //  you type H into the Serial Monitor, you will see how it looks.  Odd, yes, but
  //  totally workable.
  if ((iMin > 0 && iMin % 10 == 0 && iSec == 0) ||
      (iMin == 59 && iSec == 58)) {
    preferences.begin("Hourlys", RW_MODE);
    sprintf(chHour, "%d", iHour);
    nTemp = preferences.getInt(chHour, defaultBright);
    if (nTemp != tftBL_Lvl) {
      preferences.putInt(chHour, tftBL_Lvl);
      Serial.printf("%02i:%02i:%02i - Saved BL level for hour %2i: %i\r\n",
                    iHour, iMin, iSec, iHour, tftBL_Lvl);
    }
    preferences.end();

    // preferences.begin("UserPrefs", RW_MODE);
    // bool bTemp = preferences.getBool("showVolts", false);
    // if (bTemp != showVolts) {
    //   preferences.putBool("showVolts", showVolts);
    //   Serial.printf("%02i:%02i:%02i - Saved new showVolts setting: %s.\r\n",
    //                 iHour, iMin, iSec, showVolts ? "true" : "false");
    // }
    // preferences.end();
  }
}
/*******************************************************************************************/
void printVers()
/*******************************************************************************************/
{
  int      lastDot, lastV;
  String   sTemp;

  //  Serial.println(__FILENAME__);  // Same as __FILE__
  sTemp = String(__FILE__);
  // Get rid of the trailing .ino tab name. In this case, "\Utilities.ino"
  sTemp = sTemp.substring(0, sTemp.lastIndexOf("\\"));
  Serial.print("Running from: "); Serial.println(sTemp);

  sTemp = String(__FILE__);  // Start again for the version number.
  lastDot = sTemp.lastIndexOf(".");
  if (lastDot > -1) {  // Found a dot.  Thank goodness!
    lastV = sTemp.lastIndexOf("v");  // Find start of version number
    if (lastV > -1) {  // Oh, good, found version number, too
      sVer = sTemp.substring(lastV + 1, lastDot); // Pick up version number
      lastV = sVer.lastIndexOf("\\");
      if (lastV > -1) sVer = sVer.substring(0, lastV);
    } else {
      sVer = "0.00";  // Unknown version.
    }
  } else {
    sVer = "n/a";  // Something badly wrong here!
  }
  Serial.print("Version " + sVer + ", ");
  Serial.printf("Compiled on %s at %s.\r\n", __DATE__, __TIME__);
}
/*******************************************************************************************/
void AddStars()
/*******************************************************************************************/
{
  static int timeForStars = 0;

  // y value is optional (default is 0, so no up/down scrolling here.
  // We stand on our rights!).
  spriteSF_Base.scroll(-1);  // Scroll the entire starfield left by one pixel.

  // Was 3 then 1, now 2.  Trying to get the right starfield look.
  if (timeForStars++ == 2) {
    timeForStars = 0;
    // Redraw new stars appearing on the right
    count = random(4);
    //    Serial.printf("Creating %i stars.\r\n", count);

    // Notes on the following for loop:
    // I took out the final color so it will use whatever is around the dot to be put in.
    // That means that the alpha blend will use whatever is there instead of forcing black.
    // If a new star goes in right next to another one, it will see some white as well as
    //  some black.  So alpha blend can do its job properly. As it should be!
    //
    // Also note:
    // I made the spriteSF_Base wider than the spriteSF (SF stands for star field). Then
    //  I make new stars on the right end of spriteSF_Base but they can't be seen right away.
    // Later, they scroll into view as if appearing in a window, as it should be.  The
    //  same effect happens when a star scrolls out from behind the disk of the moon.  It
    //  is occluded behind the moon then scrolls back into view on the left side of the
    //  disk as the star field scrolls left.  As it should be!
    for (int i = 0; i < count; i++) {  // Few new stars each frame
      brightness = random(155) + 100;  // Brightness range from 100 to 255.
      // Serial.printf("Brightness %i ", brightness);
      spriteSF_Base.fillSmoothCircle(spriteSF_Base.width() - 4, random(spriteSF.height()),
                                     random(3), RGB565(brightness, brightness, brightness));
    }
  }
}
/*******************************************************************************************/
void HandleSerialInput()
/*******************************************************************************************/
{
  char input = Serial.read();  // Read one character from the serial input
  int i, iHTemp;

  input = toupper(input); // Convert the character to uppercase
  if (input != '\r' && input != '\n')
    Serial.printf("User input %c\r\n", input);  // Show user input

  // In use:
  //     D turns on the display at default brightness.  If already on, turns it off.
  //     H (testing) to see all Hourly brightness values on the Monitor.");
  //     ? for this list.  Upper or Lower case OK.");
  //     default action - Show a message that the user is drunk!  ;-))

  switch (input) {

    case 'D':
      if (tftBL_Lvl == 0) {
        Serial.println("Turning screen on.");
        tft.writecommand(ST7789_DISPON);  // Turn on display hardware.
        tftBL_Lvl = defaultBright;
        prevBL_Lvl = tftBL_Lvl;
        ledcWrite(TFT_BL, tftBL_Lvl);  // Turn on the backlight
      } else {
        tft.fillScreen(TFT_BLACK);
        ledcWrite(TFT_BL, defaultBright);  // Activate whatever was decided on.
        delay(10);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("By User request,", tft.width() / 2, dispLine2, 4);
        tft.drawString("turning off", tft.width() / 2, dispLine3, 4);
        tft.drawString("screen.", tft.width() / 2, dispLine4, 4);
        Serial.println("Turning screen off.");
        delay(5000);
        tft.fillScreen(TFT_BLACK);
        ledcWrite(TFT_BL, 0);  // Turn off the backlight
        tftBL_Lvl = 0;
        prevBL_Lvl = tftBL_Lvl;
        tft.writecommand(ST7789_DISPOFF);  // Turn off display hardware.
      }
      showInputOptions();
      break;

    case 'H':  // Show hourly brightness values
      preferences.begin("Hourlys", RO_MODE);
      for (i = 0; i < 24; i++) {
        sprintf(chHour, "%d", i);
        iHTemp = preferences.getInt(chHour, 1000);  // Never been set?
        Serial.printf("Brightness level for hour %2i is %4i\r\n", i, iHTemp);
      }
      preferences.end();
      showInputOptions();
      break;

    case '?':
      showInputOptionsFull();
      break;

    case '\r':
    case '\n':
      break;

    default:
      Serial.printf("*>> Unknown input \'%c\'!\r\n", input);  // Handle unknown input
      showInputOptions();
      break;
  }
}
/*******************************************************************************************/
void showInputOptions()
/*******************************************************************************************/
{
  Serial.println("Enter ? to see the full list of options.");
}
/*******************************************************************************************/
void showInputOptionsFull()
/*******************************************************************************************/
{
  Serial.println("Enter D to turn on the display at default brightness or off.");
  Serial.println("Enter H (testing) to see all Hourly brightness values on the Monitor.");
  Serial.println("Enter ? for this list.  Upper or Lower case OK.");
  Serial.println("If you make an unknown entry, this list is printed on the monitor.\r\n");
}
/*******************************************************************************************/
void ShowSunTimes()
/*******************************************************************************************/
{
  time_t riseEpoch, setEpoch, upTime;
  int    iHour, iMin, iSec;

  tft.setTextDatum(TL_DATUM);
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, tft.width(), dispLine5, RGB565(150, 53, 73));
  time_t epoch = get_epoch_time();
  tft.setTextColor(TFT_WHITE, RGB565(150, 53, 73));

  time(&now);

  sr.calculate(lat, lon, now);  // Get all of the answsers

  // Rise time
  riseEpoch = sr.riseTime;
  sunTimes = localtime(&riseEpoch);
  strftime(chBuffer, sizeof(chBuffer), "%T", sunTimes);
  tft.drawString("Sunrise at ", 4, dispLine1, 4);
  tft.drawString(chBuffer, 170, dispLine1, 4);
  Serial.printf("Sun rise: %s - ", chBuffer);

  setEpoch = sr.setTime;
  sunTimes = localtime(&setEpoch);
  strftime(chBuffer, sizeof(chBuffer), "%T", sunTimes);
  tft.drawString("Sunset  at ", 4, dispLine2, 4);
  tft.drawString(chBuffer, 170, dispLine2, 4);
  Serial.printf("Sun set: %s\r\n", chBuffer);

  //  Serial.printf("Rise %lu, Set %lu\r\n", riseEpoch, setEpoch);
  //  Serial.println(riseEpoch);
  //  Serial.println(setEpoch);
  //  upTime = riseEpoch - setEpoch;  // This is the closest so it may be reversed.  Check!
  //  if (upTime < 0) upTime = setEpoch - riseEpoch;
  //  iSec = upTime % 60;
  //  upTime /= 60;  // Now minutes.
  //  iMin = upTime % 60;
  //  upTime /= 60;  // Now hours.
  //  iHour = upTime % 24;
  //  Serial.printf("Day length %i:%i:%i\r\n", iHour, iMin, iSec);

  spriteBG.drawString("Visible:", spriteBG.width() / 2 - 15, dispLine3, 4);
  if (sr.isVisible)
    spriteBG.drawString("Yes", spriteBG.width() / 2 + 74, dispLine3, 4);
  else
    spriteBG.drawString("No",  spriteBG.width() / 2 + 74, dispLine3, 4);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Press either button to return.", 1, dispLine6, 4);
}
/*******************************************************************************************/
void initTime()
/*******************************************************************************************/
{
  sntp_set_sync_interval(21601358);  // Get updated time every 6 hours plus a little.
  //  sntp_set_sync_interval(86405432);  // 1 day in ms plus a little.
  //  sntp_set_time_sync_notification_cb(timeSyncCallback);
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
double AgeOfTheMoon(int d, int m, int y)
/*******************************************************************************************/
{
  int j = JulianDate(d, m, y);
  //Calculate the approximate phase of the moon
  double Phase = (j + 4.867) / 29.53059;
  return ((Phase - (int) Phase) + .5);
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
///*******************************************************************************************/
//double MoonAge(int d, int m, int y)
///*******************************************************************************************/
//{
//  // This routine came from:
//  //  https://www.codeproject.com/Articles/100174/Calculate-and-Draw-Moon-Phase
//  // I want to compare this with the one that came from David Bird.
//  double ag, ip;
//  int j = JulianDate(d, m, y);
//
//  //Calculate the approximate phase of the moon
//  ip = (j + 4.867) / 29.53059;
//  ip = ip - floor(ip);
//  // After several trials I've seen to add the following lines,
//  //  which gave the result was not bad.
//  if (ip < 0.5)
//    ag = ip * 29.53059 + 29.53059 / 2;
//  else
//    ag = ip * 29.53059 - 29.53059 / 2;
//  // Moon's age in days
//  ag = floor(ag) + 1;
//  return ag;
//}
/*******************************************************************************************/
void HourDance()
/*******************************************************************************************/
{
  //  Serial.printf("It is now %02i:00\r\n", iHour);

  // Do HourDance after updating the display to xx:00:00
  for (int i = 0; i < 2; i++) {
    tft.invertDisplay(false); delay(200);
    tft.invertDisplay(true); delay(200);
  }
}
/*******************************************************************************************/
bool jpg_output_Sprite(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
/*******************************************************************************************/
{
  // This function is be called during decoding of the jpeg file to
  // render each block to the sprite spriteBG.  It us used to load all of the BG pictures.

  // Stop further decoding as image is running off bottom of sprite
  if ( y >= spriteMoonInvis.height() ) return 0;

  // This function will clip the image block rendering
  //  automatically at the sprite boundaries
  spriteMoonInvis.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}
