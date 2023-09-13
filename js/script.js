const firebaseConfig = {
    apiKey: "AIzaSyCl6ogeFkMuHFv76Ik5ynEOKQ9VaKJEFk8",
    authDomain: "database-monitoring-ladang.firebaseapp.com",
    databaseURL: "https://database-monitoring-ladang-default-rtdb.asia-southeast1.firebasedatabase.app/",
    projectId: "database-monitoring-ladang",
    storageBucket: "database-monitoring-ladang.appspot.com",
    messagingSenderId: "8011345925",
    appId: "1:8011345925:web:597581d08e46b667df6c6c"
  };
  
  firebase.initializeApp(firebaseConfig);

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
	

    $(".btn").click(function(){
		var firebaseRef = firebase.database().ref().child("Sensor/sensor1");

		if(sensor_status == "1"){    // post to firebase
			firebaseRef.set("0");
			sensor_status = "0";
            
		} else {
			firebaseRef.set("1");
			sensor_status = "1";
            
            
		}
	})
});