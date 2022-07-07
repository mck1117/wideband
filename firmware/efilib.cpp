#include "ch.h"
#include "osal.h"

extern "C" void __cxa_pure_virtual(void) {
	osalSysHalt("Pure virtual function call.");
}
