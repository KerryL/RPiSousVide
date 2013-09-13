<?php
/**
 * WebSettings.php
 *
 * Holds WebSettings class to manage all user-specified settings associated with the 
 * PHP web interface.
 * 
 * @author Matt Jarvis
 * @copyright Copyright (C) 2013 Matt Jarvis
 * @package RPiSousVide
 * @license GNU General Public License v2.0 (http://www.gnu.org/licenses/gpl-2.0.html)
 */

    require_once("Units.php");

    /**
     * WebSettings class
     *
     * Class to manage all user settings related to the PHP web interface to the Rasberry Pi
     * Sous Vide controller.  Handles object persistence with a local settings file encoded
     * in standard json.  Uses a basic singleton pattern to prevent multiple instances.
     */
    class WebSettings
    {
        public $settingsFile = "settings.json";
        public $host;
        public $port;
        public $logfile;
        public $units;

        // Holds the single instance of this class
        private static $instance;

        /**
         * Deserialize() function
         * 
         * Attempts to load the local settings file ($settingsFile) to retrieve values for the
         * class properites.  If no file can be located, default values are assigned and serialized.
         */
        private function Deserialize()
        {
            if (file_exists($this->settingsFile))
            {
                $parameters = json_decode(file_get_contents($this->settingsFile));
                $this->host    = $parameters->{'host'};
                $this->port    = $parameters->{'port'};
                $this->logfile = $parameters->{'logfile'};
                $this->units   = $parameters->{'units'};
            }
            else
            {
                $this->host    = "localhost";
                $this->port    = "24601";
                $this->logfile = "~/sv-log.txt";
                $this->units   = Units::Fahrenheit;
                $this->Serialize();
            }

        }

        /**
         * Serialize() function
         *
         * Saves the current class properties to the local settings file.  Existing values
         * are overwritten.
         */
        public function Serialize()
        {
            $parameters = array(    'host'    => $this->host,
                                    'port'    => $this->port,
                                    'logfile' => $this->logfile,
                                    'units'   => $this->units );
            if (!file_put_contents($this->settingsFile, json_encode($parameters)))
            {
                ;
            }
        }

        // Constructor is private to prevent external instancing
        private function __construct() 
        { 
            $this->Deserialize();
        }
    
        /**
         * GetInstance() static function
         * 
         * Attempts to retrieve and return the only instance of the WebSettings class.  If the
         * instance is empty, a new one is created.
         * @return WebSettings 
         */
        public static function GetInstance()
        {
            if (empty(self::$instance))
            {
                self::$instance = new WebSettings();
            }
            return self::$instance;
        }
    }

?>