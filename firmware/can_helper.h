#pragma once

#include <cstdint>
#include <cstddef>
#include "hal.h"

/**
 * Represent a message to be transmitted over CAN.
 * 
 * Usage:
 *   * Create an instance of CanTxMessage
 *   * Set any data you'd like to transmit either using the subscript operator to directly access bytes, or any of the helper functions.
 *   * Upon destruction, the message is transmitted.
 */
class CanTxMessage
{
public:
    /**
     * Create a new CAN message, with the specified extended ID.
     */
    explicit CanTxMessage(uint32_t eid, uint8_t dlc = 8, bool isExtended = false);

    /**
     * Destruction of an instance of CanTxMessage will transmit the message over the wire.
     */
    ~CanTxMessage();

    /**
     * @brief Read & write the raw underlying 8-byte buffer.
     */
    uint8_t& operator[](size_t);

protected:
    CANTxFrame m_frame;

private:
    static CANDriver* s_device;
};

/**
 * A CAN message based on a type, removing the need for manually flipping bits/bytes.
 */
template <typename TData>
class CanTxTyped final : public CanTxMessage
{
    static_assert(sizeof(TData) <= sizeof(CANTxFrame::data8));

public:
    explicit CanTxTyped(uint32_t eid) : CanTxMessage(eid, sizeof(TData)) { }
    explicit CanTxTyped(uint32_t eid, bool isExtended) : CanTxMessage(eid, sizeof(TData), isExtended) { }

    /**
     * Access members of the templated type.  
     * 
     * So you can do:
     * CanTxTyped<MyType> d;
     * d->memberOfMyType = 23;
     */
    TData* operator->()
    {
        return reinterpret_cast<TData*>(&m_frame.data8);
    }

    TData& get()
    {
        return *reinterpret_cast<TData*>(&m_frame.data8);
    }
};

template <typename TData>
void transmitStruct(uint32_t eid)
{
    CanTxTyped<TData> frame(eid);
    // Destruction of an instance of CanTxMessage will transmit the message over the wire.
    // see CanTxMessage::~CanTxMessage()
    populateFrame(frame.get());
}
