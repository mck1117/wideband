#include "can_helper.h"

#include <cstring>

#include "can.h"

CanTxMessage::CanTxMessage(uint32_t eid, uint8_t dlc, bool isExtended) {
    CAN_EXT(m_frame) = isExtended ? CAN_IDE_EXT : CAN_IDE_STD;

    #ifdef STM32G4XX
    m_frame.common.RTR = 0;
    #else // Not CAN FD
    m_frame.RTR = CAN_RTR_DATA;
    #endif

    if (isExtended) {
        CAN_EID(m_frame) = eid;
    } else {
        CAN_SID(m_frame) = eid;
    }

    m_frame.DLC = dlc;
    memset(m_frame.data8, 0, sizeof(m_frame.data8));
}

CanTxMessage::~CanTxMessage() {
    // 100 ms timeout
    canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, TIME_MS2I(100));
}

uint8_t& CanTxMessage::operator[](size_t index) {
    return m_frame.data8[index];
}
