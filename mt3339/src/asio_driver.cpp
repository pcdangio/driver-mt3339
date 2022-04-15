#include <mt3339/asio_driver.hpp>

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <stdexcept>

using namespace mt3339;

// CONSTRUCTORS
asio_driver::asio_driver()
    : m_serial_port(m_io_context),
      m_thread_running(false)
{}

// CONTROL
void asio_driver::start(const std::string& port, mt3339::baud_rate baud_rate)
{
    // Lock thread's thread protection.
    std::lock_guard<std::mutex> lock_guard(asio_driver::m_mutex_thread);

    // Check if thread is currently running.
    if(asio_driver::m_thread_running)
    {
        throw std::runtime_error("driver is already running");
    }

    // Mark thread as running.
    asio_driver::m_thread_running = true;
    
    // Open serial port.
    boost::system::error_code error;
    asio_driver::m_serial_port.open(port, error);
    if(error)
    {
        throw std::runtime_error("failed to open serial port: " + error.message());
    }

    // Set serial port's baud rate.
    asio_driver::m_serial_port.set_option(boost::asio::serial_port_base::baud_rate(static_cast<uint32_t>(baud_rate)));

    // Start the io_context thread.
    asio_driver::m_thread = std::thread(&asio_driver::run, this);

    // Give time for serial port to spin up, and for run thread to start.
    std::this_thread::sleep_for(std::chrono::duration<double>(0.25));

    // Start asynchronously receiving.
    asio_driver::async_receive();
}
void asio_driver::stop()
{
    // Lock thread's thread protection.
    asio_driver::m_mutex_thread.lock();

    // Check if thread is running.
    if(asio_driver::m_thread_running)
    {
        // Instruct io_context to stop.
        asio_driver::m_io_context.stop();

        // Wait for thread to join.
        asio_driver::m_thread.join();

        // Close serial port.
        asio_driver::m_serial_port.close();

        // Set running flag to false.
        asio_driver::m_thread_running = false;
    }

    // Unlock thread's thread protection.
    asio_driver::m_mutex_thread.unlock();
}

// IO
void asio_driver::transmit(const std::string& nmea_string)
{
    // Put string into TX buffer.
    // Use assign to keep buffer's capacity.
    asio_driver::m_tx_buffer.assign(nmea_string.begin(), nmea_string.end());

    // Write to serial port.
    boost::asio::write(asio_driver::m_serial_port, boost::asio::dynamic_buffer(asio_driver::m_tx_buffer));
}

// SERIAL
void asio_driver::run()
{
    // Create work guard to stop io_context::run from exiting when it has no more handlers.
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(asio_driver::m_io_context.get_executor());
    
    // Run the io_context event loop.
    // NOTE: io_context::stop will force this to stop blocking.
    asio_driver::m_io_context.run();
}
void asio_driver::async_receive()
{
    // Start async receive operation.
    boost::asio::async_read_until(asio_driver::m_serial_port,
                                  boost::asio::dynamic_buffer(asio_driver::m_rx_buffer),
                                  '\n',
                                  std::bind(&asio_driver::async_receive_callback, this, std::placeholders::_1, std::placeholders::_2));
}
void asio_driver::async_receive_callback(const boost::system::error_code& error, uint32_t bytes_read)
{
    // Verify that error hasn't occured and that bytes were read.
    if(!error && bytes_read > 0)
    {
        // String may contain extra characters before "$" delimiter.
        // Try to find delimiter.
        std::size_t start_index = asio_driver::m_rx_buffer.find_first_of('$');
        // Check if delimiter was found, and that it exists before end of bytes_read.
        if(start_index != std::string::npos && start_index < bytes_read)
        {
            // Extract relevant substring and pass to receive method.
            asio_driver::receive(asio_driver::m_rx_buffer.substr(start_index, bytes_read - start_index));
        }
    }

    // Consume/clear bytes read since RX buffer is used as a dynamic buffer.
    asio_driver::m_rx_buffer.erase(0, bytes_read);

    // Continue async reading.
    asio_driver::async_receive();
}