#include <mt3339/driver.hpp>

using namespace mt3339;

// CONSTRUCTORS
driver::driver()
    : m_response("PMTK", "000"),
      m_response_timeout(0.250)
{}

// COMMANDS
bool driver::connected()
{
    // Create a PMTK_Q_RELEASE (605) firmware query message.
    nmea::sentence nmea_sentence("PMTK", "605");

    // Transmit sentence to MT3339.
    transmit(nmea_sentence.nmea_string());

    // Wait for PMTK_DT_RELEASE (705) with condition variable and timeout.
    // NOTE: The wait_for method returns true IFF the response was received and matches the requested sentence type.
    std::unique_lock<std::mutex> unique_lock(driver::m_mutex_response);
    return driver::m_condition_variable_response.wait_for(unique_lock, driver::m_response_timeout, [this]{return driver::m_response.type() == "705";});
}
bool driver::set_baud(mt3339::baud_rate baud)
{
    // Create PMTK_SET_NMEA_BAUDRATE (251) sentence.
    nmea::sentence nmea_sentence("PMTK", "251", 1);

    // Set field.
    nmea_sentence.set_field(0, std::to_string(static_cast<int32_t>(baud)));

    // Send message.
    transmit(nmea_sentence.nmea_string());

    // Get ACK.
    return driver::wait_ack("251");
}
bool driver::set_rate(double frequency)
{
    // Create PMTK_API_SET_FIX_CTL (300) sentence.
    nmea::sentence nmea_sentence("PMTK", "300", 1);

    // Convert frequency to milliseconds, with range 100-10000.
    uint16_t milliseconds = std::min(10000U, std::max(100U, static_cast<uint32_t>(1000.0/frequency)));

    // Set field.
    nmea_sentence.set_field(0, std::to_string(milliseconds));

    // Send sentence.
    transmit(nmea_sentence.nmea_string());

    // Get ACK.
    return driver::wait_ack("300");
}
bool driver::set_outputs()
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Create PMTK_API_SET_NMEA_OUTPUT (314) sentence.
    nmea::sentence nmea_sentence("PMTK", "314", 19);

    // Default all fields to 0 (disabled) ;
    for(uint8_t i = 0; i < 19; ++i)
    {
        nmea_sentence.set_field(i, "0");
    }

    // Set fields according to if callbacks exist.
    if(driver::m_callback_gga)
    {
        nmea_sentence.set_field(3, "1");
    }
    if(driver::m_callback_gll)
    {
        nmea_sentence.set_field(0, "1");
    }
    if(driver::m_callback_gsa)
    {
        nmea_sentence.set_field(4, "1");
    }
    if(driver::m_callback_gsv)
    {
        nmea_sentence.set_field(5, "1");
    }
    if(driver::m_callback_rmc)
    {
        nmea_sentence.set_field(1, "1");
    }
    if(driver::m_callback_vtg)
    {
        nmea_sentence.set_field(2, "1");
    }
    if(driver::m_callback_zda)
    {
        nmea_sentence.set_field(17, "1");
    }

    // Send sentence.
    transmit(nmea_sentence.nmea_string());

    // Wait for PMTK ACK.
    bool result = driver::wait_ack("314");

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();

    return result;
}

// PROPERTIES
void driver::set_timeout(double seconds)
{
    // Lock response thread protection.
    driver::m_mutex_response.lock();

    // Set timeout.
    driver::m_response_timeout = std::chrono::duration<double>(seconds);

    // Unlock response thread protection.
    driver::m_mutex_response.unlock();
}
double driver::get_timeout() const
{
    // Lock response thread protection.
    std::lock_guard<std::mutex> lock_guard(driver::m_mutex_response);

    return driver::m_response_timeout.count();
}

// CALLBACKS
void driver::attach_gga(std::function<void(const nmea::gga&)> callback)
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Store callback.
    driver::m_callback_gga = callback;

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();
}
void driver::attach_gll(std::function<void(const nmea::gll&)> callback)
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Store callback.
    driver::m_callback_gll = callback;

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();
}
void driver::attach_gsa(std::function<void(const nmea::gsa&)> callback)
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Store callback.
    driver::m_callback_gsa = callback;

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();
}
void driver::attach_gsv(std::function<void(const nmea::gsv&)> callback)
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Store callback.
    driver::m_callback_gsv = callback;

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();
}
void driver::attach_rmc(std::function<void(const nmea::rmc&)> callback)
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Store callback.
    driver::m_callback_rmc = callback;

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();
}
void driver::attach_vtg(std::function<void(const nmea::vtg&)> callback)
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Store callback.
    driver::m_callback_vtg = callback;

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();
}
void driver::attach_zda(std::function<void(const nmea::zda&)> callback)
{
    // Lock callback thread protection.
    driver::m_mutex_callbacks.lock();

    // Store callback.
    driver::m_callback_zda = callback;

    // Unlock callback thread protection.
    driver::m_mutex_callbacks.unlock();
}

// IO
void driver::receive(const std::string& nmea_string)
{
    // Check if string is a valid NMEA sentence.
    if(!nmea::sentence::validate(nmea_string))
    {
        // String is not valid, quit.
        return;
    }
    
    // Parse NMEA string into NMEA sentence.
    nmea::sentence nmea_sentence(nmea_string, true);

    // Check talker.
    if(nmea_sentence.talker() == "PMTK")
    {
        // Message is a response from the receiver.
        
        // Lock response thread protection.
        driver::m_mutex_response.lock();

        // Set response.
        driver::m_response = nmea_sentence;

        // Unlock thread protection.
        driver::m_mutex_response.unlock();

        // Notify condition variable.
        driver::m_condition_variable_response.notify_one();
    }
    else
    {
        // Check message type, and raise appropriate callback if enabled.

        // Lock callback thread protection.
        driver::m_mutex_callbacks.lock();

        if(driver::m_callback_gga && nmea_sentence.talker() == "GGA")
        {
            // Parse GGA message.
            nmea::gga gga(nmea_sentence);

            // Raise callback.
            driver::m_callback_gga(std::ref(gga));
        }
        else if(driver::m_callback_gll && nmea_sentence.talker() == "GLL")
        {
            // Parse GLL message.
            nmea::gll gll(nmea_sentence);

            // Raise callback.
            driver::m_callback_gll(std::ref(gll));
        }
        else if(driver::m_callback_gsa && nmea_sentence.talker() == "GSA")
        {
            // Parse GSA message.
            nmea::gsa gsa(nmea_sentence);

            // Raise callback.
            driver::m_callback_gsa(std::ref(gsa));
        }
        else if(driver::m_callback_gsv && nmea_sentence.talker() == "GSV")
        {
            // Parse GSV message.
            nmea::gsv gsv(nmea_sentence);

            // Raise callback.
            driver::m_callback_gsv(std::ref(gsv));
        }
        else if(driver::m_callback_rmc && nmea_sentence.talker() == "RMC")
        {
            // Parse RMC message.
            nmea::rmc rmc(nmea_sentence);

            // Raise callback.
            driver::m_callback_rmc(std::ref(rmc));
        }
        else if(driver::m_callback_vtg && nmea_sentence.talker() == "VTG")
        {
            // Parse VTG message.
            nmea::vtg vtg(nmea_sentence);

            // Raise callback.
            driver::m_callback_vtg(std::ref(vtg));
        }
        else if(driver::m_callback_zda && nmea_sentence.talker() == "ZDA")
        {
            // Parse ZDA message.
            nmea::zda zda(nmea_sentence);

            // Raise callback.
            driver::m_callback_zda(std::ref(zda));
        }

        // Unlock callback thread protection.
        driver::m_mutex_callbacks.unlock();
    }
}
bool driver::wait_ack(const std::string& command)
{
    // Wait for PMTK_ACK (001) on the specified command with condition variable and timeout.
    std::unique_lock<std::mutex> unique_lock(driver::m_mutex_response);
    bool received = driver::m_condition_variable_response.wait_for(unique_lock, driver::m_response_timeout, [this, command]{return driver::m_response.type() == "001" && driver::m_response.get_field(0) == command;});

    // Check if the ACK command indicates action succeeded.
    return driver::m_response.get_field(1) == "3";
}