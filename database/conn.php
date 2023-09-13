<?php
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "web_api";

$conn = mysqli_connect($servername, $username, $password, $dbname);
if (!$conn) {
    die("Koneksi ke database tidak berhasil");
}
