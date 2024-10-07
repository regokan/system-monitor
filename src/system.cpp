#include <cstddef>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
  // list of current PIDs
  vector<int> pids = LinuxParser::Pids();

  // clearing the current process list
  processes_.clear();

  // create a Process object and add it to the process list for pid
  for (int pid : pids) {
    processes_.emplace_back(pid);  // Use the Process constructor
  }

  // sorting processes by CPU utilization
  std::sort(processes_.begin(), processes_.end());
  return processes_;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() {
  if (this->kernel_.size() == 0) {
    this->kernel_ = LinuxParser::Kernel();
  }
  return this->kernel_;
}

// Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// Return the operating system name
std::string System::OperatingSystem() {
  if (this->operatingSystem_.size() == 0) {
    this->operatingSystem_ = LinuxParser::OperatingSystem();
  }
  return this->operatingSystem_;
}

// Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }
