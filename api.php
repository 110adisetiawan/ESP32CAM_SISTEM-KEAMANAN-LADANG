<?php

//koneksi ke database
include "database/conn.php";


//membuat query
$sql = "SELECT * FROM sensor";
$query = mysqli_query($conn, $sql);
if ($query) {
    header("Content-Type: JSON");
    $i = 0;
    while ($row = mysqli_fetch_array($query)) {
        $data = array(
            'id' => $row["id"],
            'nama_sensor' => $row["nama_sensor"],
            'status' => $row["status"]
        );
    }
    echo json_encode($data, JSON_PRETTY_PRINT);
} else {
    echo "Database Connection Failed";
}
