<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Page</title>
 <style>
/* Importing Google font - Open Sans */
@import url("https://fonts.googleapis.com/css2?family=Open+Sans:wght@400;500;600;700&display=swap");

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    font-family: "Open Sans", sans-serif;
}

body {
    height: 100vh;
    width: 100%;
    background: url("images/background.jpg") center/cover no-repeat;
}


header {
    position: fixed;
    width: 100%;
    top: 0;
    left: 0;
    z-index: 10;
    padding: 0 10px;
}

.navbar {
    display: flex;
    padding: 22px 0;
    align-items: center;
    max-width: 1200px;
    margin: 0 auto;
    justify-content: space-between;
}

.navbar .hamburger-btn {
    display: none;
    color: #fff;
    cursor: pointer;
    font-size: 1.5rem;
}

.navbar .logo {
    gap: 10px;
    display: flex;
    align-items: center;
    text-decoration: none;
}

.navbar .logo img {
    width: 150px;
    border-radius: 50%;
    filter: invert(1);
}

.navbar .logo h2 {
    color: #fff;
    font-weight: 600;
    font-size: 1.7rem;
}

.navbar .links {
    display: flex;
    gap: 35px;
    list-style: none;
    align-items: center;
}

.navbar .close-btn {
    position: absolute;
    right: 20px;
    top: 20px;
    display: none;
    color: #000;
    cursor: pointer;
}

.navbar .links a {
    color: #fff;
    font-size: 1.1rem;
    font-weight: 500;
    text-decoration: none;
    transition: 0.1s ease;
}

.navbar .links a:hover {
    color: #19e8ff;
}

.navbar .login-btn {
    border: none;
    outline: none;
    background: #fff;
    color: #275360;
    font-size: 1rem;
    font-weight: 600;
    padding: 10px 18px;
    border-radius: 3px;
    cursor: pointer;
    transition: 0.15s ease;
}

.navbar .login-btn:hover {
    background: #ddd;
}

.form-popup {
    position: fixed;
    top: 50%;
    left: 50%;
    z-index: 10;
    width: 100%;
    opacity: 0;
    pointer-events: none;
    max-width: 720px;
    background: #fff;
    border: 2px solid #00bcd4;
    box-shadow: 0px 0px 4px 0px;
    transform: translate(-50%, -70%);

}

.show-popup .form-popup {
    opacity: 1;
    pointer-events: auto;
    transform: translate(-50%, -50%);
    transition: transform 0.3s ease, opacity 0.1s;
}

.form-popup .close-btn {
    position: absolute;
    top: 12px;
    right: 12px;
    color: #878484;
    cursor: pointer;
}

.blur-bg-overlay {
    position: fixed;
    top: 0;
    left: 0;
    z-index: 10;
    height: 100%;
    width: 100%;
    opacity: 0;
    pointer-events: none;
    backdrop-filter: blur(5px);
    -webkit-backdrop-filter: blur(5px);
    transition: 0.1s ease;
}

.show-popup .blur-bg-overlay {
    opacity: 1;
    pointer-events: auto;
}

.form-popup .form-box {
    display: flex;
}

.form-box .form-details {
    width: 100%;
    color: #fff;
    max-width: 330px;
    text-align: center;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
}

.login .form-details {
    padding: 0 40px;
    background: url("images/login.jpg");
    background-position: center;
    background-size: cover;
}

.login .form-details .overly,
.signup .form-details .overly {
    background: rgba(0, 0, 0, .7);
    padding: 20px;
    border-radius: 10px;
}

.signup .form-details {
    padding: 0 20px;
    background: url("images/signup.jpg");
    background-position: center;
    background-size: cover;
}

.form-box .form-content {
    width: 100%;
    padding: 35px;
}

.form-box h2 {
    text-align: center;
    margin-bottom: 10px;
}

form .input-field {
    position: relative;
    height: 50px;
    width: 100%;
    margin-top: 20px;
}

.input-field input {
    height: 100%;
    width: 100%;
    background: none;
    outline: none;
    font-size: 0.95rem;
    padding: 0 15px;
    border: 1px solid #717171;
    border-radius: 3px;
}

.input-field input:focus {
    border: 1px solid #00bcd4;
}

.input-field label {
    position: absolute;
    top: 50%;
    left: 15px;
    transform: translateY(-50%);
    color: #4a4646;
    pointer-events: none;
    transition: 0.2s ease;
}

.input-field input:is(:focus, :valid) {
    padding: 16px 15px 0;
}

.input-field input:is(:focus, :valid)~label {
    transform: translateY(-120%);
    color: #00bcd4;
    font-size: 0.75rem;
}

.form-box a {
    color: #00bcd4;
    text-decoration: none;
}

.form-box a:hover {
    text-decoration: underline;
}

form :where(.forgot-pass-link, .policy-text) {
    display: inline-flex;
    margin-top: 13px;
    font-size: 0.95rem;
}

form button {
    width: 100%;
    color: #fff;
    border: none;
    outline: none;
    padding: 14px 0;
    font-size: 1rem;
    font-weight: 500;
    border-radius: 3px;
    cursor: pointer;
    margin: 25px 0;
    background: #00bcd4;
    transition: 0.2s ease;
}

form button:hover {
    background: #0097a7;
}

.form-content .bottom-link {
    text-align: center;
}

.form-popup .signup,
.form-popup.show-signup .login {
    display: none;
}

.form-popup.show-signup .signup {
    display: flex;
}

.signup .policy-text {
    display: flex;
    margin-top: 14px;
    align-items: center;
}

.signup .policy-text input {
    width: 14px;
    height: 14px;
    margin-right: 7px;
}

@media (max-width: 950px) {
    .navbar :is(.hamburger-btn, .close-btn) {
        display: block;
    }

    .navbar {
        padding: 15px 0;
    }

    .navbar .logo img {
        display: none;
    }

    .navbar .logo h2 {
        font-size: 1.4rem;
    }

    .navbar .links {
        position: fixed;
        top: 0;
        z-index: 10;
        left: -100%;
        display: block;
        height: 100vh;
        width: 100%;
        padding-top: 60px;
        text-align: center;
        background: #fff;
        transition: 0.2s ease;
    }

    .navbar .links.show-menu {
        left: 0;
    }

    .navbar .links a {
        display: inline-flex;
        margin: 20px 0;
        font-size: 1.2rem;
        color: #000;
    }

    .navbar .links a:hover {
        color: #00BCD4;
    }

    .navbar .login-btn {
        font-size: 0.9rem;
        padding: 7px 10px;
    }
}

@media (max-width: 760px) {
    .form-popup {
        width: 95%;
    }

    .form-box .form-details {
        display: none;
    }

    .form-box .form-content {
        padding: 30px 20px;
    }
}

 </style>      
    
</head>
<body>
    <header>
        <nav class="navbar">
            <span class="hamburger-btn material-symbols-rounded">menu</span>
            <a href="#" class="logo">
                <img src="images/logo.jpg" alt="logo">
                <h2> Purchasing Mangement </h2>
            </a>
            <ul class="links">
                <span class="close-btn material-symbols-rounded">close</span>
                <li></li>
                <li></li>
                <li></li>
                <li></li>
                <li></li>
            </ul>
            <button class="login-btn">LOG IN</button>
        </nav>
    </header>
    <div class="blur-bg-overlay"></div>
    <div class="form-popup">
        <span class="close-btn material-symbols-rounded">close</span>
        <div class="form-box login">
            <div class="form-details">
                <div class="overly">
                    <h2>Welcome Back</h2>
                    <p>Please log in using your personal information</p>
                    <div id="login-error-message"></div>
                </div>
            </div>
            <div class="form-content">
                <h2>LOGIN</h2>
                <form action="#" id="login-form">
                    <div class="input-field">
                        <input type="text" name ="email" required>
                        <label>Email</label>
                    </div>
                    <div class="input-field">
                        <input type="password" name="password" required>
                        <label>Password</label>
                    </div>
                    <a href="#" class="forgot-pass-link">Forgot password?</a>
                    <button type="submit">Log In</button>
                </form>
                <div class="bottom-link">
                    Don't have an account?
                    <a href="#" id="signup-link">Signup</a>
                </div>
            </div>
        </div>
        <div class="form-box signup">
            <div class="form-details">
                <div class="overly">
                    <h2>Create Account</h2>
                    <div id="sign-error-message"></div>
                </div>
           </div>
            <div class="form-content">
                <h2>SIGNUP</h2>
                <form action="#" id="signup-form">
                    <div class="input-field">
                        <input name="name" type="text" required>
                        <label>your full name</label>
                    </div>
                    <div class="input-field">
                        <input name="email" type="text" required>
                        <label>Enter your email</label>
                    </div>
                    <div class="input-field">
                        <input name="password" type="password" required>
                        <label>Create password</label>
                    </div>
                    <div class="input-field">
                        <input name="phone_number" type="number" required>
                        <label>phone number</label>
                    </div>
                    <div class="input-field">
                        <input name="CIN" type="text" required>
                        <label>CIN</label>
                    </div>
                    <div class="input-field">
                        <input name="birthdate" type="date" value="0">
                        <label>birth date</label>
                    </div>
                    <button type="submit">Sign Up</button>
                </form>
                <div class="bottom-link">
                    Already have an account? 
                    <a href="#" id="login-link">Login</a>
                </div>
            </div>
        </div>
    </div>

    <script>
        history.replaceState(null, '', '/');

        function setCookie(cname, cvalue, exdays) {
            const d = new Date();
            d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
            let expires = "expires="+d.toUTCString();
            document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
        }

        const navbarMenu = document.querySelector(".navbar .links");
        const hamburgerBtn = document.querySelector(".hamburger-btn");
        const hideMenuBtn = navbarMenu.querySelector(".close-btn");
        const showPopupBtn = document.querySelector(".login-btn");
        const formPopup = document.querySelector(".form-popup");
        const hidePopupBtn = formPopup.querySelector(".close-btn");
        const signupLoginLink = formPopup.querySelectorAll(".bottom-link a");

        // Show mobile menu
        hamburgerBtn.addEventListener("click", () => {
            navbarMenu.classList.toggle("show-menu");
        });

        // Hide mobile menu
        hideMenuBtn.addEventListener("click", () =>  hamburgerBtn.click());

        // Show login popup
        showPopupBtn.addEventListener("click", () => {
            document.body.classList.toggle("show-popup");
        });

        // Hide login popup
        hidePopupBtn.addEventListener("click", () => showPopupBtn.click());

        // Show or hide signup form
        signupLoginLink.forEach(link => {
            link.addEventListener("click", (e) => {
                e.preventDefault();
                formPopup.classList[link.id === 'signup-link' ? 'add' : 'remove']("show-signup");
            });
        });
        document.getElementById('signup-form').addEventListener('submit', function (event) {
            event.preventDefault(); // Prevent form submission
            
            const form = event.target;
            const formData = new FormData(form); // Create FormData object
            const jsonData = {}; // Create an empty object for JSON data

            // Loop through FormData and add each entry to jsonData
            formData.forEach((value, key) => {
                jsonData[key] = value;
            });

            let  errorMessages = {
                400 : "<p style='color:red' > email  is not  valid or  already  exist </p>", 
                500 : "<p style='color:red' > internal  server error</p>"
            }

            fetch("/api/employees/new" , {
                method:"POST" , 
                body: JSON.stringify(jsonData)
            }).then(res=> {
                if (res.status==200){
                    document.getElementById("login-link").click();
                }else {
                    document.getElementById("sign-error-message").innerHTML =  errorMessages[res.status]
                }
            })
        });

        document.getElementById('login-form').addEventListener('submit', function (event) {
            event.preventDefault(); // Prevent form submission
            
            const form = event.target;
            const formData = new FormData(form); // Create FormData object
            const jsonData = {}; // Create an empty object for JSON data

            // Loop through FormData and add each entry to jsonData
            formData.forEach((value, key) => {
                jsonData[key] = value;
            });

            let  errorMessages = {
                400 : "<p style='color:red' > email or password is not  valid </p>", 
                500 : "<p style='color:red' > internal  server error</p>"
            }

            fetch("/api/employees/auth" , {
                method:"POST" , 
                body: JSON.stringify(jsonData)
            }).then(res=> {
                if (res.status==200){
                    try {                     
                        res.json().then(data => {
                            setCookie("id" , data.id , 1) ;
                        })
                    }catch(e){
                        console.error(e);
                    }
                    location.reload()
                }else {
                    document.getElementById("login-error-message").innerHTML =  errorMessages[res.status]
                }
            })
        });
    </script>
</body>
</html>