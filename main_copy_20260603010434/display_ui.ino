#include "shared.h"
void showWake(DateTime now) {
  lcd.setCursor(0, 0);
  lcd.print("!! WAKE UP !!   ");
  lcd.setCursor(0, 1);
  if (now.hour()   < 10) lcd.print("0");  lcd.print(now.hour());   lcd.print(":");
  if (now.minute() < 10) lcd.print("0");  lcd.print(now.minute()); lcd.print(":");
  if (now.second() < 10) lcd.print("0");  lcd.print(now.second());
  lcd.print("        ");
}

void showClock(DateTime now) {
  lcd.setCursor(0, 0);
  lcd.print(now.year());  lcd.print("/");
  if (now.month() < 10) lcd.print("0");  lcd.print(now.month());  lcd.print("/");
  if (now.day()   < 10) lcd.print("0");  lcd.print(now.day());    lcd.print(" ");
  const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  lcd.print(days[now.dayOfWeek()]); lcd.print("  ");

  lcd.setCursor(0, 1);
  if (now.hour()   < 10) lcd.print("0");  lcd.print(now.hour());   lcd.print(":");
  if (now.minute() < 10) lcd.print("0");  lcd.print(now.minute()); lcd.print(":");
  if (now.second() < 10) lcd.print("0");  lcd.print(now.second());
  if (alarmOn) {
    lcd.print(" AL");
    if (alarmHour < 10) lcd.print("0"); lcd.print(alarmHour); lcd.print(":");
    if (alarmMin  < 10) lcd.print("0"); lcd.print(alarmMin);
  } else {
    lcd.print("        ");
  }
}