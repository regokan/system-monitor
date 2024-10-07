#include <cctype>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return this->pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
  return LinuxParser::CpuUtilization(this->pid_);
}

// Return the command that generated this process
string Process::Command() {
  if (this->command_.empty()) this->command_ = LinuxParser::Command(this->pid_);
  return this->command_;
}

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(this->pid_); }

// Return the user (name) that generated this process
string Process::User() {
  if (this->user_.empty()) this->user_ = LinuxParser::User(this->pid_);
  return this->user_;
}

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(this->pid_); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return this->CpuUtilization() >
         a.CpuUtilization();  // sort by CPU utilization in descending order
}

// Constructor
Process::Process(int pid) : pid_(pid) {
  this->User();
  this->Command();
};
