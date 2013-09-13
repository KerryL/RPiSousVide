<?php
/**
 * index.php
 *
 * Main web interface to Rasberry Pi Sous Vide temperature controller
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
        <title>Rasberry Pi Sous Vide Controller</title>
        <link href="styles.css" rel="stylesheet" type="text/css">
    </head>
    <body>
        <div id="wrapper">

            <div class="pageHeader">
            <h1>Rasberry Pi Sous Vide Controller</h1>
            </div>

            <div class="mainPanel">
                <h2>Temperature Control</h2>
                <p>Basic temperature control panel plus temperature vs time graph (javascript refresh?)</p>
                
                <span class="controlPanel">
                    <form id="tempControlPanel" action="">
                    <table>
                        <tr>
                            <td class="labelDisplay">Temp:</td>
                            <td>
                                <span class="valueDisplay"><?php echo $controller->GetTemperature(); ?></span>
                                <span class="valueSuffix"><?php echo $controller->GetUnitSuffix(); ?></span>
                            </td>
                        </tr>
                        <tr>
                            <td class="labelDisplay">Set:</td>
                            <td>
                                <input class="valueDisplay" type="text" name="setpoint" value="<?php echo $controller->GetSetpoint(); ?>" />
                                <span class="valueSuffix"><?php echo $controller->GetUnitSuffix(); ?></span>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="2">
                                <div class="buttonHolder">
                                    <input class="updateButton" type="submit" name="submit_setpoint" value="Update Setpoint" />
                                </div>
                            </td>
                        </tr>
                    </table>
                    </form>
                </span>

                <div class="graphPanel">
                    <p>Graph Goes Here</p>
                </div>
            </div>

            <div class="mainPanel">
                <h2>Device Status</h2>
                <p>State checks made to C++ controller software via network socket go here</p>
                <div>
                    <a href="settings_page.php" class="updateButton">Edit Settings</a>
                </div>
                
            </div>

        </div>
    </body>
</html>