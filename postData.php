<?php

include "database/conn.php";

if (isset($_POST['status'])) {

    session_start();


    $id = $_POST["id"];
    $changeStatus = $_POST["status"];

    $sql = "UPDATE alarm SET statusAlarm='$changeStatus' WHERE id='$id'";

    // Execute the query
    if (mysqli_query($conn, $sql)) {
        echo "UPDATE STATUS SUKSES";
    }
    $_SESSION['flash_message'] = "Status Data Alarm " . $id . " Berhasil dirubah ke " . $changeStatus;
    header('location: dataAllert.php');
}
