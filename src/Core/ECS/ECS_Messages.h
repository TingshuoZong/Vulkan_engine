#pragma once

/// @brief For making messages that are sent to a specific systems unlike broadcasts which are sent to all systems
struct Message {
    virtual ~Message() = default;
};