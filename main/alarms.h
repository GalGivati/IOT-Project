
void printAlarmCount() {
  Serial.print(F("AlarmCount = "));
  Serial.println(alarmCount);
}

void putAlarm(int index, String alarm)
{
  preferences.putString(("alarm" + String(index)).c_str(), alarm);
}

String getAlarm(int index) {
  return preferences.getString(("alarm" + String(index)).c_str());
}

/* check if alarm exist and return its index if exist */
int findAlarm(String alarm) {
  for (int i = 0; i < alarmCount; i++) {
    if (alarm == getAlarm(i)) {
      return i;
    }
  }
  return -1;
}

/* add an alarm */
void addAlarm(String alarm) {
  int index = findAlarm(alarm);
  if (index != -1) {
    return;
  }
  preferences.putString(("alarm" + String(alarmCount)).c_str(), alarm);
  alarmCount++;
  preferences.putInt("alarmCount", alarmCount);
  printAlarmCount();
  return;
}

/* remove an alarm */
void removeAlarm(String alarm)
{
  int index = findAlarm(alarm);
  if (index == -1) {
    return;
  }
  if (index != alarmCount -1) {
    String lastAlarm = getAlarm(alarmCount-1);
    preferences.putString(("alarm" + String(index)).c_str(), lastAlarm);
  }
  alarmCount--;
  preferences.putInt("alarmCount", alarmCount);
  printAlarmCount();
}

/* stop an alarm */
void stopAlarm(String alarm)
{
  if (indexAlarm == -1) {
    return;
  }
  String currentAlarm = getAlarm(indexAlarm);
  if (currentAlarm != alarm) {
    return;
  }
  alarmToStop = alarm;
}