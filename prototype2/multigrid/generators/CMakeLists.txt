#=============================================================================
# Raw evenlet generator (h5)
#=============================================================================

set(mggen_readouts_SRC
  ../../generators/generator.cpp
  ../../common/clustering/Hit.cpp
  ../mesytec/Readout.cpp
  ReaderReadouts.cpp
  )
set(mggen_readouts_INC
  ../../common/clustering/Hit.h
  ../mesytec/Readout.h
  ReaderReadouts.h
  )

create_executable(mggen_readouts)
target_compile_definitions(mggen_readouts PUBLIC GENERATOR_MULTIGRID_READOUTS)
