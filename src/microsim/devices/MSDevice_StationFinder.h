/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2001-2024 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    MSDevice_StationFinder.h
/// @author  Michael Behrisch
/// @date    2023-05-24
///
// A device which triggers rerouting to nearby charging stations
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSVehicleDevice.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSDevice_Battery;
class MSStoppingPlace;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_StationFinder
 * @brief A device which triggers rerouting to nearby charging stations
 *
 * Each device checks the battery status by being
 *  called each time step and initiaiting the search for a compatible
 *  charging station if the battery level is too low.
 *
 * @see MSDevice
 */
class MSDevice_StationFinder : public MSVehicleDevice {
public:
    /** @brief Inserts MSDevice_StationFinder-options
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether stationFinder-devices shall be built
     *  for the given vehicle.
     *
     * For each seen vehicle, the global vehicle index is increased.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[in, filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);


public:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     */
    MSDevice_StationFinder(SUMOVehicle& holder);

    /// @brief Destructor.
    ~MSDevice_StationFinder();

    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Computes current emission values and adds them to their sums
        *
        * The vehicle's current emission values
        *  are computed using the current velocity and acceleration.
        *
        * @param[in] veh The regarded vehicle
        * @param[in] oldPos Position before the move-micro-timestep.
        * @param[in] newPos Position after the move-micro-timestep.
        * @param[in] newSpeed The vehicle's current speed
        * @return false, if the vehicle is beyond the lane, true otherwise
        * @see MSMoveReminder
        * @see MSMoveReminder::notifyMove
        * @see PollutantsInterface
        */
    bool notifyMove(SUMOTrafficObject& veh, double oldPos, double newPos, double newSpeed);

    /** @brief Computes idling emission values and adds them to the emission sums
        *
        * Idling implied by zero velocity, acceleration and slope
        *
        * @param[in] veh The vehicle
        *
        * @see MSMoveReminder::notifyMove
        * @see PollutantsInterface
        */
    bool notifyIdle(SUMOTrafficObject& veh);

    /// @}

    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "stationfinder";
    }

    /** @brief Called on writing tripinfo output
     *
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     * @see MSDevice::tripInfoOutput
     */
    void generateOutput(OutputDevice* tripinfoOut) const;

    void setBattery(MSDevice_Battery* battery) {
        myBattery = battery;
    }

protected:
    /** @brief Internal notification about the vehicle moves, see MSMoveReminder::notifyMoveInternal()
     *
     */
    void notifyMoveInternal(const SUMOTrafficObject& veh,
                            const double frontOnLane,
                            const double timeOnLane,
                            const double meanSpeedFrontOnLane,
                            const double meanSpeedVehicleOnLane,
                            const double travelledDistanceFrontOnLane,
                            const double travelledDistanceVehicleOnLane,
                            const double meanLengthOnLane);

private:
    /// @brief The corresponding battery device
    MSDevice_Battery* myBattery;

    /// @brief The safety buffer when calculating expected consumption
    double myReserveFactor;

    /// @brief To which station we are currently travelling
    MSStoppingPlace* myChargingStation;

private:
    /// @brief Invalidated copy constructor.
    MSDevice_StationFinder(const MSDevice_StationFinder&);

    /// @brief Invalidated assignment operator.
    MSDevice_StationFinder& operator=(const MSDevice_StationFinder&);

};
