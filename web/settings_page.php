<?php
/**
 * settings_page.php
 *
 * User settings interface for Rasberry Pi Sous Vide temperature controller
 * 
 * @author Matt Jarvis
 * @copyright Copyright (C) 2013 Matt Jarvis
 * @package RPiSousVide
 * @license GNU General Public License v2.0 (http://www.gnu.org/licenses/gpl-2.0.html)
 */

    require_once("WebSettings.php");
    require_once("ControllerManager.php");

    $settings   = WebSettings::GetInstance();
    $controller = new ControllerManager();
?>

<!doctype html>
<html>
    <head>
        <title>Rasberry Pi Sous Vide Controller - Modify Settings</title>
        <link href="styles.css" rel="stylesheet" type="text/css">
    </head>
    <body>
        <div id="wrapper">

            <div class="pageHeader">
            <h1>Rasberry Pi Sous Vide Controller</h1>
            </div>

            <div class="mainPanel">
                <h2>Web Interface Settings</h2>
                <p>This panel allows you to modify the settings for the PHP web interface to the RPi Sous Vide controller.</p>
                
                <div class="settingsTableFrame">
                <form id="webSettingsForm" action="websettings_update.php" method="POST">
                    <div class="tableHeader">Web Settings</div>
                    <table>
                        <tr>
                            <td class="smallLabelDisplay alignRight">Units:</td>
                            <td class="smallLabelDisplay alignLeft"><input type="radio" name="units" <?php echo ($settings->units == Units::Kelvin) ? "checked" : "" ?> value="kelvin">Kelvin</td>
                        </tr>
                        <tr>
                            <td class="smallLabelDisplay alignRight"></td>
                            <td class="smallLabelDisplay alignLeft"><input type="radio" name="units" <?php echo ($settings->units == Units::Celsius) ? "checked" : "" ?> value="celsius">Celsius</td>
                        </tr>
                        <tr>
                            <td class="smallLabelDisplay alignRight"></td>
                            <td class="smallLabelDisplay alignLeft"><input type="radio" name="units" <?php echo ($settings->units == Units::Fahrenheit) ? "checked" : "" ?> value="fahrenheit">Fahrenheit</td>
                        </tr>
                        <tr>
                            <td class="smallLabelDisplay alignRight">C++ Host:</td>
                            <td class="smallLabelDisplay alignLeft"><input class="settingsTextBox smallLabelDisplay" type="text" name="host" value="<?php echo $settings->host; ?>" /></td>
                        </tr>
                        <tr>
                            <td class="smallLabelDisplay alignRight">C++ Port:</td>
                            <td class="smallLabelDisplay alignLeft"><input class="settingsTextBox smallLabelDisplay" type="text" name="port" value="<?php echo $settings->port; ?>" /></td>
                        </tr>
                        <tr>
                            <td class="smallLabelDisplay alignRight">C++ Log File:</td>
                            <td class="smallLabelDisplay alignLeft"><input class="settingsTextBox smallLabelDisplay" type="text" name="logfile" value="<?php echo $settings->logfile; ?>" /></td>
                        </tr>
                    </table>
                    <input class="updateButton" type="submit" name="submit_websettings" value="Update Web Settings" />
                </form>
                </div>

                <div>
                    <a href="index.php" class="updateButton">Return To Main</a>
                </div>
                
            </div>

            <div class="mainPanel">
                <h2>PID Software Settings</h2>
                <p>This panel allows you to modify the settings for the C++ PID software running on the Rasberry Pi</p>
                <div>
                    <a href="index.php" class="updateButton">Return To Main</a>
                </div>
                
            </div>

        </div>
    </body>
</html>