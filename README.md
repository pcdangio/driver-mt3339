# driver-mt3339
A C++ driver for the MediaTek MT3339 GPS sensor.

# Overview
This C++ library serves as a driver for interfacing with a MediaTek MT3339 GPS sensor. It includes methods for configuring certain aspects of the GPS, as well as reading NMEA strings received from the GPS. On the ```main``` branch, a base ```mt3339::driver``` class provides base functionality for the interface. Additional [specializations](#specializations) are provided in different branches that implement the serial interface that the ```mt3339::driver``` class requires for actual communication. These specializations are placed in dedicated branches to avoid dependency issues. A doxyfile is provided for generating documentation.

# Specializations
1. **Boost ASIO:** Uses a Boost ASIO serial port for communication, available in the [asio](https://github.com/pcdangio/mt3339/tree/asio) branch.

You can create your own specializations by inheriting from the base ```mt3339::driver``` and following the Boost ASIO specialization as an example.

# Usage
The following example uses the Boost ASIO specialization.
```cpp
#include <mt3339/asio_driver.hpp>

#include <iostream>
#include <iomanip>

// Create callback for GGA message.
void callback_gga(const nmea::gga& message)
{
    std::cout << "Received GGA message.\n";

    std::cout << "UTC Time: " << message.utc.get() << "\n";

    if(message.latitude.exists() && message.longitude.exists())
    {
        std::cout << "Latitude: " << std::fixed << std::setprecision(6) << message.latitude.get() << "\n";
        std::cout << "Longitude: " << std::fixed << std::setprecision(6) << message.longitude.get() << "\n";
    }
    else
    {
        std::cout << "No fix available...\n";
    }
}

int32_t main(int32_t argc, char** argv)
{
    // Create driver instance.
    mt3339::asio_driver driver;

    // Attach GGA message callback.
    driver.attach_gga(callback_gga);

    // Start the driver.
    driver.start("/dev/ttyACM0", mt3339::baud_rate::B_9600);

    // Update MT3339 outputs based on attached message callbacks.
    driver.set_outputs();

    // Set MT3339 output rate to 5Hz.
    driver.set_rate(5.0);

    // Sleep this thread while the driver's thread receives messages.
    sleep(10);

    // Stop driver.
    driver.stop();
}
```