#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  long hours = seconds / 3600;
  long minutes = (seconds % 3600) / 60;
  long seconds_left = (seconds % 3600) % 60;

  string hours_str = std::to_string(hours);
  string minutes_str = std::to_string(minutes);
  string seconds_str = std::to_string(seconds_left);

  if (hours < 10) {
    hours_str = "0" + hours_str;
  }
  if (minutes < 10) {
    minutes_str = "0" + minutes_str;
  }
  if (seconds_left < 10) {
    seconds_str = "0" + seconds_str;
  }
  return hours_str + ":" + minutes_str + ":" + seconds_str;
}
