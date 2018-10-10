/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Factory and Class for dynamically loadable detector types
///
//===----------------------------------------------------------------------===//

#pragma once

#include <CLI/CLI.hpp>
#include <atomic>
#include <common/NewStats.h>
#include <functional>
#include <map>
#include <memory>
#include <stdio.h>
#include <string>
#include <thread>

// All settings should be initialized.
// clang-format off
struct BaseSettings {
  std::string   DetectorPluginName   = {""};
  std::string   DetectorAddress      = {"0.0.0.0"};
  std::uint16_t DetectorPort         = {9000};
  std::uint16_t CommandServerPort    = {8888};
  std::int32_t  ReceiveMaxBytes      = {9000}; // Jumbo frame support
  std::int32_t  DetectorRxBufferSize = {2000000};
  std::int32_t  DetectorTxBufferSize = {200000};
  std::string   KafkaBroker          = {"localhost:9092"};
  std::string   GraphiteAddress      = {"127.0.0.1"};
  std::uint16_t GraphitePort         = {2003};
  std::string   KafkaTopic           = {""};
  std::string   ConfigFile           = {""};
  std::uint64_t UpdateIntervalSec    = {1};
  std::uint32_t StopAfterSec         = {0xffffffffU};
  std::uint16_t MinimumMTU           = {9000};
};
// clang-format on

struct ThreadInfo {
  std::function<void(void)> func;
  std::string name;
  std::thread thread;
};

class Detector {
public:
  using CommandFunction =
      std::function<int(std::vector<std::string>, char *, unsigned int *)>;
  using ThreadList = std::vector<ThreadInfo>;
  Detector(std::string Name, BaseSettings settings) : EFUSettings(settings), Stats(Name), DetectorName(Name) {};

  virtual ~Detector() {
    stopThreads();
  };

  /// \brief document
  virtual int statsize() { return Stats.size(); }

  /// \brief document
  virtual int64_t statvalue(size_t index) { return Stats.value(index); }

  /// \brief document
  virtual std::string &statname(size_t index) { return Stats.name(index); }

  void setStatsPrefix(std::string NewStatsPrefix) {
    Stats.setPrefix(NewStatsPrefix);
  }

  virtual const char *detectorname() { return DetectorName.c_str(); }

  virtual ThreadList &GetThreadInfo() { return Threads; };

  virtual std::map<std::string, CommandFunction> GetDetectorCommandFunctions() {
    return DetectorCommands;
  }

  virtual void startThreads() {
    for (auto &tInfo : Threads) {
      tInfo.thread = std::thread(tInfo.func);
    }
  }

  virtual void stopThreads() {
    runThreads.store(false);
    for (auto &tInfo : Threads) {
      if (tInfo.thread.joinable()) {
        tInfo.thread.join();
      }
    }
  };

protected:
  void AddThreadFunction(std::function<void(void)> &func,
                         std::string funcName) {
    Threads.emplace_back(ThreadInfo{func, std::move(funcName), std::thread()});
  };
  void AddCommandFunction(std::string Name, CommandFunction FunctionObj) {
    DetectorCommands[Name] = FunctionObj;
  };
  ThreadList Threads;
  std::map<std::string, CommandFunction> DetectorCommands;
  std::atomic_bool runThreads{true};
  BaseSettings EFUSettings;
  NewStats Stats;

private:
  std::string DetectorName;
};

struct PopulateCLIParser {
  std::function<void(CLI::App &)> Function;
};

/// \brief Base class for the creation of detector factories.
class DetectorFactoryBase {
public:
  virtual ~DetectorFactoryBase() = default;
  virtual std::shared_ptr<Detector> create(BaseSettings settings) = 0;
};

/// \brief Template for creating detector factories in dynamically loaded detector modules.
///
/// Usage example: DetectorFactory<Sonde> Factory;
template <class DetectorModule>
class DetectorFactory : public DetectorFactoryBase {
public:
  /// \brief Instantiates the corresponding detector module.
  ///
  /// This member function is only called by the efu when loading a detector module.
  std::shared_ptr<Detector> create(BaseSettings Settings) override {
    return std::shared_ptr<Detector>(new DetectorModule(Settings));
  }
};
