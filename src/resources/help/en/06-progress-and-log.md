# Progress and log

## The progress bar

The bar at the bottom shows the current phase:

- **Compression**: creating the archive.
- **par2**: creating the recovery files.
- **Upload**: sending the articles. Here you also see the speed, the size, the number of articles and the **ETA** (estimated time left).
- **Completed**, **Paused** or **Stopped**.

The color tells the state: blue while working, gray when paused, green when completed, orange if some articles failed, red if the post stopped with an error. The percentage is also shown in the window title.

The ETA is an estimate based on the recent speed: it changes when the speed changes.

## The log

The log colors the lines: phases, success, warnings and errors are easy to tell apart. At the end of a post it shows a summary with the time of each phase (compression, par2, upload) and the total.

## Latency

At the end of a post the log also shows the **latency**: the median time of a round trip with the server (the command POST and its answer, with no data) and of the connection. It is measured only from what ngPost already does, nothing is sent for it. A high value (more than half a second) means the delay comes from the network, the distance to the server or the server itself, for example a VPN, and not from your computer.

## Diagnostic report

If something does not work, the **Diagnostic report** button, below the log in Quick Post, saves a report of what the program did, with a **Latency** section for each server (connection, SSL, login, round trip: minimum, median, 95th percentile and maximum). It is a text file you can attach when you ask for help. Check it before sharing: it should not contain your password.

## Notifications

ngPost can tell you when your posts are done, so you do not have to watch the window. Turn it on in **Settings**, card **Notifications**.

- **System notification**: a message from the icon in the notification area (desktop version only).
- **Discord**: paste the address of a Discord webhook.
- **Telegram**: the token of your bot and the id of the chat that receives the messages.
- **Other webhook**: any address that accepts a JSON POST with a `title` and a `text` (Slack-like tools, ntfy, Home Assistant).

Choose what is notified (finished, errors or failures) and how: **one message when all the posts are done** (default, best for Auto Posting) or **a message after each post**. The button **Send a test message** shows at once if it works.

The message has the name of the post, the result, the size, the time and the speed. It never contains the password or the names of the files. The addresses and the token are saved in clear text in the configuration file, like the other passwords: keep it private. If a message cannot be sent, a line in the log says so and the post is not affected.

On the command line, or in Docker, use the options `--notify_discord`, `--notify_telegram_token`, `--notify_telegram_chat`, `--notify_webhook` and `--notify_when each|all_done`, or the keys `notify_*` of the configuration file.

