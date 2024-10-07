#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
  std::vector<std::string> cpu_utilizations = LinuxParser::CpuUtilization();
  float total_utilization = 0.0;

  // Sum the individual CPU utilizations
  for (const std::string& utilization_str : cpu_utilizations) {
    total_utilization += std::stof(utilization_str);  // Convert string to float
  }

  // Return the aggregate utilization (average across all CPUs)
  return total_utilization / cpu_utilizations.size();  // Aggregate utilization
}
