#include <Arduino.h>
class MQSensor
{
private:
    float m_LoadResistor; 
    float m_R0;
    float m_volt;

    uint8_t m_AnalogPort;
    
    float m_Resulution;
    float m_Bits;

public:
    MQSensor(uint8_t analogPort, float loadResistor, float resolution = 5, float bits = 10) :  m_AnalogPort(analogPort),  m_LoadResistor(loadResistor), m_Resulution(resolution), m_Bits(bits) {    };

    float readVoltage()
    {
        float ADC= this->m_Resulution * (analogRead(this->m_AnalogPort) / (pow(2.0, this->m_Bits) - 1.0));

        float VoltageDivider = (1000.0+2000.0)/2000.0;

        return (ADC*VoltageDivider);
    };

    float calculateR0(int samples = 1)
    {
        float avg = 0;
        
        for(int i = 0; i < samples; i++)
            avg += readRS();

        avg = avg / samples;

        this->m_R0 = avg;
        return this->m_R0;
    }
        float calculateVolt(int samples = 10)
    {
        float avg = 0;
        
        for(int i = 0; i < samples; i++)
            avg += readVoltage();

        avg = avg / samples;

        this->m_volt = avg;
        return this->m_volt;
    }

        float setR0(float R0)
        {
            return this->m_R0 = R0;
        }

        // Calculate RS (sensor resistance)
 float readRS() 
    {  
    float RL = this->m_LoadResistor; 
    float VC = 5.0;
    //float rs = RL*VC/(calculateVolt()-1.0);
    float rs = (calculateVolt()*RL)/(VC-calculateVolt());
    return rs; 
    }

    float readSavedR0()
    {
        return this->m_R0;
    }

    float readRSR0()
    {
        return (readRS()/this->m_R0);
    }

    float readPPM(float a, float b)
    {
        return a * pow(readRSR0(), b);
    }
};