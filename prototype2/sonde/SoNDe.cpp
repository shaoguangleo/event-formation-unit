/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief SoNDe detector
///
//===----------------------------------------------------------------------===//

#include <sonde/SoNDeBase.h>
#include <common/Detector.h>


static struct SoNDeSettings LocalSoNDeSettings;

void SetCLIArguments(CLI::App __attribute__((unused)) & parser) {
  parser.add_option("--dumptofile", LocalSoNDeSettings.fileprefix,
                    "dump to specified file")->group("SoNDe");
}

PopulateCLIParser PopulateParser{SetCLIArguments};

class SONDEIDEA : public SONDEIDEABase {
public:
  explicit SONDEIDEA(BaseSettings Settings)
      : SONDEIDEABase(std::move(Settings), LocalSoNDeSettings) {}
};

DetectorFactory<SONDEIDEA> Factory;
