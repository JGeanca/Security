<?php
//session_start(); // To identify the user

define("MAX_WORDS", 9999);
$wordsUrl = "https://raw.githubusercontent.com/danielmiessler/SecLists/master/Passwords/xato-net-10-million-passwords-1000000.txt";
$wordsArray = file($wordsUrl, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$wordsArray = array_slice($wordsArray, 0, MAX_WORDS);


function loadUsersFromCSV($file)
{
  $users = [];
  if (($handle = fopen($file, "r")) !== false) { // Open the file in read mode
    while (($data = fgetcsv($handle, 1000, ",")) !== false) {
      $users[$data[0]] = $data[1];
    }
    fclose($handle);
  }
  return $users;
}

$usernameError = $passwordError = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") { // Wait for the form to be submitted
  // Check if the username field is empty
  if (empty($_POST['username'])) {
    $usernameError = "Por favor, ingrese un nombre de usuario";
  } else {
    $username = $_POST['username'];
  }

  // Check if the password field is empty
  if (empty($_POST['password'])) {
    $passwordError = "Por favor, genere una contraseña";
  } else {
    $password = $_POST['password'];
  }

  // If both username and password are provided, proceed
  if (!empty($username) && !empty($password)) {
    $hashedPassword = hash('sha256', $password);

    if (($handle = fopen('users.csv', 'a')) !== false) {
      fputcsv($handle, [$username, $hashedPassword]);
      fclose($handle);
    } else {
      echo "DATABASE ERROR: Could not open file";
    }
  }
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Registrar usuario</title>
  <script>
    function generatePassword() {
      var words = <?php echo json_encode($wordsArray); ?>;
      var newPassword = '';
      for (var i = 0; i < 3; i++) {
        newPassword += words[Math.floor(Math.random() * words.length)];
      }
      return newPassword;
    }

    function generateNewPassword() {
      var newPassword = generatePassword();
      document.getElementById("password").value = newPassword;
    }
  </script>
</head>

<body>
  <h2>Registrar nuevo usuario</h2>
  <form action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>" method="post">
    <span style="color: red;"><?php echo $usernameError; ?></span><br>
    <label for="username">Nombre de usuario:</label><br>
    <input type="text" id="username" name="username"><br>

    <br><span style="color: red;"><?php echo $passwordError; ?></span><br>
    <label>Contraseña:</label><br>
    <input type="text" id="password" name="password" value="" readonly><br><br>
    <input type="button" value="Generar contraseña" onclick="generateNewPassword()"><br><br>


    <input type="submit" value="Crear usuario">
  </form>
</body>

</html>