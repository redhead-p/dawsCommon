/**
 @file daws.h



 @author  Paul Redhead on 29/08/2020.
 @copyright © 2020, 2021 Paul Redhead.
 @version 0.a  Initial release

 @brief System defines and macros

 This file contains definitions, enums etc common to multiple elements of the DAWS system.

 */
//

#ifndef ____daws__
#define ____daws__

/**
 @mainpage
 This documents the DAWS project.  Current mobile components include
 - a brushed DC PWM traction motor controller for use with
 TI DRV8871 H bridge. With:
        - PI (proportional-integral) control using bemf derived speed error,
        - bemf sampling during gap in pwm pulse train,
        - bemf filtering using a statistical median digital filter and,
        - a simple Infinite Impulse Response (IIR) low pass digital filter
 - RFID reader to read tags embedded in the track.
 - VL53L0X time-of-flight IR range sensors for the detection of buffer stops, rolling stock and other obstacles
 - an IR  drive wheel revolution counter for measuring distance moved (odometer) and estimating speed.

 A differential amplifier measures bemf across
 motor terminals.

 Accessory (static) components include
 - point motor servo drivers
 - quadrature decoder for user input

 Bluetooth Low Energy is used for communications between static and mobile components.

 @image html DAWS.png  "DAWS - mindmap" width = 1000

 */

/** @defgroup mbedClasses  mbed classes


 These are the mbed classes used within daws.

 See mbed documentation for full details.

 @note Not an exhaustive list


 @{
 */
/**
 @class rtos::Thread

 @brief rtos::Thread stub

 This is the rtos Thread class.
 */

/**
 @class mbed::Ticker

 @brief mbed::Ticker stub

 This is the mbed Ticker class
 */
/**
 @class rtos::Semaphore

 @brief rtos::Semaphore stub

 This is the rtos Semaphore class
 */

/**
 @class events::EventQueue

 @brief events::EventQueue stub

 This is the rtos events EventQueue class
 */

/**
 @class mbed::Timeout

 @brief mbed::Timeout stub

 This is the mbed Timeout class
 */

/**
 @class mbed::NonCopyable

 @brief mbed::NonCopyable stub

 This is the mbed NonCopyable class.  Classes inheriting from this are not copyable.
 */

/**
 @class ble::Gap::EventHandler
 @brief ble::Gap::EventHandler stub

 This is the mbed BLE event handler.
 */

/**
 @class ble::GattService
 @brief ble::GattService stub

 This is the mbed BLE GattService base class.
 */

/**
 @}
 */

/** @defgroup ISR Interrupt Service Routines

These Interrupt Service Routines are directly driven by Hardware events.
 */

/**
 @brief Fixed point binary

 32 bit (long) fractional arithmetic is used here.
 The low order 8 bits are a fractional representation x/256. Alternatively
 view this as a fixed point binary number.

 */
typedef long FixPnt2408;

#define INVERSE_SCALE 19.05 ///<  as in 1 in 19.05
#define WHEEL_DIAM 32       ///<  Driving wheel diameter 32 mm

/**
 ******************************
 @brief Rounding macro
 *********************************

 Macro to round fixed point binary numbers to integers or lower precision etc.
 Equivalent to decimal point rounding by adding 0.5 and then truncating to integer.
 E.g ROUND(x, 8) will round a fixed point FixPnt2408 number to an integer.
 @param v - fixed point binary to be rounded
 @param p - number of binary digits to be rounded out

 @return integer result of rounding
 *********************************/
#define ROUND(v, p) ((v + (((long)1) << (p - 1))) >> (p))

/**
 ******************************
 @brief Recursive Filter
 *********************************

 Performs a simple single stage recursive or
 infinite impulse response (IIR) filter where

 y[k] = x[k-1] + alpha*y[k-1]

 and y[k] is the output for this iteration k, x[k-1] is the input for this iteration and
 y[k-1] is the output of the previous iteration.  This is analogous to an analogue RC low pass
 and as in an RC filter, the filter may be cascaded to increase the frequency cut off slope.
 In this implementation alpha takes a value of ((2^n) - 1)/(2^n).  (e.g. 1/2, 3/4, 7/8 and so on).

@note  y is the rolling weighted sum - not the rolling weighted average so
 the filter has a steady state gain of 2^n, which has to be allowed for.
 I.e before use as a filtered value or input to a cascaded filter stage y[k] has to be
 divided by (2^n).

 @param x - input
 @param y - saved output of previous iteration
 @param n - as in alpha expression

 @return - output of this iteration
 *********************************/
#define LP_FILTER(x, y, n) ((y) - (ROUND(y, n)) + (x))

/** @defgroup hwAssignments Pin assignments etc for various hardwired connections, peripherals etc.

 Pins are defined here but may be addressed directly.  They are defined in terms of
 Arduino pin numbers although mbed/HW references may be used in the code.

 @todo Rationalise pin numbering so a consistent approach is taken throughout.

 @{
 */

/**
 @brief Hardware  Pins



 This is the (primary) pin number (Arduino numbering is used for reference to Arduino
 pin layouts etc).

 Reporters without direct hardware access are assigned NO_PIN (0).



 This list coveres both mobile (locomotive) and static (accessory) usage.
 It excludes standard pin Arduino pin allocations for SPI, I2C etc
*/
enum HWpin : byte
{
    NO_PIN = 0,      ///< no hw pin assignment
    VL53_FWD = 4,    ///< forward VL53L0X - XSHUT on pin D4
    VL53_REV = 5,    ///< reverse VL53L0X - XSHUT on pin D5
    PWM1 = 6,        ///<  PWM Motor 1 / Point 1
    PWM2 = 3,        ///<  PWM Motor 2 / Point 2
    PWM3 = 2,        ///< PWM Motor 3
    MOTOR_BEMF = A0, ///<  propulsion motor bemf sensor  - D14 (A0) - AIN2 -  ADC channel 3
    BATT = A1,       ///< battery monitor on D15 (A1) -
    NFC_CS = 10,     ///< PN532 - SPI CS is on D10
    NFC_IRQ = 8,     ///< PN532 - IRQ on D8
    ODO = A2         ///< odometer on D16(A2) - Comparator  input - AIN6  N.B this assignment is hard coded in driver!///
};

/**
 @}
 */

#define AUTO_PRIORITY osPriorityAboveNormal  ///< Automaton thread priority - uses I2C
#define MOTOR_PRIORITY osPriorityHigh        ///< Motor control thread priority
#define ODO_PRIORITY osPriorityHigh          ///< Odometer measurement thread priority
#define PN532_PRIORITY osPriorityAboveNormal ///< PN532 (NFC) controller thread priority
#define POINT_PRIORITY osPriorityHigh        ///< Point thread priority
#define BLE_PRIORITY osPriorityNormal        ///< BLE priority
#define Vl53_PRIORITY osPriorityAboveNormal  ///< IR TFL sensor priority - uses I2C
#define MAIN_PRIORITY osPriorityBelowNormal  ///< after initialisation only deals with UI
/**
@brief Direction

 This is used for direction of motion and for orientation of relevant objects (e.g. forward and reverse
 facing sensors) with respect to mobiles (e.g. locos etc)

@note The values in this enum are used in arithmetic.  They must not be changed.
 */

enum Dir_t : int8_t
{
    FORWARD = 1, ///< 1
    STOPPED = 0, ///< 0
    REVERSE = -1 ///< -1
};

/**
 @brief Point states

 An enumerated list of point states.

 */
enum PointState_t : byte
{
    P_UNAVAIL,       ///< server disconnected (only applicable to client)
    P_UNKNOWN,       ///< state unknown (e.g. start of day)
    P_INDETERMINATE, ///< indeterminate - command being actioned or sensors (if any) inconsistent
    P_NORMAL,        ///< Normal (closed) - typically set straight
    P_REVERSE        ///< Reverse (thrown) - typically set divergent
};

/**
 @brief Point positions as an enumerated list

 This defines the point positions to be use as values for the command characteristic.

 */
enum PointPos_t : char
{
    POINT_NORMAL = 'N', ///< Point normal (closed) - set straight
    POINT_REVERSE = 'R' ///< Point reverse (thrown) - set divergent
};

#endif /* daws_h */
