//
/**
 @file dawsReporter.h
 @author Paul Redhead
 @copyright (C) 2019/2020 Paul Redhead
 @version 0.a  Initial release
 */

//
//  This file is part of DAWS.
//  DAWS is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  DAWS is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with DAWS.  If not, see <http://www.gnu.org/licenses/>.
//


#ifndef ____dawsReporter__
#define ____dawsReporter__





/**
@brief Reporter Type

 This is used to allow a reporter reference to be cast to the correct class reference type
to permit access to the relevant class methods for inheriting classes.
 
 Types cover both static accessory and mobile usage.
 
 */
enum ReporterType : char {
    SERVO_REP   = 'S', ///< servo reporter
    MOTOR_REP   = 'M', ///< motor state reporter  - reserved - not in use
    VL53_REP    = 'V', ///< IR time of flight distance sensor
    NFC_REP     = 'N', ///< Near Field Comms controller
    NTAG_REP    = 'U', ///< NFC NTAG Target
    DEP_REP     = 'D', ///< NFC DEP target
    ODO_REP     = 'O', ///< Odometer reporter
    RA_REP      = 'R', ///< Remote Accessory
    ACC_REP     = 'A', ///< Accessory reporter
    QDEC_REP    = 'Q', ///< Quadrature decoder reporter
    AUTO_REP    = 'L', ///< Loco Automaton Reporter
    BLE_REP     = 'B'  ///< BLE reporter
};
/**
@brief Report Type

 This defines the type of report so the event handler can process it accordingly.
 
 @note some report types only apply to mobile sketch usage, others are static/accessory usage and some
 are common to both
 */
enum EventType : byte
{
    REPORT_OVERRUN,      ///< previous report from this source has not been processed.
    
    LOCO_STOP,           ///< loco stopped
    
    VL53_RANGE_CLOSE,  ///<  distance read - below critical distance
    VL53_RANGE_NORMAL,  ///< distance read - above critical distance
    VL53_OUT_OF_RANGE,   ///< range read error indicating out of range
    VL53_ERR,            ///< range read error indicating HW problem
    
    NTAG_NDEF,           ///< NFC ULTRALIGHT (NTAG21x) found with NDEF content
    NTAG_NONDEF,         ///< NFC ULTRALIGHT (NTAG21x) found - null NDEF
    
    MIFARE_C1K_FOUND,    ///< NFC MIFARE CLASSIC 1K found
    MIFARE_DEP_FOUND,    ///< DEP target found
    MIFARE_DEP_MSG,      ///< DEP message received (either way)
    MIFARE_DEP_PASSIVE,  ///< been found as a DEP passive target
    
    NFC_OTHER_FOUND,     ///< NFC other recognised tag found
    NFC_TAG_TYPE_UNKNOWN,///<NFC tag found - type unknown
    
    RA_DISCOVERED,       ///< Discovered Accessory for this remote
    RA_STATE_CHANGE,     ///< Remote accessory status update
    RA_CONNECTED,        ///< Remote accessory now connected
    RA_DISCONNECTED,     ///<Remote accessory disconnected
    
    BLE_SCAN_START,      ///<BLE Scan started
    BLE_SCAN_DONE,       ///< BLE Scan completed
    
    BLE_PEER_FOUND,      ///< BLE peer found by scan
    BLE_CONNECTED,       ///< BLE - client (central) connection to peer made but discovery/service init required
    BLE_SERVICES_AVAIL,  ///< BLE -  client (central) connection completely open - services ready
    BLE_CONNECT_FAIL,    ///< BLE -  client (central) connection open failed
    BLE_DISCONNECTED,    ///< BLE - client (central) disconnected
    
    ACC_STATE_CHANGE,    ///< Local accessory status update
    ROTQ_ROT,            ///< Rotary switch rotation
    ROTQ_ERR,            ///<Rotary switch double change (error)
    ///
    SET_AUTO              ///<Set Loco Driver Auto mode
    
    ///
};


class Reporter; // forward declaration needed for following typedef
/*typedef void (*eHandlerP_t)(EventType, Reporter*, int);  ///<  type for report handler
*/
/**
 @brief General purpose report structure
 
  This structure defines the content of a report.
 */
typedef struct
{
    EventType repType;  ///< type of report
    Reporter* source;    ///< reporter based object initiating report
    unsigned long timeStampIn; ///< time added to queue
    unsigned long timeStampOut; ///< time removed from queue
    int info; ///< addition information - usage depends on report type
} report_t;



/**
 @brief General purpose event reporter

 
 This is a virtual class.
 
 When a device manager or similar detects a significant event this event is reported.  Event reports are inserted by reference into an rtos queue.
 Many devices may detect and report events.  The report queue has a fixed capaccity.  
 
 There is a single reader for events which processes them in sequence.
 
 Reporters are chained together.  Each holding a
 link to the next in the chain, except for the
 last.  These links are created when reporters
 are constructed.  There is a static class
 variable holding the pointer to the first
 reporter.  This allows high level code
 to cycle through reporters without needing
 explicit reference to any.

 
 
 @note Reporter based class objects are not copyable
 
 */
class Reporter: mbed::NonCopyable<Reporter>
{
public:
    Reporter(ReporterType);
    Reporter(ReporterType, byte);
    virtual void setup();

    Reporter* getNextReporter();
    static Reporter* getFirstReporter();
    byte getId();

    /**
    @brief Get Reporter Type

     @note this is a pure virtual funcation and must be defined in the inheriting class.

    @return reporter type identifier as an enum member.
    *********************************/
    virtual ReporterType getType() = 0;
    void queueReport(EventType, int);
    uint16_t getQueueFullCount(); ///< not implemented yet
    static bool tryGetReport(report_t*);
    static bool tryGetReport(report_t*, rtos::Kernel::Clock::duration_u32 );

    
private:
    static rtos::Mail<report_t, 16> _reportQueue;  // report queue

    static volatile uint16_t _queueFullCount;    // count of report queue full incidents


    Reporter* _nextReporter;    ///< pointer to next reporter in chain
    byte _id;       ///< unique id
    static byte _lastId;  ///< last allocated id
    //ReporterType _type;
    static Reporter* _lastInstantiated;  ///< pointer to the last reporter to be constructed
    static Reporter* _firstReporter;     ///< pointer to first reporter in the chain
    void _link(Reporter*);  ///< link this to next reporter in chain
};


#endif /* defined(____dawsReporter__) */
