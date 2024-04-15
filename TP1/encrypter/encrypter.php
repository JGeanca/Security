<?php
define("MAX_WORDS", 1000); // # palabras de la lista, si usamos todas gasta mucha memoria (son 10 millones)
define("MAX_COMBINATIONS", 10);

$wordsUrl = "https://raw.githubusercontent.com/danielmiessler/SecLists/master/Passwords/xato-net-10-million-passwords-1000000.txt";
$wordsArray = file($wordsUrl, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$wordsArray = array_slice($wordsArray, 0, MAX_WORDS);


//* This is random keyCombinations:
// $passwordCombinations = [];
// for ($i = 0; $i < MAX_COMBINATIONS; $i++) {
//   $randomPassword = "";
//   for ($j = 0; $j < 3; $j++) {
//     $randomWord = $wordsArray[array_rand($wordsArray)];
//     $randomPassword .= $randomWord;
//   }
//   $passwordCombinations[] = $randomPassword;
// }

//* Same keyCombinations:
$passwordCombinations = [];
for ($i = 0; $i < MAX_COMBINATIONS; $i++) {
  $password = "";
  for ($j = 0; $j < 3; $j++) {
    $word = $wordsArray[$i * $i + $j * 13];
    $password .= $word;
  }
  $passwordCombinations[] = $password;
}

echo "<br>Usuarios y contraseñas: <br><br>";
$users = [];
for ($i = 0; $i < 10; $i++) {
  $username = "user" . $i;
  $password = $passwordCombinations[$i];
  echo "Usuario: $username, Contraseña: $password <br>";
  $users[$username] = hash('sha256', $password);
}



$csvFile = fopen('users.csv', 'w');
foreach ($users as $username => $hashedPassword) {
  fputcsv($csvFile, [$username, $hashedPassword]);
}
fclose($csvFile);
