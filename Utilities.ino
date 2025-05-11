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
        tft.drawString("New Moon ", 0, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (inRange(moon.angle, 85, 95)) {
      if (!FQ_Done) {
        FQ_Done = true;
        tft.drawString("First Quarter ", 0, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (inRange(moon.angle, 175, 185)) {
      if (!FM_Done) {
        FM_Done = true;
        tft.drawString("Full Moon ", 0, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    if (inRange(moon.angle, 265, 285)) {
      if (!LQ_Done) {
        LQ_Done = true;
        tft.drawString("Last Quarter ", 0, useLine, 4);
        tft.drawString(chBuffer, 170, useLine, 4); useLine += font4Height;
      }
    }
    epoch += 3600;  // Check next hour.
  }
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Press either button to return.", 0, dispLine6, 4);
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
    delay(50);
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
    delay(50);
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
          case 1:  // case 'B':
            ShowFuturePhases();
            //            Serial.println(digitalRead(incrPin));
            //            Serial.println(digitalRead(decrPin));
            while ((digitalRead(incrPin) == 1) && (digitalRead(decrPin) == 1));
            return;
            break;

          //          case 2:  // case 'F':
          //            break;
          //
          //          case 3:  // case 'V':
          //            break;
          //
          case 4:  // Return to clock
            return;
            //
            //          default:
            //            break;  // Do nothing if not found.
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

  if (!WakeUp) tftBL_Lvl = 0;
  if (tftBL_Lvl > 500) tftBL_Lvl = defaultBright;  // == 1000 is not working.  Why?

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
