<!doctype html>
<html>
	<head>
		<title>Rasberry Pi Sous Vide Controller</title>
		<style>
			
			html {
				height: 100%;
			}

			table {
				border: 0;
			}
			body {
				background-image: linear-gradient(0deg, rgb(240,240,240), rgb(210,210,210));
				height: 100%;
				margin: 0;
				background-repeat: no-repeat;
				background-attachment: fixed;
				font-family: Arial, Helvetica, sans-serif;
			}

			.pageHeader h1 {
				font-family: Arial, Helvetica, sans-serif;
				color: rgb(100,100,100);
				text-shadow: 1px 1px 2px rgb(200,200,200);
				padding: 5px;
				border-bottom: 2px solid rgb(100,100,100);
				font-family: Tahoma, Helvetica, sans-serif;

			}

			.mainPanel {
				border: 1px solid rgb(100, 100, 100);
				border-radius: 5px;
				margin: 10px 0px 10px 0px;
				padding: 10px;
				box-shadow: 2px 2px 4px rgb(150, 150, 150);
				background-color: rgb(250, 250, 250);
				white-space: nowrap;
			}

			.mainPanel h2 {
				margin: 0;
				font-family: Tahoma, Helvetica, sans-serif;
				text-shadow: 1px 1px 2px rgb(200,200,200);
				color: rgb(100,100,100);
			}

			.controlPanel {
				display: inline-block;
			
			}

			.labelDisplay {
				text-align: right;
				font-size: 1.5em;
				font-weight: bold;
			}

			.valueDisplay {
				display: inline-block;
				width: 4em;
				text-align: center;
				border: 1px solid rgb(100,100,100);
				font-size: 1.5em;
				border-radius: 3px;

			}

			.buttonHolder {
				text-align: right;
			}
			.updateButton {
				font-size: 1.1em;
				background-color: rgb(100, 100, 200);
				color: rgb(255, 255, 255);
				border-radius: 3px;
				border: 1px solid rgb(100, 100, 100);
			}

			.graphPanel {
				height: 400px;
				text-align: center;
				color: rgb(200, 200, 200);
				border: 1px dashed rgb(200, 200, 200);
			}

			#wrapper {
				margin-left: 44px;
				margin-right: 44px;
				border-radius: 5px;
				
			}
		</style>
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
							<td><span class="valueDisplay">100</span></td>
						</tr>
						<tr>
							<td class="labelDisplay">Set:</td>
							<td><input class="valueDisplay" type="text" name="setpoint" /></td>
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
			</div>
		</div>
	</body>
</html>