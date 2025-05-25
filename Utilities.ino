/***************************************************************************/
void Build_and_Show()
/***************************************************************************/
{
  if (iSec % 10 == 0) AddStars();  // Adds stars to spriteSF_Base.
  /* Well, the following needs a little explanation.  It took a while to figure out the order
      of stuff to do and I want to remember and you to know what is happening here.  It is
      fairly simple to understand now that it is complete.

      First, a note: spriteSF_Base is some 20 pixels wider than spriteSF.

      Step 1: I have to put the starfield that is kept in spriteSF_Base onto the
      display starfield.  spriteSF_Base is a bit wider then the display starfield sprite
      (spriteSF).  That's so I can create the "stars" and have them appear to scroll
      onto the screen rather than suddenly appear, fully formed on the right edge of the moon
      window. So extra stars are waiting, hidden, to come onto the screen in a minute or two.
      That's done by the next code statement of pushToSprite.  The right end of the Base
      starfield gets truncated and that's exactly what I wanted to happen. So, now, we have
      a  black background and a starfield in spriteBG sprite.

      Note again: I have to determine the moon phase picture to use, then...
      Step 2: I have to push the moon jpg into a sprite.  But, since all jpgs
      are square, it has four sorta-triangles on it in four places -- the "corners".
      I had to edit these and make them totally black for the next step.

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

      And, suddenly, we are done.

      Step 4 and onward: That just involves putting text on the screen. Nothing special here.
      All of that stuff goes right onto the spriteBG and when I am done with the text, I
      throw spriteBG on the screen for the happy users to see and watch the stars scroll by.

      This all happens every second and, last time I checked, takes about 60 ms.
      That's very little of the one second I have to wait to do the next update so
      there is lots of just waiting around...

      Is it soup yet?  No.
      Is it soup yet?  No.
      Is it soup yet?  No.
      Is it soup yet?  No.
      Is it soup yet?  No.

      Cards, anyone?
  */

  spriteSF_Base.pushToSprite(&spriteSF, 22, 22);
  spriteMoonInvis.fillSprite(TFT_BLACK);

  time_t epoch = get_epoch_time();
  moon = moonPhase.getPhase(epoch);
  Phase = moon.angle;
#if defined DO_DEBUG_PRINTS
  if (prevHour != iHour)
    Serial.printf("moon.angle returned %i\r\n", Phase);
#endif
  if (Hemisphere == "south") Phase = 1 - Phase;
  Phase = Phase & ~1;  // Pictures are in 2º increments so "and off" the 1 bit.

  // Draw a jpeg image stored in LittleFS at x,y named by Phase.
  sprintf(moonPhasePic, "/m%03i.jpg", Phase);
#if defined DO_DEBUG_PRINTS
  if (prevHour != iHour)
    Serial.printf("Moon phase pic: %s\r\n", moonPhasePic);
#endif
  JPB_RC = TJpgDec.drawFsJpg(0, 0, moonPhasePic, LittleFS);
  if (JPB_RC != JDR_OK) Serial.printf("Picture %s load failed for moon phase with "
                                        "error code %i\r\n", moonPhasePic, JPB_RC);
  // While the disk looks totally dark, draw a circle so we can see where the moon
  //  is even though we cannot see it.
  if (iInRange(Phase, 330, 28))
    spriteMoonInvis.drawCircle(50, 50, 39, TFT_WHITE);  // The moon width is 78 pixels.
  spriteMoonInvis.pushToSprite(&spriteSF, 19, 19, TFT_BLACK);
  // Leave off corners.
  spriteSF.pushToSprite(&spriteBG, 0, 0);  // , RGB565(0, 0, 166));

  //Date
  strftime(chBuffer, sizeof(chBuffer), "%D", &timeinfo);
  //  Serial.print(chBuffer);  Serial.print(", ");
  spriteBG.drawString("Date:",  spriteBG.width() / 2 - 15, dispLine2, 4);
  spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine2, 4);

  // Time
  strftime(chBuffer, sizeof(chBuffer), "%T", &timeinfo);
  //  Serial.println(chBuffer);
  spriteBG.drawString("Time:",  spriteBG.width() / 2 - 15, dispLine1, 4);
  spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine1, 4);

  // Moon times
  time(&now);

  mr.calculate(lat, lon, now);  // Get all of the answsers

  spriteBG.setTextColor(TFT_WHITE, RGB565(0, 0, 166));

#if defined DO_DEBUG_PRINTS
  moonTimes = localtime(&mr.riseTime);
  strftime(chBuffer, sizeof(chBuffer), "%x %X", moonTimes);
  if (prevHour != iHour) {
    Serial.printf("Moon rise: %s - ", chBuffer);
  }
#endif

  // Moon Rise
  spriteBG.drawString("Rise:",  spriteBG.width() / 2 - 15, dispLine3, 4);
  if (mr.riseTime < now)
    //                        Medium grey      ,      My Blue
    spriteBG.setTextColor(RGB565(170, 170, 170), RGB565(0, 0, 166));
  if (mr.hasRise) {
    moonTimes = localtime(&mr.riseTime);
    strftime(chBuffer, sizeof(chBuffer), "%T", moonTimes);
    spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine3, 4);
  } else {
    spriteBG.drawString("None", spriteBG.width() / 2 + 54, dispLine3, 4);
  }

#if defined DO_DEBUG_PRINTS
  moonTimes = localtime(&mr.setTime);
  strftime(chBuffer, sizeof(chBuffer), "%x %X", moonTimes);
  if (prevHour != iHour)
    Serial.printf("Moon set: %s\r\n", chBuffer);
#endif

  spriteBG.setTextColor(TFT_WHITE, RGB565(0, 0, 166));
  // Moon Set
  spriteBG.drawString("Set:",   spriteBG.width() / 2 - 15, dispLine4, 4);
  if (mr.setTime < now)
    //                        Medium grey      ,      My Blue
    spriteBG.setTextColor(RGB565(170, 170, 170), RGB565(0, 0, 166));
  if (mr.hasSet) {
    moonTimes = localtime(&mr.setTime);
    strftime(chBuffer, sizeof(chBuffer), "%T", moonTimes);
    spriteBG.drawString(chBuffer, spriteBG.width() / 2 + 54, dispLine4, 4);
  } else {
    spriteBG.drawString("None", spriteBG.width() / 2 + 50, dispLine4, 4);
  }

  spriteBG.setTextColor(TFT_WHITE, RGB565(0, 0, 166));
  spriteBG.drawString("Visible:", spriteBG.width() / 2 - 15, dispLine5, 4);
  if (mr.isVisible)
    spriteBG.drawString("Yes",   spriteBG.width() / 2 + 74, dispLine5, 4);
  else
    spriteBG.drawString("No",   spriteBG.width() / 2 + 74, dispLine5, 4);

  // Phase
  spriteBG.fillRect(0, dispLine6, tft.width(), tft.height(), RGB565(0, 0, 166));
  spriteBG.setTextColor(TFT_YELLOW, RGB565(0, 0, 166));
  if (millis() > BLChangeMillis + showPhaseDelay) {
    spriteBG.drawString(MoonPhase(timeinfo.tm_mday, timeinfo.tm_mon + 1,
                                  timeinfo.tm_year + 1900, Hemisphere),
                        4, dispLine6 + 5, 4);
  } else {
    tft.setTextDatum(TL_DATUM);
    spriteBG.setTextPadding(tft.width());
    spriteBG.drawString(chBLChange, 4, dispLine6 + 5, 4);
    spriteBG.setTextPadding(0);
  }
  spriteBG.pushSprite(0, 0);
}
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

  if ((digitalRead(topButton) == BUTTON_PRESSED) ||
      (digitalRead(bottomButton) == BUTTON_PRESSED)) {
    delay(250);
    if ((digitalRead(topButton) == BUTTON_PRESSED) &&
        (digitalRead(bottomButton) == BUTTON_PRESSED)) {
      Serial.printf("%lu - 1 Both pressed, do menuing.\r\n", millis());
      while ((digitalRead(topButton) == BUTTON_PRESSED) ||
             (digitalRead(bottomButton) == BUTTON_PRESSED));
      doMenu();
      return;
    }
  }

  tft.setTextDatum(TL_DATUM);
  // Allow the user to adjust backlight brightness.
  while ((digitalRead(topButton) == BUTTON_PRESSED) &&
         (tftBL_Lvl <= MAX_BRIGHTNESS))
  {
    delay(200);
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
      Serial.println("Turning on display");
      delay(100);
      prevBL = tftBL_Lvl;
    }
    if (tftBL_Lvl > MAX_BRIGHTNESS)
      tftBL_Lvl = MAX_BRIGHTNESS;
    ledcWrite(TFT_BL, tftBL_Lvl);
    BLChangeMillis = millis();
    sprintf(chBLChange, "Brightness: %i", tftBL_Lvl);
    // Serial.println(chBLChange);
    Build_and_Show();
  }

  while ((digitalRead(bottomButton)) == BUTTON_PRESSED &&
         (tftBL_Lvl >= MIN_BRIGHTNESS))
  {
    delay(200);
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

    if (tftBL_Lvl < MIN_BRIGHTNESS) tftBL_Lvl = MIN_BRIGHTNESS;

    ledcWrite(TFT_BL, tftBL_Lvl);
    BLChangeMillis = millis();
    if (tftBL_Lvl == MIN_BRIGHTNESS && prevBL > MIN_BRIGHTNESS) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.setTextDatum(TC_DATUM);
      ledcWrite(TFT_BL, 200);
      tft.drawString("User commanded to", tft.width() / 2, dispLine2, 4);
      tft.drawString("turn off screen.", tft.width() / 2, dispLine4, 4);
      Serial.println("User commanded screen off.");
      delay(3000);
      ledcWrite(TFT_BL, tftBL_Lvl);
      tft.writecommand(ST7789_DISPOFF);  // Turn off display hardware.
      delay(100);
    } else {
      sprintf(chBLChange, "Brightness: %i", tftBL_Lvl);
      // Serial.println(chBLChan
      Build_and_Show();
    }
    prevBL = tftBL_Lvl;
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
#if defined DO_DEBUG_PRINTS
  Serial.print("In sleep time? ");
  Serial.println(SleepTime ? "Yes" : "No");
#endif
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

    // Notes on the following for loop:
    // I took out the final color so it will use whatever is around the dot to be put in.
    // That means that the alpha blend will use whatever is there instead of forcing black.
    // If a new star goes in right next to another one, it will see some white as well as
    //  some black.  So alpha blend can do its job properly. As it should be!
    //
    // Also note:
    // I made the spriteSF_Base wider than the spriteSF (SF stands for star field). Then
    //  I make new stars on the right side of spriteSF_Base but they can't be seen yet.
    // Later, they scroll into view as if appearing in a window, as it should be.  The
    //  same effect happens when a star scrolls out from behind the disk of the moon.  It
    //  is occluded behind the moon then scrolls back into view on the left side of the
    //  disk as the star field scrolls left.  As it should be!
    count = random(4);
    //    Serial.printf("Creating %i stars.\r\n", count);
    for (int i = 0; i <= count; i++) {  // Few new stars each frame
      brightness = random(155) + 100;  // Brightness range from 100 to 255.
      // Serial.printf("Brightness %i ", brightness);
      switch (i) {
        case 1:  // Almost always going to be a 1
          spriteSF_Base.fillSmoothCircle(spriteSF_Base.width() - 4,
                                         random(spriteSF.height()), random(3),
                                         RGB565(brightness, brightness, brightness));
          break;
        case 2:  // Less often a 2 making a blue dot
          spriteSF_Base.fillSmoothCircle(spriteSF_Base.width() - 4,
                                         random(spriteSF.height()), random(3),
                                         RGB565(234, 245, 255));  // Little bit Blue.
          Serial.println("Made a blue circle.");
          break;
        case 3:  // Even more rare, a 3 making a redish/pinkish dot
          spriteSF_Base.fillSmoothCircle(spriteSF_Base.width() - 4,
                                         random(spriteSF.height()), random(3),
                                         RGB565(255, 211, 204));  // Little bit Red.
          Serial.println("Made a red circle.");
          break;

        default:  // Create no stars or no more stars.
//          Serial.printf("Got to switch default with i %i\r\n", i);
          break;
      }
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
        delay(3000);
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

    case 'I':  // Show some internal info.
      //      Serial.print("Picture loaded: /m"); Serial.print(usePic);
      //      Serial.print(".jpg for phase ");    Serial.println(Phase);
      Serial.printf("Picture loaded m%i.jpg\r\n", Phase);
      //      Serial.printf("Moon phase: %f\r\n", Phase);
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
  Serial.printf("Sunrise: %s - ", chBuffer);

  setEpoch = sr.setTime;
  sunTimes = localtime(&setEpoch);
  strftime(chBuffer, sizeof(chBuffer), "%T", sunTimes);
  tft.drawString("Sunset  at ", 4, dispLine2, 4);
  tft.drawString(chBuffer, 170, dispLine2, 4);
  Serial.printf("Sun set: %s\r\n", chBuffer);

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
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

#if defined CONFIG_FOR_JOE
  configTzTime("PST8PDT,M3.2.0,M11.1.0", "pool.ntp.org");
#else
  configTzTime("PHT-8", "oceania.pool.ntp.org");
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
  if (iInRange(moon.angle, 355,   5)) return TXT_MOON_NEW;
  if (iInRange(moon.angle,   6,  84)) return TXT_MOON_WAXING_CRESCENT;
  if (iInRange(moon.angle,  85,  95)) return TXT_MOON_FIRST_QUARTER;
  if (iInRange(moon.angle,  96, 174)) return TXT_MOON_WAXING_GIBBOUS;
  if (iInRange(moon.angle, 175, 185)) return TXT_MOON_FULL;
  if (iInRange(moon.angle, 186, 264)) return TXT_MOON_WANING_GIBBOUS;
  if (iInRange(moon.angle, 265, 275)) return TXT_MOON_THIRD_QUARTER;
  if (iInRange(moon.angle, 286, 354)) return TXT_MOON_WANING_CRESCENT;
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
time_t get_epoch_time()
/*******************************************************************************************/
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}
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
