/// \file mt3339/asio_driver.hpp
/// \brief Defines the mt3339::asio_driver class.
#ifndef MT3339___ASIO_DRIVER_H
#define MT3339___ASIO_DRIVER_H

#include <mt3339/driver.hpp>

#include <boost/asio/serial_port.hpp>

#include <thread>

namespace mt3339 {

/// \brief An MT3339 driver using a Boost ASIO serial port.
class asio_driver
    : public driver
{
public:
    // CONSTRUCTORS
    /// \brief Creates a new asio_driver instance.
    asio_driver();

    /// \brief Starts the driver.
    /// \param port The serial port to communicate with the MT3339 over.
    /// \param baud_rate The baud rate to use for the serial connection.
    /// \exception std::runtime_error if the port fails to open.
    void start(const std::string& port, mt3339::baud_rate baud_rate);
    /// \brief Stops the driver.
    void stop();

private:
    // IO
    void transmit(const std::string& nmea_string) override;

    // THREAD
    /// \brief The thread for running asynchronous read/write operations on the serial port.
    std::thread m_thread;
    /// \brief Indicates if the driver's thread is currently running.
    bool m_thread_running;
    /// \brief Provides thread protection for the driver's thread.
    std::mutex m_mutex_thread;
    /// \brief The worker function for this driver's thread.
    void run();

    // SERIAL
    /// \brief The ASIO IO context for this driver.
    boost::asio::io_context m_io_context;
    /// \brief The serial port for communicating with the MT3339.
    boost::asio::serial_port m_serial_port;
    /// \brief The RX buffer for read operations.
    std::vector<uint8_t> m_rx_buffer;
    /// \brief The TX buffer for write operations.
    std::vector<uint8_t> m_tx_buffer;
    /// \brief Starts an asynchronous read operation on the serial port.
    void async_receive();
    /// \brief Handles data received from asynchronous read operations.
    void async_receive_callback(const boost::system::error_code& error, uint32_t bytes_read);
};

}

#endif