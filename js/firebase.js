var storage = firebase.storage();
var storageRef = storage.ref();

$('#List').find('tbody').html('');

var i=0;

storageRef.child('datas/').listAll().then(function(result){

    result.items.forEach(function(imageRef){

        console.log("Image reference : " + imageRef.toString());
    });
});