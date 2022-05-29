/**
@file dawsReporter.cpp
@author Paul Redhead on 18/7/2020.
@copyright (C) 2020 Paul Redhead
@version 0.a
 */
//  
//
//
//  This file is part of DAWS.
//  LocoOS is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  LocoOS is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with LocoOS.  If not, see <http://www.gnu.org/licenses/>.
//
//  Version 0.a First released version
//
//
//
#include <Arduino.h>
#include <mbed.h>
#include <daws.h>
#include "dawsReporter.h"

#define DEBUG false  ///< Enable Reporter debug if needed







Reporter* Reporter::_lastInstantiated;   // pointer to last created reporter
Reporter* Reporter::_firstReporter;      // pointer to first reporter in chain
byte Reporter::_lastId;                  // last ID allocated

/**
 @brief Queue for reported events.
 
 This  queue is used by reporter based objects to report events requiring application
 level processing.  For report types see EventType.
 
 @note the rtos queue holds a list of pointers.  Here the pointers refer to reports.
 */
rtos::Queue<report_t, 16> Reporter::_reportQueue;

volatile uint16_t Reporter::_overRunCount = 0;      ///< count of overrun incidents
volatile uint16_t Reporter::_queueFullCount = 0;    ///< count of report queue full incidents



//  no void constructor as cannot be instantiated free standing.
/**
 @brief Construct reporter
 
 Construct a reporter using the given type. The id will be automatically assigned using the 
 next available id.
 
 @param type - the type of reporter to be constructed
 
 @todo reporter type is now held by the derived class and returned by a virtual function - to be removed from here.
 
 */
Reporter::Reporter(ReporterType type)
{
    //_type = type;  // type now held by inheriting class
    _id = ++_lastId;  // assign id automatically
    if (_firstReporter == nullptr) // I'm the first
    {
     
        _firstReporter = this;  // set static class variable
    
    }
    else
    {
        _lastInstantiated->_link(this); // link the former last reporter to this one
    }
    _lastInstantiated = this;   // update static class variable so this is now the last
    _nextReporter = nullptr;       // and ensure that the next reporter in chain is null
    _lastRep.timeStampIn = 0;  // set time stamps to mark event processed
    _lastRep.timeStampOut = 0;
}

/**
 @brief Construct reporter with id
 
 Construct a reporter using the given type and unique identifier.
 
 @note Does not check that id is not already in use.  The id should not be in the range of those assigned automatically.
 
 @param type - the type of reporter to be constructed
 @param id - the identity number for the reporter
 
 @note use of this constructor is deprecated - id's to be automatically assigned.
 
 */
Reporter::Reporter(ReporterType type, byte id) // as above but id specified.
{
    _id = id;
    //_type = type;
    if (_firstReporter == nullptr) // I'm the first
    {
        
        _firstReporter = this;
        
    }
    else
    {
        _lastInstantiated->_link(this); // link the previous one to this one
    }
    _lastInstantiated = this;   // make this one the last one
    _nextReporter = nullptr;
}

/**
 @brief Get Next Reporter
 *********************************
 
 Return the pointer to the next reporter in the chain.
 This will be nullptr if this is the last reporter.
 
 @returns the pointer to the next reporter or nullptr if last
 */

Reporter* Reporter::getNextReporter()
{
    return(_nextReporter);
}

/**
@brief Set up

Default version of the setUp function
for derived classes that do not need their own.
 
 The defaut version does nothing.
 
 @note This is declared as being virtual.

*/
void Reporter::setup()
{
    // default version of virtual function
}

/*********************************
 _link
 *********************************
 
 Link the next reporter in the chain to this reporter.
 This reporter should be the last in the chain,
 and the linked reporter will be the last in the chain.
 Warning  - neither of these is checked.
 
 parameters  - pointer to the new last reporter in the chain
 
 returns none
 *********************************/

void Reporter::_link(Reporter* newReporter)
{
    _nextReporter = newReporter;     // set up next in chain
}

/**
 @brief Get First Reporter
 
 A static routine (i.e not associated with a class object) to
 return the pointer to the first reporter.  This is a static
 varialble common to all class members.
 
 @note This is a static function
 
 @returns pointer to the first reporter in the chain.
 *********************************/

Reporter* Reporter::getFirstReporter()
{
    return(_firstReporter);
}

/**
@brief Get Identifier

 Retreives the reporter id
 


@return unique reporter id
*********************************/
byte Reporter::getId()
{
    return(_id);
}



/**
 Add a report to the queue.
 
 This adds a report to the  report queue.  An overrun report  is generated if
 a prevous report from this object has not been processed.
 
 @param repType - type of report to be added
 
 @param info - generic information, usage depends on originating source
 
 @note this is callable from ISR and therefore should not include DEBUG prints.
 
 */
void Reporter::queueReport(EventType repType, int info)
{
    bool overRun = (_lastRep.timeStampIn > 0); // last report unacknowledged
    
    report_t* rp;
    if (overRun)
    {
        
        rp = &_overRunRep;
        
        rp->repType = REPORT_OVERRUN;
        _overRunRep.info = repType;     // save actual report type as info
        _overRunCount++;
        
    }
    else
    {
        rp = &_lastRep;
        rp->repType = repType;
        rp->info = info;
    }
    rp->source = this;
    rp->timeStampIn = micros();
    rp->timeStampOut = 0;
    if(!_reportQueue.try_put(rp))
    {
        // queue full
        _queueFullCount++;
    }
}
/**
 Get a report from queue without waiting.
 
 This attempts to retrieve a report from the queue.  It returns false immediately if
 the queue is empty.  If there is a report, its content is copied to the requestor and
 true is returned. The time stamp as set by the originator in the originator's report is set to 0 to mark that
 responsiblity for processing the report is taken.
 @param rdp - pointer to where the report is to be copied.
 
 @return true if a report returned.
 */
bool Reporter::tryGetReport(report_t* rdp)
{
    return (tryGetReport(rdp, (rtos::Kernel::Clock::duration_u32)0));
}

/**
 Get a report from queue with wait time
 
 This attempts to retrieve a report from the queue.  It returns false if wait time is exceeded.
 If there is a report, its content is copied to the requestor and
 true is returned.  The time stamp as set by the originator in the originator's report is set to 0 to mark that
 responsiblity for processing the report is taken.
 @param rdp - pointer to where the report is to be copied.
 @param waitTime - time to wait specified as an rtos clock duration.
 
 @return true if a report returned.
 */


bool Reporter::tryGetReport(report_t* rdp, rtos::Kernel::Clock::duration_u32 waitTime)
{
    report_t* rsp;  // we will put the source pointer here
    if (_reportQueue.try_get_for(waitTime, &rsp))
    {
        // there's something there
        // copy data fields to target
        rdp->repType = rsp->repType;
        rdp->source = rsp->source;
        rdp->info = rsp->info;
        rdp->timeStampIn = rsp->timeStampIn;  // including time stamp
        rdp->timeStampOut = micros();         // set time now for recipient
        rsp->timeStampOut = rdp->timeStampOut; // and source
        rsp->timeStampIn = 0;                   // but clear originating time to free
        return(true);
    }
    else
    {
        return(false);
    }
}
