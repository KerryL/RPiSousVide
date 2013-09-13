<?php
/**
 * Units.php
 *
 * Holds abstract Units class for handling conversion between temperature units.
 * 
 * @author Matt Jarvis
 * @copyright Copyright (C) 2013 Matt Jarvis
 * @package RPiSousVide
 * @license GNU General Public License v2.0 (http://www.gnu.org/licenses/gpl-2.0.html)
 */
    
    /**
     * Units class
     *
     * Abstract class used as a combination enum and converter for temperature units
     */
    abstract class Units
    {
        const Kelvin     = 0;
        const Celsius    = 1;
        const Fahrenheit = 2;
    
        /**
         * GetSuffix() static function
         *
         * Takes an integer representing a set of units and returns a suffix for that unit (K, C, or F)
         * @param   int     $units     integer enum of requested unit system for suffix
         * @return  string
         */
        public static function GetSuffix($units)
        {
            $suffix = " K";
            
            if ($units == self::Celsius)
            {
                $suffix = " C";
            }

            if ($units == self::Fahrenheit)
            {
                $suffix = " F";
            }

            return $suffix;
        }

        /**
         * ConvertFromKelvin() static function
         *
         * Takes a floating point value (in Kelvins) and an integer representation of a unit system
         * and converts temperature into that unit system.
         * @param   float   $value      temperature
         * @param   int     $units      integer enum of desired unit system
         * @return  float
         */
        public static function ConvertFromKelvin($value, $units)
        {
            if ($units == self::Celsius)
            {
                $value = $value - 273.15;
            }

            if ($units == self::Fahrenheit)
            {
                $value  = ($value - 273.15) * 1.8 + 32;
            }

            return $value;
        }

        /**
         * ConvertToKelvin() static function
         *
         * Takes a floating point value (in Kelvins) and an integer representation of a unit system
         * and converts temperature into that unit system.
         * @param   float   $value      temperature to convert
         * @param   int     $units      integer enum of $value's unit system
         * @return  float
         */
        public static function ConvertToKelvin($value, $units)
        {
            if ($units == self::Celsius)
            {
                $value = value + 273.15;
            }

            if ($units == self::Fahrenheit)
            {
                $value = ($value - 32) / 1.8 + 273.15; 
            }

            return $value;
        }

    }
?>