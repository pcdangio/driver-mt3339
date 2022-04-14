/// \file mt3339/driver.hpp
/// \brief Defines the mt3339::driver class.
#ifndef MT3339___DRIVER_H
#define MT3339___DRIVER_H

#include <mt3339/baud_rate.hpp>

#include <nmea/message/gga.hpp>
#include <nmea/message/gll.hpp>
#include <nmea/message/gsa.hpp>
#include <nmea/message/gsv.hpp>
#include <nmea/message/rmc.hpp>
#include <nmea/message/vtg.hpp>
#include <nmea/message/zda.hpp>

#include <functional>
#include <condition_variable>

/// \brief Contains all code related to interfacing with the MediaTek MT3339 GPS receiver.
namespace mt3339 {

/// \brief The base driver for the MT3339.
class driver
{
public:
    // CONSTRUCTORS
    /// \brief Creates a new driver instance.
    driver();

    // CALLBACKS
    /// \brief Attaches a callback for handling received GGA messages.
    /// \param callback The callback to attach. Set to NULLPTR to detach.
    void attach_gga(std::function<void(const nmea::gga&)> callback);
    /// \brief Attaches a callback for handling received GLL messages.
    /// \param callback The callback to attach. Set to NULLPTR to detach.
    void attach_gll(std::function<void(const nmea::gll&)> callback);
    /// \brief Attaches a callback for handling received GSA messages.
    /// \param callback The callback to attach. Set to NULLPTR to detach.
    void attach_gsa(std::function<void(const nmea::gsa&)> callback);
    /// \brief Attaches a callback for handling received GSV messages.
    /// \param callback The callback to attach. Set to NULLPTR to detach.
    void attach_gsv(std::function<void(const nmea::gsv&)> callback);
    /// \brief Attaches a callback for handling received RMC messages.
    /// \param callback The callback to attach. Set to NULLPTR to detach.
    void attach_rmc(std::function<void(const nmea::rmc&)> callback);
    /// \brief Attaches a callback for handling received VTG messages.
    /// \param callback The callback to attach. Set to NULLPTR to detach.
    void attach_vtg(std::function<void(const nmea::vtg&)> callback);
    /// \brief Attaches a callback for handling received ZDA messages.
    /// \param callback The callback to attach. Set to NULLPTR to detach.
    void attach_zda(std::function<void(const nmea::zda&)> callback);

    // COMMANDS
    /// \brief Tests the serial connection to the MT3339.
    /// \returns TRUE if the MT3339 is connected, otherwise FALSE.
    bool connected();
    /// \brief Sets the baud rate that the MT3339 uses.
    /// \param baud The baud rate to apply.
    /// \returns TRUE if the command succeeded, otherwise FALSE.
    bool set_baud(mt3339::baud_rate baud);
    /// \brief Sets the output rate of the MT3339.
    /// \param frequency The output frequency (Hz). Frequency is clamped between 0.1Hz and 10Hz (inclusive).
    /// \returns TRUE if the command succeeded, otherwise FALSE.
    bool set_rate(double frequency);
    /// \brief Updates the MT3339 to send only the NMEA messages that have callbacks attached to this driver.
    /// \returns TRUE if the command succeeded, otherwise FALSE.
    bool set_outputs();

    // PROPERTIES
    /// \brief Sets the command response timeout (sec).
    /// \param seconds The timeout to set.
    void set_timeout(double seconds);
    /// \brief Gets the current command response timeout.
    /// \returns The current command response timeout (sec).
    double get_timeout() const;

protected:
    // IO
    /// \brief Transmits an NMEA string to the MT3339 over a serial port.
    /// \param nmea_string The NMEA string to transmit.
    /// \note The NMEA string will have CRLF already appended.
    virtual void transmit(const std::string& nmea_string) = 0;
    /// \brief Passes a received NMEA string to the driver for processing.
    /// \param nmea_string The received NMEA string.
    void receive(const std::string& nmea_string);

private:
    // RESPONSE
    /// \brief Stores the last received response from the MT3339.
    nmea::sentence m_response;
    /// \brief The timeout to wait for responses from the MT3339.
    std::chrono::duration<double> m_response_timeout;
    /// \brief Provides thread protection for response variables.
    mutable std::mutex m_mutex_response;
    /// \brief Enables waiting for responses from the MT3339 with timeout.
    std::condition_variable m_condition_variable_response;
    
    // CALLBACKS
    /// \brief The callback for handling received GGA messages.
    std::function<void(const nmea::gga&)> m_callback_gga;
    /// \brief The callback for handling received GLL messages.
    std::function<void(const nmea::gll&)> m_callback_gll;
    /// \brief The callback for handling received GSA messages.
    std::function<void(const nmea::gsa&)> m_callback_gsa;
    /// \brief The callback for handling received GSV messages.
    std::function<void(const nmea::gsv&)> m_callback_gsv;
    /// \brief The callback for handling received RMC messages.
    std::function<void(const nmea::rmc&)> m_callback_rmc;
    /// \brief The callback for handling received VTG messages.
    std::function<void(const nmea::vtg&)> m_callback_vtg;
    /// \brief The callback for handling received ZDA messages.
    std::function<void(const nmea::zda&)> m_callback_zda;
    /// \brief Provides thread protection for message callbacks.
    std::mutex m_mutex_callbacks;

    // IO
    /// \brief Waits for an ACK response from the MT3339 for a specific command.
    /// \param command The command to wait for an ACK response on.
    /// \returns TRUE if the ACK response indicated success, otherwise FALSE.
    bool wait_ack(const std::string& command);
};

}

#endif