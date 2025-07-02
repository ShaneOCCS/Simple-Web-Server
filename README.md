# 📨 ContactMe HTTP Server (C / Winsock2)

A lightweight HTTP server built in C using Winsock2 and the [cJSON](https://github.com/DaveGamble/cJSON) library.  
It listens on **port 8080**, handles simple GET/OPTIONS requests, and parses JSON POST requests to log contact messages.

---

## Features

- 🔌 Handles incoming HTTP requests (GET, OPTIONS)
- 🧾 Parses JSON request bodies using `cJSON`
- 🛂 CORS-compliant (supports preflight OPTIONS requests)
- 📝 Logs contact form data (`fullname`, `email`, `message`) to a text file with timestamps
- 📂 Written entirely in C using native Winsock APIs

---
