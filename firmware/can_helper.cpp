#include "can_helper.h"

#include <cstring>

CanTxMessage::CanTxMessage(uint32_t eid, uint8_t dlc, bool isExtended) {
    m_frame.IDE = isExtended ? CAN_IDE_EXT : CAN_IDE_STD;
    m_frame.EID = eid;
    m_frame.RTR = CAN_RTR_DATA;
    m_frame.DLC = dlc;
    memset(m_frame.data8, 0, sizeof(m_frame.data8));
}

CanTxMessage::~CanTxMessage() {
    // 100 ms timeout
    canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, TIME_IMMEDIATE);
}

uint8_t& CanTxMessage::operator[](size_t index) {
    return m_frame.data8[index];
}
