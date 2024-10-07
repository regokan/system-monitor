#include <dirent.h>
#include <filesystem>
#include <iterator>
#include <string>
#include <unistd.h>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// Read and return the system's CPU
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return the system's kernel
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Return a vector composed of the system's processes
vector<int> LinuxParser::Pids() {
  vector<int> pids;

  for (const auto& entry :
       std::filesystem::directory_iterator(kProcDirectory)) {
    if (entry.is_directory()) {
      // Extract the filename from the directory entry
      string filename = entry.path().filename().string();

      // Check if the filename consists only of digits (i.e., it's a PID)
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }

  return pids;
}
// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  float memTotal = 0.0f;
  float memAvailable = 0.0f;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "MemTotal:") {
        linestream >> memTotal;
      } else if (key == "MemAvailable:") {
        linestream >> memAvailable;
      }
    }
  }

  // avoid division by zero
  if (memTotal > 0) {
    // memoru utilization as a fraction of MemTotal
    return (memTotal - memAvailable) / memTotal;
  }

  return 0.0f;  // 0 if unable to calculate memory utilization
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  float uptime = 0.0;
  float idletime = 0.0;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }

  return static_cast<long>(uptime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  std::vector<std::string> cpu_values = CpuUtilization();
  if (cpu_values.empty()) return 0;

  long user = std::stol(cpu_values[0]);
  long nice = std::stol(cpu_values[1]);
  long system = std::stol(cpu_values[2]);
  long idle = std::stol(cpu_values[3]);
  long iowait = std::stol(cpu_values[4]);
  long irq = std::stol(cpu_values[5]);
  long softirq = std::stol(cpu_values[6]);
  long steal = std::stol(cpu_values[7]);

  // total jiffies = sum of all CPU states
  return user + nice + system + idle + iowait + irq + softirq + steal;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::string line, value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    std::vector<std::string> values;

    // all values in the line
    while (linestream >> value) {
      values.push_back(value);
    }

    // Fields 14 (utime) and 15 (stime) represent active jiffies for the process
    long utime = std::stol(values[13]);
    long stime = std::stol(values[14]);

    // total active jiffies for the process
    return utime + stime;
  }

  return 0;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::vector<std::string> cpu_values = CpuUtilization();
  if (cpu_values.empty()) return 0;

  long user = std::stol(cpu_values[0]);
  long nice = std::stol(cpu_values[1]);
  long system = std::stol(cpu_values[2]);
  long irq = std::stol(cpu_values[5]);
  long softirq = std::stol(cpu_values[6]);
  long steal = std::stol(cpu_values[7]);

  // active jiffies = sum of non-idle jiffies
  return user + nice + system + irq + softirq + steal;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::vector<std::string> cpu_values = CpuUtilization();
  if (cpu_values.empty()) return 0;

  long idle = std::stol(cpu_values[3]);
  long iowait = std::stol(cpu_values[4]);

  // idle jiffies = idle + iowait
  return idle + iowait;
}

// Read and return CPU utilization
std::vector<std::string> LinuxParser::CpuUtilization() {
  std::vector<std::string> cpu_utilization;
  std::string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      std::string cpu_label;
      linestream >> cpu_label;

      // only interested in lines that start with "cpu" (cpu0, cpu1,
      // etc.) but not exactly "cpu"
      if (cpu_label.find("cpu") == 0 && cpu_label != "cpu") {
        std::string value;
        long user, nice, system, idle, iowait, irq, softirq, steal;

        // reading the relevant CPU values from the line
        linestream >> user >> nice >> system >> idle >> iowait >> irq >>
            softirq >> steal;

        // total idle time = idle + iowait
        long idle_time = idle + iowait;

        // total active time = user + nice + system + irq + softirq + steal
        long active_time = user + nice + system + irq + softirq + steal;

        // total CPU time = active + idle
        long total_time = active_time + idle_time;

        // CPU utilization as the ratio of active to total time
        float utilization = static_cast<float>(active_time) / total_time;
        cpu_utilization.push_back(std::to_string(utilization));
      }
    }
  }

  return cpu_utilization;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int totalProcesses = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "processes") {
        linestream >> totalProcesses;
      }
    }
  }

  return totalProcesses;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int runningProcesses = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "procs_running") {
        linestream >> runningProcesses;
      }
    }
  }

  return runningProcesses;
}

// Read and return the command associated with a process
std::string LinuxParser::Command(int pid) {
  std::string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

// Read and return the memory used by a process (in MB)
std::string LinuxParser::Ram(int pid) {
  std::string line, key;
  int ram_kb;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> ram_kb;
        return std::to_string(ram_kb / 1024);  // Convert from KB to MB
      }
    }
  }
  return "0";  // "0" if no memory usage info is found
}

// Read and return the user ID associated with a process
std::string LinuxParser::Uid(int pid) {
  std::string line, key, uid;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> uid;
        return uid;
      }
    }
  }
  return "";
}

// Read and return the user associated with a process
std::string LinuxParser::User(int pid) {
  std::string uid = LinuxParser::Uid(pid);
  std::string line, user, x, id;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> user >> x >> id;
      if (id == uid) {
        return user;
      }
    }
  }
  return "";
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::string line;
  long starttime = 0;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    std::vector<std::string> values(
        (std::istream_iterator<std::string>(linestream)),
        std::istream_iterator<std::string>());
    if (values.size() > 21) {
      starttime =
          std::stol(values[21]) /
          sysconf(_SC_CLK_TCK);  // converting from clock ticks to seconds
    }
  }
  return starttime;
}

// Read and return the CPU utilization of a process
float LinuxParser::CpuUtilization(int pid) {
  std::string line, value;
  long utime, stime, starttime;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    std::vector<std::string> values;

    // reading all fields in the line
    while (linestream >> value) {
      values.push_back(value);
    }

    // Field 14: utime, Field 15: stime, Field 22: starttime
    utime = std::stol(values[13]);
    stime = std::stol(values[14]);
    starttime = std::stol(values[21]);

    // system uptime
    long system_uptime = LinuxParser::UpTime();

    // total time the process has been running
    long total_time = utime + stime;

    // clock ticks per second (usually 100)
    long clock_ticks = sysconf(_SC_CLK_TCK);

    // seconds the process has been running
    float seconds = system_uptime - (starttime / clock_ticks);

    // avoiding division by zero in case of some error
    if (seconds > 0) {
      return ((total_time / clock_ticks) / seconds);
    }
  }

  return 0.0;
}
