<?php
/**
 * ControllerManager.php
 *
 * Holds ControllerManager class for managing PHP interaction with the C++ controller
 * software.
 * 
 * @author Matt Jarvis
 * @copyright Copyright (C) 2013 Matt Jarvis
 * @package RPiSousVide
 * @license GNU General Public License v2.0 (http://www.gnu.org/licenses/gpl-2.0.html)
 */
               
    require_once("WebSettings.php");
    require_once("Units.php");

    /**  
     * ControllerManager class
     * 
     * Basic class to abstract all communication between the C++ PID controller
     * software and the PHP web interface.  
     */
    class ControllerManager
    {
        private $settings;
        
        /**
         * GetUnitSuffix() function
         * 
         * Using the global settings object this function returns a string with the 
         * unit suffix for the selected temperature units. 
         * @return string
         */
        public function GetUnitSuffix()
        {
            return Units::GetSuffix($this->settings->units);
        }

        /**
         * GetTemperature() function
         * 
         * Uses a network socket to connect with the C++ PID temperature controller and
         * request the currently read temperature.  
         * @return float
         */
        public function GetTemperature()
        {
            //TODO: Replace this dummy value with actual network logic
            $temp = 290;
            
            return Units::ConvertFromKelvin($temp, $this->settings->units);
        }

        /**
         * GetSetpoint() function
         * 
         * Uses a network socket to connect with the C++ PID temperature controller and
         * request the current temperature setpoint.
         * @return float
         */
        public function GetSetpoint()
        {
            //TODO: Replace this dummy value with actual network logic
            $temp = 310;
            
            return Units::ConvertFromKelvin($temp, $this->settings->units);

        }

        /**
         * PutSetpoint() function
         *
         * Uses a network socket to connect with the C++ PID temperature controller and
         * assign a temperature setpoint.
         * @param   float   $value      - temperature value in local unit system
         * @return bool     true if successful
         */
        public function PutSetpoint($value)
        {
            $kelvin = Units::ConvertToKelvin($value, $this->settings->units);
            //TODO: insert setpoint assignment here

            return true;
        }

        public function __construct()
        {
            $this->settings = WebSettings::GetInstance();
        }

    }



?>