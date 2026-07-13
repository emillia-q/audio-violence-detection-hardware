#pragma once

#include <Arduino.h>

// --- SETUP CONFIGURATION PAGE  ---
const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Device Configuration</title>
    <style>
        body { font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, sans-serif; background-color: #f8fafc; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }
        .container { background: #ffffff; padding: 35px; border-radius: 16px; box-shadow: 0 10px 25px rgba(15, 23, 42, 0.08); width: 100%; max-width: 380px; box-sizing: border-box; border: 1px solid #e2e8f0; }
        h2 { color: #0f172a; margin-top: 0; margin-bottom: 25px; font-size: 22px; text-align: center; font-weight: 600; letter-spacing: -0.5px; }
        p { color: #475569; font-size: 14px; line-height: 1.6; text-align: center; }
        label { display: block; margin-bottom: 6px; font-weight: 600; color: #334155; font-size: 13px; }
        input[type="text"], input[type="password"], input[type="email"] { width: 100%; padding: 12px 14px; margin-bottom: 20px; border: 1px solid #cbd5e1; border-radius: 8px; box-sizing: border-box; font-size: 15px; background-color: #f8fafc; transition: all 0.2s ease; color: #0f172a; }
        input[type="text"]:focus, input[type="password"]:focus, input[type="email"]:focus { border-color: #0ea5e9; background-color: #ffffff; box-shadow: 0 0 0 3px rgba(14, 165, 233, 0.15); outline: none; }
        button { width: 100%; padding: 14px; background-color: #0f172a; color: white; border: none; border-radius: 8px; font-size: 15px; font-weight: 600; cursor: pointer; transition: background-color 0.2s ease; }
        button:hover { background-color: #1e293b; }
        button:active { transform: scale(0.99); }
    </style>
</head>
<body>
<div class="container">
    <h2>Audio Device Setup</h2>
    <form action="/save" method="POST">
        <label for="ssid">Wi-Fi Network Name (SSID)</label>
        <input type="text" id="ssid" name="ssid" placeholder="Enter SSID" required>

        <label for="password">Wi-Fi Password</label>
        <input type="password" id="password" name="password" placeholder="Enter password" required>

        <label for="email">User E-mail</label>
        <input type="email" id="email" name="email" placeholder="user@example.com" required>

        <button type="submit">Save & Restart Device</button>
    </form>
</div>
</body>
</html>
)rawliteral";

// --- SUCCESS PAGE  ---
const char SUCCESS_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Configuration Saved</title>
    <style>
        body { font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, sans-serif; background-color: #f8fafc; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; }
        .container { background: #ffffff; padding: 35px; border-radius: 16px; box-shadow: 0 10px 25px rgba(15, 23, 42, 0.08); width: 100%; max-width: 380px; box-sizing: border-box; border: 1px solid #e2e8f0; }
        h2 { color: #0f172a; margin-top: 0; margin-bottom: 25px; font-size: 22px; text-align: center; font-weight: 600; letter-spacing: -0.5px; }
        p { color: #475569; font-size: 14px; line-height: 1.6; text-align: center; }
    </style>
</head>
<body>
<div class="container">
    <h2>Success!</h2>
    <p>Configuration has been saved.</p>
    <p>The device is restarting now and will attempt to connect to your Wi-Fi network.</p>
</div>
</body>
</html>
)rawliteral";