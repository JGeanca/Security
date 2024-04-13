<?php
session_start(); // To identify the user

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

if ($_SERVER["REQUEST_METHOD"] == "POST") { // Wait for the form to be submitted
  // Check if the username and password fields are set
  if (isset($_POST['username']) && isset($_POST['password'])) {
    $users = loadUsersFromCSV('users.csv');

    $username = $_POST['username'];
    $password = $_POST['password'];

    // Hash the password to compare it with the stored one
    $hashedPassword = hash('sha256', $password);

    if (isset($users[$username]) && $users[$username] === $hashedPassword) {
      $_SESSION['username'] = $username;
      header("Location: loginSuccessPage.html");
      exit();
    } else {
      $error = "Usuario o contraseña incorrecta.";
    }
  }
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Login</title>
</head>

<body>
  <h2>Iniciar sesión</h2>
  <?php if (isset($error)) { ?>
    <p style="color: red;"><?php echo $error; ?></p>
  <?php } ?>
  <form action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>" method="post">
    <label for="username">Usuario:</label><br>
    <input type="text" id="username" name="username"><br>
    <label for="password">Contraseña:</label><br>
    <input type="password" id="password" name="password"><br><br>
    <input type="submit" value="Iniciar sesión">
  </form>
</body>

</html>