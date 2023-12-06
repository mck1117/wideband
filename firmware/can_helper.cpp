#include "can_helper.h"
#include "timer.h"

#include <cstring>

CanTxMessage::CanTxMessage(uint32_t eid, uint8_t dlc, bool isExtended) {
    m_frame.IDE = isExtended ? CAN_IDE_EXT : CAN_IDE_STD;
    m_frame.EID = eid;
    m_frame.RTR = CAN_RTR_DATA;
    m_frame.DLC = dlc;
    memset(m_frame.data8, 0, sizeof(m_frame.data8));
}

static int txFailureCounter = 0;

static Timer txFailureCounterReset;

bool isTxIssue() {
    if (txFailureCounterReset.hasElapsedSec(10)) {
        txFailureCounterReset.reset();
        txFailureCounter = 0;
    }
    // 10 times 100ms timeout would take a second to enter error condition
    return txFailureCounter > 10;
}

CanTxMessage::~CanTxMessage() {
    // 100 ms timeout
    msg_t msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, TIME_MS2I(100));
    if (msg != MSG_OK) {
        txFailureCounter++;
    }
}

uint8_t& CanTxMessage::operator[](size_t index) {
    return m_frame.data8[index];
}
