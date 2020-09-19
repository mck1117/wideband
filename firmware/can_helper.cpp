#include "can_helper.h"

#include <cstring>

/*static*/ CANDriver* CanTxMessage::s_device = nullptr;

/*static*/ void CanTxMessage::setDevice(CANDriver& device) {
	s_device = &device;
}

CanTxMessage::CanTxMessage(uint32_t eid, uint8_t dlc) {
	m_frame.IDE = CAN_IDE_STD;
	m_frame.EID = eid;
	m_frame.RTR = CAN_RTR_DATA;
	m_frame.DLC = dlc;
	memset(m_frame.data8, 0, sizeof(m_frame.data8));
}

CanTxMessage::~CanTxMessage() {
	auto device = s_device;

	// 100 ms timeout
	canTransmit(device, CAN_ANY_MAILBOX, &m_frame, TIME_MS2I(100));
}

uint8_t& CanTxMessage::operator[](size_t index) {
	return m_frame.data8[index];
}