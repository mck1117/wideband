// this is a work-around since our Makefile does not recognice .cc format
// todo: adjust the makefile so that we can remove this

// This line ensures that gmock.h can be compiled on its own, even
// when it's fused.
#include "gmock/gmock.h"

// The following lines pull in the real gmock *.cc files.
#include "src/gmock-cardinalities.cc"
#include "src/gmock-internal-utils.cc"
#include "src/gmock-matchers.cc"
#include "src/gmock-spec-builders.cc"
#include "src/gmock.cc"
