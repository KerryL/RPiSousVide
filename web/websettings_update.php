<?php
/**
 * websettings_update.php
 *
 * PHP script to update web settings object based on a form submit
 * 
 * @author Matt Jarvis
 * @copyright Copyright (C) 2013 Matt Jarvis
 * @package RPiSousVide
 * @license GNU General Public License v2.0 (http://www.gnu.org/licenses/gpl-2.0.html)
 */

    require_once("WebSettings.php");

    $settings   = WebSettings::GetInstance();

    if (isset($_POST['submit_websettings']))
    {
        //print_r($_POST);

        if ($_POST['units'] == "kelvin")
            $settings->units = Units::Kelvin;
        if ($_POST['units'] == "celsius")
            $settings->units = Units::Celsius;
        if ($_POST['units'] == "fahrenheit")
            $settings->units = Units::Fahrenheit;

        $settings->host = $_POST['host'];
        $settings->port = $_POST['port'];
        $settings->logfile = $_POST['logfile'];

        $settings->Serialize();

        header("location:settings_page.php");

    }
?>