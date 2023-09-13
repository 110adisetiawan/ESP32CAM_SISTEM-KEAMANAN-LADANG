$(document).ready(function(){

	var sensor_status = document.getElementById("sensor_status");

	
		// sensor_status = snap.val().Sensor.sensor1;
		if(sensor_status == "1"){    // check from the firebase
			//$(".Light1Status").text("The light is off");
			document.getElementById("aktif").disabled = true;
			document.getElementById("mati").disabled = false;
			document.getElementById("status").innerHTML = "Nyala (ON)";
            document.getElementById("status").className = "h1 mb-0 font-weight-bold text-success";
		} else if (sensor_status=="0"){
			//$(".Light1Status").text("The light is on");
			document.getElementById("mati").disabled = true;
			document.getElementById("aktif").disabled = false;
			document.getElementById("status").innerHTML = "Mati (OFF)";
			document.getElementById("status").className = "h1 mb-0 font-weight-bold text-danger";
		}
        console.log(sensor_status)
});