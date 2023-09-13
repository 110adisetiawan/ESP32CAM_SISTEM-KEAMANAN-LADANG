var storage = firebase.storage();
var storageRef = storage.ref();
//var database = firebase.database();

$('#List').find('tbody').html('');
$('#List').find('content').html('');
$('#List').find('dataAllert').html('');

var i=0;


storageRef.child('datas/').listAll().then(function(result){

    result.items.forEach(function(imageRef){

        //console.log("Image reference : " + imageRef);
        // var lastURLSegment = imageRef.toString().substr(imageRef.lastIndexOf('/') + 1);
        // console.log(lastURLSegment);
        var pageURL = imageRef.toString();
        var lastURLSegment = pageURL.substr(pageURL.lastIndexOf('/') + 1);
        //console.log(lastURLSegment);
        i++;
        displayImage(i,lastURLSegment, imageRef);
        sortData(i,lastURLSegment, imageRef);
        displayImage2(i, lastURLSegment, imageRef);
        //data.push(pageURL);

    });
});

function displayImage(row, lastURLSegment, images){

    images.getDownloadURL().then(function(url){

        //console.log(url);

        let new_html = '';
        new_html += '<tr>';
        new_html += '<td>';
        new_html += row;
        new_html += '</td>';
        new_html += '<td>';
        new_html += lastURLSegment;
        new_html += '</td>';
        new_html += '<td>';
        new_html += '<img src ="'+url+'" style="float:right"> ';
        new_html += '</td>';
        new_html += '</tr>';
        $('#List1').find('tbody').append(new_html);


    });
}
function displayImage2(no, lastURLSegment, images){

    images.getDownloadURL().then(function(url){
        let imagess = '';
        imagess += '<div class="col mt-2 mb-2">';
        imagess += '<div class="card ">';
        imagess += '<img src="'+url+'" class="img-fluid" alt="img-thumbnails">';
        imagess += '<div class="card-body">';
        imagess += '<h5 class="card-title">'+lastURLSegment+'</h5>';
        imagess += '</div>';
        imagess += '</div>';
        imagess += '</div>';
        
        $('#List').find('dataAllert').append(imagess);

    });
}

function sortData(i, lastURLSegment, images){
    if (i==1) {

        images.getDownloadURL().then(function(url){
            
        let images = '';
        images += '<div class="col">';
        images += '<div class="card" style="width: 35rem;">';
        images += '<img src="'+url+'" class="card-img-top" alt="img-thumbnails">';
        images += '<div class="card-body">';
        images += '<h5 class="card-title">'+lastURLSegment+'</h5>';
        images += '</div>';
        images += '</div>';
        images += '</div>';

        $('#List').find('content').append(images);
        });
    }

}

function update() {

}