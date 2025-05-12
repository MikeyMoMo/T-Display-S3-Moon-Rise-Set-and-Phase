/***************************************************************************/
void ShowFuturePhases()
/***************************************************************************/
{
  // This is a lamda statement.  It must be inside a local scope like it
  //  is defined here, inside of the ShowFuturePhases routine.  Seems like
  //  lots of power here.  Don't know much about them.  CoPilot sent me this
  //  little snippet of code to use.
  //  Well, CoPilot offered up a different lamda that check for both
  //   wrapping around 0 from 355 to 5 and for normal checking around 180
  //   from 175 to 185.  Sweet. I am falling in love with CoPilot!  ;-))
  auto inRange = [](int v, int low, int high) {
    return (low <= high) ? (v >= low && v <= high) : (v >= low || v <= high);
  };

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

    if (inRange(moon.angle, 355, 5)) {
      if (!NM_Done) {
        NM_Done = true;
        tft.drawString("New Moon", 4, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (inRange(moon.angle, 85, 95)) {
      if (!FQ_Done) {
        FQ_Done = true;
        tft.drawString("First Quarter", 4, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (inRange(moon.angle, 175, 185)) {
      if (!FM_Done) {
        FM_Done = true;
        tft.drawString("Full Moon", 4, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (inRange(moon.angle, 265, 285)) {
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

  if ((digitalRead(incrPin) == 0) && (digitalRead(decrPin) == 0)) {
    //    Serial.printf("%lu - 1 Both pressed, do menuing.\r\n", millis());
    doMenu();
    return;
  }

  // Allow the user to adjust backlight brightness.
  while ((digitalRead(incrPin) == 0) &&
         (tftBL_Lvl <= MAX_BRIGHTNESS))
  {
    delay(50);
    if (digitalRead(decrPin) == 0) {
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
    //    Serial.printf("Brightness %i\r\n", tftBL_Lvl);
    tft.setTextColor(TFT_YELLOW, RGB565(0, 0, 166));
    tft.setTextPadding(tft.width());
    tft.drawString("Brightness " + String(tftBL_Lvl), 4, dispLine6 + 5, 4);
    tft.setTextPadding(0);
    delay(100);
  }
  while ((digitalRead(decrPin)) == 0 &&
         (tftBL_Lvl >= MIN_BRIGHTNESS))
  {
    delay(50);
    if (digitalRead(incrPin) == 0) {
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
    //    Serial.printf("Brightness %i\r\n", tftBL_Lvl);
    tft.setTextColor(TFT_YELLOW, RGB565(0, 0, 166));
    tft.setTextPadding(tft.width());
    tft.drawString("Brightness " + String(tftBL_Lvl), 4, dispLine6 + 5, 4);
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
  while ((digitalRead(incrPin) == 0) || (digitalRead(decrPin) == 0));

  menuHide = millis() + MENU_HIDE_TIME;

  while (millis() < menuHide) {
    spriteMenu.fillSprite(TFT_BLACK);
    if (highlight == 1)
      spriteMenu.setTextColor(TFT_BLACK, TFT_YELLOW);
    else
      spriteMenu.setTextColor(TFT_YELLOW, TFT_BLACK);
    spriteMenu.drawString("Show Phase Dates", 0, 10, 4);
    spriteMenu.drawFastHLine(0, SPR_MENU_HEIGHT - 1, tft.width(), TFT_YELLOW);
    spriteMenu.pushSprite(0, 10);

    spriteMenu.fillSprite(TFT_BLACK);
    if (highlight == 2)
      spriteMenu.setTextColor(TFT_BLACK, TFT_YELLOW);
    else
      spriteMenu.setTextColor(TFT_YELLOW, TFT_BLACK);
    spriteMenu.drawString(" ", 0, 10, 4);
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
    if (digitalRead(decrPin) == 0) {  // Bottom button pressed.
      delay(50);
      if (digitalRead(decrPin) == 0) {  // Still pressed?
        highlight++; if (highlight == 2) highlight = 4;  // Skip empty choices.
        if (highlight > 4) highlight = 1;
        menuHide = millis() + MENU_HIDE_TIME;  // Extend auto close time.
      }
      while (digitalRead(decrPin) == 0);  // Wait for unpress to avoid multiple actions.
    }

    // Execute selected

    if (digitalRead(incrPin) == 0) {  // Bottom button pressed.
      delay(50);
      if (digitalRead(incrPin) == 0) {  // Still pressed?
        switch (highlight)
        {
          case 1:
            ShowFuturePhases();
            while ((digitalRead(incrPin) == 1) && (digitalRead(decrPin) == 1));
            return; break;

          // case 2:
          //   while ((digitalRead(incrPin) == 0) || (digitalRead(decrPin) == 0));
          //   break;
          //
          // case 3:
          //   while ((digitalRead(incrPin) == 0) || (digitalRead(decrPin) == 0));
          //   break;

          case 4:
            while ((digitalRead(incrPin) == 1) && (digitalRead(decrPin) == 1));
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

  if (WakeupHour > SleepHour)
    WakeUp = (iHour >= WakeupHour || iHour <= SleepHour);
  else
    WakeUp = (iHour >= WakeupHour && iHour <= SleepHour);

  // Serial.printf("tftBL_Lvl now set to %i\r\n", tftBL_Lvl);

  if (tftBL_Lvl > 500) { // Unchanged default value still in there.
    if (WakeUp)
      tftBL_Lvl = defaultBright;
    else
      tftBL_Lvl = 0;
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

  // y value is optional (default is 0, so no up/down scroll).
  spriteSF_Base.scroll(-1);  // Scroll left by one pixel
  if (timeForStars++ == 1) {  // Was 3
    timeForStars = 0;
    // Redraw new stars appearing on the right
    count = random(3);
    //    Serial.printf("Creating %i stars.\r\n", count);
    for (int i = 0; i < count; i++) { // Few new stars each frame
      brightness = random(155) + 100;
      // Serial.printf("Brightness %i ", brightness);
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
      spriteSF_Base.fillSmoothCircle(spriteSF_Base.width() - 4, random(spriteSF.height()),
                                     random(3), RGB565(brightness, brightness, brightness));
    }
  }
}
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
        tft.drawString("Turning off", tft.width() / 2, dispLine3, 4);
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
