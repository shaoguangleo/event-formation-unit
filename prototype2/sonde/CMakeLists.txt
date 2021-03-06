#=============================================================================
# Detector Pipeline Plugins
#=============================================================================
set(sonde_common_inc
  ../common/Hists.h
  ../common/HistSerializer.h
  SoNDeBase.h
  ideas/Data.h
)
set(sonde_common_src
  ../common/Hists.cpp
  ../common/HistSerializer.cpp
  SoNDeBase.cpp
  ideas/Data.cpp
)



# IDEA based readout for SoNDe detector
set(sonde_INC
  ${sonde_common_inc}
)
set(sonde_SRC
  SoNDe.cpp
  ${sonde_common_src}
)
create_module(sonde)


#=============================================================================
# Tools and Data generators
#=============================================================================


#=============================================================================
# Tests
#=============================================================================

set(SoNDeBaseTest_INC
  ${sonde_common_inc}
  ../../libs/include/Socket.h
  ../../libs/include/TSCTimer.h
)
set(SoNDeBaseTest_SRC
  SoNDeBaseTest.cpp
  ${sonde_common_src}
  ../../libs/source/Socket.cpp
  ../../libs/source/TSCTimer.cpp
  ../adc_readout/test/TestUDPServer.cpp
)
create_test_executable(SoNDeBaseTest)


set(IDEASDataTest_SRC
  ideas/DataTest.cpp
  ideas/Data.cpp
  )
set(IDEASDataTest_INC
  ideas/Data.h
  ideas/TestData.h
  )
create_test_executable(IDEASDataTest)


set(IDEASGeometryTest_SRC
  ideas/GeometryTest.cpp)
set(IDEASGeometryTest_INC
  Geometry.h)
create_test_executable(IDEASGeometryTest)
